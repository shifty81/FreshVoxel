#include "ui/EditorToolbar.h"

#ifdef _WIN32
#include "ui/native/Win32Toolbar.h"
#endif

namespace fresh
{

EditorToolbar::EditorToolbar()
    : m_activeTool(Tool::Select)
    , m_playMode(PlayMode::Stopped)
{
#ifdef _WIN32
    m_nativeToolbar = std::make_unique<Win32Toolbar>();
#endif
}

EditorToolbar::~EditorToolbar() = default;

bool EditorToolbar::initialize()
{
#ifdef _WIN32
    // Win32Toolbar initialization is done separately when attached to window
    // For now, we just return success as the toolbar is created
    return true;
#else
    // Non-Windows platforms would initialize ImGui toolbar here
    return true;
#endif
}

void EditorToolbar::setToolCallback(ToolCallback callback)
{
    m_toolCallback = callback;
}

void EditorToolbar::setPlayModeCallback(PlayModeCallback callback)
{
    m_playModeCallback = callback;
}

void EditorToolbar::setActiveTool(Tool tool)
{
    if (m_activeTool != tool) {
        m_activeTool = tool;
        if (m_toolCallback) {
            m_toolCallback(tool);
        }
    }
}

EditorToolbar::Tool EditorToolbar::getActiveTool() const
{
    return m_activeTool;
}

void EditorToolbar::setPlayMode(PlayMode mode)
{
    if (m_playMode != mode) {
        m_playMode = mode;
        if (m_playModeCallback) {
            m_playModeCallback(mode);
        }
    }
}

EditorToolbar::PlayMode EditorToolbar::getPlayMode() const
{
    return m_playMode;
}

} // namespace fresh
