#pragma once
#include "IGameModule.h"
#include <string>
#include <memory>

namespace fresh {

enum class ModuleLoadResult {
    Success,
    NotFound,
    SymbolMissing,
    AlreadyLoaded,
};

class ModuleLoader {
public:
    ModuleLoader();
    ~ModuleLoader();

    // Load a game module from a shared library path
    ModuleLoadResult load(const std::string& path);

    // Unload the currently loaded module
    void unload();

    // Set a statically linked module (for tests or static builds)
    void setStaticModule(std::unique_ptr<IGameModule> mod);

    // Access the loaded module
    IGameModule* getModule() const;
    bool isLoaded() const;

private:
    std::unique_ptr<IGameModule> m_module;
    void* m_handle = nullptr;
};

} // namespace fresh
