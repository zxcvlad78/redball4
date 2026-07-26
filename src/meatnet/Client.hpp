#pragma once

#include "Types.hpp"
#include <string>

namespace MeatNet {

class Client {
public:
    Client();
    ~Client();

    bool Connect(const std::string& address);
    void Disconnect();
    void Update();

    //
    bool Send(const void* data, uint32_t size, bool reliable = true);

    bool IsConnected() const;
    ConnectionID GetConnectionID() const { return m_hConnection; }

    void SetOnConnected(OnConnectedCallback cb)         { m_onConnected = cb; }
    void SetOnDisconnected(OnDisconnectedCallback cb)   { m_onDisconnected = cb; }
    void SetOnMessageReceived(OnServerMessageCallback cb) { m_onMessage = cb; }

private:
    void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
    void PollIncomingMessages();
    void PollConnectionStateChanges();

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
    static Client* s_pCallbackInstance;

    ISteamNetworkingSockets* m_pInterface = nullptr;
    ConnectionID m_hConnection = k_HSteamNetConnection_Invalid;
    bool m_connected = false;

    OnConnectedCallback m_onConnected;
    OnDisconnectedCallback m_onDisconnected;
    OnServerMessageCallback m_onMessage;
};

} // namespace MeatNet