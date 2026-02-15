#pragma once

#include <glm/glm.hpp>
#include "editor/SelectionManager.h"

namespace fresh
{

// Forward declarations
namespace devtools
{
class DebugRenderer;
}

/**
 * @brief Renders visual feedback for voxel selection
 * 
 * Shows selection bounds and highlights selected voxels using wireframe rendering.
 */
class SelectionRenderer
{
public:
    SelectionRenderer();
    ~SelectionRenderer();

    /**
     * @brief Initialize the selection renderer
     * @param debugRenderer Pointer to debug renderer for drawing
     * @return true if successful
     */
    bool initialize(devtools::DebugRenderer* debugRenderer);

    /**
     * @brief Render the current selection
     * @param selectionManager Selection manager to visualize
     */
    void render(const SelectionManager* selectionManager);

    /**
     * @brief Set selection color
     * @param color RGBA color for selection outline
     */
    void setSelectionColor(const glm::vec4& color)
    {
        m_selectionColor = color;
    }

    /**
     * @brief Get selection color
     * @return Current selection color
     */
    const glm::vec4& getSelectionColor() const
    {
        return m_selectionColor;
    }

    /**
     * @brief Set if selection should be rendered
     * @param enabled true to enable rendering
     */
    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
    }

    /**
     * @brief Check if selection rendering is enabled
     * @return true if enabled
     */
    bool isEnabled() const
    {
        return m_enabled;
    }

    /**
     * @brief Render paste preview with ghost block effect
     * @param selectionManager Selection manager with preview data
     * @param alpha Transparency for ghost blocks (0.0 = fully transparent, 1.0 = opaque)
     */
    void renderPastePreview(const SelectionManager* selectionManager, float alpha = 0.5f);

    /**
     * @brief Set paste preview color
     * @param color RGBA color for preview outline
     */
    void setPreviewColor(const glm::vec4& color)
    {
        m_previewColor = color;
    }

    /**
     * @brief Get paste preview color
     * @return Current preview color
     */
    const glm::vec4& getPreviewColor() const
    {
        return m_previewColor;
    }

private:
    devtools::DebugRenderer* m_debugRenderer;
    glm::vec4 m_selectionColor;
    glm::vec4 m_previewColor;
    bool m_enabled;
};

} // namespace fresh
