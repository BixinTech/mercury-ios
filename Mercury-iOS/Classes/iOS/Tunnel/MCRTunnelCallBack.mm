//
//  MCRTunnelCallBack.m
//  Mercury
//
//  Created by 姜腾 on 2019/11/7.
//

#import "MCRTunnelCallBack.h"

namespace mercury {

void MCRTunnelCallBack::OnNetworkStatusCallback(NetStatus status) {
    if (networkStatusCallback_) {
        networkStatusCallback_(status);
    }
}
void MCRTunnelCallBack::OnRecieveCallback(TunnelMessage message) {
    if (recieveCallback_) {
        recieveCallback_(message);
    }
}
void MCRTunnelCallBack::OnLogCallback(std::string log) {
    if (logCallback_) {
        logCallback_(log);
    }
}
void MCRTunnelCallBack::OnGetLoginInfo(LoginInfo& loginInfo) {
    if (loginCallback_) {
        loginCallback_(loginInfo);
    }
}
void MCRTunnelCallBack::OnGetAddress(Address& address,Address& backupAddress) {
    if (addressCallback_) {
        addressCallback_(address);
    }
}
void MCRTunnelCallBack::OnReportPerformanceStatistics(TunnelStatisticsResult result) {
    if (handStatisticsCallback_) {
        handStatisticsCallback_(result);
    }
}

void MCRTunnelCallBack::SetRecieveCallback(std::function<void(TunnelMessage)> recieveCallback) {
    recieveCallback_ = recieveCallback;
}
void MCRTunnelCallBack::SetAddressCallback(std::function<void(Address &)> addressCallback) {
    addressCallback_ = addressCallback;
}
void MCRTunnelCallBack::SetNetworkStatusCallback(std::function<void(NetStatus)> networkStatusCallback) {
    networkStatusCallback_ = networkStatusCallback;
}
void MCRTunnelCallBack::SetLogCallback(std::function<void(std::string)> logCallback) {
    logCallback_ = logCallback;
}
void MCRTunnelCallBack::SetLoginCallback(std::function<void(LoginInfo &)> loginCallback) {
    loginCallback_ = loginCallback;
}
void MCRTunnelCallBack::SetHandStatisticsCallback(std::function<void(TunnelStatisticsResult)> handStatisticsCallback) {
    handStatisticsCallback_ = handStatisticsCallback;
}
}
