#include "meatnet/Utils.hpp"
#include "meatnet/Server.hpp"
#include "meatnet/Client.hpp"
#include "meatnet/Serialization.hpp"
#include "common/ConsoleInput.hpp"
#include <cstdio>
#include <string>
#include <chrono>
#include <thread>

using namespace MeatNet;

static bool shouldQuit = false;
static Client* client = nullptr;


void OnConnected() {
    printf("[CLIENT] Connected to server!\n");
}

void OnDisconnected(int reason, const char* debug) {
    printf("[CLIENT] Disconnected. Reason: %d (%s)\n", reason, debug);
    shouldQuit = true;
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


    client = new Client();
    client->SetOnConnected(OnConnected);
    client->SetOnDisconnected(OnDisconnected);
    client->SetOnMessageReceived(OnClientMessage);
    if (!client->Connect(serverAddress)) {
        fprintf(stderr, "Failed to connect\n");
        delete client;
        ShutdownNetwork();
        return 1;
    }
    
    printf("Connected to %s. Type 'quit' to exit.\n", serverAddress.c_str());
    

    while (!shouldQuit) {
        client->Update();

        std::string cmd;
        if (console.GetNextLine(cmd)) {
            if (cmd == "quit" || cmd == "exit") {
                shouldQuit = true;
                client->Close();
            } else {
                BinaryWriter writer;
                writer.WriteString(cmd);
                client->Send(writer.GetBuffer().data(), writer.Size(), true);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    console.Stop();
    delete client;
    ShutdownNetwork();
    return 0;
}