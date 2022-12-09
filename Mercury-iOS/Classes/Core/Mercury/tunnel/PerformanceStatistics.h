#pragma once
#include "TunnelDefine.h"
#include <chrono>

namespace mercury {
	enum class StatisticErrors
	{
		Invalid = -10000,
		HandShakeResponseError = -3,
		TunnelDisconnect = -2,
		TimeOut = -1,
		Success = 0,
	};
	class PerformanceStatistics
	{
	public:
		PerformanceStatistics(PerformanceStatisticsType type);
		~PerformanceStatistics();
		void Start();
		void Stop();
		void SetErrorInfo(int code,std::string details="");
		TunnelStatisticsResult GetResult();

	private:
		TunnelStatisticsResult result_;
		StatisticErrors errorcode_ = StatisticErrors::Invalid;
		StatisticErrors lastErrorCode_ = StatisticErrors::Invalid;
		std::chrono::time_point<std::chrono::steady_clock> countStart_;
	};
};

