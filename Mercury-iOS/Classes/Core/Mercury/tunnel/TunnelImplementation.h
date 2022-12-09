#pragma once
#include "TunnelDefine.h"
#include "../socket/NetClient.h"
#include "../utils/IoService.h"
#include "MessageProcessor.h"
#include "PerformanceStatistics.h"

namespace mercury {
// > 0 系统错误
// 0 成功
//-1 没有client
//-2 没有连接成功
//-3 没有握手成功
enum class SendResult
{
    client_distroyed = -1,
    client_disconnect = -2,
    client_connecting = -3,
    client_hand_shake_failed = -4,
};
enum class TunnelConnectionStatus
{
    disconnect = 0,
    connecting = 1,
    connected = 2,
    handleshaked = 3,
};

class NetClient;
class TunnelImplementation : public std::enable_shared_from_this<TunnelImplementation>
{
public:
    TunnelImplementation(std::shared_ptr<IoService> ioService);
    ~TunnelImplementation();
    
    /**
     * @brief: 设置tunnel回调
     */
    void SetCallback(ITunnelCallback* tunnelCallback);
    
    /**
     * @brief:创建新连接
     */
    void Start();
    
    /**
     * @brief:删除连接
     */
    void Stop();
    
    /**
     * @brief:发送消息
     * @param mercuryMessage 消息
     * @param sendCallback 发送结果回调
     */
    void SendMessage(const TunnelMessage& mercuryMessage, std::function<void(int)> sendCallback = nullptr);
    
    /**
     * @brief: 更新tunnel内部配置信息
     * @param config tunnel配置信息(connect/handshake/ping/close策略配置)
     */
    void SetConfig(TunnelConfig config);
    
    /**
     * @brief:获取最后一条消息距现在的时间间隔
     * @return:time elapse
     */
    uint64_t GetTimeElapseFromLastMessage();
    
    /**
     * @brief:切换前后台状态，默认前台
     * @param status true:后台 false:前台
     */
    void OnBackGround(bool status);
    
    /**
     * @brief:网络状态切换
     * @param connected true:已连接 false:断开
     */
    void OnNetworkChanged(bool connected);
private:
    TunnelConfig tunnelConfig_;
    ITunnelCallback* tunnelCallback_ = nullptr;
    std::function<void(std::string)> logoutCallback_;
    Address serverAddress_;
    PerformanceStatistics dnsResolvePerformance_;
    PerformanceStatistics connectPerformance_;
    PerformanceStatistics handShakePerformance_;
    
    TunnelConnectionStatus tunnelConnectionStatus_ = TunnelConnectionStatus::disconnect;
    bool  reconnectStatus_ = true;
    int reconnectTimes_ = 0;
    bool    manualStopped_ = true;
    bool    isLogin_ = false;
    std::atomic_bool    runInBackGround_ = ATOMIC_VAR_INIT(false);
    std::vector<std::string> highLevelMessage_;
    std::vector<std::string> middleLevelAckMessage_;
    std::vector<std::string> lowLevelAckMessage_;
    std::chrono::time_point<std::chrono::steady_clock> lastMessageTimePoint_;
    
    std::shared_ptr<IoService> ioService_;
    boost::asio::steady_timer pingTimer_;
    boost::asio::steady_timer pingTimeoutTimer_;
    boost::asio::steady_timer connectTimeoutTimer_;
    boost::asio::steady_timer loginTimeoutTimer_;
    boost::asio::steady_timer reconnectTimeoutTimer_;
    std::shared_ptr<NetClient> netClient_;
private:
    void launch_tunnel();
    void stop_tunnel();
    void init_callback();
    void create_new_connection();
    void try_connect();
    void reconnect();
    int  get_next_connect_interval();
    void disconnect();
    void start_ping();
    void start_login();
    void start_connect_timeout_check();
    void stop_connect_timeout_check();
    void start_login_timeout_check();
    void stop_login_timeout_check();
    void start_ping_timeout_check();
    void stop_ping_timeout_check();
    void process_message(const MessageProtocol& msg);
    void process_login_message(const MessageProcessor& messageProcessor);
    std::string get_login_info();
    bool get_reachable_address(SocketAddressList& addressList);
    std::vector<uint8_t> compress_body(CompressType compressType, const std::vector<uint8_t>& body);
    MessageProtocol make_send_data(const TunnelMessage& mercuryMessage);
};
}

