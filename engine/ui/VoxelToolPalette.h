#pragma once

// VoxelToolPalette is now implemented using Windows native Win32TerraformingPanel
// This header provides a compatibility wrapper for backward compatibility
// Direct use of Win32TerraformingPanel is preferred - see ui/native/Win32TerraformingPanel.h

#include <functional>
#include "ui/native/Win32TerraformingPanel.h"

namespace fresh
{
    class TerraformingSystem;
    enum class VoxelType : uint8_t;

    /**
     * @brief Compatibility wrapper for VoxelToolPalette
     * 
     * This class wraps Win32TerraformingPanel to maintain API compatibility
     * with legacy code that expects VoxelToolPalette interface.
     * 
     * For new code, use Win32TerraformingPanel directly.
     */
    class VoxelToolPalette
    {
    public:
        VoxelToolPalette() = default;
        ~VoxelToolPalette() = default;

        // Legacy API compatibility methods - actual functionality is in Win32TerraformingPanel
        bool initialize(TerraformingSystem* /*terraformingSystem*/) { return true; }
        void render() { }
        void setVoxelTypeCallback(std::function<void(VoxelType)> /*callback*/) { }
    };
}
