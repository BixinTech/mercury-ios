#include "Tunnel.h"
#include "TunnelImplementation.h"
#include "../utils/IoService.h"

namespace mercury
{
Tunnel::Tunnel()
:ioService_(std::make_shared<IoService>())
,impl_(std::make_shared<TunnelImplementation>(ioService_))
{
    ioService_->Start();
}
Tunnel::~Tunnel()
{
    ioService_->Stop();
}

void Tunnel::SetCallback(ITunnelCallback* tunnelCallback)
{
    impl_->SetCallback(tunnelCallback);
}

void Tunnel::Start()
{
    impl_->Start();
}

void Tunnel::Send(const TunnelMessage& message, std::function<void(int)> sendCallback)
{
    impl_->SendMessage(message, sendCallback);
}

void Tunnel::Stop()
{
    impl_->Stop();
}

void Tunnel::UpdateConfig(TunnelConfig config)
{
    impl_->SetConfig(config);
}

uint64_t Tunnel::GetTimeElapseFromLastMessage()
{
    return impl_->GetTimeElapseFromLastMessage();
}

void Tunnel::OnBackGround(bool status)
{
    impl_->OnBackGround(status);
}

void Tunnel::OnNetworkChanged(bool connected)
{
    impl_->OnNetworkChanged(connected);
}
}
