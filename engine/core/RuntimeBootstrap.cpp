#include "core/RuntimeBootstrap.h"
#include "core/Logger.h"

namespace fresh {

RuntimeBootstrap::RuntimeBootstrap() = default;
RuntimeBootstrap::~RuntimeBootstrap() = default;

bool RuntimeBootstrap::initialize(RuntimeMode mode) {
    if (m_initialized) {
        Logger::info("RuntimeBootstrap: already initialized, skipping");
        return true;
    }

    m_mode = mode;

    loadCoreSystems();
    loadPlatformSystems(mode);
    loadGameModules();

    m_initialized = true;
    Logger::info("RuntimeBootstrap: initialization complete");
    return true;
}

void RuntimeBootstrap::loadCoreSystems() {
    Logger::info("RuntimeBootstrap: core systems loaded");
}

void RuntimeBootstrap::loadPlatformSystems(RuntimeMode mode) {
    switch (mode) {
        case RuntimeMode::Editor:
            Logger::info("RuntimeBootstrap: editor platform systems loaded");
            break;
        case RuntimeMode::Game:
            Logger::info("RuntimeBootstrap: game platform systems loaded");
            break;
        case RuntimeMode::Server:
            Logger::info("RuntimeBootstrap: server platform systems loaded");
            break;
    }
}

void RuntimeBootstrap::loadGameModules() {
    Logger::info("RuntimeBootstrap: game modules loaded (hook point)");
}

} // namespace fresh
