#pragma once

#include "Types.hpp"
#include <cstdint>
#include <map>
#include <mutex>
#include <vector>

namespace MeatNet {

class Server {
public:
    Server();
    ~Server();

    bool Start(uint16_t port = kDefaultPort);

    void Stop();

    void Update();

    bool SendToClient(ConnectionID client, const void* data, uint32_t size, bool reliable = true);

    void Broadcast(const void* data, uint32_t size, ConnectionID exclude = k_HSteamNetConnection_Invalid, bool reliable = true);

    void Kick(ConnectionID client, const char* reason = nullptr);

    std::vector<ConnectionID> GetConnectedClients() const;

    size_t GetClientCount() const;

    void SetOnClientConnected(OnClientConnectedCallback cb)    { m_onConnected = cb; }
    void SetOnClientDisconnected(OnClientDisconnectedCallback cb) { m_onDisconnected = cb; }
    void SetOnMessageReceived(OnMessageReceivedCallback cb)   { m_onMessage = cb; }

    bool GetClientInfo(ConnectionID client, ClientInfo& outInfo) const;

private:
    void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
    void PollIncomingMessages();
    void PollConnectionStateChanges();

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
    static Server* s_pCallbackInstance;

    ISteamNetworkingSockets* m_pInterface = nullptr;
    ListenSocket m_hListenSocket = k_HSteamListenSocket_Invalid;
    PollGroup m_hPollGroup = k_HSteamNetPollGroup_Invalid;

    std::map<ConnectionID, ClientInfo> m_clients;
    mutable std::mutex m_clientMutex;

    OnClientConnectedCallback m_onConnected;
    OnClientDisconnectedCallback m_onDisconnected;
    OnMessageReceivedCallback m_onMessage;

    bool m_running = false;
};

} // namespace MeatNet