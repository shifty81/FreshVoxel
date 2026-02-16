#include "core/Engine.h"
#include "core/EngineConfig.h"
#include "core/Logger.h"
#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    fresh::Logger::getInstance().initialize();

    std::cout << "FreshVoxel Server - Starting..." << std::endl;
    LOG_INFO("FreshVoxel Server starting (headless)...");

    try {
        fresh::Engine engine;
        auto config = fresh::EngineConfig::createDefault(fresh::EngineMode::Server);

        if (!engine.initialize(config)) {
            std::cerr << "Failed to initialize engine" << std::endl;
            LOG_ERROR("Failed to initialize engine");
            fresh::Logger::getInstance().shutdown();
            return EXIT_FAILURE;
        }

        LOG_INFO("Server initialized successfully");
        engine.run();

        LOG_INFO("Server shutting down...");
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
