#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

namespace sf
{
    struct TextureLoader {
        using result_type = std::shared_ptr<Texture>;

        std::shared_ptr<Texture> operator()(const std::string& path) const {
            auto texture = std::make_shared<Texture>();
            if (!texture->loadFromFile(path)) {
                return nullptr;
            }
            return texture;
        }
    };

    struct SoundBufferLoader {
        using result_type = std::shared_ptr<SoundBuffer>;
        
        std::shared_ptr<SoundBuffer> operator()(const std::string& path) const {
            auto sb = std::make_shared<SoundBuffer>();
            if (!sb->loadFromFile(path)) {
                return nullptr;
            }
            return sb;
        }
    };

    struct ShaderLoader {
        using result_type = std::shared_ptr<Shader>;
        
        std::shared_ptr<Shader> operator()(const std::string& vertex_path, const std::string& fragment_path) const {
            auto shader = std::make_shared<Shader>();
            if (!shader->loadFromFile(vertex_path, fragment_path)) {
                return nullptr;
            }
            return shader;
        }
    };

}