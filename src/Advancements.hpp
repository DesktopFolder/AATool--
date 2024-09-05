#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <string>
#include <vector>

#include "utilities.hpp"

// ok. let's rewrite this to be a bit more... sane?
// instead of having a separate map of completed/incompleted advancements,
// let's encode it into the actual advancement type.

namespace aa
{
struct Criteria
{
    // minecraft:snowy_taiga_hills
    std::string slug;

    // snowy_taiga_hills
    size_t discriminator_position{};
    std::string_view discriminator() const
    {
        return std::string_view{slug}.substr(discriminator_position);
    }

    // Snowy Taiga Hills
    std::string name;

    // Do we have this criteria?
    bool achieved = false;

    // Pointer to resource-managed texture.
    const sf::Texture* icon{};

    void reset_for_load() {
        achieved = false;
    }

    void reset() {
        achieved = false;
    }

    static Criteria from_slug(std::string_view slug_, std::string icon = "")
    {
        Criteria c{std::string{slug_}};
        c.discriminator_position = c.slug.find(':') + 1;

        // Basic assert for this.
        AA_ASSERT(c.discriminator_position < c.slug.size());

        c.name = c.discriminator();
        std::replace(c.name.begin(), c.name.end(), '_', ' ');
        c.name = title_case(c.name);
        c.icon = c.get_texture(icon);

        return c;
    }

private:
    const sf::Texture* get_texture(const std::string& icon) const;
};

struct Advancement
{
    /* Advancement - The canonical type.
     *
     * "minecraft:adventure/adventuring_time" ->
     * category: adventure
     * name: adventuring_time
     * criteria: [ "plains", "wooded_hills", ... ]
     *   -> Criteria can have a custom name.
     * icon: adventuring_time.png -> loaded into sf::Texture
     */

    // minecraft:adventure/adventuring_time
    std::string slug;

    // adventure/adventuring_time
    size_t discriminator_position{};
    std::string_view discriminator() const
    {
        return std::string_view{slug}.substr(discriminator_position);
    }

    // Adventuring Time
    std::string name;

    // Unknown, set externally
    std::string pretty_name; // Two Birds, One Arrow
    std::string short_name; // Two Birds

    // Do we have this advancement?
    bool achieved = false;

    // std::string full_id() const { return category + "/" + name; }

    string_map<Criteria> criteria;
    std::vector<std::string> criteria_list;

    enum class Source {
        Manifest,
        Found,
    } source = Source::Manifest;

    // Does nothing for now, but might be useful in the future.
    // Want to ensure we have this in the right locations.
    void reset_for_load()
    {
        achieved = false;
        for (auto& [k, crit] : criteria)
        {
            crit.reset_for_load();
        }
    }

    void reset()
    {
        achieved = false;
        for (auto& [k, crit] : criteria)
        {
            crit.reset();
        }
    }

    void mark_completed()
    {
        achieved = true;
        for (auto& [k, crit] : criteria)
        {
            crit.achieved = true;
        }
    }

    void add_criteria(std::string_view slug, std::string_view icon)
    {
        criteria.emplace(std::string{slug}, Criteria::from_slug(slug, std::string{icon}));
        criteria_list.emplace_back(std::string{slug});
    }

    static Advancement from_slug(std::string_view slug_, Source source = Source::Manifest)
    {
        Advancement a{std::string{slug_}};
        a.discriminator_position = a.slug.find(':') + 1;

        // Basic assert for this.
        AA_ASSERT(a.discriminator_position < a.slug.size());

        // adventure/adventuring_time
        const auto disc = a.discriminator();

        // adventuring_time
        auto name = disc;
        if (const auto slash = disc.find('/'); slash != std::string::npos)
        {
            name = disc.substr(slash + 1);
        }
        a.name = std::string{name};

        // Adventuring Time
        // std::replace(a.name.begin(), a.name.end(), '_', ' ');
        // a.name = title_case(a.name);
        // a.icon = a.get_texture();

        return a;
    }

    const sf::Texture* icon{};
};

/* AllAdvancements
 * Gives information on the possible/total advancements.
 * Parsed by AllAdvancements + the current Advancement file.
 */
struct AllAdvancements
{
    static AllAdvancements from_file(std::string_view filename);

    string_map<Advancement> advancements;

    void update_from_file(std::string_view filename);

    void reset() {
        for (auto& [k, v] : advancements) {
            v.reset();
        }
    }

    // for now, idk
    struct
    {
        bool has_egap = false;
        bool valid    = true;
    } meta{};
};
} // namespace aa
