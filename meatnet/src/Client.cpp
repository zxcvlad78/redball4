#include "meatnet/Client.hpp"
#include <cassert>
#include <cstdio>

namespace MeatNet {

Client* Client::s_pCallbackInstance = nullptr;

Client::Client()
    : NetworkPeer()
    , m_hConnection(k_HSteamNetConnection_Invalid)
    , m_connected(false) {
}

Client::~Client() {
    Disconnect();
}

bool Client::Connect(const std::string& address) {
    if (m_connected || m_hConnection != k_HSteamNetConnection_Invalid) {
        Log(LogLevel::Warning, "Already connected or connecting");
        return false;
    }
    if (!m_pInterface) {
        Log(LogLevel::Error, "SteamNetworkingSockets interface not available");
        return false;
    }

    SteamNetworkingIPAddr addr;
    if (!addr.ParseString(address.c_str())) {
        Log(LogLevel::Error, ("Invalid address: " + address).c_str());
        return false;
    }

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               (void*)SteamNetConnectionStatusChangedCallback);

    m_hConnection = m_pInterface->ConnectByIPAddress(addr, 1, &opt);
    if (m_hConnection == k_HSteamNetConnection_Invalid) {
        Log(LogLevel::Error, ("Failed to connect to " + address).c_str());
        return false;
    }

    s_pCallbackInstance = this;
    Log(LogLevel::Msg, ("Connecting to " + address + "...").c_str());
    return true;
}

void Client::Disconnect() {
    if (m_pInterface && m_hConnection != k_HSteamNetConnection_Invalid) {
        m_pInterface->CloseConnection(m_hConnection, 0, "Client disconnect", true);
        m_hConnection = k_HSteamNetConnection_Invalid;
    }
    m_connected = false;
    s_pCallbackInstance = nullptr;
}

void Client::Update() {
    if (!m_pInterface) return;
    PollCallbacks();
    PollIncomingMessages();
}

bool Client::Send(const void* data, uint32_t size, bool reliable) {
    if (!m_connected || !m_pInterface || m_hConnection == k_HSteamNetConnection_Invalid) {
        return false;
    }
    int flags = reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable;
    return m_pInterface->SendMessageToConnection(m_hConnection, data, size, flags, nullptr) == k_EResultOK;
}

void Client::OnConnectionStateChange(SteamNetConnectionStatusChangedCallback_t* pInfo) {
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

void Client::OnIncomingMessage(ConnectionID conn, const void* data, uint32_t size) {
    if (m_onMessage) {
        m_onMessage(data, size);
    }
}

void Client::PollIncomingMessages() {
    ISteamNetworkingMessage* pMsg = nullptr;
    int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pMsg, 1);
    while (numMsgs > 0 && m_connected) {
        if (numMsgs < 0) {
            Log(LogLevel::Error, "ReceiveMessagesOnConnection error");
            break;
        }
        OnIncomingMessage(m_hConnection, pMsg->m_pData, pMsg->m_cbSize);
        pMsg->Release();
        numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pMsg, 1);
    }
}

void Client::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo) {
    if (s_pCallbackInstance) {
        s_pCallbackInstance->OnConnectionStateChange(pInfo);
    }
}

} // namespace MeatNet