#include "Server.hpp"
#include "Utils.hpp"
#include <cassert>
#include <cstring>

namespace MeatNet {

Server* Server::s_pCallbackInstance = nullptr;

Server::Server() {
    m_pInterface = GetSocketsInterface();
}

Server::~Server() {
    Stop();
}

bool Server::Start(uint16_t port) {
    if (m_running)
        return false;

    if (!m_pInterface) {
        fprintf(stderr, "Server: SteamNetworkingSockets interface not available. Call InitNetwork first.\n");
        return false;
    }

    SteamNetworkingIPAddr localAddr;
    localAddr.Clear();
    localAddr.m_port = port;

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               (void*)SteamNetConnectionStatusChangedCallback);

    m_hListenSocket = m_pInterface->CreateListenSocketIP(localAddr, 1, &opt);
    if (m_hListenSocket == k_HSteamListenSocket_Invalid) {
        fprintf(stderr, "Server: Failed to create listen socket on port %d\n", port);
        return false;
    }

    m_hPollGroup = m_pInterface->CreatePollGroup();
    if (m_hPollGroup == k_HSteamNetPollGroup_Invalid) {
        fprintf(stderr, "Server: Failed to create poll group\n");
        m_pInterface->CloseListenSocket(m_hListenSocket);
        m_hListenSocket = k_HSteamListenSocket_Invalid;
        return false;
    }

    m_running = true;
    s_pCallbackInstance = this;

    printf("Server started on port %d\n", port);
    return true;
}

void Server::Stop() {
    if (!m_running)
        return;

    m_running = false;

    {
        std::lock_guard<std::mutex> lock(m_clientMutex);
        for (auto& pair : m_clients) {
            m_pInterface->CloseConnection(pair.first, 0, "Server shutting down", true);
        }
        m_clients.clear();
    }

    if (m_hListenSocket != k_HSteamListenSocket_Invalid) {
        m_pInterface->CloseListenSocket(m_hListenSocket);
        m_hListenSocket = k_HSteamListenSocket_Invalid;
    }

    if (m_hPollGroup != k_HSteamNetPollGroup_Invalid) {
        m_pInterface->DestroyPollGroup(m_hPollGroup);
        m_hPollGroup = k_HSteamNetPollGroup_Invalid;
    }

    s_pCallbackInstance = nullptr;
    printf("Server stopped.\n");
}

void Server::Update() {
    if (!m_running)
        return;

    PollIncomingMessages();
    PollConnectionStateChanges();
}

bool Server::SendToClient(ConnectionID client, const void* data, uint32_t size, bool reliable) {
    if (!m_running || !m_pInterface)
        return false;

    int flags = reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable;
    return m_pInterface->SendMessageToConnection(client, data, size, flags, nullptr) == k_EResultOK;
}

void Server::Broadcast(const void* data, uint32_t size, ConnectionID exclude, bool reliable) {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    for (auto& pair : m_clients) {
        if (pair.first != exclude) {
            SendToClient(pair.first, data, size, reliable);
        }
    }
}

void Server::Kick(ConnectionID client, const char* reason) {
    if (m_pInterface) {
        m_pInterface->CloseConnection(client, 0, reason ? reason : "Kicked by server", true);
    }
}

std::vector<ConnectionID> Server::GetConnectedClients() const {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    std::vector<ConnectionID> result;
    result.reserve(m_clients.size());
    for (auto& pair : m_clients)
        result.push_back(pair.first);
    return result;
}

size_t Server::GetClientCount() const {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    return m_clients.size();
}

bool Server::GetClientInfo(ConnectionID client, ClientInfo& outInfo) const {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    auto it = m_clients.find(client);
    if (it == m_clients.end())
        return false;
    outInfo = it->second;
    return true;
}

void Server::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
    ConnectionID conn = pInfo->m_hConn;

    switch (pInfo->m_info.m_eState) {
        case k_ESteamNetworkingConnectionState_None:
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
            std::lock_guard<std::mutex> lock(m_clientMutex);
            auto it = m_clients.find(conn);
            if (it != m_clients.end()) {
                ClientInfo info = it->second;
                m_clients.erase(it);

                if (m_onDisconnected) {
                    m_onDisconnected(conn, pInfo->m_info.m_eEndReason, pInfo->m_info.m_szEndDebug);
                }
            }

            m_pInterface->CloseConnection(conn, 0, nullptr, false);
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting: {
            if (m_pInterface->AcceptConnection(conn) != k_EResultOK) {
                m_pInterface->CloseConnection(conn, 0, nullptr, false);
                break;
            }

            if (!m_pInterface->SetConnectionPollGroup(conn, m_hPollGroup)) {
                m_pInterface->CloseConnection(conn, 0, nullptr, false);
                break;
            }

            ClientInfo info;
            info.id = conn;
            info.address = pInfo->m_info.m_szConnectionDescription;

            {
                std::lock_guard<std::mutex> lock(m_clientMutex);
                m_clients[conn] = info;
            }

            if (m_onConnected) {
                m_onConnected(conn, info.address.c_str());
            }
            break;
        }

        case k_ESteamNetworkingConnectionState_Connected:
            break;

        default:
            break;
    }
}

void Server::PollIncomingMessages() {
    ISteamNetworkingMessage* pMsg = nullptr;
    int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, &pMsg, 1);
    while (numMsgs > 0) {
        if (numMsgs < 0) {
            fprintf(stderr, "Server: ReceiveMessagesOnPollGroup error\n");
            break;
        }

        ConnectionID sender = pMsg->m_conn;
        if (m_onMessage) {
            m_onMessage(sender, pMsg->m_pData, pMsg->m_cbSize);
        }

        pMsg->Release();

        numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, &pMsg, 1);
    }
}

void Server::PollConnectionStateChanges() {
    if (s_pCallbackInstance == this) {
        m_pInterface->RunCallbacks();
    } else {
        s_pCallbackInstance = this;
        m_pInterface->RunCallbacks();
    }
}

void Server::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo) {
    if (s_pCallbackInstance) {
        s_pCallbackInstance->OnConnectionStatusChanged(pInfo);
    }
}

} // namespace MeatNet