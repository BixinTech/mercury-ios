#include "IoService.h"
#include <iostream>

IoService::IoService():ioserviceWorkguad_(boost::asio::make_work_guard(*this))
{
}

IoService::~IoService()
{
    Stop();
}

void IoService::Start()
{
    std::lock_guard<std::mutex> lg(mutex_);
    if (!started_)
    {
        boost::asio::io_context::restart();
        workThread_ = std::make_shared<std::thread>(
                                                    [&]() {
                                                        boost::asio::io_context::run();
                                                        std::cout << std::endl << "tunnel service quit...";
                                                    });
        started_ = true;
    }
}

void IoService::Stop()
{
    std::lock_guard<std::mutex> lg(mutex_);
    if (started_)
    {
        started_ = false;
        boost::asio::io_context::stop();
        if (workThread_)
        {
            if (workThread_->joinable())
            {
                workThread_->join();
            }
        }
    }
}

void IoService::Post(std::function<void()> task)
{
    std::lock_guard<std::mutex> lg(mutex_);
    if (started_)
    {
        boost::asio::io_context::post(task);
    }
}
