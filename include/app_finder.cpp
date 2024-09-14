#include "app_finder.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <Windows.h>

// NO REORDER THIS WINDOWS H MUST COME FIRST IT IS THE KING OF FILES LOL
#include <WinUser.h>
#include <ntstatus.h>
#include <winternl.h>
#endif

#include "logging.hpp"

#include <unordered_map>
#include <unordered_set>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
// Windows specific code for getting the current application details.
#include <codecvt>
#include <locale>

ApplicationInfo aa::get_focused_application()
{
    auto hWnd = GetForegroundWindow();
    wchar_t buf[12]{};
    // auto res   = GetWindowTextW();
    DWORD pid   = 0;
    std::ignore = GetWindowThreadProcessId(hWnd, &pid);
    ApplicationInfo ret{};
    ret.name = "UNIMPLEMENTED";
    ret.exec = "UNIMPLEMENTED";
    ret.pid  = pid;
    return ret;
}

std::optional<std::wstring> GetProcCommandLine(DWORD pid)
{
    const auto& logger = aa::get_logger("GetProcCommandLine");

    // https://stackoverflow.com/questions/75607614/how-to-get-the-command-line-arguments-from-a-given-process-pid
    // thanks to answer by remy lebeau
    std::wstring commandLine;

    // Open a handle to the process
    // TODO - WHY is this not a SCOPE_EXIT???
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess == NULL)
    {
        logger.debug("Could not call OpenProcess.");
        return {};
    }
    // Get the size of the command line arguments from the process
    ULONG size = 0;
    auto ProcessCommandLineInformation =
        static_cast<PROCESSINFOCLASS>(60); /* MAYBE YOU NEED TO INCLUDE FROM htdll?? */
    NTSTATUS status =
        NtQueryInformationProcess(hProcess, ProcessCommandLineInformation, NULL, 0, &size);
    /* Why do we check this?
     * Very simple. This 2-step query process is the method by which we get the 'current size'.
     * The first NtQueryInformationProcess here is EXPECTED to fail, because we pass it 0/nullptr
     * to store its data in. It gives us an EXPECTED length mismatch, but also gives us the data
     * for the size we need to preallocate.
     */
    if (status == STATUS_INFO_LENGTH_MISMATCH)
    {
        // Get the command line arguments from the process
        std::vector<BYTE> buffer;
        buffer.reserve(size);
        status = NtQueryInformationProcess(hProcess, ProcessCommandLineInformation, buffer.data(),
                                           size, NULL);
        if (status == STATUS_SUCCESS)
        {
            PUNICODE_STRING commandLineArgs = reinterpret_cast<PUNICODE_STRING>(buffer.data());
            commandLine.assign(commandLineArgs->Buffer, commandLineArgs->Length / sizeof(WCHAR));
            CloseHandle(hProcess);
            return commandLine;
        }
        else
        {
            logger.debug("Main NtQueryInformationProcess query did not succeed.");
        }
    }
    else
    {
        // https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
        logger.debug("NtQueryInformationProcess(", hProcess, ", ", ProcessCommandLineInformation,
                     "...) failed: ", status, " (size set to: ", size, "): ", std::hex, status,
                     std::dec);
    }

    CloseHandle(hProcess);

    return {};
}

std::optional<std::string> aa::get_focused_minecraft()
{
    const auto& logger = get_logger("get_focused_minecraft");

    // get hard W indicator, which points to the window ig
    auto hWnd = GetForegroundWindow();
    // create a buffer and store window title in it :)
    // this are W chars so just, like, chars but more W
    wchar_t buf[12]{};
    auto res = GetWindowTextW(hWnd, buf, 11);
    if (res == 0)
    {
        return {};
    }
    std::wstring wintext{buf};
    logger.debug("Found a window with title: ",
                 std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wintext));

    // ok!
    if (not wintext.starts_with(L"Minecraft"))
    {
        // what an L, you're not playing minecraft
        return {};
    }
    logger.debug("Found a Minecraft instance.");

    // get the pid. if you don't know what a pid is, consider
    // taking an operating systems course :) it's useful
    DWORD pid   = 0;
    std::ignore = GetWindowThreadProcessId(hWnd, &pid);

    // Cache the results because realistically speaking, that's fine.
    static std::unordered_map<int, std::optional<std::string>> cache;
    if (cache.contains(pid))
    {
        return cache[pid];
    }

    auto cli = GetProcCommandLine(pid);
    if (not cli.has_value())
    {
        return {};
    }
    std::string str = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(*cli);

    logger.debug("Found process arguments of size: ", str.size());
    std::string_view haystack = str;
    const auto pos            = haystack.find(aa::MMC_LIB_PATH_PREFIX);
    if (pos == std::string_view::npos)
    {
        logger.debug("Could not find Java library path argument in process arguments.");
        return {};
    }

    std::string_view sv = haystack.substr(pos);
    // Handle instances with space in their path
    if (pos != 0 && haystack[pos] - 1 == '"')
    {
        // Let's not worry about instances with another quotation mark.
        sv = sv.substr(0, sv.find('"'));
    }
    else
    {
        // Consume until a space character.
        sv = sv.substr(0, sv.find(' '));
    }

    logger.debug("Found library path: discovering instance.");
    std::string_view inst_str = "/instances/";
    size_t inst_pos           = sv.rfind(inst_str);
    if (inst_pos == std::string_view::npos)
    {
        logger.warning("Could not find instances path: no /instances/.");
        cache.emplace(pid, std::nullopt);
        return {};
    }

    size_t end = sv.find('/', inst_pos + inst_str.size());

    auto result =
        std::string{sv.substr(MMC_LIB_PATH_PREFIX.size(), end - MMC_LIB_PATH_PREFIX.size())};
    cache.insert_or_assign(pid, result);
    return result;
}
#elif __APPLE__
// Apple specific code for getting the current application details.
#include "osx/FocusedApplicationFinder.hpp"

#include <cassert>
#include <sys/sysctl.h>

ApplicationInfo aa::get_focused_application() { return osx::get_current_application(); }

std::optional<std::string> aa::get_focused_minecraft()
{
    const auto& logger = get_logger("get_focused_minecraft");

    const auto s = get_focused_application();
    if (not s.exec.starts_with("java"))
    {
        static std::unordered_set<std::string> ignored;
        if (ignored.emplace(s.exec).second)
        {
            // Info log this - we only log once, so.
            // Theoretically very helpful for debug dumps.
            logger.info("Ignored application: ", s.exec);
        }
        return {};
    }
    const int pid = s.pid;

    // Note - something about this function is SUPER laggy, causing us
    // to skip around a little when we're running it 1x/60frames.
    // Cache the results because realistically speaking, that's fine.
    static std::unordered_map<int, std::optional<std::string>> cache;
    if (cache.contains(pid))
    {
        return cache[pid];
    }

    logger.debug("Got minecraft with PID: ", pid);

    // https://gist.github.com/nonowarn/770696
    // WTF?

    int query_buf[3]{}; // Our queries live here.

    query_buf[0] = CTL_KERN;
    query_buf[1] = KERN_ARGMAX;

    int argmax     = 0;
    size_t argsize = sizeof(argmax);
    if (sysctl(query_buf, 2, &argmax, &argsize, NULL, 0) == -1)
    {
        logger.debug("Failed SYSCTL to acquire argument size: ", s.exec);
        return {};
    }

    // Allocate space for the arguments.
    std::vector<char> process_arguments;
    process_arguments.resize(argmax);

    // Set up our query buffer for the PROCARGS2 query.
    query_buf[0] = CTL_KERN;
    query_buf[1] = KERN_PROCARGS2;
    query_buf[2] = pid;

    argsize = (size_t)argmax;
    if (sysctl(query_buf, 3, process_arguments.data(), &argsize, NULL, 0) == -1)
    {
        logger.error("Failed SYSCTL to acquire process arguments: ", s.exec);
        return {};
    }

    // I THINK. THINK. That this is prefixed argc data.
    // Makes the most sense if you think that we are basically
    // injecting ourselves into "main(int argc, char** argv)",
    // and more or less just reading the memory present there.
    int argc = 0;
    memcpy(&argc, process_arguments.data(), sizeof(argc));

    size_t itr = sizeof(argc);

    /* Skip the saved exec_path. */
    assert(argsize <= process_arguments.size());
    for (; itr < argsize; itr++)
    {
        if (process_arguments[itr] == '\0')
        {
            break;
        }
    }

    /* Skip trailing '\0' characters. */
    for (; itr < argsize; itr++)
    {
        if (process_arguments[itr] != '\0')
        {
            /* Beginning of first argument reached. */
            break;
        }
    }

    if (itr == argsize)
    {
        logger.error("SYSCTL did not provide process arguments: ", s.exec);
        return {};
    }

    const size_t argument_start = itr;

    // DEBUG (get_focused_minecraft):
    // Got arguments: '/opt/homebrew/opt/openjdk/bin/java -Xdock:icon=icon.png
    // -Xdock:name="Prism Launcher: AA Tracker Testing" -XstartOnFirstThread
    // -Xms512m -Xmx4096m -Duser.language=en
    // -Djava.library.path=/Users/user/Library/Application Support/PrismLauncher/instances/AA
    // Tracker Testing/natives -cp
    // /private/var/folders/l9/xlt__xp92pbg92834dhv38r40000gn/T/AppTranslocation/927FB467-2D15-47ED-8346-
    int arg_counter       = 0;
    size_t start_location = argument_start;
    for (; arg_counter < argc && itr < argsize; itr++)
    {
        // itr >= start_location
        if (process_arguments[itr] == '\0')
        {
            // Increment argument counter.
            arg_counter++;

            // 'Understand' this argument.
            // argument is start_location -> itr
            const char* d = process_arguments.data();
            std::string_view sv(d + start_location);
            logger.debug("Found argument: ", sv, " of size: ", sv.size());
            std::string_view prefix = "-Djava.library.path=";
            if (sv.starts_with(prefix))
            {
                logger.debug("Found library path: discovering instance.");
                std::string_view inst_str = "/instances/";
                size_t pos                = sv.rfind(inst_str);
                if (pos == std::string_view::npos)
                {
                    logger.warning("Could not find instances path: no /instances/.");
                    cache.emplace(pid, std::nullopt);
                    return {};
                }

                size_t end = sv.find('/', pos + inst_str.size());

                auto result = std::string{sv.substr(prefix.size(), end - prefix.size())};
                cache.insert_or_assign(pid, result);
                return result;
            }

            // Start location is the NEXT character.
            start_location = itr + 1;
        }
    }

    /*
    // EXAMPLE CODE - SIMPLE
    int arg_counter = 0;
    size_t last_i   = 0;
    for (; arg_counter < argc && itr < argsize; itr++)
    {
        if (process_arguments[itr] == '\0')
        {
            arg_counter++;
            if (last_i != 0)
            {
                // Overwrite last \0 to ' '
                process_arguments[last_i] = ' ';
            }
            // Previous nullptr, overwrite to ' ' if we find another.
            last_i = itr;
        }
    }

     // sp points to the beginning of the arguments/environment string, and
     // np should point to the '\0' terminator for the string.
    if (last_i == 0)
    {
        // Empty or unterminated string.
        logger.error("Unterminated string? ", s.exec);
        return {};
    }

    std::string str = process_arguments.data() + start_location;
    logger.debug("Got arguments: '", str, "'")
        .debug("With argsize: ", argsize)
        .debug("And initial location: ", start_location)
        .debug("And final location: ", itr)
        .debug("And argc: ", argc);
    */

    cache.emplace(pid, std::nullopt);
    return {};
}

#elif __linux__
// Linux specific code for getting the current application details.
ApplicationInfo aa::get_focused_application() { 
    ApplicationInfo ret{};
    ret.name = "UNIMPLEMENTED";
    ret.exec = "UNIMPLEMENTED";
    ret.pid  = 0;
    return ret;
}

std::optional<std::string> aa::get_focused_minecraft()
{
    const auto& logger = get_logger("get_focused_minecraft");

    const auto s = get_focused_application();
    return {};
}
#else
#error "Unsupported OS? D:"
#endif
