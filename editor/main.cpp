#include "core/Engine.h"
#include "core/Logger.h"
#include <cstdlib>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

#ifdef _WIN32
    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    }
#endif

    fresh::Logger::getInstance().initialize();

    std::cout << "Fresh Editor - Starting..." << std::endl;
    LOG_INFO("Fresh Editor starting...");

    try {
        fresh::Engine engine;

        if (!engine.initialize()) {
            std::cerr << "Failed to initialize engine" << std::endl;
            LOG_ERROR("Failed to initialize engine");
            fresh::Logger::getInstance().shutdown();
            return EXIT_FAILURE;
        }

        LOG_INFO("Editor initialized successfully");
        engine.run();

        LOG_INFO("Editor shutting down...");
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
