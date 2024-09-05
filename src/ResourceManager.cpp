#include "ResourceManager.hpp"

#include "logging.hpp"
#include "ConfigProvider.hpp"

#include <SFML/Graphics.hpp>
#include <filesystem>

namespace aa
{
aa::ResourceManager& ResourceManager::instance()
{
    static aa::ResourceManager mgr{};
    return mgr;
}

sf::Drawable& ResourceManager::basic_marker(float radius, float x, float y)
{
    static sf::CircleShape shape{};

    shape.setRadius(radius);
    shape.setFillColor(sf::Color::Cyan);
    shape.setPosition(x, y);

    return shape;
}

void asset_helper(auto& dir_entry, auto& assets)
{
    if (!dir_entry.is_regular_file()) return;
    const auto p = dir_entry.path();
    const auto s = p.string();
    if (not(s.ends_with(".png") or s.ends_with(".gif"))) return;
    if (assets.contains(aa::ResourceManager::assetName(s)))
    {
        // we have this asset loaded already
        // continue unless this is HIGH RESOLUTION OOOO
        if (s.size() < 7 || s[s.size() - 7] != '^') return;
        auto& logger = get_logger("ResourceManager");
        // okay that doesn't actually work properly lol.
        // we always just want the highest resolution asset.
        // if we make it 'here', we know that s[s.size() - 3] == '^'.
        // We can kind of hack together how this works. Quite easily.
        logger.debug("Got high quality image: ", s);
        const auto& current = assets[aa::ResourceManager::assetName(s)];
        logger.debug("Current image is: ", current);
        if (current[current.size() - 6] > s[s.size() - 6])
        {
            logger.debug("Decided to keep the current image.");
            return;
        }
        logger.debug("Decided to overwrite with the new image.");
    }
    assets.insert_or_assign(aa::ResourceManager::assetName(s), s);
}

std::unordered_map<std::string, std::string> ResourceManager::getAllAssets()
{
    namespace fs = std::filesystem;

    static std::unordered_map<std::string, std::string> assets = []()
    {
        std::unordered_map<std::string, std::string> result;
        for (const fs::directory_entry& dir_entry :
             fs::recursive_directory_iterator("az/custom/"))
        {
            asset_helper(dir_entry, result);
        }
        for (const fs::directory_entry& dir_entry :
             fs::recursive_directory_iterator("az/ctm/sprites/global/"))
        {
            asset_helper(dir_entry, result);
        }
        for (const fs::directory_entry& dir_entry :
             fs::recursive_directory_iterator("az/ctm/sprites/gif/"))
        {
            asset_helper(dir_entry, result);
        }

        return result;
    }();

    return assets;
}

const sf::Font& ResourceManager::get_font()
{
    static const sf::Font font = []()
    {
        sf::Font font;
        font.loadFromFile("az/fonts/minecraft.otf");
        font.setSmooth(aa::conf::get_or(aa::conf::get(), "antialiasing", false));
        return font;
    }();
    return font;
}

const sf::Texture* ResourceManager::remap_texture(const sf::Texture* base, uint64_t new_size)
{
    const auto [x, y] = base->getSize();
    if (x == new_size)
    {
        // No need to remap here.
        return base;
    }
    sf::Sprite s;
    // Say, 32x32
    float initial = static_cast<float>(x);
    // Say, 48x48
    float desired = static_cast<float>(new_size);
    // Say, 48 / 32 = 1.5
    float factor = desired / initial;
    s.setScale(factor, factor);
    s.setTexture(*base);

    random_rerenders.emplace_back(std::make_unique<sf::RenderTexture>());
    auto& t = *random_rerenders.back().get();
    if (not t.create(new_size, new_size))
    {
        // This will likely crash us, but I mean. Um. It's a problem.
        get_logger("ResourceManager")
            .error("Failed to create a render texture of size ", new_size, "!!!");
        return nullptr;
    }
    t.draw(s);
    t.display();

    return &t.getTexture();
}

ResourceManager::~ResourceManager() {}

void ResourceManager::loadCriteria(std::string path, std::string name)
{
    AA_ASSERT(std::filesystem::exists(path));
    auto t = std::make_unique<sf::Texture>();
    t->loadFromFile(path);
    criteria_map.emplace(name, std::move(t));
    get_logger("ResourceManager")
        .info("Loaded criteria: ", path, " as name: ", name);
}

void ResourceManager::loadAllCriteria()
{
    // loadCriteria("az/ctm/sprites/global/criteria/animals/axolotl.png", "axolotl");
    loadCriteria("az/ctm/sprites/global/criteria/animals/bee.png", "bee");
    // loadCriteria("az/ctm/sprites/global/criteria/animals/camel.png", "camel");
    loadCriteria("az/ctm/sprites/global/criteria/animals/cat.png", "cat");
    loadCriteria("az/ctm/sprites/global/criteria/animals/chicken.png", "chicken");
    loadCriteria("az/ctm/sprites/global/criteria/animals/cow.png", "cow");
    loadCriteria("az/ctm/sprites/global/criteria/animals/donkey.png", "donkey");
    loadCriteria("az/ctm/sprites/global/criteria/animals/fox.png", "fox");
    // loadCriteria("az/ctm/sprites/global/criteria/animals/frog.png", "frog");
    // loadCriteria("az/ctm/sprites/global/criteria/animals/goat.png", "goat");
    loadCriteria("az/ctm/sprites/global/criteria/animals/horse.png", "horse");
    loadCriteria("az/ctm/sprites/global/criteria/animals/llama.png", "llama");
    loadCriteria("az/ctm/sprites/global/criteria/animals/mooshroom.png", "mooshroom");
    loadCriteria("az/ctm/sprites/global/criteria/animals/mule.png", "mule");
    loadCriteria("az/ctm/sprites/global/criteria/animals/ocelot.png", "ocelot");
    loadCriteria("az/ctm/sprites/global/criteria/animals/panda.png", "panda");
    loadCriteria("az/ctm/sprites/global/criteria/animals/pig.png", "pig");
    loadCriteria("az/ctm/sprites/global/criteria/animals/polar_bear.png", "polar_bear");
    loadCriteria("az/ctm/sprites/global/criteria/animals/rabbit.png", "rabbit");
    loadCriteria("az/ctm/sprites/global/criteria/animals/sheep.png", "sheep");
    loadCriteria("az/ctm/sprites/global/criteria/animals/sniffer.png", "sniffer");
    loadCriteria("az/ctm/sprites/global/criteria/animals/strider.png", "strider");
    loadCriteria("az/ctm/sprites/global/criteria/animals/turtle.png", "turtle");
    loadCriteria("az/ctm/sprites/global/criteria/animals/wolf.png", "wolf");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/badlands_plateau.png", "badlands_plateau");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/badlands.png", "badlands");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/bamboo_jungle_hills.png",
                 "bamboo_jungle_hills");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/bamboo_jungle.png", "bamboo_jungle");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/basalt_deltas.png", "basalt_deltas");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/beach.png", "beach");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/birch_forest_hills.png",
                 "birch_forest_hills");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/birch_forest.png", "birch_forest");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/cold_ocean.png", "cold_ocean");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/crimson_forest.png", "crimson_forest");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/dark_forest.png", "dark_forest");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/deep_cold_ocean.png", "deep_cold_ocean");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/deep_frozen_ocean.png",
                 "deep_frozen_ocean");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/deep_lukewarm_ocean.png",
                 "deep_lukewarm_ocean");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/deep_ocean.png", "deep_ocean");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/desert_hills.png", "desert_hills");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/desert_lakes.png", "desert_lakes");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/desert.png", "desert");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/forest_hills.png", "forest_hills");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/forest.png", "forest");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/frozen_river.png", "frozen_river");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/giant_tree_taiga_hills.png",
                 "giant_tree_taiga_hills");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/giant_tree_taiga.png", "giant_tree_taiga");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/jungle_edge.png", "jungle_edge");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/jungle_hills.png", "jungle_hills");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/jungle.png", "jungle");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/lukewarm_ocean.png", "lukewarm_ocean");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/mountains.png", "mountains");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/mushroom_field_shore.png",
                 "mushroom_field_shore");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/mushroom_fields.png", "mushroom_fields");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/nether_wastes.png", "nether_wastes");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/ocean.png", "ocean");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/plains.png", "plains");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/river.png", "river");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/savanna_plateau.png", "savanna_plateau");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/savanna.png", "savanna");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/snowy_beach.png", "snowy_beach");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/snowy_mountains.png", "snowy_mountains");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/snowy_taiga_hills.png",
                 "snowy_taiga_hills");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/snowy_taiga.png", "snowy_taiga");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/snowy_tundra.png", "snowy_tundra");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/soul_sand_valley.png", "soul_sand_valley");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/stone_shore.png", "stone_shore");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/swamp.png", "swamp");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/taiga_hills.png", "taiga_hills");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/taiga.png", "taiga");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/warm_ocean.png", "warm_ocean");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/warped_forest.png", "warped_forest");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/wooded_badlands_plateau.png",
                 "wooded_badlands_plateau");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/wooded_hills.png", "wooded_hills");
    loadCriteria("az/ctm/sprites/global/criteria/biomes/wooded_mountains.png", "wooded_mountains");
    // Why Minecraft. Why.
    // TODO - we can actually fix this. CTM's asset map properly maps these assets in @icon. For
    // now, whatever, this system works. Later we should do better asset management...
    loadCriteria("az/ctm/sprites/global/criteria/cats/black.png",
                 "black");
    loadCriteria("az/ctm/sprites/global/criteria/cats/british_shorthair.png",
                 "british_shorthair");
    loadCriteria("az/ctm/sprites/global/criteria/cats/calico.png",
                 "calico");
    loadCriteria("az/ctm/sprites/global/criteria/cats/jellie.png",
                 "jellie");
    // loadCriteria("az/ctm/sprites/global/criteria/cats/ocelot.png",
    // "ocelot");
    loadCriteria("az/ctm/sprites/global/criteria/cats/persian.png",
                 "persian");
    loadCriteria("az/ctm/sprites/global/criteria/cats/ragdoll.png",
                 "ragdoll");
    loadCriteria("az/ctm/sprites/global/criteria/cats/red.png", "red");
    loadCriteria("az/ctm/sprites/global/criteria/cats/siamese.png",
                 "siamese");
    loadCriteria("az/ctm/sprites/global/criteria/cats/tabby.png", "tabby");
    loadCriteria("az/ctm/sprites/global/criteria/cats/tuxedo.png",
                 "tuxedo");
    loadCriteria("az/ctm/sprites/global/criteria/cats/white.png", "white");
    loadCriteria("az/ctm/sprites/global/criteria/food/apple.png", "apple");
    loadCriteria("az/ctm/sprites/global/criteria/food/baked_potato.png", "baked_potato");
    loadCriteria("az/ctm/sprites/global/criteria/food/beef.png", "beef");
    loadCriteria("az/ctm/sprites/global/criteria/food/beetroot.png", "beetroot");
    loadCriteria("az/ctm/sprites/global/criteria/food/beetroot_soup.png", "beetroot_soup");
    loadCriteria("az/ctm/sprites/global/criteria/food/bread.png", "bread");
    loadCriteria("az/ctm/sprites/global/criteria/food/carrot.png", "carrot");
    loadCriteria("az/ctm/sprites/global/criteria/food/chorus_fruit.png", "chorus_fruit");
    loadCriteria("az/ctm/sprites/global/criteria/food/cod.png", "cod");
    loadCriteria("az/ctm/sprites/global/criteria/food/cooked_beef.png", "cooked_beef");
    loadCriteria("az/ctm/sprites/global/criteria/food/cooked_chicken.png", "cooked_chicken");
    loadCriteria("az/ctm/sprites/global/criteria/food/cooked_cod.png", "cooked_cod");
    loadCriteria("az/ctm/sprites/global/criteria/food/cooked_mutton.png", "cooked_mutton");
    loadCriteria("az/ctm/sprites/global/criteria/food/cooked_porkchop.png", "cooked_porkchop");
    loadCriteria("az/ctm/sprites/global/criteria/food/cooked_rabbit.png", "cooked_rabbit");
    loadCriteria("az/ctm/sprites/global/criteria/food/cooked_salmon.png", "cooked_salmon");
    loadCriteria("az/ctm/sprites/global/criteria/food/cookie.png", "cookie");
    loadCriteria("az/ctm/sprites/global/criteria/food/dried_kelp.png", "dried_kelp");
    loadCriteria("az/custom/enchanted_golden_apple.png", "enchanted_golden_apple");
    // loadCriteria("az/ctm/sprites/global/criteria/food/glow_berries.png", "glow_berries");
    loadCriteria("az/ctm/sprites/global/criteria/food/golden_apple.png", "golden_apple");
    loadCriteria("az/ctm/sprites/global/criteria/food/golden_carrot.png", "golden_carrot");
    loadCriteria("az/ctm/sprites/global/criteria/food/honey_bottle.png", "honey_bottle");
    loadCriteria("az/ctm/sprites/global/criteria/food/melon_slice.png", "melon_slice");
    loadCriteria("az/ctm/sprites/global/criteria/food/mushroom_stew.png", "mushroom_stew");
    loadCriteria("az/ctm/sprites/global/criteria/food/mutton.png", "mutton");
    loadCriteria("az/ctm/sprites/global/criteria/food/poisonous_potato.png", "poisonous_potato");
    loadCriteria("az/ctm/sprites/global/criteria/food/porkchop.png", "porkchop");
    loadCriteria("az/ctm/sprites/global/criteria/food/potato.png", "potato");
    loadCriteria("az/ctm/sprites/global/criteria/food/pufferfish.png", "pufferfish");
    loadCriteria("az/ctm/sprites/global/criteria/food/pumpkin_pie.png", "pumpkin_pie");
    loadCriteria("az/ctm/sprites/global/criteria/food/rabbit_stew.png", "rabbit_stew");
    loadCriteria("az/ctm/sprites/global/criteria/food/raw_chicken.png", "raw_chicken");
    loadCriteria("az/ctm/sprites/global/criteria/food/raw_rabbit.png", "raw_rabbit");
    loadCriteria("az/ctm/sprites/global/criteria/food/rotten_flesh.png", "rotten_flesh");
    loadCriteria("az/ctm/sprites/global/criteria/food/salmon.png", "salmon");
    loadCriteria("az/ctm/sprites/global/criteria/food/spider_eye.png", "spider_eye");
    loadCriteria("az/ctm/sprites/global/criteria/food/suspicious_stew.png", "suspicious_stew");
    loadCriteria("az/ctm/sprites/global/criteria/food/sweet_berries.png", "sweet_berries");
    loadCriteria("az/ctm/sprites/global/criteria/food/tropical_fish.png", "tropical_fish");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/blaze.png", "blaze");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/cave_spider.png", "cave_spider");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/creeper.png", "creeper");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/drowned.png", "drowned");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/elder_guardian.png", "elder_guardian");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/ender_dragon.png", "ender_dragon");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/enderman.png", "enderman");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/endermite.png", "endermite");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/evoker.png", "evoker");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/ghast.png", "ghast");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/guardian.png", "guardian");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/hoglin.png", "hoglin");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/husk.png", "husk");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/magma_cube.png", "magma_cube");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/phantom.png", "phantom");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/piglin_brute.png", "piglin_brute");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/piglin.png", "piglin");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/pillager.png", "pillager");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/ravager.png", "ravager");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/shulker.png", "shulker");
    // loadCriteria("az/ctm/sprites/global/criteria/mobs/silverfish^16.png", "silverfish^16");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/silverfish^48.png", "silverfish");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/skeleton.png", "skeleton");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/slime.png", "slime");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/spider.png", "spider");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/stray.png", "stray");
    // loadCriteria("az/ctm/sprites/global/criteria/mobs/vex_1.19.3.png", "vex_1");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/vex.png", "vex");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/vindicator.png", "vindicator");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/witch.png", "witch");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/wither^16.png", "wither^16");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/wither^32.png", "wither^32");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/wither^48.png", "wither");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/wither_skeleton.png", "wither_skeleton");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/zoglin.png", "zoglin");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/zombie_pigman.png", "zombie_pigman");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/zombie.png", "zombie");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/zombie_villager.png", "zombie_villager");
    loadCriteria("az/ctm/sprites/global/criteria/mobs/zombified_piglin.png", "zombified_piglin");
}
} // namespace aa
