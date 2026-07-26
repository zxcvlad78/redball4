#include "Client.hpp"
#include "Utils.hpp"
#include <cassert>

namespace MeatNet {

Client* Client::s_pCallbackInstance = nullptr;

Client::Client() {
    m_pInterface = GetSocketsInterface();
}

Client::~Client() {
    Disconnect();
}

bool Client::Connect(const std::string& address) {
    if (!m_pInterface) {
        fprintf(stderr, "Client: SteamNetworkingSockets interface not available.\n");
        return false;
    }

    if (m_connected || m_hConnection != k_HSteamNetConnection_Invalid) {
        fprintf(stderr, "Client: Already connected or connecting.\n");
        return false;
    }

    SteamNetworkingIPAddr addr;
    if (!addr.ParseString(address.c_str())) {
        fprintf(stderr, "Client: Invalid address format. Use 'ip:port' or 'domain:port'.\n");
        return false;
    }

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               (void*)SteamNetConnectionStatusChangedCallback);

    m_hConnection = m_pInterface->ConnectByIPAddress(addr, 1, &opt);
    if (m_hConnection == k_HSteamNetConnection_Invalid) {
        fprintf(stderr, "Client: Failed to create connection to %s\n", address.c_str());
        return false;
    }

    s_pCallbackInstance = this;
    printf("Client: Connecting to %s...\n", address.c_str());
    return true;
}

void Client::Disconnect() {
    if (m_pInterface && m_hConnection != k_HSteamNetConnection_Invalid) {
        m_pInterface->CloseConnection(m_hConnection, 0, "Client disconnecting", true);
        m_hConnection = k_HSteamNetConnection_Invalid;
    }
    m_connected = false;
    s_pCallbackInstance = nullptr;
}

void Client::Update() {
    if (!m_pInterface)
        return;

    PollIncomingMessages();
    PollConnectionStateChanges();
}

bool Client::Send(const void* data, uint32_t size, bool reliable) {
    if (!m_connected || !m_pInterface || m_hConnection == k_HSteamNetConnection_Invalid)
        return false;

    int flags = reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable;
    return m_pInterface->SendMessageToConnection(m_hConnection, data, size, flags, nullptr) == k_EResultOK;
}

bool Client::IsConnected() const {
    return m_connected;
}

void Client::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
    assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

    switch (pInfo->m_info.m_eState) {
        case k_ESteamNetworkingConnectionState_None:
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
            bool wasConnected = m_connected;
            m_connected = false;

            if (wasConnected && m_onDisconnected) {
                m_onDisconnected(pInfo->m_info.m_eEndReason, pInfo->m_info.m_szEndDebug);
            }

            if (m_pInterface) {
                m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
            }
            m_hConnection = k_HSteamNetConnection_Invalid;
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
            break;

        case k_ESteamNetworkingConnectionState_Connected:
            m_connected = true;
            if (m_onConnected) {
                m_onConnected();
            }
            break;

        default:
            break;
    }
}

void Client::PollIncomingMessages() {
    ISteamNetworkingMessage* pMsg = nullptr;
    int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pMsg, 1);
    while (numMsgs > 0 && m_connected) {
        if (numMsgs < 0) {
            fprintf(stderr, "Client: ReceiveMessagesOnConnection error\n");
            break;
        }

        if (m_onMessage) {
            m_onMessage(pMsg->m_pData, pMsg->m_cbSize);
        }

        pMsg->Release();

        numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pMsg, 1);
    }
}

void Client::PollConnectionStateChanges() {
    if (s_pCallbackInstance == this) {
        m_pInterface->RunCallbacks();
    } else {
        s_pCallbackInstance = this;
        m_pInterface->RunCallbacks();
    }
}

void Client::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo) {
    if (s_pCallbackInstance) {
        s_pCallbackInstance->OnConnectionStatusChanged(pInfo);
    }
}

} // namespace MeatNet