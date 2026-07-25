#pragma once
#include "Components.hpp"

namespace Game::Physics {
    namespace Systems {
        void gravity(entt::registry& registry, float dt);
        void movement(entt::registry& registry, float dt);
        void collider(entt::registry& registry);
        void rigid_body(entt::registry& registry);

        void update(entt::registry& registry, float dt);
        void render(entt::registry& registry, sf::RenderWindow& window);
    }
}