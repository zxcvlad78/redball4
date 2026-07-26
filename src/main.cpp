#include "meatnet/Utils.hpp"
#include "meatnet/Server.hpp"
#include "meatnet/Client.hpp"
#include "meatnet/ConsoleInput.hpp"
#include "meatnet/Serialization.hpp"
#include <cstdio>
#include <string>
#include <chrono>
#include <thread>

using namespace MeatNet;

static bool g_quit = false;

void SendChatMessage(Client client, std::string messageText) {
    BinaryWriter writer;
    
    writer.WriteString(messageText);

    client.Send(writer.GetBuffer().data(), writer.Size(), true);
}

void OnClientConnected(ConnectionID conn, const char* address) {
    printf("[SERVER] Client %s connected (ID: %u)\n", address, conn);
}

void OnClientDisconnected(ConnectionID conn, int reason, const char* debug) {
    printf("[SERVER] Client %u disconnected. Reason: %d (%s)\n", conn, reason, debug);
}

void OnServerMessage(ConnectionID conn, const void* data, uint32_t size) {
    printf("[SERVER] Received %u bytes from %u\n", size, conn);
    // Server received message

    // Deserialize message
    BinaryReader reader(static_cast<const uint8_t*>(data), size);
    std::string messageText;
    if (!reader.ReadString(messageText)) {
        fprintf(stderr, "[SERVER] Failed to deserialize data\n");
        return;
    }

    //messageText = conn + messageText;

    // Send message to all clients
    


}

void OnConnected() {
    printf("[CLIENT] Connected to server!\n");
}

void OnDisconnected(int reason, const char* debug) {
    printf("[CLIENT] Disconnected. Reason: %d (%s)\n", reason, debug);
    g_quit = true;
}

void OnClientMessage(const void* data, uint32_t size) {
    printf("[CLIENT] Received %u bytes from server\n", size);
    // Client received message
}

void PrintUsage() {
    printf(
R"usage(Usage:
    client SERVER_ADDR
    server [--port PORT]
)usage"
    );
}

int main(int argc, const char* argv[]) {
    bool bServer = false;
    bool bClient = false;
    uint16_t port = kDefaultPort;
    std::string serverAddress;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "server") == 0) {
            bServer = true;
        } else if (strcmp(argv[i], "client") == 0) {
            bClient = true;
        } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = static_cast<uint16_t>(atoi(argv[++i]));
        } else if (bClient && serverAddress.empty()) {
            serverAddress = argv[i];
        } else {
            PrintUsage();
            return 1;
        }
    }

    if ((bServer && bClient) || (!bServer && !bClient)) {
        PrintUsage();
        return 1;
    }
    if (bClient && serverAddress.empty()) {
        PrintUsage();
        return 1;
    }

    // Network initialization
    if (!InitNetwork()) {
        fprintf(stderr, "Failed to initialize network\n");
        return 1;
    }

    SetLogCallback([](LogLevel level, const char* msg) {
        // Logging
    });

    ConsoleInput console;
    console.Start();

    if (bServer) {
        Server server;
        server.SetOnClientConnected(OnClientConnected);
        server.SetOnClientDisconnected(OnClientDisconnected);
        server.SetOnMessageReceived(OnServerMessage);

        if (!server.Start(port)) {
            fprintf(stderr, "Failed to start server\n");
            ShutdownNetwork();
            return 1;
        }

        printf("Server is running. Type 'quit' to stop.\n");

        // Main loop
        while (!g_quit) {
            server.Update();

            std::string cmd;
            if (console.GetNextLine(cmd)) {
                if (cmd == "quit" || cmd == "exit") {
                    g_quit = true;
                } else {
                    printf("Unknown command: %s\n", cmd.c_str());
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        server.Stop();
    } else if (bClient) {
        Client client;
        client.SetOnConnected(OnConnected);
        client.SetOnDisconnected(OnDisconnected);
        client.SetOnMessageReceived(OnClientMessage);

        if (!client.Connect(serverAddress)) {
            fprintf(stderr, "Failed to connect to server\n");
            ShutdownNetwork();
            return 1;
        }

        printf("Client connecting... Type 'quit' to exit.\n");

        while (!g_quit) {
            client.Update();

            std::string cmd;
            if (console.GetNextLine(cmd)) {
                if (cmd == "quit" || cmd == "exit") {
                    g_quit = true;
                    client.Disconnect();
                } else {
                    // Sending message to server
                    client.Send(cmd.c_str(), static_cast<uint32_t>(cmd.size() + 1), true);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        client.Disconnect();
    }

    console.Stop();
    ShutdownNetwork();

    return 0;
}