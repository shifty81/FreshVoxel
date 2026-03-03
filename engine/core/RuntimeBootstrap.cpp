#include "core/RuntimeBootstrap.h"
#include "core/Logger.h"

namespace fresh {

RuntimeBootstrap::RuntimeBootstrap() = default;
RuntimeBootstrap::~RuntimeBootstrap() = default;

bool RuntimeBootstrap::initialize(RuntimeMode mode) {
    if (m_initialized) {
        Logger::getInstance().info("RuntimeBootstrap: already initialized, skipping");
        return true;
    }

    m_mode = mode;

    loadCoreSystems();
    loadPlatformSystems(mode);
    loadGameModules();

    m_initialized = true;
    Logger::getInstance().info("RuntimeBootstrap: initialization complete");
    return true;
}

void RuntimeBootstrap::loadCoreSystems() {
    Logger::getInstance().info("RuntimeBootstrap: core systems loaded");
}

void RuntimeBootstrap::loadPlatformSystems(RuntimeMode mode) {
    switch (mode) {
        case RuntimeMode::Editor:
            Logger::getInstance().info("RuntimeBootstrap: editor platform systems loaded");
            break;
        case RuntimeMode::Game:
            Logger::getInstance().info("RuntimeBootstrap: game platform systems loaded");
            break;
        case RuntimeMode::Server:
            Logger::getInstance().info("RuntimeBootstrap: server platform systems loaded");
            break;
    }
}

void RuntimeBootstrap::loadGameModules() {
    Logger::getInstance().info("RuntimeBootstrap: game modules loaded (hook point)");
}

} // namespace fresh
