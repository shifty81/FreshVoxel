#pragma once

#include <memory>
#include <string>

namespace fresh {

class Engine;

enum class RuntimeMode {
    Editor,
    Game,
    Server
};

class RuntimeBootstrap {
public:
    RuntimeBootstrap();
    ~RuntimeBootstrap();

    RuntimeBootstrap(const RuntimeBootstrap&) = delete;
    RuntimeBootstrap& operator=(const RuntimeBootstrap&) = delete;

    bool initialize(RuntimeMode mode);
    bool isInitialized() const { return m_initialized; }
    RuntimeMode mode() const { return m_mode; }

private:
    void loadCoreSystems();
    void loadPlatformSystems(RuntimeMode mode);
    void loadGameModules();

    bool m_initialized = false;
    RuntimeMode m_mode = RuntimeMode::Game;
};

} // namespace fresh
