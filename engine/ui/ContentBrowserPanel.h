#pragma once

#ifdef _WIN32
#include "ui/native/Win32ContentBrowserPanel.h"

namespace fresh
{
    // Windows native UI implementation
    using ContentBrowserPanel = Win32ContentBrowserPanel;
}
#else
#include <string>
#include <functional>

namespace fresh
{

/**
 * @brief Stub ContentBrowserPanel for non-Win32 platforms
 */
class ContentBrowserPanel
{
public:
    using ItemSelectedCallback = std::function<void(const std::string&)>;
    using ItemActivatedCallback = std::function<void(const std::string&)>;

    ContentBrowserPanel() = default;
    virtual ~ContentBrowserPanel() = default;

    bool initialize(const std::string& = "") { return true; }
    void render() {}
    void refresh() {}
    void navigateTo(const std::string&) {}
    std::string getCurrentDirectory() const { return ""; }
    void setItemSelectedCallback(ItemSelectedCallback) {}
    void setItemActivatedCallback(ItemActivatedCallback) {}
    void showImportDialog() {}
};

} // namespace fresh
#endif
