#include "Application.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <filesystem>
#include <iostream>
#include <thread>

#include "app_finder.hpp"
#include "logging.hpp"

#include "ConfigProvider.hpp"
#include "FileProvider.hpp"
#include "InstanceProvider.hpp"
#include "Map.hpp"
#include "Overlay.hpp"
#include "ResourceManager.hpp"
#include "WindowManager.hpp"

#include "Advancements.hpp"
#include "TurnTable.hpp"

namespace aa
{
AppConfig Application::configure()
{
    auto& conf = aa::conf::get();
    if (const auto logfile = aa::conf::get_if<std::string>(conf, "log"); logfile.has_value())
    {
        set_default_file(logfile.value());
    }

    Logger::stdout_default = aa::conf::get_or(conf, "verbose", false);
    Logger::set_level(aa::conf::get_or<std::string>(conf, "log-level", "info"));

    return {aa::conf::get_or(conf, "loop-sleep", 1ull), aa::conf::get_or(conf, "vsync", false),
            aa::conf::get_or<std::string>(conf, "manifest", "advancements.json")};
}

void Application::run()
{
    // Todo: WTF is this design? Abstract out subapps and run with an event queue/etc
    // Fix window management system w/this also...

    // Todo: It would be really cool if we could do lazy/background loading
    // of assets here. Just launch off a std::thread and do window setup @ simul.
    auto manifest = AllAdvancements::from_file(conf.manifest);

    // Doesn't do anything, we're just creating bindings.
    auto& ovWindow   = aa::WindowManager::instance().get(aa::WindowID::Overlay);
    auto& mapWindow  = aa::WindowManager::instance().get(aa::WindowID::Map);
    auto& mainwindow = aa::WindowManager::instance().get(aa::WindowID::Main);
    ovWindow.setVerticalSyncEnabled(conf.vsync);

    aa::OverlayManager ov(manifest);
    aa::MapManager mapper{conf::getNS("map")};

    aa::InstanceProvider instance{};
    aa::WorldProvider world{};

    aa::CurrentFileProvider fp;
    auto& rm = aa::ResourceManager::instance();
    auto& wm = aa::WindowManager::instance();

    uint64_t ticks = 0;
    log::debug("Starting main window loop.");
    while (!wm.is_shutdown())
    {
        // Handle events. We may need to add more stuff in here later.
        // Keep all this jammed in here until making an Application class later.
        // Or until making this all data driven/scripted? Not sure, really.
        auto events = wm.handleEvents();
        for (auto& event : events)
        {
            if (event.type == sf::Event::KeyReleased)
            {
                if (event.key.code == sf::Keyboard::A)
                {
                    log::debug("Got the key A! :)");
                }
                else if (event.key.code == sf::Keyboard::B)
                {
                    log::debug("Parsing test advancements file (1) - testing/all-everything.json");
                    manifest.update_from_file("testing/all-everything.json");
                    ov.update_to(manifest);
                }
                else if (event.key.code == sf::Keyboard::C)
                {
                    log::debug("Parsing test advancements file (2) - testing/no-recipes.json");
                    manifest.update_from_file("testing/no-recipes.json");
                    ov.update_to(manifest);
                }
                else if (event.key.code == sf::Keyboard::D)
                {
                    log::debug("Parsing test advancements file (3) - testing/less.json");
                    manifest.update_from_file("testing/less.json");
                    ov.update_to(manifest);
                }
                else if (event.key.code == sf::Keyboard::E)
                {
                    log::debug("Parsing test advancements file (4) - testing/most-complete.json");
                    manifest.update_from_file("testing/most-complete.json");
                    ov.update_to(manifest);
                }
                else if (event.key.code == sf::Keyboard::R)
                {
                    log::debug("Resetting to all advancements required.");
                    manifest.reset();
                    ov.update_to(manifest);
                }
                else if (event.key.code == sf::Keyboard::P)
                {
                    log::debug("Dumping all available debug information.");
                    ov.debug();
                    log::debug("Ticks processed: ", ticks);
                    fp.debug();
                    log::debug("Finished dumping debug information.");
                }
            }
        }
        wm.clearAll();

        instance.poll(ticks);
        const auto li = instance.current_instance();
        // Once we have the current instance, we can get current information in the latest world.
        // TODO - is this something we can get from just the saves dir? I don't remember.
        if (li && world.poll(*li, ticks))
        {
            auto l = world.get_latest().get_advancements();
            if (not l.empty())
            {
                log::debug("Attempting to reset from found updated file: ", l);
                manifest.update_from_file(std::string{l});
                ov.update_to(manifest);
            }
        }

        /*
        for (int i = 0; i < ovWindow.getSize().x / 64; i++) {
            auto& tile = rb.get(i);
            s.setTexture(tile.text);
            s.setPosition(i * 64 + 8, 56);
            ovWindow.draw(s);
        }
        */
        ov.render(ovWindow);
        mapper.render(mapWindow, ticks);

        // Render loop end.
        wm.displayAll();

        // Give control to the OS - we don't want to consume too many resources.
        std::this_thread::sleep_for(std::chrono::milliseconds(conf.sleep_ms));
        ticks += 1; // it's the completed # of ticks
    }
    if (mainwindow.isOpen())
    {
        mainwindow.close();
    }
}
} // namespace aa
