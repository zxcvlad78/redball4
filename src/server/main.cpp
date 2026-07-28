//Server main
#include "../common/game/Components.hpp"
#include "../common/game/Systems.hpp"

#include "../common/game/physics/Components.hpp"
#include "../common/game/physics/Systems.hpp"

#include "meatnet/Utils.hpp"
#include "meatnet/Server.hpp"
#include "meatnet/Client.hpp"
#include "meatnet/Serialization.hpp"
#include "../common/ConsoleInput.hpp"
#include <cstdio>
#include <string>
#include <chrono>
#include <thread>

using namespace MeatNet;

static bool shouldQuit = false;
static Server* server = nullptr;

void OnClientConnected(ConnectionID conn, const char* address) {
    printf("Client %s connected (ID: %u)\n", address, conn);
}

void OnClientDisconnected(ConnectionID conn, int reason, const char* debug) {
    printf("Client %u disconnected. Reason: %d (%s)\n", conn, reason, debug);
}

void OnMessageReceived(ConnectionID conn, const void* data, uint32_t size) {
    BinaryReader reader(static_cast<const uint8_t*>(data), size);

    std::string text;
    if (reader.ReadString(text)) {
        printf("[SERVER] Player %u says: %s\n", conn, text.c_str());
        BinaryWriter writer;
        writer.WriteString(std::to_string(conn) + ": " + text);
        server->Send(writer.GetBuffer().data(), writer.Size(), true);
    }
}

void PrintUsage() {
    printf(
R"usage(Usage:
    [--port PORT]
)usage"
    );
}

void ParseCmd(const std::string& cmd) {
    if (cmd == "quit" || cmd == "exit") {
        shouldQuit = true;
        server->Close();
    }

    else {
        printf("Unknown command. Type 'quit' to stop.\n");
    }
}

int main(int argc, const char* argv[]) {
    uint16_t port = kDefaultPort;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = static_cast<uint16_t>(atoi(argv[++i]));
        }
    }

    if (!InitNetwork()) {
        fprintf(stderr, "Failed to initialize network\n");
        return 1;
    }

    ConsoleInput console;
    console.Start();

    server = new Server();
    server->SetOnClientConnected(OnClientConnected);
    server->SetOnClientDisconnected(OnClientDisconnected);
    server->SetOnMessageReceived(OnMessageReceived);
    if (!server->Start(port)) {
        fprintf(stderr, "Failed to start server\n");
        delete server;
        ShutdownNetwork();
        return 1;
    }
    
    printf("Server running. Type 'quit' to stop.\n");
    
    sf::Clock clock;
    entt::registry registry;

    while (!shouldQuit) {
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();

        server->Update();

        std::string cmd;
        if (console.GetNextLine(cmd)) {
            ParseCmd(cmd);
        }

        Game::Physics::Systems::update(registry, dt);
        Game::Systems::update(registry, dt);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    console.Stop();
    delete server;
    ShutdownNetwork();
    return 0;
}