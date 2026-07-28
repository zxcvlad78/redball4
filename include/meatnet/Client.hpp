#pragma once

#include "NetworkPeer.hpp"
#include <string>

namespace MeatNet {

class Client : public NetworkPeer {
public:
    Client();
    ~Client() override;

    bool Connect(const std::string& address);
    void Close() override; 

    void Update() override;
    bool Send(const void* data, uint32_t size, bool reliable = true) override;
    bool IsActive() const override { return m_connected; }

    ConnectionID GetConnectionID() const { return m_hConnection; }

    void SetOnConnected(OnConnectedCallback cb)           { m_onConnected = cb; }
    void SetOnDisconnected(OnDisconnectedCallback cb)     { m_onDisconnected = cb; }
    void SetOnMessageReceived(OnServerMessageCallback cb) { m_onMessage = cb; }

protected:
    void OnIncomingMessage(ConnectionID conn, const void* data, uint32_t size) override;
    void OnConnectionStateChange(SteamNetConnectionStatusChangedCallback_t* pInfo) override;

private:
    ConnectionID m_hConnection;
    bool m_connected;

    OnConnectedCallback m_onConnected;
    OnDisconnectedCallback m_onDisconnected;
    OnServerMessageCallback m_onMessage;

    void PollIncomingMessages();

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
    static Client* s_pCallbackInstance;
};

} // namespace MeatNet