#pragma once

#include "entt/entt.hpp"
#include "sfml/Graphics.hpp"
#include "meatnet/Serialization.hpp"

struct NetId {
    long value = 0;

    const uint8_t* Serialize() {
        
    }

    // static NetId Deserialize(const uint8_t* data) {
    //     return 0;
    // }
};

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

    const uint8_t* Serialize() {
        MeatNet::BinaryWriter writer(16);
        
        writer.WriteInt(25);
        writer.WriteFloat(position.x);
        writer.WriteFloat(position.y);
        //writer.WriteFloat(rotation.asRadians()); // TODO
        writer.WriteFloat(scale.x);
        writer.WriteFloat(scale.y);
        
        return writer.GetBuffer().data();
    }

    static void Deserialize(entt::registry& registry, entt::entity e, const uint8_t* data) {
        auto& t = registry.emplace_or_replace<Transform>(e);
        MeatNet::BinaryReader reader(data, 16);
       
        reader.ReadFloat(t.position.x);
        reader.ReadFloat(t.position.y);
        //reader.ReadFloat(t->rotation.asRadians); // TODO
        reader.ReadFloat(t.scale.x);
        reader.ReadFloat(t.scale.y);
    }
};

struct Velocity {
    sf::Vector2f linear = {0.f, 0.f};
    float angular;
};
