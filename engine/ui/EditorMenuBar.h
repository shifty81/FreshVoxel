#pragma once

// This header is deprecated - Windows native UI is used instead
// For Windows: Use ui/native/Win32MenuBar.h
// This stub exists for backward compatibility during migration

#ifdef _WIN32
    #include "ui/native/Win32MenuBar.h"
    
    namespace fresh
    {
        // Type alias for backward compatibility
        using EditorMenuBar = Win32MenuBar;
    }
#else
    #include <string>
    #include <functional>

    namespace fresh
    {
        /**
         * @brief Stub EditorMenuBar for non-Win32 platforms
         */
        class EditorMenuBar
        {
        public:
            EditorMenuBar() = default;
            ~EditorMenuBar() = default;

            bool initialize(void* = nullptr, void* = nullptr) { return true; }
            void render() {}
            void setSceneHierarchyVisible(bool*) {}
            void setInspectorVisible(bool*) {}
            void setContentBrowserVisible(bool*) {}
            void setConsoleVisible(bool*) {}
            void setToolPaletteVisible(bool*) {}
            void setEditorSettingsCallback(std::function<void()>) {}
            void setImportAssetsCallback(std::function<void()>) {}
            void setDialogueEditorCallback(std::function<void()>) {}
            void setSelectAllCallback(std::function<void()>) {}
            void setDeselectAllCallback(std::function<void()>) {}
            void setLoadLayoutCallback(std::function<void(const std::string&)>) {}
            void setSaveLayoutCallback(std::function<void()>) {}
            void setResetLayoutCallback(std::function<void()>) {}
            void setUndoCallback(std::function<void()>) {}
            void setRedoCallback(std::function<void()>) {}
            void setCutCallback(std::function<void()>) {}
            void setCopyCallback(std::function<void()>) {}
            void setPasteCallback(std::function<void()>) {}
            void setDeleteCallback(std::function<void()>) {}
        };
    }
#endif
