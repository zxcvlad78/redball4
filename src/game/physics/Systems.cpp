#include "Systems.hpp"
#include <cmath>

namespace Game::Physics::Systems {
    void gravity(entt::registry& registry, float dt) {
        auto view = registry.view<RigidBody, Velocity, Gravity>();
        for (auto [e, rb, v, g] : view.each()) {
            v.linear.y += PIXEL_G * g.scale * dt;
        }
    }

    void movement(entt::registry& registry, float dt) {
        auto view = registry.view<Transform, Velocity>();

        for (auto [entity, t, v] : view.each()) {
            t.position += v.linear * dt;
            t.rotation += sf::radians(v.angular * dt);
        }
    }

    void collider(entt::registry& registry) {
        auto view = registry.view<Transform, Collider>();
        
        //registry.clear<CollisionEvent>();

        std::vector<entt::entity> entities(view.begin(), view.end());
        
        for (size_t i = 0; i < entities.size(); ++i) {
            auto entity1 = entities[i];
            auto& transform1 = view.get<Transform>(entity1);
            auto& col1 = view.get<Collider>(entity1);
            
            for (size_t j = i + 1; j < entities.size(); ++j) {
                auto entity2 = entities[j];
                auto& transform2 = view.get<Transform>(entity2);
                auto& col2 = view.get<Collider>(entity2);
                
                sf::Vector2f size1(col1.size.x * transform1.scale.x, 
                                col1.size.y * transform1.scale.y);
                sf::Vector2f size2(col2.size.x * transform2.scale.x, 
                                col2.size.y * transform2.scale.y);
                
                sf::FloatRect rect1(transform1.position, size1);
                sf::FloatRect rect2(transform2.position, size2);
                
                auto intersection = rect1.findIntersection(rect2);
                if (!intersection.has_value()) continue;
                
                auto overlap = intersection.value();
                if (overlap.size.x < 0.01f && overlap.size.y < 0.01f) continue;
                
                sf::Vector2f center1(transform1.position.x + size1.x / 2.f,
                                    transform1.position.y + size1.y / 2.f);
                sf::Vector2f center2(transform2.position.x + size2.x / 2.f,
                                    transform2.position.y + size2.y / 2.f);
                
                sf::Vector2f direction = center2 - center1;
                
                sf::Vector2f normal;
                float penetration;
                
                float overlapLeft = (transform1.position.x + size1.x) - transform2.position.x;
                float overlapRight = (transform2.position.x + size2.x) - transform1.position.x;
                float overlapTop = (transform1.position.y + size1.y) - transform2.position.y;
                float overlapBottom = (transform2.position.y + size2.y) - transform1.position.y;
                
                float minOverlapX = std::min(overlapLeft, overlapRight);
                float minOverlapY = std::min(overlapTop, overlapBottom);
                
                if (minOverlapX < minOverlapY) {
                    penetration = minOverlapX;
                    normal.x = overlapLeft < overlapRight ? -1.f : 1.f;
                    normal.y = 0.f;
                } else {
                    penetration = minOverlapY;
                    normal.x = 0.f;
                    normal.y = overlapTop < overlapBottom ? -1.f : 1.f;
                }
                
                
                registry.emplace_or_replace<CollisionEvent>(entity1, entity2, normal, penetration);
                registry.emplace_or_replace<CollisionEvent>(entity2, entity1, -normal, penetration);
            }
        }
    }

    void rigid_body(entt::registry& registry) {
        auto view = registry.view<Velocity, RigidBody, CollisionEvent>();

        std::vector<entt::entity> ce_to_delete;

        for (auto [e, v, rb, ce] : view.each()) {
            Velocity* v2_ptr = registry.try_get<Velocity>(ce.entity);
            Velocity v2 = v2_ptr ? *v2_ptr : Velocity{};
            RigidBody rb2 = registry.try_get<RigidBody>(ce.entity) ? *registry.try_get<RigidBody>(ce.entity) : RigidBody{0.f};

            float inv_mass = 0.f;
            if (rb.mass > 0.0000001) {
                inv_mass = 1.0f / rb.mass;
            }

            float inv_mass2 = 0.f;
            if (rb2.mass > 0.0000001) {
                inv_mass2 = 1.0f / rb2.mass;
            }

            sf::Vector2f relative_velocity = v.linear - v2.linear;
            
            float velocity_along_normal = relative_velocity.x * ce.normal.x + 
                                    relative_velocity.y * ce.normal.y;
            
            if (velocity_along_normal > 0) continue;
            
            float restitution = 0.8f;
            
            float impulse_magnitude = -(1.0f + restitution) * velocity_along_normal;
            impulse_magnitude /= (inv_mass + inv_mass2);
            
            sf::Vector2f impulse = impulse_magnitude * ce.normal;

            v.linear += impulse * inv_mass;

            if (v2_ptr) {
                v2_ptr->linear -= impulse * inv_mass2;
            }

            ce_to_delete.push_back(e);
        }

        for (auto e : ce_to_delete) {
            registry.remove<CollisionEvent>(e);
        }
    }
        
    void update(entt::registry& registry, float dt) {
        gravity(registry, dt);
        movement(registry, dt);
        collider(registry);
        rigid_body(registry);
    }

    inline void render_debug(entt::registry& registry, sf::RenderWindow& window) {
        auto view = registry.view<Transform, Collider>();
        for (auto [e, t, c] : view.each()) {

            auto rect = sf::RectangleShape(c.size);
            rect.setFillColor(sf::Color(255, 0, 0, 78));
            rect.setPosition(t.position);
            rect.setRotation(t.rotation);
            rect.setScale(t.scale);
            
            window.draw(rect);
        }
    }

    void render(entt::registry& registry, sf::RenderWindow& window) {
        //render_debug(registry, window);
    }
}