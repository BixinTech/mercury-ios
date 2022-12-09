#pragma once
#include "../utils/BoostHeader.h"
#include <deque>
#include "MsgProtocol.h"
#include "../utils/IoService.h"
#include <vector>

namespace mercury {
using SocketAddressList = boost::asio::ip::tcp::resolver::results_type;
class  NetClient : public std::enable_shared_from_this<NetClient>
{
    struct  SendItem
    {
        MessageProtocol msgProtocol_;
        std::function<void(int)> callback_;
        SendItem(MessageProtocol proto, std::function<void(int)> callback)
        :msgProtocol_(proto)
        , callback_(callback)
        {
            
        }
    };
public:
    NetClient(std::shared_ptr<IoService> ioService);
    ~NetClient();
    
    /**
     * @brief: 设置连接状态回调
     * @param netStatusCallback 连接状态回调
     */
    void NetworkStatusCallback(std::function<void(int)> netStatusCallback);
    
    /**
     * @brief: 连接
     * @param addressList 服务ip列表
     * @param completed 服务端口
     */
    void Connect(SocketAddressList addressList, std::function<void(int)> completed);
    
    /**
     * @brief: 发送消息
     * @param msg 发送的消息体
     * @param callback 发送结果回调
     * @return: true 加入发送队列成功; false 加入队列失败;
     */
    bool SendMessage(const MessageProtocol& msg, std::function<void(int)> callback = nullptr);
    
    /**
     * @brief: 注册接收消息回调
     * @param recieveCallback 收到消息回调
     */
    void RecieveMessageCallback(std::function<void(MessageProtocol msg)> recieveCallback);
    
    /**
     * @brief: 关闭连接
     */
    void Close();
    
    /**
     * @brief: 发送一个ping包
     */
    void Ping();
    
    /**
     * @brief: 接收服务发送的ping包
     * @param pingCallback 接收服务发送的ping包
     */
    void RecievePongCallback(std::function<void()> pingCallback);
    
    /**
     * @brief: 日志回调接口
     * @param logCallback C层日志详情
     */
    void LogCallback(std::function<void(std::string)> logCallback = nullptr);
private:
    void do_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints, std::function<void(int)> callback);
    void do_read_meta();
    void do_read_msg_id(std::vector<uint8_t> values);
    void do_read_cmd();
    void do_read_payload_length(std::vector<uint8_t> values);
    void do_read_payload();
    void do_write();
    void do_error_close(boost::system::error_code ec);
    void handle_ping();
private:
    boost::asio::ip::tcp::socket socket_;
    std::deque<SendItem> send_msg_queue_;
    Meta_t buf_meta_;
    std::vector<uint8_t> buf_payload_;
    
    std::vector<uint8_t> sendBuf_;
    std::function<void(MessageProtocol)> recieveCallback_;
    std::function<void()>                recievePingCallback_;
    std::function<void(int)>             netStatusCallback_;
    std::function<void(std::string)>     logCallback_;
    
    uint8_t varIntByte_ = 0;
    bool isConnected = false;
};
}
