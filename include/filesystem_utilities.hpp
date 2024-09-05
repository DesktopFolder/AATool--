#pragma once

#include <filesystem>
#include <string_view>
#include <type_traits>

#include "logging.hpp"

namespace aa
{
inline std::filesystem::path as_valid_directory(std::string_view p)
{
    if (p.starts_with('~'))
    {
        aa::log::fatal_error(p, " is not a valid path. (Path cannot start with ~)");
    }
    std::filesystem::path path{p};
    if (not std::filesystem::is_directory(path))
    {
        aa::log::fatal_error(p, " is not a valid path. (Must be existing directory)");
    }
    return path;
}

template <typename F>
auto read_lines(std::string_view filename, F&& callback)
{
    using cb_t = decltype(callback(std::declval<const std::string&>()));

    std::ifstream input_file(filename);

    if constexpr (not std::is_same_v<cb_t, void>)
    {
        std::vector<cb_t> return_list{};
        if (input_file.fail()) return return_list;

        std::string line;
        while (std::getline(input_file, line))
        {
            return_list.emplace_back(callback(line));
        }
        return return_list;
    }
    else
    {
        if (input_file.fail()) return;
        std::string line;
        while (std::getline(input_file, line))
        {
            callback(line);
        }
        return;
    }
}

template <typename T, typename F>
auto write_lines(std::string_view filename, std::vector<T> ts, F&& f)
{
    std::ofstream output_file(filename, /*std::ios_base::app |*/ std::ios_base::out);
    for (const auto& t : ts)
    {
        output_file << f(t) << '\n';
    }
}

std::optional<std::filesystem::path> to_saves_path(std::string_view p)
{
    std::filesystem::path ip{p};
    if (not std::filesystem::exists(ip)) return {};
    if (not std::filesystem::is_directory(ip)) return {};

    if (p.find(".minecraft") == std::string::npos)
    {
        // Try to add .minecraft.
        ip /= ".minecraft";
        if (not std::filesystem::is_directory(ip)) return {};
    }

    // We should either have the form ".../.minecraft" or ".../saves".
    const auto fn = ip.filename();
    log::debug(fn, ip);
    if (fn == ".minecraft")
        ip /= "saves";
    else if (fn != "saves")
        return {}; // not a good sign!

    return ip;
}

std::filesystem::path saves_path_to_root(std::string_view p)
{
    std::filesystem::path sp{p};
    return sp.parent_path();
}
} // namespace aa
