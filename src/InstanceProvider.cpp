#include "InstanceProvider.hpp"

#include "ConfigProvider.hpp"
#include "app_finder.hpp"
#include "filesystem_utilities.hpp"

aa::InstanceProvider::InstanceProvider()
{
    // Ensure we can access our cache/configuration.
    config = conf::getNS("instance");

    config_location =
        aa::as_valid_directory(conf::get_or<std::string>(config, "cache_location", "./"));

    known_instances =
        aa::read_lines(std::string{config_location / "instances.txt"},
                       [&](std::string_view s) { return MinecraftInstance::from_string(s); });

    allow_autodetect = conf::get_or<bool>(config, "allow_autodetect", true);
}

void aa::InstanceProvider::poll(uint64_t ticks)
{
    // Different offset. InstanceProvider does different things depending on time.
    if (ticks % 60 == 35)
    {
        if (known_instances.empty())
        {
            return;
        }
        // If we have known instances, let's figure out which one we should be providing.
        // Pretty simple. If we have explicit instances, we prioritize those.
        // Otherwise, we send the latest modified one's log files.
        if (sorted_instances.empty())
        {
            if (std::ranges::any_of(known_instances, [](const MinecraftInstance& i)
                                    { return i.watched == ALWAYS_WATCH; }))
            {
                // In this case, we only copy over the always watched instances.
                // I feel like I should be able to just do = | to_vector but nope! Oh well.
                // I guess that's C++ for you. Just, you know, the most obvious use case...
                for (auto mi :
                     known_instances | std::views::filter([](const MinecraftInstance& i)
                                                          { return i.watched == ALWAYS_WATCH; }))
                {
                    sorted_instances.emplace_back(std::move(mi));
                }
            }
            else
            {
                sorted_instances = known_instances;
            }
        }

        // Okay, we always have instances now. At this point we need to order them based on
        // the one that has most recent log file entries.
        // We assume the player only ever has (1) instance open. If there are multiple,
        // this assumption will likely cause issues here, although they can be fixed by just
        // using explicit instance paths, or ... eh, whatever. Lots of possibilities.
        // I just really don't want to poll for active window as much as I would need to, to be
        // truly safe with this kind of thing.
        for (auto& mi : sorted_instances)
        {
            std::error_code ec;
            mi.log_write = std::filesystem::last_write_time(saves_path_to_root(mi.saves), ec);
            if (ec)
            {
                mi.log_write = std::filesystem::file_time_type::max();
            }
        }

        std::sort(sorted_instances.begin(), sorted_instances.end(),
                  [](const MinecraftInstance& l, const MinecraftInstance& r)
                  { return l.log_write < r.log_write; });

        return;
    }

    if (not known_instances.empty() || ticks % 60 != 23)
    {
        return;
    }

    // Otherwise, we should find our instances.

    // How this works is pretty simple. If the user gives us a hardcoded instance path,
    // we use that. Otherwise, we wait until we autodetect an instance, and use that.
    // I've left space to expand this system later if the need arises. However, that
    // would likely require a level of UI integration that would be weird to bake in now.
    auto paths = conf::get_or<std::vector<std::string>>(config, "instance_paths", {});
    if (not paths.empty())
    {
        for (const auto& p : paths)
        {
            const auto ip = to_saves_path(p);
            if (not ip.has_value()) continue;
            known_instances.emplace_back(std::string{ip.value()}, ALWAYS_WATCH);
        }
    }

    if (allow_autodetect)
    {
        const auto fm = get_focused_minecraft();
        if (fm.has_value())
        {
            const auto ip = to_saves_path(fm.value());
            if (ip.has_value())
            {
                log::debug("InstanceProvider: Found saves path: ", *ip);
                known_instances.emplace_back(std::string{ip.value()}, AUTO_WATCH);
            }
        }
    }

    if (not known_instances.empty())
    {
        // Cool, we have some # of instances now. Cache this information for our next start?
        // Eh... actually, for now that just kind of seems like a bad idea. Let's not.
    }
}

std::filesystem::path aa::MinecraftWorld::get_advancements() const
{
    static std::string uuid{};

    if (uuid.empty())
    {
        const auto p = path / "advancements";
        for (auto& dir_entry : std::filesystem::directory_iterator{p})
        {
            const auto fn = std::string{dir_entry.path().filename()};
            if (fn.find(".json") == std::string::npos) continue;
            uuid = fn;
            break;
        }
    }

    // No advancements, error case I guess.
    if (uuid.empty())
    {
        return {};
    }

    return path / "advancements" / uuid;
}

bool aa::WorldProvider::poll(const MinecraftInstance& instance, uint64_t ticks)
{
    namespace fs = std::filesystem;
    if (ticks % 60 != 38)
    {
        return false;
    }

    if (not fs::is_directory(instance.saves))
    {
        log::error("Directory does not exist: ", instance.saves);
        return false;
    }

    fs::path saves_path{instance.saves};
    bool changed = false;

    for (auto& dir_entry : fs::directory_iterator{saves_path})
    {
        const auto new_time = fs::last_write_time(dir_entry.path());
        if (new_time > latest.last_modified)
        {
            latest.last_modified = new_time;
            latest.path          = std::move(dir_entry.path());
            changed              = true;
        }
    }

    return changed;
}
