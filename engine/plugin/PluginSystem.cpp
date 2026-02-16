#include "PluginSystem.h"
#include <algorithm>

namespace fresh {

static int extractMajorVersion(const std::string& version) {
    if (version.empty()) return -1;
    auto dot = version.find('.');
    std::string major = (dot != std::string::npos) ? version.substr(0, dot) : version;
    try {
        return std::stoi(major);
    } catch (...) {
        return -1;
    }
}

PluginLoadResult PluginValidator::validate(const PluginDescriptor& descriptor, const std::string& engineVersion) {
    if (descriptor.name.empty() || descriptor.version.empty()) {
        return PluginLoadResult::InvalidDescriptor;
    }
    if (!descriptor.deterministic) {
        return PluginLoadResult::NonDeterministic;
    }
    if (!isVersionCompatible(descriptor.engineVersion, engineVersion)) {
        return PluginLoadResult::IncompatibleVersion;
    }
    return PluginLoadResult::Success;
}

bool PluginValidator::isVersionCompatible(const std::string& pluginVersion, const std::string& engineVersion) {
    int pluginMajor = extractMajorVersion(pluginVersion);
    int engineMajor = extractMajorVersion(engineVersion);
    if (pluginMajor < 0 || engineMajor < 0) return false;
    return pluginMajor == engineMajor;
}

bool PluginRegistry::registerPlugin(const PluginDescriptor& descriptor) {
    if (PluginValidator::validate(descriptor) != PluginLoadResult::Success) {
        return false;
    }
    if (findPlugin(descriptor.name) != nullptr) {
        return false;
    }
    m_plugins.push_back(descriptor);
    return true;
}

bool PluginRegistry::unregisterPlugin(const std::string& name) {
    auto it = std::remove_if(m_plugins.begin(), m_plugins.end(),
                             [&name](const PluginDescriptor& d) { return d.name == name; });
    if (it == m_plugins.end()) return false;
    m_plugins.erase(it, m_plugins.end());
    return true;
}

const PluginDescriptor* PluginRegistry::findPlugin(const std::string& name) const {
    for (const auto& p : m_plugins) {
        if (p.name == name) return &p;
    }
    return nullptr;
}

size_t PluginRegistry::pluginCount() const {
    return m_plugins.size();
}

std::vector<PluginDescriptor> PluginRegistry::getAll() const {
    return m_plugins;
}

std::vector<PluginDescriptor> PluginRegistry::getByType(const std::string& type) const {
    std::vector<PluginDescriptor> result;
    for (const auto& p : m_plugins) {
        if (p.type == type) result.push_back(p);
    }
    return result;
}

} // namespace fresh
