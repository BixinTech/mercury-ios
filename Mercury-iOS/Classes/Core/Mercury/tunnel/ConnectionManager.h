#pragma once
#include <memory>
class NetClient;
class ConnectionManager
{
public:
    ConnectionManager(std::shared_ptr<NetClient> netClient);
    ~ConnectionManager();
private:
    std::shared_ptr<NetClient> netClient_;
};

