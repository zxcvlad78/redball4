// #include "game/Components.hpp"
// #include "game/Systems.hpp"

// #include "game/sprite/Components.hpp"
// #include "game/sprite/Systems.hpp"

// #include "game/physics/Components.hpp"
// #include "game/physics/Systems.hpp"

// #include "resourceloader/ResourceLoader.hpp"


// #define CLEAR_COLOR sf::Color::Black
// #define WINDOW_SIZE sf::Vector2u(1280, 720)


// int main() {
//     sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE), "redball4");
//     window.setFramerateLimit(0);

//     sf::Clock clock;
//     entt::registry registry;

//     auto floor = registry.create(); {
//         auto& transform = registry.emplace<Transform>(floor); {
//             transform.position = {
//                 32.f,
//                 static_cast<float>(WINDOW_SIZE.y) - 200.f
//             };
//         }
//         auto& sprite = registry.emplace<Sprite>(floor,
//             resourceloader.load<sf::Texture, sf::TextureLoader>("res/textures/floor.png")
//         );
//         auto& collider = registry.emplace<Collider>(floor, 256.f, 32.f);

//     }


//     //MainLoop
//     while (window.isOpen()) {
//         while (const std::optional event = window.pollEvent()) {
//             if (event->is<sf::Event::Closed>()) {
//                 window.close();
//             }
         
//             if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
//                 sf::Keyboard::Key code = keyPressed->code;
                
//                 if (code == sf::Keyboard::Key::Space) {
//                     auto ball = registry.create(); {
//                         auto& transform = registry.emplace<Transform>(ball); {
//                             transform.position = {25.f, 25.f};
//                             transform.scale = {3.f, 3.f};
//                         }

//                         auto& velocity = registry.emplace<Velocity>(ball);
//                         auto& sprite = registry.emplace<Sprite>(ball,
//                             resourceloader.load<sf::Texture, sf::TextureLoader>("res/textures/ball.png")
//                         );

//                         auto& rb = registry.emplace<RigidBody>(ball);
//                         auto& collider = registry.emplace<Collider>(ball, 16.f, 16.f);
//                         auto& gravity = registry.emplace<Gravity>(ball);
//                     }
//                 }
                
//             }
//         }
    
//     sf::Time elapsed = clock.restart();
//     float dt = elapsed.asSeconds();

//     Game::Physics::Systems::update(registry, dt);

//     window.clear(CLEAR_COLOR);

//     Game::Systems::update(registry, dt);
//     SpriteSystems::update(registry, window);

//     //render relative to camera
//     Game::Physics::Systems::render(registry, window);

//     window.setView(window.getDefaultView()); 
//     //render relative to screen
    

//     window.display();
//     }

//     return 0;
// }