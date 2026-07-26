#pragma once

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <cstdint>
#include <functional>
#include <vector>
#include <string>

namespace MeatNet {

using ConnectionID = HSteamNetConnection;
using ListenSocket = HSteamListenSocket;
using PollGroup = HSteamNetPollGroup;

constexpr uint16 kDefaultPort = 27020;
constexpr int kMaxMessageSize = 4096;

// Callback type for client connection event (server)
//   connection - connection identifier of the new client
//   remoteAddress - string with client address
using OnClientConnectedCallback = std::function<void(ConnectionID, const char*)>;

// Callback type for client disconnection event (server)
//   connection - connection identifier of the disconnected client
//   reason - reason code
//   debug - debug message
using OnClientDisconnectedCallback = std::function<void(ConnectionID, int, const char*)>;

// Callback type for receiving a message from a client (server)
//   connection - sender
//   data - pointer to data
//   size - size of data in bytes
using OnMessageReceivedCallback = std::function<void(ConnectionID, const void*, uint32_t)>;

// Callback type for connection to server event (client)
using OnConnectedCallback = std::function<void()>;

// Callback type for disconnection from server event (client)
//   reason - reason code
//   debug - debug message
using OnDisconnectedCallback = std::function<void(int, const char*)>;

// Callback type for receiving a message from the server (client)
using OnServerMessageCallback = std::function<void(const void*, uint32_t)>;

struct ClientInfo {
    ConnectionID id;
    std::string address;
    // Custom data
};

} // namespace MeatNet