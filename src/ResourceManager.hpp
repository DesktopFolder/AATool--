#pragma once

#include "utilities.hpp"
#include "logging.hpp"
#include "compat.hpp"

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <memory>
#include <string>

namespace sf { class Font; }

namespace aa
{
inline std::pair<std::string, std::string> basic_string_splitter(std::string_view s)
{
    // Very simple algorithm.
    // Our goal is to split at the 'most central' point of the string.
    const size_t center = s.size() / 2;
    size_t left         = center;
    size_t right        = center;
    while (true)
    {
        if (s[left] == ' ')
        {
            return {std::string{s.substr(0, left)}, std::string{s.substr(left + 1)}};
        }
        if (s[right] == ' ')
        {
            return {std::string{s.substr(0, right)}, std::string{s.substr(right + 1)}};
        }
        if (left == 0 && right == s.size() - 1)
        {
            log::error("Cannot split the string: ", s);
            throw std::runtime_error("Failed to split a string.");
        }
        if (left != 0)
        {
            left -= 1;
        }
        if (right != s.size() - 1)
        {
            right += 1;
        }
    }
}

struct ResourceManager
{
    static ResourceManager& instance();
    static std::unordered_map<std::string, std::string> getAllAssets();
    static std::string assetName(std::string filePath)
    {
        aa::normalize_path(filePath);
        if (filePath.size() < 3) return filePath;
        // remove .png
        if (filePath.ends_with(".png") || filePath.ends_with(".gif"))
            filePath = filePath.substr(0, filePath.size() - 4);
        if (filePath[filePath.size() - 3] == '^')
            return assetName(std::string{filePath.begin(), filePath.end() - 3});

        auto it = std::find(filePath.crbegin(), filePath.crend(), '/');
        if (it == filePath.crend()) return filePath;
        return std::string{it.base(), filePath.cend()};
    }
    void loadAllCriteria();

    static sf::Drawable& basic_marker(float radius, float x, float y);

    void commit() { no_load = true; }

    /* Literally just jams a texture into a vector of unique pointers. */
    const sf::Texture* store_texture_at(std::string path)
    {
        random_textures.emplace_back(std::make_unique<sf::Texture>());
        random_textures.back()->loadFromFile(path);
        return random_textures.back().get();
    }

    const sf::Font& get_font();

    const sf::Texture* remap_texture(const sf::Texture* base, uint64_t new_size);

    std::vector<std::unique_ptr<sf::Texture>> random_textures;
    std::vector<std::unique_ptr<sf::RenderTexture>> random_rerenders;

    // actually use this lol!
    string_map<std::unique_ptr<sf::Texture>> criteria_map;

private:
    ResourceManager() { loadAllCriteria(); }
    ~ResourceManager();
    void loadCriteria(std::string, std::string);
    // IMPLEMENTATION DETAIL DON'T LOOK IT IS VERY BAD :)
    // later: wtf
    std::unordered_map<std::string, sf::Texture>* current_ = nullptr;
    bool no_load                                           = false;
};
} // namespace aa
