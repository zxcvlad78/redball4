#pragma once
#include "../Components.hpp"


namespace Game::Physics {
    constexpr float M_G = 9.81f;
    constexpr float PIXEL_G = M_G * 100.f;
};


struct RigidBody {
    float mass = 1.0f;
};

struct Gravity {
    float scale = 1.0f;
};

struct Collider {
    sf::Vector2f size;

    Collider(float sx, float sy) : size{sx, sy} {}
};

struct CollisionEvent {
    entt::entity entity = entt::null;
    sf::Vector2f normal;
    float penetration;
};