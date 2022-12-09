#pragma once
#include <sstream>
#include <functional>
#include <iostream>
class Log
{
public:
    Log(std::function<void(std::string)> logHandler) 
    {
        logHandler_ = logHandler;
    };
    ~Log() 
    {
        if (logHandler_)
        {
            logHandler_(logStream_.str());
        }
        //std::cout << logStream_.str() << std::endl;
    };
    template <typename Elem>
    Log& operator<<(Elem elem)
    {
        logStream_ << elem;
        return *this;
    }
private:
    std::stringstream logStream_;
    std::function<void(std::string)> logHandler_;
};
