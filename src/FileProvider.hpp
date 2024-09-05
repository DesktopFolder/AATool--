#pragma once

#include "dmon.hpp"

#include <unordered_map>
#include <vector>
#include <string>
#include <filesystem>

namespace aa
{
struct Logger;

enum class DetectionMode {
    Automatic,
    Explicit,
};

struct CurrentFileProvider
{
    // FileProvider is an abstraction that allows us to swap out/improve how
    // and when we decide to poll for AA json files.
    // For now, we are only thinking about 1 player worlds.
    CurrentFileProvider();

    // What does this actually return?
    // Ah, it is the latest advancement file json path.
    std::optional<std::string> poll(uint64_t ticks = 0);

    void debug();

    bool has_active_watch() const noexcept
    {
        //return not active_watch.empty();
        return active_watch.is_active();
    }

    // Note: It's very easy to modify these watchers...
    // Please be careful with this API.
    // ONLY call if there is an active watch.
    dmon::Watch& get_active_watch()
    {
        /*
        if (not has_active_watch())
        {
            logger->fatal_error("Active watch does not exist.");
        }
        return watches.at(active_watch);
        */
        return active_watch;
    }

private:
    Logger* logger;

    std::vector<std::string> instances;
    DetectionMode mode = DetectionMode::Automatic;

    dmon::Watch active_watch;

    // e.g. instances/XYZ
    std::string active_instance;
    // e.g. XYZ/saves/world1/advancements
    std::string active_advancement_dir;
    // e.g. XYZ/saves/world1/advancements/uuid.json
    std::string active_advancements;
    std::filesystem::file_time_type last_modified;

    // Should these defaults all be in like, DEFAULTS.hpp
    // so they can be properly documented/referred to?
    // e.g. /* poll_interval */ DEFAULT_POLL_INTERVAL
    uint64_t poll_interval = 60;
};
}  // namespace aa
