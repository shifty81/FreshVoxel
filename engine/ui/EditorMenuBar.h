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
#endif
