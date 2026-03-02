#include "core/Engine.h"
#include "core/EngineConfig.h"
#include "core/Logger.h"
#include <cstdlib>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

static void PrintUsage()
{
    std::cout << "FreshVoxel Client v0.2.7" << std::endl;
    std::cout << "Usage: FreshClient [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help               Show this help message" << std::endl;
    std::cout << "  --project <path>     Set project directory (for editor live testing)" << std::endl;
    std::cout << "  --world <path>       Load a specific world save file" << std::endl;
    std::cout << "  --width <pixels>     Set window width (default: 1280)" << std::endl;
    std::cout << "  --height <pixels>    Set window height (default: 720)" << std::endl;
    std::cout << "  --windowed           Run in windowed mode (default)" << std::endl;
}

int main(int argc, char* argv[])
{
    std::string projectPath;
    std::string worldPath;
    int windowWidth = 1280;
    int windowHeight = 720;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            PrintUsage();
            return EXIT_SUCCESS;
        } else if (arg == "--project" && i + 1 < argc) {
            projectPath = argv[++i];
        } else if (arg == "--world" && i + 1 < argc) {
            worldPath = argv[++i];
        } else if (arg == "--width" && i + 1 < argc) {
            windowWidth = std::atoi(argv[++i]);
        } else if (arg == "--height" && i + 1 < argc) {
            windowHeight = std::atoi(argv[++i]);
        } else if (arg == "--windowed") {
            // Already default
        }
    }

#ifdef _WIN32
    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    }
#endif

    fresh::Logger::getInstance().initialize();

    std::cout << "FreshVoxel Client - Starting..." << std::endl;
    if (!projectPath.empty()) {
        std::cout << "  Project: " << projectPath << std::endl;
    }
    if (!worldPath.empty()) {
        std::cout << "  World:   " << worldPath << std::endl;
    }
    LOG_INFO("FreshVoxel Client starting...");

    try {
        fresh::Engine engine;
        auto config = fresh::EngineConfig::createDefault(fresh::EngineMode::Client);
        config.windowWidth = windowWidth;
        config.windowHeight = windowHeight;

        // If a world path is provided, set auto-load
        if (!worldPath.empty()) {
            config.autoLoadLastWorld = true;
        }

        if (!engine.initialize(config)) {
            std::cerr << "Failed to initialize engine" << std::endl;
            LOG_ERROR("Failed to initialize engine");
            fresh::Logger::getInstance().shutdown();
            return EXIT_FAILURE;
        }

        LOG_INFO("Client initialized successfully");
        engine.run();

        LOG_INFO("Client shutting down...");
        engine.shutdown();

        fresh::Logger::getInstance().shutdown();
        return EXIT_SUCCESS;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        LOG_FATAL(std::string("Fatal error: ") + e.what());
        fresh::Logger::getInstance().shutdown();
        return EXIT_FAILURE;
    }
}
