#include "ModLoader.h"
#include <filesystem>
#include <fstream>

namespace fresh {

ModLoadResult ModLoader::registerMod(const ModDescriptor& descriptor) {
    if (descriptor.id.empty() || descriptor.name.empty()) {
        return ModLoadResult::InvalidDescriptor;
    }
    if (m_mods.find(descriptor.id) != m_mods.end()) {
        return ModLoadResult::AlreadyLoaded;
    }
    m_mods[descriptor.id] = descriptor;
    return ModLoadResult::Success;
}

bool ModLoader::unregisterMod(const std::string& id) {
    return m_mods.erase(id) > 0;
}

bool ModLoader::enableMod(const std::string& id) {
    auto it = m_mods.find(id);
    if (it == m_mods.end()) return false;
    it->second.enabled = true;
    return true;
}

bool ModLoader::disableMod(const std::string& id) {
    auto it = m_mods.find(id);
    if (it == m_mods.end()) return false;
    it->second.enabled = false;
    return true;
}

bool ModLoader::hasMod(const std::string& id) const {
    return m_mods.find(id) != m_mods.end();
}

const ModDescriptor* ModLoader::getMod(const std::string& id) const {
    auto it = m_mods.find(id);
    if (it == m_mods.end()) return nullptr;
    return &it->second;
}

std::vector<std::string> ModLoader::enabledMods() const {
    std::vector<std::string> result;
    for (const auto& [id, mod] : m_mods) {
        if (mod.enabled) result.push_back(id);
    }
    return result;
}

size_t ModLoader::modCount() const {
    return m_mods.size();
}

bool ModLoader::validateDependencies(const std::string& id) const {
    return missingDependencies(id).empty();
}

std::vector<std::string> ModLoader::missingDependencies(const std::string& id) const {
    std::vector<std::string> missing;
    auto it = m_mods.find(id);
    if (it == m_mods.end()) return missing;
    for (const auto& dep : it->second.dependencies) {
        if (m_mods.find(dep) == m_mods.end()) {
            missing.push_back(dep);
        }
    }
    return missing;
}

size_t ModLoader::scanDirectory(const std::string& dir) {
    size_t count = 0;
    if (!std::filesystem::exists(dir)) return count;

    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_directory()) {
            std::filesystem::path modJson = entry.path() / "mod.json";
            if (std::filesystem::exists(modJson)) {
                std::ifstream f(modJson.string());
                if (f.is_open()) {
                    ModDescriptor desc;
                    desc.id = entry.path().filename().string();
                    desc.name = desc.id;
                    desc.entryPath = entry.path().string();
                    if (registerMod(desc) == ModLoadResult::Success) {
                        count++;
                    }
                }
            }
        }
    }
    return count;
}

void ModLoader::clear() {
    m_mods.clear();
}

} // namespace fresh
