#include "Systems.hpp"
#include "Components.hpp"
#include <algorithm>
#include <vector>
#include <cstddef>


namespace SpriteSystems {
    void sprite(entt::registry& registry, sf::RenderWindow& window) {
        auto view = registry.view<Transform, Sprite>();

        for (auto [e, t, s] : view.each()) {
            s.sprite.setPosition(t.position);
            s.sprite.setRotation(t.rotation);
            s.sprite.setScale(t.scale);
            window.draw(s.sprite);
        }
    }

    void update(entt::registry& registry, sf::RenderWindow& window) {
        sprite(registry, window);
    }
}