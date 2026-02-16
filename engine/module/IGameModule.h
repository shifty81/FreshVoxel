#pragma once
#include <cstdint>
#include <memory>

namespace fresh {

struct GameModuleDesc {
    const char* name = "";
    uint32_t version = 0;
};

class IGameModule {
public:
    virtual GameModuleDesc describe() const = 0;
    virtual void onStart() { }
    virtual void onTick(float dt) { (void)dt; }
    virtual void onShutdown() { }
    virtual ~IGameModule() = default;
};

using CreateGameModuleFn = IGameModule*(*)();

} // namespace fresh
