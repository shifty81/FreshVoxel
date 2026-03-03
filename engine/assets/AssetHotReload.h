#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
#include <functional>

namespace fresh::asset {

struct AssetEntry {
    std::string id;
    std::string path;
    uint64_t version = 1;
};

class AssetHotReload {
public:
    using ReloadCallback = std::function<void(const AssetEntry&)>;

    void scan(const std::string& root);
    const AssetEntry* get(const std::string& id) const;

    void setReloadCallback(ReloadCallback cb);
    void pollHotReload();

    size_t count() const;

private:
    std::unordered_map<std::string, AssetEntry> m_assets;
    std::unordered_map<std::string, std::filesystem::file_time_type> m_timestamps;
    ReloadCallback m_onReload;
};

} // namespace fresh::asset
