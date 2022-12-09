#include "NetClient.h"
#include "../utils/Conversion.h"
#include "../utils/LogUtil.h"
#include "../utils/platform_adapter.h"
#include "../utils/VarInt.h"
#include "../utils/MessageIdGenerator.h"

namespace
{
const int MAX_SEND_QUEUE = 200 * 1024;//发送队列最大支持消息缓存
}

using boost::asio::ip::tcp;
namespace mercury {
NetClient::NetClient(std::shared_ptr<IoService> ioService)
:socket_(*ioService)
{
}

NetClient::~NetClient()
{
    
}

void NetClient::NetworkStatusCallback(std::function<void(int)> netStatusCallback)
{
    netStatusCallback_ = netStatusCallback;
}

void NetClient::Connect(SocketAddressList addressList, std::function<void(int)> completed)
{
    do_connect(addressList, completed);
}

bool NetClient::SendMessage(const MessageProtocol& msg, std::function<void(int)> callback)
{
    if (send_msg_queue_.size() >= MAX_SEND_QUEUE || !socket_.is_open())
    {
        return false;
    }
    SendItem sendItem(msg, callback);
    bool isQueueEmpty = send_msg_queue_.empty();
    send_msg_queue_.push_back(sendItem);
    if (isQueueEmpty)
    {
        do_write();
    }
    return true;
}

void NetClient::RecieveMessageCallback(std::function<void(MessageProtocol msg)> recieveCallback)
{
    recieveCallback_ = recieveCallback;
}

void NetClient::Close()
{
    if (socket_.is_open())
    {
        if (isConnected)
        {
            boost::system::error_code ec;
            socket_.shutdown(tcp::socket::shutdown_both, ec);
            if (ec)
            {
                Log(logCallback_) << "Socket> Close: socket shutdown error:" << ec.value();
            }
        }
        else
        {
            boost::system::error_code ec;
            socket_.cancel(ec);
            if (ec)
            {
                Log(logCallback_) << "Socket> Close: socket cancel error:" << ec.value();
            }
        }
        boost::system::error_code ec;
        socket_.close(ec);
        if (ec)
        {
            Log(logCallback_) << "Socket> Close: socket close error:" << ec.value();
        }
        isConnected = false;
    }
}

void NetClient::Ping()
{
    MessageProtocol message;
    Meta_t meta;
    meta.fixedInfo.type = MessageType::REQ;
    meta.fixedInfo.twoWay = 0;
    meta.fixedInfo.heartbeat = 1;
    meta.fixedInfo.version = 0;
    meta.msgId = MessageIdGenerator::instance().Gen_msg_id();
    message.SetMeta(meta);
    SendMessage(message);
}

void NetClient::RecievePongCallback(std::function<void()> pingCallback)
{
    recievePingCallback_ = pingCallback;
}

void NetClient::LogCallback(std::function<void(std::string)> logCallback /*= nullptr*/)
{
    logCallback_ = logCallback;
}

void NetClient::do_connect(const tcp::resolver::results_type& endpoints, std::function<void(int)> callback)
{
    isConnected = false;
    auto self = shared_from_this();
    boost::asio::async_connect(socket_, endpoints,
                               [self, this, callback](boost::system::error_code ec, tcp::endpoint)
                               {
        if (!ec)
        {
            isConnected = true;
            do_read_meta();
        }
        else
        {
            isConnected = false;
        }
        
        if (callback)
        {
            callback(ec.value());
        }
    });
}

void NetClient::do_read_meta()
{
    auto self = shared_from_this();
    boost::asio::async_read(socket_, boost::asio::buffer(&buf_meta_.fixedInfo, sizeof(Meta_Fixed_t)),
                            boost::asio::transfer_exactly(sizeof(Meta_Fixed_t)),
                            [self, this](boost::system::error_code ec, std::size_t)
                            {
        if (!ec)
        {
            std::vector<uint8_t> msgId;
            do_read_msg_id(msgId);
        }
        else
        {
            do_error_close(ec);
        }
    });
}

void NetClient::do_read_msg_id(std::vector<uint8_t> values) {
    auto self = shared_from_this();
    boost::asio::async_read(socket_, boost::asio::buffer(&varIntByte_, sizeof(varIntByte_)),
                            boost::asio::transfer_exactly(sizeof(varIntByte_)),
                            [self, values, this](boost::system::error_code ec, std::size_t) mutable {
        if (!ec) {
            values.push_back(varIntByte_);
            if (!(varIntByte_ & 128)) {
                // read done
                size_t size = 0;
                buf_meta_.msgId = decodeVarint(&values[0], size);
                if (buf_meta_.fixedInfo.heartbeat == 1) {
                    // read done
                    handle_ping();
                    do_read_meta();
                } else {
                    do_read_cmd();
                }
            } else {
                do_read_msg_id(values);
            }
        } else {
            do_error_close(ec);
        }
    });
}

void NetClient::do_read_cmd() {
    auto self = shared_from_this();
    boost::asio::async_read(socket_, boost::asio::buffer(&buf_meta_.command, sizeof(buf_meta_.command)),
                            boost::asio::transfer_exactly(sizeof(buf_meta_.command)),
                            [self, this](boost::system::error_code ec, std::size_t) {
        
        if (!ec) {
            std::vector<uint8_t> values;
            do_read_payload_length(values);
        } else {
            do_error_close(ec);
        }
    });
}

void NetClient::do_read_payload_length(std::vector<uint8_t> values) {
    auto self = shared_from_this();
    boost::asio::async_read(socket_, boost::asio::buffer(&varIntByte_, sizeof(varIntByte_)),
                            [self, values, this](boost::system::error_code ec, std::size_t) mutable {
        if (!ec) {
            values.push_back(varIntByte_);
            if (!(varIntByte_ & 128)) {
                // read done
                size_t size = 0;
                buf_meta_.payloadLength = decodeVarint(&values[0], size);
                do_read_payload();
            } else {
                do_read_payload_length(values);
            }
        } else {
            do_error_close(ec);
        }
    });
}

void NetClient::do_read_payload()
{
    // header size + payload length(1 byte)
    uint64_t totalLength = sizeof(uint8_t) + buf_meta_.payloadLength;
    buf_payload_.resize(totalLength);
    auto self = shared_from_this();
    boost::asio::async_read(socket_, boost::asio::buffer(buf_payload_, totalLength),
                            boost::asio::transfer_exactly(totalLength),
                            [self, this](boost::system::error_code ec, std::size_t)
                            {
        if (!ec)
        {
            MessageProtocol reciveMessage;
            reciveMessage.SetMeta(buf_meta_);
            reciveMessage.SetPayload(buf_payload_);
            if (recieveCallback_)
            {
                recieveCallback_(reciveMessage);
            }
            do_read_meta();
        }
        else
        {
            do_error_close(ec);
        }
    });
}

void NetClient::do_write()
{
    auto sendData = send_msg_queue_.front().msgProtocol_.Encode();
    sendBuf_.swap(sendData);
    auto self = shared_from_this();
    boost::asio::async_write(socket_, boost::asio::buffer(sendBuf_),
                             [self, this](boost::system::error_code ec, std::size_t) {
        auto sendResultCallback = send_msg_queue_.front().callback_;
        if (!ec)
        {
            send_msg_queue_.pop_front();
            if (!send_msg_queue_.empty())
            {
                do_write();
            }
        }
        else
        {
            send_msg_queue_.clear();
            boost::system::error_code err;
            socket_.close(err);
            if (err)
            {
                Log(logCallback_) << "Socket> do_write: socket close error:" << err.value();
            }
        }
        if (sendResultCallback && ec != boost::asio::error::operation_aborted)
        {
            sendResultCallback(ec.value());
        }
    });
}

void NetClient::do_error_close(boost::system::error_code ec)
{
    boost::system::error_code err;
    socket_.close(err);
    if (err)
    {
        Log(logCallback_) << "Socket> do close socket error:" << err.value();
    }
    if (netStatusCallback_ && ec != boost::asio::error::operation_aborted) {
        netStatusCallback_(ec.value());
    }
}

void NetClient::handle_ping()
{
    if (buf_meta_.fixedInfo.type == MessageType::REQ && buf_meta_.fixedInfo.twoWay == 1) {
        MessageProtocol message;
        Meta_t meta;
        meta.fixedInfo.type = MessageType::RESP;
        meta.fixedInfo.twoWay = 0;
        meta.fixedInfo.heartbeat = 1;
        meta.msgId = buf_meta_.msgId;
        message.SetMeta(meta);
        SendMessage(message);
    }
    if (recievePingCallback_) {
        recievePingCallback_();
    }
}
}
