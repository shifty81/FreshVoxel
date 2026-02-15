#pragma once

#include <functional>
#include <memory>

namespace fresh
{

#ifdef _WIN32
class Win32Toolbar;
#endif

/**
 * @brief Editor toolbar for tool selection and playmode control
 * 
 * Provides toolbar interface for the editor with tool selection (Move, Rotate, Scale)
 * and play mode control (Playing, Stopped, Paused).
 */
class EditorToolbar
{
public:
    enum class Tool
    {
        Move,
        Rotate,
        Scale,
        Select,
        Paint,
        Erase
    };

    enum class PlayMode
    {
        Stopped,
        Playing,
        Paused
    };

    using ToolCallback = std::function<void(Tool)>;
    using PlayModeCallback = std::function<void(PlayMode)>;

    EditorToolbar();
    ~EditorToolbar();

    // Prevent copying
    EditorToolbar(const EditorToolbar&) = delete;
    EditorToolbar& operator=(const EditorToolbar&) = delete;

    /**
     * @brief Initialize the toolbar
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Set callback for tool selection changes
     * @param callback Function to call when tool is selected
     */
    void setToolCallback(ToolCallback callback);

    /**
     * @brief Set callback for play mode changes
     * @param callback Function to call when play mode changes
     */
    void setPlayModeCallback(PlayModeCallback callback);

    /**
     * @brief Set the active tool
     * @param tool Tool to activate
     */
    void setActiveTool(Tool tool);

    /**
     * @brief Get the currently active tool
     * @return Currently active tool
     */
    Tool getActiveTool() const;

    /**
     * @brief Set the play mode
     * @param mode Play mode to set
     */
    void setPlayMode(PlayMode mode);

    /**
     * @brief Get the current play mode
     * @return Current play mode
     */
    PlayMode getPlayMode() const;

private:
#ifdef _WIN32
    std::unique_ptr<Win32Toolbar> m_nativeToolbar;
#endif
    Tool m_activeTool;
    PlayMode m_playMode;
    ToolCallback m_toolCallback;
    PlayModeCallback m_playModeCallback;
};

} // namespace fresh
