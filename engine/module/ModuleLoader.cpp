#include "ModuleLoader.h"
#include "core/Logger.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace fresh {

ModuleLoader::ModuleLoader() = default;

ModuleLoader::~ModuleLoader() {
    unload();
}

ModuleLoadResult ModuleLoader::load(const std::string& path) {
    if (m_module) {
        return ModuleLoadResult::AlreadyLoaded;
    }

#ifdef _WIN32
    HMODULE lib = LoadLibraryA(path.c_str());
    if (!lib) {
        Logger::getInstance().error("ModuleLoader: failed to load " + path, "ModuleLoader");
        return ModuleLoadResult::NotFound;
    }
    auto fn = reinterpret_cast<CreateGameModuleFn>(
        GetProcAddress(lib, "CreateGameModule"));
    if (!fn) {
        FreeLibrary(lib);
        Logger::getInstance().error("ModuleLoader: CreateGameModule symbol not found in " + path, "ModuleLoader");
        return ModuleLoadResult::SymbolMissing;
    }
    m_handle = static_cast<void*>(lib);
#else
    void* lib = dlopen(path.c_str(), RTLD_NOW);
    if (!lib) {
        Logger::getInstance().error(std::string("ModuleLoader: ") + dlerror(), "ModuleLoader");
        return ModuleLoadResult::NotFound;
    }
    auto fn = reinterpret_cast<CreateGameModuleFn>(dlsym(lib, "CreateGameModule"));
    if (!fn) {
        dlclose(lib);
        Logger::getInstance().error("ModuleLoader: CreateGameModule symbol not found in " + path, "ModuleLoader");
        return ModuleLoadResult::SymbolMissing;
    }
    m_handle = lib;
#endif

    m_module.reset(fn());
    return ModuleLoadResult::Success;
}

void ModuleLoader::unload() {
    m_module.reset();

    if (m_handle) {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(m_handle));
#else
        dlclose(m_handle);
#endif
        m_handle = nullptr;
    }
}

void ModuleLoader::setStaticModule(std::unique_ptr<IGameModule> mod) {
    unload();
    m_module = std::move(mod);
}

IGameModule* ModuleLoader::getModule() const {
    return m_module.get();
}

bool ModuleLoader::isLoaded() const {
    return m_module != nullptr;
}

} // namespace fresh
