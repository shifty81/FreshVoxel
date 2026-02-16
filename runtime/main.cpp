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
    std::cout << "Fresh Runtime v0.1.0" << std::endl;
    std::cout << "Usage: FreshRuntime [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help               Show this help message" << std::endl;
}

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            PrintUsage();
            return EXIT_SUCCESS;
        }
    }

#ifdef _WIN32
    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    }
#endif

    fresh::Logger::getInstance().initialize();

    std::cout << "Fresh Runtime - Starting..." << std::endl;
    LOG_INFO("Fresh Runtime starting...");

    try {
        fresh::Engine engine;
        auto config = fresh::EngineConfig::createDefault(fresh::EngineMode::Runtime);

        if (!engine.initialize(config)) {
            std::cerr << "Failed to initialize engine" << std::endl;
            LOG_ERROR("Failed to initialize engine");
            fresh::Logger::getInstance().shutdown();
            return EXIT_FAILURE;
        }

        LOG_INFO("Runtime initialized successfully");
        engine.run();

        LOG_INFO("Runtime shutting down...");
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
