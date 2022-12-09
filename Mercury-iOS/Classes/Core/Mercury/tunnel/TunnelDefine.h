#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include <string>
#include "../utils/Constants.h"
#include "../utils/MessageIdGenerator.h"

namespace mercury {
struct Address
{
    std::string host;
    std::string port;
    bool isValid() {
        if (!host.empty() && !port.empty())
        {
            return true;
        }
        return false;
    };
};
enum class NetStatus
{
    connecting = 0,
    connected = 1,
    disconnect = 2,
};

enum class CompressType
{
    no = 0,
    zip = 1,
    //		gzip = 2,
};
struct TunnelMessage
{
    struct TunnelMessageHeader
    {
        uint8_t headerType;
        std::vector<uint8_t> headerBody;
    };
    
    CompressType compressType = CompressType::no;
    uint8_t command;
    uint8_t twoWay;
    MessageType type;
    uint64_t msgId;
    std::vector<uint8_t> body;
    std::vector<TunnelMessageHeader> headerList;
    void AddHeader(uint8_t headerType,std::vector<uint8_t> headerBody)
    {
        TunnelMessageHeader header{ headerType,headerBody };
        headerList.push_back(header);
    }
};

struct TunnelConfig
{
    uint32_t connectTimeout = 8;
    uint32_t shakehandTimeout = 25;
    uint32_t pingTimeout = 25;
    uint32_t pingInterval = 60;
    uint32_t pingIntervalBackground = 2 * 60;
    bool     closeMercury = false;
};

// 登录信息
struct LoginInfo
{
    std::string deviceId;
    int platform = -1;
    int appId = -1;
    std::string systemVersion ;
    std::string appVersion;
    std::string userId;
    int type = 0;
    std::string model;
    std::string bundleId;
};

enum class PerformanceStatisticsType
{
    dns_resolve = 0,
    connect = 1,
    hand_shake = 2
};
//Tunnel层统计数据
struct TunnelStatisticsResult
{
    PerformanceStatisticsType type_;
    int errorCode_ = 0;
    double reportRate_ = 1.0;
    uint64_t timeUsing_ = 0;
    std::string details;
};

class ITunnelCallback
{
public:
    virtual void OnNetworkStatusCallback(NetStatus status) = 0;
    virtual void OnRecieveCallback(TunnelMessage message) = 0;
    virtual void OnLogCallback(std::string log) =0;
    virtual void OnGetLoginInfo(LoginInfo& loginInfo) =0;
    virtual void OnGetAddress(Address& address,Address& backupAddress) =0;
    virtual void OnReportPerformanceStatistics(TunnelStatisticsResult result) =0;
};

enum class MessageCommand
{
    login = 1,
};
enum class MessageHeaderType
{
    compressed = 1,
    room = 2,
    batch = 3,
};
}
