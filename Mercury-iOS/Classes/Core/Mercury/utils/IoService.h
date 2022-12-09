#pragma once
#include "BoostHeader.h"
class IoService : public boost::asio::io_context
{
public:
    IoService();
    ~IoService();
    void Start();
    void Stop();
    void Post(std::function<void()> task);
private:
    std::mutex mutex_;
    bool started_ = false;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> ioserviceWorkguad_;
    std::shared_ptr<std::thread> workThread_;
};

