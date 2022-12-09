#include "TunnelImplementation.h"
#include "../utils/LogUtil.h"
#include "json.h"
#include "zlib.h"
#include <string>
#include "../utils/platform_adapter.h"
#include "../utils/MessageIdGenerator.h"

namespace mercury
{
namespace
{
const int FOREGROUND_MAX_RECONNECT_TIME_INDEX = 11;
const int BACKGROUND_MAX_RECONNECT_TIME_INDEX = 12;
int reconnect_interval(int times)
{
    if (times < 0)
        return 0;
    if (times == 0 || times == 1)
        return times;
    else
        return reconnect_interval(times - 1) + reconnect_interval(times - 2);
}
}
TunnelImplementation::TunnelImplementation(std::shared_ptr<IoService> ioService)
:ioService_(ioService)
,pingTimer_(*ioService_)
,pingTimeoutTimer_(*ioService_)
,connectTimeoutTimer_(*ioService_)
,loginTimeoutTimer_(*ioService_)
,reconnectTimeoutTimer_(*ioService_)
,dnsResolvePerformance_(PerformanceStatisticsType::dns_resolve)
,connectPerformance_(PerformanceStatisticsType::connect)
,handShakePerformance_(PerformanceStatisticsType::hand_shake)
{
}
TunnelImplementation::~TunnelImplementation()
{
}

void TunnelImplementation::SetCallback(ITunnelCallback* tunnelCallback)
{
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    ioService_->Post([weak, this, tunnelCallback]() {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        tunnelCallback_ = tunnelCallback;
        logoutCallback_ = std::bind(&ITunnelCallback::OnLogCallback, tunnelCallback_, std::placeholders::_1);
    });
}


void TunnelImplementation::Start()
{
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    ioService_->Post([weak, this]() {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        Log(logoutCallback_) << "Tunnel> Start.";
        manualStopped_ = false;
        launch_tunnel();
    });
}

void TunnelImplementation::Stop()
{
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    ioService_->Post([weak, this]() {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        Log(logoutCallback_) << "Tunnel> Stop.";
        stop_tunnel();
        manualStopped_ = true;
    });
}

void TunnelImplementation::SendMessage(const TunnelMessage& mercuryMessage,
                                       std::function<void(int)> sendCallback)
{
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    ioService_->Post([weak, this, mercuryMessage, sendCallback]() {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        else if (!netClient_)
        {
            Log(logoutCallback_) << "Tunnel> client has been destroyed,can not send data.";
            if (sendCallback)
            {
                sendCallback((int)SendResult::client_distroyed);
            }
        }
        else if (TunnelConnectionStatus::disconnect == tunnelConnectionStatus_)
        {
            Log(logoutCallback_) << "Tunnel> client disconnect,can not send data.";
            if (sendCallback)
            {
                sendCallback((int)SendResult::client_disconnect);
            }
        }
        else if (TunnelConnectionStatus::connecting == tunnelConnectionStatus_)
        {
            Log(logoutCallback_) << "Tunnel> client is connecting,can not send data.";
            if (sendCallback)
            {
                sendCallback((int)SendResult::client_connecting);
            }
        }
        else if (TunnelConnectionStatus::handleshaked != tunnelConnectionStatus_)
        {
            Log(logoutCallback_) << "Tunnel> client has not hand shake,can not send data.";
            if (sendCallback)
            {
                sendCallback((int)SendResult::client_hand_shake_failed);
            }
            //start_hand_shake();
        }
        else
        {
            auto msg = make_send_data(mercuryMessage);
            netClient_->SendMessage(msg, sendCallback);
        }
    });
}

void TunnelImplementation::SetConfig(TunnelConfig config)
{
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    ioService_->Post([weak, this, config]() {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        tunnelConfig_ = config;
        if (manualStopped_)
        {
            return;
        }
        if (tunnelConfig_.closeMercury)
        {
            if (netClient_)
            {
                Log(logoutCallback_) << "Tunnel> tunnel closed by manager.";
                stop_tunnel();
            }
        }
        else
        {
            if (!netClient_)
            {
                Log(logoutCallback_) << "Tunnel> tunnel start by manager.";
                launch_tunnel();
            }
        }
    });
}

uint64_t TunnelImplementation::GetTimeElapseFromLastMessage()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastMessageTimePoint_).count();
}

void TunnelImplementation::OnBackGround(bool status)
{
    if (status != runInBackGround_)
    {
        Log(logoutCallback_) << "Tunnel> app change run in " << (status?"background.":"foreground.");
        runInBackGround_ = status;
    }
}

void TunnelImplementation::OnNetworkChanged(bool connected)
{
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    ioService_->Post([weak, this, connected]() {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        if (connected)
        {
            Log(logoutCallback_) << "Tunnel> app network connected.";
            reconnectTimes_ = 0;
            if (TunnelConnectionStatus::disconnect == tunnelConnectionStatus_)
            {
                reconnect();
            }
        }
        else
        {
            Log(logoutCallback_) << "Tunnel> app network disconnected.";
        }
    });
}

void TunnelImplementation::stop_tunnel()
{
    Log(logoutCallback_) << "Tunnel> stop tunnel.";
    reconnectTimes_ = 0;
    if (!netClient_)
    {
        Log(logoutCallback_) << "Tunnel> can not stop again, client has been destroyed.";
        return;
    }
    disconnect();
    netClient_.reset();
}

void TunnelImplementation::launch_tunnel()
{
    if (tunnelConfig_.closeMercury)
    {
        Log(logoutCallback_) << "Tunnel> manager has closed tunnel, cannot start.";
        return;
    }
    if (TunnelConnectionStatus::disconnect != tunnelConnectionStatus_)
    {
        Log(logoutCallback_) << "Tunnel> can not launch again, client has been started.";
        return;
    }
    create_new_connection();
    try_connect();
}

void TunnelImplementation::create_new_connection()
{
    netClient_ = std::make_shared<NetClient>(ioService_);
    // reset message id
    MessageIdGenerator::instance().Reset_msg_id();
    init_callback();
}

void TunnelImplementation::init_callback()
{
    if (!netClient_)
    {
        return;
    }
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    netClient_->NetworkStatusCallback([weak,this](int errCode) {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        if (0 != errCode)
        {
            Log(logoutCallback_) << "Tunnel> connect break [" << errCode << "]";
            if (isLogin_)
            {
                isLogin_ = false;
                handShakePerformance_.SetErrorInfo((int)StatisticErrors::TunnelDisconnect);
                handShakePerformance_.Stop();
                if (tunnelCallback_)
                {
                    tunnelCallback_->OnReportPerformanceStatistics(handShakePerformance_.GetResult());
                }
            }
            disconnect();
            reconnect();
        }
    });
    netClient_->LogCallback([weak, this](std::string log) {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        Log(logoutCallback_) << log;
    });
    netClient_->RecievePongCallback([weak, this]() {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        Log(logoutCallback_) << "Tunnel> pong";
        lastMessageTimePoint_ = std::chrono::steady_clock::now();
        stop_ping_timeout_check();
    });
    netClient_->RecieveMessageCallback([weak, this](MessageProtocol msg) {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        lastMessageTimePoint_ = std::chrono::steady_clock::now();
        process_message(msg);
    });
}

void TunnelImplementation::try_connect()
{
    if (!netClient_)
    {
        return;
    }
    if (manualStopped_ || tunnelConfig_.closeMercury || TunnelConnectionStatus::disconnect != tunnelConnectionStatus_)
    {
        return;
    }
    
    tunnelConnectionStatus_ = TunnelConnectionStatus::connecting;
    if (tunnelCallback_)
    {
        tunnelCallback_->OnNetworkStatusCallback(NetStatus::connecting);
    }
    
    Log(logoutCallback_) << "Tunnel> start dns resolve...";
    SocketAddressList addressList;
    auto isOk = get_reachable_address(addressList);
    if (!isOk)
    {
        tunnelConnectionStatus_ = TunnelConnectionStatus::disconnect;
        if (tunnelCallback_)
        {
            tunnelCallback_->OnNetworkStatusCallback(NetStatus::disconnect);
        }
        reconnect();
        return;
    }
    
    Log(logoutCallback_) << "Tunnel> start connect...";
    start_connect_timeout_check();
    connectPerformance_.Start();
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    netClient_->Connect(addressList, [weak, this](int errCode) {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        if (0 == errCode)
        {
            Log(logoutCallback_) << "Tunnel> connect success";
            connectPerformance_.SetErrorInfo(errCode);
            connectPerformance_.Stop();
            if (tunnelCallback_)
            {
                tunnelCallback_->OnReportPerformanceStatistics(connectPerformance_.GetResult());
            }
            tunnelConnectionStatus_ = TunnelConnectionStatus::connected;
            start_ping();
            start_login();
            reconnectTimeoutTimer_.cancel();
        }
        else
        {
            tunnelConnectionStatus_ = TunnelConnectionStatus::disconnect;
            connectPerformance_.SetErrorInfo(errCode);
            connectPerformance_.Stop();
            if (errCode != boost::asio::error::operation_aborted)
            {
                Log(logoutCallback_) << "Tunnel> connection failed [" << errCode << "]";
                if (tunnelCallback_)
                {
                    tunnelCallback_->OnReportPerformanceStatistics(connectPerformance_.GetResult());
                }
                if (tunnelCallback_)
                {
                    tunnelCallback_->OnNetworkStatusCallback(NetStatus::disconnect);
                }
            }
            reconnect();
        }
        stop_connect_timeout_check();
    });
}

void TunnelImplementation::reconnect()
{
    if (manualStopped_ || tunnelConfig_.closeMercury || TunnelConnectionStatus::disconnect != tunnelConnectionStatus_)
    {
        return;
    }
    
    auto interval = get_next_connect_interval();
    Log(logoutCallback_) << "Tunnel> connect retry times [" << reconnectTimes_ << "] after interval [" << interval << " s]";
    
    reconnectTimeoutTimer_.cancel();
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    reconnectTimeoutTimer_.expires_after(std::chrono::seconds(interval));
    reconnectTimeoutTimer_.async_wait([weak,this, interval](const boost::system::error_code& e) {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        if (e != boost::asio::error::operation_aborted)
        {
            if (TunnelConnectionStatus::disconnect == tunnelConnectionStatus_) {
                try_connect();
            }
        }
    });
}

int TunnelImplementation::get_next_connect_interval()
{
    int currentReconnectTimes = ++reconnectTimes_;
    if (runInBackGround_)
    {
        if (currentReconnectTimes > BACKGROUND_MAX_RECONNECT_TIME_INDEX)
        {
            currentReconnectTimes = BACKGROUND_MAX_RECONNECT_TIME_INDEX;
        }
    }
    else
    {
        if (currentReconnectTimes > FOREGROUND_MAX_RECONNECT_TIME_INDEX)
        {
            currentReconnectTimes = FOREGROUND_MAX_RECONNECT_TIME_INDEX;
        }
    }
    return reconnect_interval(currentReconnectTimes);
}

void TunnelImplementation::disconnect()
{
    if (tunnelConnectionStatus_ == TunnelConnectionStatus::disconnect)
    {
        Log(logoutCallback_) << "Tunnel> client has been disconnected";
        return;
    }
    Log(logoutCallback_) << "Tunnel> disconnect";
    stop_ping_timeout_check();
    stop_login_timeout_check();
    pingTimer_.cancel();
    reconnectTimeoutTimer_.cancel();
    tunnelConnectionStatus_ = TunnelConnectionStatus::disconnect;
    if (tunnelCallback_)
    {
        tunnelCallback_->OnNetworkStatusCallback(NetStatus::disconnect);
    }
    if (!netClient_)
    {
        return;
    }
    netClient_->Close();
}

void TunnelImplementation::start_ping()
{
    auto interval = tunnelConfig_.pingInterval;
    if (runInBackGround_)
    {
        interval = tunnelConfig_.pingIntervalBackground;
    }
    
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    pingTimer_.expires_after(std::chrono::seconds(interval));
    pingTimer_.async_wait([weak,this](const boost::system::error_code& e) {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        if (!e)
        {
            if (!netClient_ || TunnelConnectionStatus::connected > tunnelConnectionStatus_)
            {
                return;
            }
            //发起ping，并检测ping超时
            Log(logoutCallback_) << "Tunnel> start ping";
            netClient_->Ping();
            start_ping_timeout_check();
            //检查ack命令
            //				check_send_ack_message();
            //预定下一次ping
            start_ping();
        }
    });
}

void TunnelImplementation::start_login()
{
    if (TunnelConnectionStatus::connected == tunnelConnectionStatus_)
    {
        if (!netClient_)
        {
            return;
        }
        auto shakeInfoBody = get_login_info();
        MessageProtocol msg;
        Meta_t meta;
        meta.fixedInfo.type = MessageType::REQ;
        meta.fixedInfo.twoWay = 1;
        meta.msgId = MessageIdGenerator::instance().Gen_msg_id();
        meta.command = (uint8_t)MessageCommand::login;
        msg.SetMeta(meta);
        msg.SetBodyData(std::vector<uint8_t>((uint8_t*)shakeInfoBody.data(), (uint8_t*)shakeInfoBody.data()+shakeInfoBody.size()));
        
        Log(logoutCallback_) << "Tunnel> start handshake [" << shakeInfoBody << "]";
        std::weak_ptr<TunnelImplementation> weak = weak_from_this();
        auto ret = netClient_->SendMessage(msg, [weak, this](int errcode) {
            auto self = weak.lock();
            if (!self)
            {
                return;
            }
            if (0 == errcode)
            {
                isLogin_ = true;
                start_login_timeout_check();
                handShakePerformance_.Start();
            }
            else
            {
                Log(logoutCallback_) << "Tunnel> start hand shake failed,send request failed.";
            }
        });
        if (!ret)
        {
            Log(logoutCallback_) << "Tunnel> start hand shake failed,send request failed.";
        }
    }
    else
    {
        Log(logoutCallback_) << "Tunnel> can not hand shake, client is not connected.";
    }
}

void TunnelImplementation::start_connect_timeout_check()
{
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    connectTimeoutTimer_.expires_after(std::chrono::seconds(tunnelConfig_.connectTimeout));
    connectTimeoutTimer_.async_wait([weak,this](const boost::system::error_code& e) {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        if (e != boost::asio::error::operation_aborted)
        {
            Log(logoutCallback_) << "Tunnel> connect timeout";
            disconnect();
            connectPerformance_.SetErrorInfo((int)StatisticErrors::TimeOut);
            connectPerformance_.Stop();
            if (tunnelCallback_)
            {
                tunnelCallback_->OnReportPerformanceStatistics(connectPerformance_.GetResult());
            }
        }
    });
}

void TunnelImplementation::stop_connect_timeout_check()
{
    connectTimeoutTimer_.cancel();
}

void TunnelImplementation::start_login_timeout_check()
{
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    loginTimeoutTimer_.expires_after(std::chrono::seconds(tunnelConfig_.shakehandTimeout));
    loginTimeoutTimer_.async_wait([weak,this](const boost::system::error_code& e) {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        if (e != boost::asio::error::operation_aborted)
        {
            Log(logoutCallback_) << "Tunnel> handshake timeout";
            disconnect();
            reconnect();
            handShakePerformance_.SetErrorInfo((int)StatisticErrors::TimeOut);
            handShakePerformance_.Stop();
            if (tunnelCallback_)
            {
                tunnelCallback_->OnReportPerformanceStatistics(handShakePerformance_.GetResult());
            }
        }
        isLogin_ = false;
    });
}

void TunnelImplementation::stop_login_timeout_check()
{
    loginTimeoutTimer_.cancel();
}

void TunnelImplementation::start_ping_timeout_check()
{
    std::weak_ptr<TunnelImplementation> weak = weak_from_this();
    pingTimeoutTimer_.expires_after(std::chrono::seconds(tunnelConfig_.pingTimeout	));
    pingTimeoutTimer_.async_wait([weak,this](const boost::system::error_code& e) {
        auto self = weak.lock();
        if (!self)
        {
            return;
        }
        if (e != boost::asio::error::operation_aborted)
        {
            Log(logoutCallback_) << "Tunnel> ping timeout";
            disconnect();
            reconnect();
        }
    });
}

void TunnelImplementation::stop_ping_timeout_check()
{
    pingTimeoutTimer_.cancel();
}

void TunnelImplementation::process_message(const MessageProtocol& msg)
{
    MessageProcessor messageProcessor(msg);
    messageProcessor.Process();
    switch ((MessageCommand)messageProcessor.GetMessageCommand())
    {
        case MessageCommand::login:
            //process handshake message
            process_login_message(messageProcessor);
            break;
        default:
            //抛出消息
            if (tunnelCallback_)
            {
                TunnelMessage tunnelMessage;
                tunnelMessage.command = messageProcessor.GetMessageCommand();
                tunnelMessage.body = messageProcessor.GetMessageBodyData();
                tunnelMessage.msgId = messageProcessor.GetMessageId();
                tunnelMessage.twoWay = messageProcessor.GetMessageTwoWay();
                tunnelMessage.type = messageProcessor.GetMessageType();
                for (auto header : messageProcessor.GetMessageHeaders())
                {
                    tunnelMessage.AddHeader(header.headerType, header.headerData);
                }
                tunnelCallback_->OnRecieveCallback(tunnelMessage);
            }
            break;
    }
}

void TunnelImplementation::process_login_message(const MessageProcessor& messageProcessor)
{
    stop_login_timeout_check();
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());
    Json::Value root;
    JSONCPP_STRING errs;
    auto bodyData = messageProcessor.GetMessageBodyData();
    std::string str_body(bodyData.begin(), bodyData.end());
    Log(logoutCallback_) << "Tunnel> handshake reponse [" << str_body << "]";
    
    bool ret = jsonReader->parse(str_body.c_str(), str_body.c_str() + str_body.length(), &root, &errs);
    if (!ret || !errs.empty())
    {
        Log(logoutCallback_) << "Tunnel> handshake response json parse error";
        disconnect();
        reconnect();
        handShakePerformance_.SetErrorInfo((int)StatisticErrors::HandShakeResponseError);
        handShakePerformance_.Stop();
        if (tunnelCallback_)
        {
            tunnelCallback_->OnReportPerformanceStatistics(handShakePerformance_.GetResult());
        }
        return;
    }
    else
    {
        int code = -1;
        if (root.isObject() && root["c"].isInt())
        {
            code = root["c"].asInt();
        }
        if (0 != code)
        {
            Log(logoutCallback_) << "Tunnel> handshake response error code: "<<code;
            disconnect();
            reconnect();
            handShakePerformance_.SetErrorInfo((int)StatisticErrors::HandShakeResponseError);
            handShakePerformance_.Stop();
            if (tunnelCallback_)
            {
                tunnelCallback_->OnReportPerformanceStatistics(handShakePerformance_.GetResult());
            }
            return;
        }
    }
    reconnectTimes_ = 0;
    tunnelConnectionStatus_ = TunnelConnectionStatus::handleshaked;
    if (tunnelCallback_)
    {
        tunnelCallback_->OnNetworkStatusCallback(NetStatus::connected);
    }
    handShakePerformance_.SetErrorInfo((int)StatisticErrors::Success);
    handShakePerformance_.Stop();
    if (tunnelCallback_)
    {
        tunnelCallback_->OnReportPerformanceStatistics(handShakePerformance_.GetResult());
    }
}

std::string TunnelImplementation::get_login_info()
{
    LoginInfo loginInfo;
    if (tunnelCallback_)
    {
        tunnelCallback_->OnGetLoginInfo(loginInfo);
    }
    Json::Value jsonbody;
    jsonbody["d"] = "123";
    jsonbody["p"] = loginInfo.platform;
    jsonbody["sv"] = loginInfo.systemVersion;
    jsonbody["u"] = loginInfo.userId;
    jsonbody["t"] = loginInfo.type;
    jsonbody["m"] = loginInfo.model;
    jsonbody["b"] = (int)runInBackGround_;
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string body = Json::writeString(builder, jsonbody);
    return body;
}

bool TunnelImplementation::get_reachable_address(SocketAddressList& addressList)
{
    bool bResolved = true;
    Address address, backupAddress;
    if (tunnelCallback_)
    {
        tunnelCallback_->OnGetAddress(address, backupAddress);
    }
    Log(logoutCallback_) << "Tunnel> prime address dns resolving ["<< address.host<<":"<<address.port<<"]";
    std::string strIpList;
    dnsResolvePerformance_.Start();
    boost::asio::ip::tcp::resolver resolver(*ioService_);
    boost::system::error_code ec;
    addressList = resolver.resolve(address.host, address.port, ec);
    if (ec)
    {
        Log(logoutCallback_) << "Tunnel> prime address dns resolve failed, error: " << ec.value()<<", check backup address";
        Log(logoutCallback_) << "Tunnel> backup address [" << backupAddress.host << ":" << backupAddress.port << "]";
        boost::system::error_code backupAddrResolveError;
        addressList = resolver.resolve(backupAddress.host, backupAddress.port, backupAddrResolveError);
        if (backupAddrResolveError)
        {
            Log(logoutCallback_) << "Tunnel> backup address is invalid, resolve error: " << backupAddrResolveError.value();
            bResolved = false;
        }
    }
    else
    {
        std::stringstream logAddrList;
        logAddrList << "Tunnel> resolved address: ";
        boost::asio::ip::tcp::resolver::iterator end;
        auto tmpAddressList = addressList;
        while (tmpAddressList != end)
        {
            boost::asio::ip::tcp::endpoint endpoint = *tmpAddressList++;
            logAddrList << endpoint << ";";
        }
        strIpList = logAddrList.str();
        Log(logoutCallback_) << strIpList;
    }
    
    dnsResolvePerformance_.SetErrorInfo(ec.value(), strIpList);
    dnsResolvePerformance_.Stop();
    if (tunnelCallback_)
    {
        tunnelCallback_->OnReportPerformanceStatistics(dnsResolvePerformance_.GetResult());
    }
    return bResolved;
}

std::vector<uint8_t> TunnelImplementation::compress_body(CompressType compressType, const std::vector<uint8_t>& body)
{
    if (CompressType::zip == compressType)
    {
        unsigned long preCompressedLen = compressBound(body.size());
        std::vector<uint8_t> compressedBody(preCompressedLen);
        auto ret = compress(&compressedBody[0], &preCompressedLen, &body[0], body.size());
        if (Z_OK == ret)
        {
            compressedBody.resize(preCompressedLen);
            return compressedBody;
        }
    }
    
    return std::vector<uint8_t>();
}

MessageProtocol TunnelImplementation::make_send_data(const TunnelMessage& mercuryMessage)
{
    MessageProtocol msg;
    Meta_t meta;
    meta.msgId = mercuryMessage.msgId;
    meta.fixedInfo.twoWay = mercuryMessage.twoWay;
    meta.fixedInfo.type = mercuryMessage.type;
    meta.command = mercuryMessage.command;
    msg.SetMeta(meta);
    if (CompressType::no != mercuryMessage.compressType && mercuryMessage.body.size() > 2 * 1024)
    {
        auto compressedBody = compress_body(mercuryMessage.compressType, mercuryMessage.body);
        if (compressedBody.empty())
        {
            msg.SetBodyData(mercuryMessage.body);
        }
        else
        {
            std::string strCompressType = std::to_string((int)mercuryMessage.compressType);
            msg.AddExtensionHeader((uint8_t)MessageHeaderType::compressed, std::vector<uint8_t>(strCompressType.begin(), strCompressType.end()));
            msg.SetBodyData(compressedBody);
        }
    }
    else
    {
        msg.SetBodyData(mercuryMessage.body);
    }
    for (auto header : mercuryMessage.headerList)
    {
        msg.AddExtensionHeader(header.headerType, header.headerBody);
    }
    return msg;
}

}
