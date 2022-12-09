//
//  MCRTunnelCallBack.h
//  Mercury
//
//  Created by 姜腾 on 2019/11/7.
//

#include "TunnelDefine.h"

namespace mercury {
        
class MCRTunnelCallBack : public ITunnelCallback {
    
public:
    
    void SetRecieveCallback(std::function<void(TunnelMessage)> recieveCallback);
    void SetAddressCallback(std::function<void(Address &)> addressCallback);
    void SetNetworkStatusCallback(std::function<void(NetStatus)> networkStatusCallback);
    void SetLogCallback(std::function<void(std::string)> logCallback = nullptr);
    void SetLoginCallback(std::function<void(LoginInfo &)> loginCallback);
    void SetHandStatisticsCallback(std::function<void(TunnelStatisticsResult)> handStatisticsCallback);
    
    
    virtual void OnNetworkStatusCallback(NetStatus status);
    virtual void OnRecieveCallback(TunnelMessage message);
    virtual void OnLogCallback(std::string log);
    virtual void OnGetLoginInfo(LoginInfo& LoginInfo);
    virtual void OnGetAddress(Address& address,Address& backupAddress);
    virtual void OnReportPerformanceStatistics(TunnelStatisticsResult result);
private:
    std::function<void(TunnelStatisticsResult)>  handStatisticsCallback_;
    std::function<void(TunnelMessage)>  recieveCallback_;
    std::function<void(Address&)>        addressCallback_;
    std::function<void(NetStatus)>       networkStatusCallback_;
    std::function<void(std::string)>     logCallback_;
    std::function<void(LoginInfo&)>  loginCallback_;
};
}
