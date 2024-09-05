#pragma once

#include <cstdint>
#include <filesystem>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

namespace aa
{
enum WatchStatus
{
    ALWAYS_WATCH = 1,
    NEVER_WATCH  = 2,
    AUTO_WATCH   = 3
};

struct MinecraftInstance
{
    MinecraftInstance(std::string s, WatchStatus w) : saves(s), watched(w) {}

    std::string saves;
    WatchStatus watched = AUTO_WATCH;

    std::filesystem::file_time_type log_write{};

    std::string as_string() const { return fmt::format("{} {}", static_cast<uint8_t>(watched), saves); }

    static MinecraftInstance from_string(std::string_view str)
    {
        return MinecraftInstance{std::string{str.substr(2)},
                                 static_cast<WatchStatus>(str[0] - '0')};
    }
};

/** InstanceProvider
 * Provides a subsystem for detecting Minecraft instance(s).
 * Uses a variety of fallbacks.
 */
class InstanceProvider
{
public:
    InstanceProvider();

    void poll(uint64_t ticks);

    std::optional<MinecraftInstance> current_instance() const {
        if (sorted_instances.empty())
        {
            return {};
        }
        return sorted_instances.back();
    }

private:
    std::filesystem::path config_location;

    std::vector<MinecraftInstance> known_instances;
    std::vector<MinecraftInstance> sorted_instances;

    // Config (overall) + individual config brought out.
    nlohmann::json config;
    bool allow_autodetect = true;
};

struct MinecraftWorld
{
    std::filesystem::path path; 
    std::filesystem::file_time_type last_modified;

    std::filesystem::path get_advancements() const;
};

// Let's just put this in here for now.
class WorldProvider
{
public:
    bool poll(const MinecraftInstance& instance, uint64_t ticks);

    const MinecraftWorld& get_latest() { return latest; }

private:
    MinecraftWorld latest{};
};
} // namespace aa
