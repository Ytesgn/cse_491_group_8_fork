//
// Created by Vincenzo on 10/18/2023.
//

#pragma once
#include <map>
#include "SFML/Graphics.hpp"
#include "../core/Data.hpp"
#include "../core/InterfaceBase.hpp"

namespace i_2D
{
    using namespace cse491;
    class TextureHolder {

    private:
        std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textures_;

    public:
        TextureHolder() = default;
        ~TextureHolder() = default;
        void LoadTexture(std::string id, std::string file_name);
        const sf::Texture& GetTexture(std::string id);
        std::map<std::string, sf::Texture> MazeTexture();
        std::map<std::string, sf::Texture> SecondWorldTexture();
        std::map<std::string, sf::Texture> ManualWorldTexture();
    };
}

