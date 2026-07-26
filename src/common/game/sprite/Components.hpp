#pragma once
#include "../Components.hpp"

struct Sprite {
    sf::Sprite sprite;
    Sprite(entt::resource<sf::Texture> texture) : sprite(*texture) {  }
};