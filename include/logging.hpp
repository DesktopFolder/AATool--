#pragma once

/**
 * logging.hpp
 *
 * An set of abstractions for logging necessities.
 * For now, everything goes in the header.
 */

#include "utilities.hpp"
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <memory>

namespace aa
{
// just for now, for safety reasons... lol
namespace detail
{
string_map<std::unique_ptr<std::ofstream>>& get_files();
}
std::ofstream* get_file(std::string_view name);

inline void set_default_file(std::string_view name)
{
    auto& files = detail::get_files();

    // aatool.log -> ofstream*
    // overlay.log -> ofstream*
    // __default__ (default.log) -> ofstream*
    // default.log -> nullptr

    if (files.contains("__default__"))
    {
        for (auto& [k, v] : files)
        {
            if (v.get() == nullptr)
            {
                files.find("__default__")->second.swap(v);
            }
        }
        files.erase("__default__");
    }
    files.emplace("__default__", nullptr);
    std::ignore = get_file(name);

    files.find(name)->second.swap(files.find("__default__")->second);
}

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error,
    None,
};

struct Logger
{
    // Not super configurable, but it works for now.
    mutable std::ostream* file{}; // WEAK ptr
    bool write_stdout;

    static bool stdout_default;
    static LogLevel level;

    Logger(std::string name)
        : name_(std::move(name)), str_debug_("DEBUG (" + name_ + "): "),
          str_warning_("WARNING (" + name_ + "): "), str_error_("ERROR (" + name_ + "): "),
          str_info_("INFO (" + name_ + "): "), write_stdout(stdout_default)
    {
    }

    [[maybe_unused]] static LogLevel set_level(std::string_view level);

    // Level 0 - Only logged when we are at the most verbose.
    // Anything that would hugely spam the logs should be under this.
    template <typename... Ts>
    [[maybe_unused]] const Logger& debug(Ts&&... ts) const
    {
        if (to_underlying(level) <= to_underlying(LogLevel::Debug))
        {
            write_endl(str_debug_, std::forward<Ts>(ts)...);
        }
        return *this;
    }

    template <typename... Ts>
    [[maybe_unused]] const Logger& info(Ts&&... ts) const
    {
        if (to_underlying(level) <= to_underlying(LogLevel::Info))
        {
            // Info = 1, Debug = 0.
            write_endl(str_info_, std::forward<Ts>(ts)...);
        }
        return *this;
    }

    template <typename... Ts>
    [[maybe_unused]] const Logger& warning(Ts&&... ts) const
    {
        if (to_underlying(level) <= to_underlying(LogLevel::Warning))
        {
            write_endl(str_warning_, std::forward<Ts>(ts)...);
        }
        return *this;
    }

    template <typename... Ts>
    [[maybe_unused]] const Logger& error(Ts&&... ts) const
    {
        if (to_underlying(level) <= to_underlying(LogLevel::Error))
        {
            write_endl(str_error_, std::forward<Ts>(ts)...);
        }
        return *this;
    }

    template <typename... Ts>
    void fatal_error(Ts&&... ts)
    {
        std::ostringstream ss;

        auto b  = write_stdout;
        auto* f = file;

        write_stdout = false;
        file         = &ss;
        error(std::forward<Ts>(ts)...);

        write_stdout = b;
        file         = f;
        error(std::forward<Ts>(ts)...);

        throw std::runtime_error(ss.str());
    }

private:
    template <typename T>
    void write(T&& t) const
    {
        if (write_stdout)
        {
            std::cout << t;
        }
        if (file)
        {
            *file << t;
        }
    }

    template <typename T, typename... Ts>
    void write(T&& t, Ts&&... ts) const
    {
        write(std::forward<T>(t));
        write(std::forward<Ts>(ts)...);
    }

    template <typename... Ts>
    void write_endl(Ts&&... ts) const
    {
        write(std::forward<Ts>(ts)...);
        // Doesn't work, not sure why, no google atm...
        // write(std::endl);
        if (write_stdout)
        {
            std::cout << std::endl;
        }
        if (file)
        {
            *file << std::endl;
        }
    }

private:
    std::string name_;
    std::string str_debug_;
    std::string str_info_;
    std::string str_warning_;
    std::string str_error_;
};

Logger& get_logger(std::string_view name);

namespace log
{
// Similar to Python - Default to root logger.
template <typename... Ts>
void debug(Ts&&... ts)
{
    get_logger("root").debug(std::forward<Ts>(ts)...);
}
template <typename... Ts>
void info(Ts&&... ts)
{
    get_logger("root").info(std::forward<Ts>(ts)...);
}
template <typename... Ts>
void warning(Ts&&... ts)
{
    get_logger("root").warning(std::forward<Ts>(ts)...);
}
template <typename... Ts>
void error(Ts&&... ts)
{
    get_logger("root").error(std::forward<Ts>(ts)...);
}
} // namespace log
} // namespace aa
