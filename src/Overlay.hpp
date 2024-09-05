#pragma once

// What is the overlay?
// It's basically a set of turntables.
// Oh, how the turn tables.
#include "TurnTable.hpp"
#include "ResourceManager.hpp"
#include "Advancements.hpp"
#include "utilities.hpp"

#include <nlohmann/json_fwd.hpp>

#include <vector>

namespace aa
{
struct OverlayAdvancement {
    // let's not care too much atm
    OverlayAdvancement(std::string c, std::string n, std::string i) : category(c), name(n), icon(i)
    {
    }
    std::string category;
    std::string name;
    std::string icon;
};

struct OverlayCriteria {
    OverlayCriteria(std::string c, std::string n, std::string i) : advancement(c), name(n), icon(i)
    {
    }
    std::string advancement;
    std::string name;
    std::string icon;
};

struct OverlayManager
{
    TurnTable prereqs;
    TurnTable reqs;
    sf::Texture testText;

    std::vector<OverlayAdvancement> advancements;
    std::vector<OverlayCriteria> criteria;

    OverlayManager(AllAdvancements& status);

    void update_to(const AllAdvancements& advancements);

    void debug();

    void setRate(uint8_t rate)
    {
        prereqs.rate_ = rate;
        reqs.rate_    = rate;
    }

    void render(sf::RenderWindow& win)
    {
        prereqs.animateDraw(win);
        reqs.animateDraw(win);
    }

    void remap_textures(AllAdvancements& manifest);
};
}  // namespace aa
