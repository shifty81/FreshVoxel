#pragma once

#ifdef _WIN32
#include "ui/native/Win32ConsolePanel.h"

namespace fresh
{
    // Windows native UI implementation
    using ConsolePanel = Win32ConsolePanel;
}
#else
#include <string>
#include <functional>

namespace fresh
{

/**
 * @brief Stub ConsolePanel for non-Win32 platforms
 */
class ConsolePanel
{
public:
    using CommandCallback = std::function<void(const std::string&)>;

    ConsolePanel() = default;
    virtual ~ConsolePanel() = default;

    bool initialize() { return true; }
    void render() {}
    void addMessage(int, const std::string&) {}
    void clear() {}
    void setAutoScroll(bool) {}
    void setFilterInfo(bool) {}
    void setFilterWarning(bool) {}
    void setFilterError(bool) {}
    void setCommandCallback(CommandCallback) {}
};

} // namespace fresh
#endif
