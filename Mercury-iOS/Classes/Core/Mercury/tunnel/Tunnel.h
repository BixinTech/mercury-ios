#pragma once
#include "TunnelDefine.h"
class IoService;
namespace mercury {
class TunnelImplementation;
class Tunnel
{
public:
    Tunnel();
    ~Tunnel();
    
    /**
     * @brief: 设置tunnel回调
     */
    void SetCallback(ITunnelCallback* tunnelCallback);
    
    /**
     * @brief: 开启连接
     */
    void Start();
    
    /**
     * @brief 发送消息
     * @param mercuryMessage 消息内容
     * @return true 发送消息操作成功; false 发送操作失败, 连接已经断开;
     */
    void Send(const TunnelMessage& message, std::function<void(int)> sendCallback=nullptr);
    
    /**
     * @brief:断开连接
     */
    void Stop();
    
    /**
     * @brief:更新tunnel内部配置信息
     * @param config tunnel配置信息(connect/handshake/ping/close策略配置)
     */
    void UpdateConfig(TunnelConfig config);
    
    /**
     * @brief:获取最后一条消息距现在的时间间隔
     * @return:time elapse (ms)
     */
    uint64_t GetTimeElapseFromLastMessage();
    
    /**
     * @brief:切换前后台状态，默认前台
     * @param status true: 后台 false: 前台
     */
    void OnBackGround(bool status);
    
    /**
     * @brief:网络状态切换
     * @param connected true: 已连接 false: 断开
     */
    void OnNetworkChanged(bool connected);
private:
    std::shared_ptr<IoService> ioService_;
    std::shared_ptr < TunnelImplementation> impl_;
};
}

