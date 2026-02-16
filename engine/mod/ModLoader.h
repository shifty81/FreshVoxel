#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace fresh {

struct ModDescriptor {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::vector<std::string> dependencies;
    std::string entryPath;
    bool enabled = false;
};

enum class ModLoadResult {
    Success,
    NotFound,
    InvalidDescriptor,
    MissingDependency,
    AlreadyLoaded,
};

class ModLoader {
public:
    // Register a mod from descriptor
    ModLoadResult registerMod(const ModDescriptor& descriptor);

    // Unregister a mod by id
    bool unregisterMod(const std::string& id);

    // Enable / disable
    bool enableMod(const std::string& id);
    bool disableMod(const std::string& id);

    // Query
    bool hasMod(const std::string& id) const;
    const ModDescriptor* getMod(const std::string& id) const;
    std::vector<std::string> enabledMods() const;
    size_t modCount() const;

    // Dependency validation
    bool validateDependencies(const std::string& id) const;
    std::vector<std::string> missingDependencies(const std::string& id) const;

    // Scan a directory for mod descriptors (looks for mod.json files)
    size_t scanDirectory(const std::string& dir);

    // Clear all mods
    void clear();

private:
    std::unordered_map<std::string, ModDescriptor> m_mods;
};

} // namespace fresh
