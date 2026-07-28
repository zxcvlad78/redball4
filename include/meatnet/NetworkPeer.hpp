#pragma once

#include "Types.hpp"
#include "Utils.hpp"
#include <functional>
#include <vector>
#include <cstdint>

namespace MeatNet {

class NetworkPeer {
public:
    NetworkPeer() : m_pInterface(GetSocketsInterface()) { }
    virtual ~NetworkPeer() = default;

    virtual void Update() = 0;
    virtual bool Send(const void* data, uint32_t size, bool reliable = true) = 0;

    virtual void Close() = 0;
    virtual bool IsActive() const = 0;

    virtual ISteamNetworkingSockets* GetInterface() const { return m_pInterface; }

    void SetLogCallback(LogCallback cb) { m_logCallback = cb; }

protected:
    ISteamNetworkingSockets* m_pInterface = nullptr;
    LogCallback m_logCallback;

    virtual void OnIncomingMessage(ConnectionID conn, const void* data, uint32_t size) = 0;
    virtual void OnConnectionStateChange(SteamNetConnectionStatusChangedCallback_t* pInfo) = 0;

    void PollCallbacks();
    void Log(LogLevel level, const char* msg);
};

} // namespace MeatNet