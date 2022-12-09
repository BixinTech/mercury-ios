#include "ConnectionManager.h"
#include "../socket/NetClient.h"

ConnectionManager::ConnectionManager(std::shared_ptr<NetClient> netClient)
:netClient_(netClient)
{
    
}

ConnectionManager::~ConnectionManager()
{
    
}
