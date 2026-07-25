#pragma once

#include "entt/entt.hpp"
#include "sfml/Graphics.hpp"

struct ZIndex {
    unsigned int value = 0;
};

struct Transform {
    sf::Vector2f position;
    sf::Angle rotation;
    sf::Vector2f scale = {1.f, 1.f};

    Transform& operator=(const Transform* t) {
        if (t != nullptr && this != t) {
            position = t->position;
            rotation = t->rotation;
            scale = t->scale;
        }
        return *this;
    }
};

struct Velocity {
    sf::Vector2f linear = {0.f, 0.f};
    float angular;
};
