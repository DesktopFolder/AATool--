#include "Advancements.hpp"

#include "logging.hpp"

#include "ResourceManager.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace aa
{
const sf::Texture* Criteria::get_texture(const std::string& icon) const
{
    auto& rm              = ResourceManager::instance();
    std::string_view disc = icon.empty() ? discriminator() : std::string_view{icon};
    if (not rm.criteria_map.contains(disc))
    {
        get_logger("Criteria::get_texture")
            .fatal_error(fmt::format("Could not find {} (icon: {}) ({})", disc, icon, slug));
    }
    return rm.criteria_map.find(disc)->second.get();
}

namespace manifest
{
// This namespace exists to group parsers that only act on
// manifest JSON files (i.e. JSON files that define advancements)
// Specifically the format of file that is generated from the XML.
std::string unprefixed(std::string s)
{
    if (const auto itr = s.find(':'); itr != std::string::npos)
    {
        return s.substr(itr + 1);
    }
    return s;
}

auto unprefixed_id(auto& j)
{
    // Remove minecraft: prefix from identifier. We don't care about it.
    return unprefixed(j["@id"].template get<std::string>());
}

template <typename T>
auto after_last(auto delim, T iterable)
{
    // now we have a normalized id, something like: end/root or hello/world
    auto it = std::find(iterable.crbegin(), iterable.crend(), delim);
    return T{it.base(), iterable.cend()};
}

auto parse_advancement_id(auto& j)
{
    // Specifically used on manifest load.
    struct ParseResult
    {
        std::string full_id;
        std::string category;
        std::string icon;
    };

    auto id = unprefixed_id(j);

    // now we have a normalized id, something like: end/root or hello/world
    auto it = std::find(id.crbegin(), id.crend(), '/');
    if (it == id.crend()) throw std::runtime_error("Could not parse advancement ID: " + id);

    auto icon     = std::string{it.base(), id.cend()};
    auto category = std::string{id.cbegin(), it.base() - 1};

    return ParseResult{std::move(id), std::move(category), std::move(icon)};
}

std::string get_icon(auto& j)
{
    if (j.contains("@icon")) return j["@icon"].template get<std::string>();
    return "";
}
} // namespace manifest

// Create our initial advancement state out of a manifest file.
AllAdvancements AllAdvancements::from_file(std::string_view filename /* advancements.json */)
{
    auto& logger = get_logger("AllAdvancements::from_file");
    AllAdvancements ret{};

    logger.debug("Loading core advancement manifest from file: ", filename);

    std::ifstream f(filename.data() /* msvc */);
    if (!f.good())
    {
        logger.fatal_error("Failed to load core advancement manifest from file: ", filename);
    }

    logger.debug("Parsing manifest JSON.");
    auto advancements = json::parse(f);

    // We load all assets here, during manifest creation.
    // Generally, we only create one manifest. However, we can create more, not sure
    // why we would, but hey. However, let's load everything statically regardless.
    auto& rm = aa::ResourceManager::instance();

    logger.debug("Loading map of available assets.");
    auto assets = aa::ResourceManager::getAllAssets();
    for (auto& [k, v] : assets)
    {
        // take this out of startup logs for now
        // I know, ideally we'd log it, but I kind of want readable logs in console...
        // logger.debug("Asset named ", k, " located at ", v);
    }
    logger.debug("Was able to find ", assets.size(), " potential assets.");

    for (auto& k : advancements)
    {
        /*
         *   ...,  {
         *           "@name": "Adventure",
         *           "advancement": [...]
         */
        const std::string cat = k["@name"];
        logger.debug("Found advancement category: ", cat, ". Now iterating advancements...");
        for (auto& manif_adv : k["advancement"])
        {
            /*
             *  ..., {
             *         "@id": "minecraft:adventure/two_birds_one_arrow",
             *         "@name": "Two Birds, One Arrow",
             *         "@short_name": "Two Birds",
             *         "@type": "challenge",
             *         "@half": "false"
             *       },
             */
            auto id /* slug */ = manifest::parse_advancement_id(manif_adv);
            id.full_id = manif_adv["@id"].template get<std::string>();
            std::string pretty_name  = manif_adv["@name"];
            std::string short_name   = get_or(manif_adv, "@short_name", pretty_name);
            logger.debug("Got ID: ", id.full_id, " (category: ", id.category,
                         ", icon name: ", id.icon, ") actual name: ", pretty_name,
                         " short name: ", short_name);
            // "minecraft:adventure/two_birds_one_arrow"
            // -> ["adventure/two_birds_one_arrow", "two_birds_one_arrow"]
            auto [itr, success] =
                ret.advancements.emplace(id.full_id, Advancement::from_slug(id.full_id));
            if (!success)
            {
                logger.warning("Failed to insert advancement: ", id.full_id);
            }
            auto& adv       = itr->second;
            adv.pretty_name = std::move(pretty_name);
            adv.short_name  = std::move(short_name);

            // Load all criteria.
            if (manif_adv.contains("criteria"))
            {
                if (not manif_adv["criteria"].contains("criterion"))
                {
                    logger.fatal_error(adv.slug,
                                       ": Failed to load criteria: key 'criterion' missing.");
                }

                // Load criteria.
                logger.debug("Loading criteria for ", adv.name);
                for (auto& c : manif_adv["criteria"]["criterion"])
                {
                    // Try to load the criteria. The right way.
                    const auto crit = manifest::unprefixed_id(c);
                    const std::string icon =
                        c.contains("@icon") ? c["@icon"].template get<std::string>() : crit;
                    logger.debug("Adding criterion: ", crit);
                    /*
                    if (not rm.criteria_map.contains(icon))
                    {
                        if (not rm.criteria_map.contains(crit))
                        {
                            logger.fatal_error("Could not find criteria texture for: ", icon,
                                               " (In advancement: ", adv.slug, ")");
                        }
                        adv.add_criteria(crit, icon);
                        continue;
                    }
                    */
                    adv.add_criteria(crit, icon);
                }
            }

            // Now all that is missing is our own icon.

            // Load EXPLICIT icon.
            if (const auto explicit_icon = manifest::get_icon(manif_adv); explicit_icon != "")
            {
                if (not assets.contains(explicit_icon))
                {
                    logger.fatal_error("Could not locate icon: ", explicit_icon, " (for ",
                                       adv.slug, ")");
                }

                adv.icon = rm.store_texture_at(assets[explicit_icon]);
                logger.debug("Loaded explicit icon for ", adv.name,
                             " from file: ", assets[explicit_icon]);

                continue;
            }

            // Load IMPLICIT icon.
            if (assets.contains(adv.name))
            {
                adv.icon = rm.store_texture_at(assets[adv.name]);
                logger.debug("Loaded implicit icon for ", adv.name,
                             " from file: ", assets[adv.name]);

                continue;
            }

            // Error out if we don't get it.
            logger.fatal_error("Could not load asset for name: ", adv.name);
        }
    }

    logger.info("Loaded ", ret.advancements.size(), " advancements from: ", filename);

    return ret;
}

void AllAdvancements::update_from_file(std::string_view filename)
{
    auto& logger = get_logger("AllAdvancements::update_from_file");
    logger.debug("Loading advancements from file: ", filename);

    std::ifstream f(filename.data() /* msvc */);
    if (!f.good())
    {
        logger.error("Could not open file: ", filename);
        return;
    }

    auto advancements_json = json{};
    try
    {
        advancements_json = json::parse(f);
    }
    catch (std::exception& e)
    {
        logger.error("Failed to parse JSON: ", e.what());
        return;
    }

    // Copy :sob:
    // Kind of have to do this. Because we are 'removing' things,
    // not 'adding' them. Oh well.
    // ret.incomplete = manifest.advancements;
    // Leaving the above here as a reminder. Things are so much better now.
    // :)
    for (auto& [key, adv] : advancements)
    {
        adv.reset_for_load();
    }

    const std::string_view adv_prefix = "minecraft:";
    for (const auto& [key, value] : advancements_json.items())
    {
        if (key.starts_with("minecraft:recipes/") or not key.starts_with(adv_prefix)) continue;

        logger.debug("Found advancement: ", key);
        // this is probably definitely totally an advancement :)
        if (not advancements.contains(key))
        {
            logger.error("We don't have the advancement: ", key, " - hot-loading it.");
            advancements.emplace(key, Advancement::from_slug(key, Advancement::Source::Found));
        }

        const auto is_completed = value.contains("done") && value["done"].template get<bool>();
        Advancement& adv        = advancements.find(key)->second;

        if (is_completed)
        {
            adv.mark_completed();
            continue;
        }

        if (value.contains("criteria"))
        {
            // Wow, this is also so much better with this system.
            // We literally just mark criteria as completed. Easy peasy.
            // Let's not 'find' criteria? Sounds awkward.
            // Oh, but we kind of have to, I guess.
            // No, but we don't have to... yeah.
            for (const auto& [k, v] : value["criteria"].items())
            {
                if (not adv.criteria.contains(k))
                {
                    // This is fine actually. We will get random
                    // criteria from parsing that aren't 'real'
                    // criteria. If they're not in manifest,
                    // just ignore them. Not real! Fake! !!!!!
                    // lol..? ^ what is this referring to...
                    continue;
                }
                adv.criteria.find(k)->second.achieved = true;
            }
        }
    }
}
} // namespace aa
