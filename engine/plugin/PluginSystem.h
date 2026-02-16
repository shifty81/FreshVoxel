#pragma once
#include <string>
#include <vector>
#include <memory>

namespace fresh {

struct PluginDescriptor {
    std::string name;
    std::string version;
    std::string engineVersion;
    std::string type; // "graph-extension", "editor-panel", "asset-importer"
    bool deterministic = true;
};

enum class PluginLoadResult {
    Success,
    NotFound,
    InvalidDescriptor,
    IncompatibleVersion,
    NonDeterministic,
};

class PluginValidator {
public:
    static PluginLoadResult validate(const PluginDescriptor& descriptor, const std::string& engineVersion = "0.1.0");
    static bool isVersionCompatible(const std::string& pluginVersion, const std::string& engineVersion);
};

class PluginRegistry {
public:
    bool registerPlugin(const PluginDescriptor& descriptor);
    bool unregisterPlugin(const std::string& name);
    const PluginDescriptor* findPlugin(const std::string& name) const;
    size_t pluginCount() const;
    std::vector<PluginDescriptor> getAll() const;
    std::vector<PluginDescriptor> getByType(const std::string& type) const;
private:
    std::vector<PluginDescriptor> m_plugins;
};

} // namespace fresh
