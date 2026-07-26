#include "meatnet/Utils.hpp"
#include "meatnet/Server.hpp"
#include "meatnet/Client.hpp"
#include "meatnet/Serialization.hpp"
#include "meatnet/ConsoleInput.hpp"
#include <cstdio>
#include <string>
#include <chrono>
#include <thread>

using namespace MeatNet;

static bool g_quit = false;
static NetworkPeer* peer = nullptr;

void OnClientConnected(ConnectionID conn, const char* address) {
    printf("[SERVER] Client %s connected (ID: %u)\n", address, conn);
}

void OnClientDisconnected(ConnectionID conn, int reason, const char* debug) {
    printf("[SERVER] Client %u disconnected. Reason: %d (%s)\n", conn, reason, debug);
}

void OnServerMessage(ConnectionID conn, const void* data, uint32_t size) {
    BinaryReader reader(static_cast<const uint8_t*>(data), size);

    std::string text;
    if (reader.ReadString(text)) {
        printf("[SERVER] Player %u says: %s\n", conn, text.c_str());
        BinaryWriter writer;
        writer.WriteString(std::to_string(conn) + ": " + text);
        peer->Send(writer.GetBuffer().data(), writer.Size(), true);
    }
}

void OnConnected() {
    printf("[CLIENT] Connected to server!\n");
}

void OnDisconnected(int reason, const char* debug) {
    printf("[CLIENT] Disconnected. Reason: %d (%s)\n", reason, debug);
    g_quit = true;
}

void OnClientMessage(const void* data, uint32_t size) {
    BinaryReader reader(static_cast<const uint8_t*>(data), size);
    
    std::string msgText;
    if (reader.ReadString(msgText)) {
        printf("[CHAT] %s\n", msgText.c_str());
    }

    
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
    bool bServer = false, bClient = false;
    uint16_t port = kDefaultPort;
    std::string serverAddress;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "server") == 0) bServer = true;
        else if (strcmp(argv[i], "client") == 0) bClient = true;
        else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = static_cast<uint16_t>(atoi(argv[++i]));
        } else if (bClient && serverAddress.empty()) {
            serverAddress = argv[i];
        } else {
            PrintUsage();
            return 1;
        }
    }

    if ((bServer && bClient) || (!bServer && !bClient) || (bClient && serverAddress.empty())) {
        PrintUsage();
        return 1;
    }

    if (!InitNetwork()) {
        fprintf(stderr, "Failed to initialize network\n");
        return 1;
    }

    ConsoleInput console;
    console.Start();


    if (bServer) {
        Server* server = new Server();
        server->SetOnClientConnected(OnClientConnected);
        server->SetOnClientDisconnected(OnClientDisconnected);
        server->SetOnMessageReceived(OnServerMessage);
        if (!server->Start(port)) {
            fprintf(stderr, "Failed to start server\n");
            delete server;
            ShutdownNetwork();
            return 1;
        }
        peer = server;
        printf("Server running. Type 'quit' to stop.\n");
    } else {
        Client* client = new Client();
        client->SetOnConnected(OnConnected);
        client->SetOnDisconnected(OnDisconnected);
        client->SetOnMessageReceived(OnClientMessage);
        if (!client->Connect(serverAddress)) {
            fprintf(stderr, "Failed to connect\n");
            delete client;
            ShutdownNetwork();
            return 1;
        }
        peer = client;
        printf("Client connecting. Type 'quit' to exit.\n");
    }

    while (!g_quit) {
        peer->Update();

        std::string cmd;
        if (console.GetNextLine(cmd)) {
            if (cmd == "quit" || cmd == "exit") {
                g_quit = true;
                if (bClient) {
                    static_cast<Client*>(peer)->Disconnect();
                } else {
                    static_cast<Server*>(peer)->Stop();
                }
            } else if (bClient) {
                BinaryWriter writer;
                writer.WriteString(cmd);
                peer->Send(writer.GetBuffer().data(), writer.Size(), true);
            } else {
                printf("Unknown command. Type 'quit' to stop.\n");
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    console.Stop();
    delete peer;
    ShutdownNetwork();
    return 0;
}