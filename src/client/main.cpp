#include "../common/game/Components.hpp"
#include "../common/game/Systems.hpp"

#include "../common/game/physics/Components.hpp"
#include "../common/game/physics/Systems.hpp"

#include "../common/game/sprite/Components.hpp"
#include "../common/game/sprite/Systems.hpp"

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
static Client* client = nullptr;

#define CLEAR_COLOR sf::Color::Black
#define WINDOW_SIZE sf::Vector2u(1280, 720)


void OnConnected() {
    printf("Connected to server!\n");
}

void OnDisconnected(int reason, const char* debug) {
    printf("Disconnected. Reason: %d (%s)\n", reason, debug);
}

void OnMessageReceived(const void* data, uint32_t size) {
    printf("Message received: ");
    BinaryReader reader(static_cast<const uint8_t*>(data), size);
    
    std::string msgText;
    if (reader.ReadString(msgText)) {
        printf("'[CHAT] %s'\n", msgText.c_str());
    }
}

void PrintUsage() {
    printf(
R"usage(Usage:
    SERVER_ADDR
)usage"
    );
}

std::vector<std::string> ParseArgs(const std::string& cmd) {
    std::vector<std::string> args;
    std::string current;
    bool inQuotes = false;
    for (size_t i = 0; i < cmd.size(); ++i) {
        char c = cmd[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ' ' && !inQuotes) {
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }//ща у меня пацан в телефоне говорит
    if (!current.empty()) args.push_back(current);
    return args;
}

void ParseCmd(const std::string& cmd) {
    auto args = ParseArgs(cmd);
    if (args.empty()) return;
    const std::string& command = args[0];

    if (command == "quit") {
        shouldQuit = true;
        if (client) client->Close();
    }
    else if (command == "connect") {
        if (args.size() < 1) { return; }
        if (client) delete client;

        const std::string serverAddress = args[1];

        client = new Client();
        client->SetOnConnected(OnConnected);
        client->SetOnDisconnected(OnDisconnected);
        client->SetOnMessageReceived(OnMessageReceived);
        if (!client->Connect(serverAddress)) {
            fprintf(stderr, "Failed to connect\n");
            delete client;
            return;
        }
    }
    else if (command == "disconnect") {
        if (client) {
            //client->Close();
            delete client;
        }
    }
    else if (command == "msg") {
        if (args.size() < 2) { return; }

        BinaryWriter writer;
        writer.WriteString(args[1]);
        client->Send(writer.GetBuffer().data(), writer.Size(), true);

    }
    else {

    }
}



int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE), "redball4 Client");
    window.setFramerateLimit(0);

    if (!InitNetwork()) {
        fprintf(stderr, "Failed to initialize network\n");
        return 1;
    }

    ConsoleInput console;
    console.Start();

    sf::Clock clock;
    entt::registry registry;

    while (!shouldQuit) {
        while (window.isOpen()) {
            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
            
            }
            
            sf::Time elapsed = clock.restart();
            float dt = elapsed.asSeconds();
        
            if (client) client->Update();
    
            std::string cmd;
            if (console.GetNextLine(cmd)) {
                ParseCmd(cmd);
            }

            window.clear(CLEAR_COLOR);
            //render relative to camera
            SpriteSystems::update(registry, window);

            window.setView(window.getDefaultView()); 
            //render relative to screen

            window.display();
        }
        
        
    }

    console.Stop();
    delete client;
    ShutdownNetwork();
    return 0;
}