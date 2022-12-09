#include "PerformanceStatistics.h"

namespace mercury
{
PerformanceStatistics::PerformanceStatistics(PerformanceStatisticsType type)
{
    result_.type_ = type;
}

PerformanceStatistics::~PerformanceStatistics()
{
    
}

void PerformanceStatistics::Start()
{
    countStart_ = std::chrono::steady_clock::now();
}
void PerformanceStatistics::Stop()
{
    result_.timeUsing_ = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now() - countStart_).count();
}

void PerformanceStatistics::SetErrorInfo(int code, std::string details)
{
    result_.errorCode_ = code;
    result_.details = details;
}

TunnelStatisticsResult PerformanceStatistics::GetResult()
{
    return result_;
}
}
