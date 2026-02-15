#include "editor/SelectionRenderer.h"
#include "devtools/DebugRenderer.h"
#include "core/Logger.h"

namespace fresh
{

SelectionRenderer::SelectionRenderer()
    : m_debugRenderer(nullptr)
    , m_selectionColor(0.2f, 0.7f, 1.0f, 0.8f) // Light blue with some transparency
    , m_previewColor(0.3f, 1.0f, 0.3f, 0.5f)    // Light green with transparency
    , m_enabled(true)
{
}

SelectionRenderer::~SelectionRenderer() = default;

bool SelectionRenderer::initialize(devtools::DebugRenderer* debugRenderer)
{
    if (debugRenderer == nullptr) {
        Logger::getInstance().error("Cannot initialize SelectionRenderer with null DebugRenderer", "SelectionRenderer");
        return false;
    }

    m_debugRenderer = debugRenderer;
    Logger::getInstance().info("SelectionRenderer initialized", "SelectionRenderer");
    return true;
}

void SelectionRenderer::render(const SelectionManager* selectionManager)
{
    if (!m_enabled || m_debugRenderer == nullptr || selectionManager == nullptr) {
        return;
    }

    // Check if there's a selection to render
    if (!selectionManager->hasSelection()) {
        return;
    }

    glm::ivec3 min, max;
    if (!selectionManager->getSelectionBounds(min, max)) {
        return;
    }

    // Calculate center and size for the bounding box
    glm::vec3 center = glm::vec3(min + max) * 0.5f + glm::vec3(0.5f);
    glm::vec3 size = glm::vec3(max - min) + glm::vec3(1.0f);

    // Draw the selection box
    m_debugRenderer->drawBox(center, size, m_selectionColor, 0.0f);

    // Optionally, draw individual voxel outlines for smaller selections
    const auto& selectedVoxels = selectionManager->getSelectedVoxels();
    if (selectedVoxels.size() < 100) { // Only for small selections to avoid performance issues
        for (const auto& voxelPos : selectedVoxels) {
            glm::vec3 voxelCenter = glm::vec3(voxelPos.x, voxelPos.y, voxelPos.z) + glm::vec3(0.5f);
            glm::vec3 voxelSize = glm::vec3(1.0f);
            
            // Use a slightly different color for individual voxels
            glm::vec4 voxelColor = m_selectionColor;
            voxelColor.a *= 0.5f; // More transparent
            
            m_debugRenderer->drawBox(voxelCenter, voxelSize, voxelColor, 0.0f);
        }
    }
}

void SelectionRenderer::renderPastePreview(const SelectionManager* selectionManager, float alpha)
{
    if (!m_enabled || m_debugRenderer == nullptr || selectionManager == nullptr) {
        return;
    }

    // Check if paste preview is active
    if (!selectionManager->isPastePreviewActive()) {
        return;
    }

    // Get preview bounds
    glm::ivec3 min, max;
    if (!selectionManager->getPastePreviewBounds(min, max)) {
        return;
    }

    // Calculate center and size for the preview bounding box
    glm::vec3 center = glm::vec3(min + max) * 0.5f + glm::vec3(0.5f);
    glm::vec3 size = glm::vec3(max - min) + glm::vec3(1.0f);

    // Apply alpha to preview color
    glm::vec4 previewColor = m_previewColor;
    previewColor.a = alpha;

    // Draw the preview box with ghost effect
    m_debugRenderer->drawBox(center, size, previewColor, 0.0f);

    // Get preview voxel positions and types
    std::vector<VoxelPosition> positions;
    std::vector<VoxelType> types;
    
    if (!selectionManager->getPastePreviewData(positions, types)) {
        return;
    }

    // Draw individual preview voxels for smaller selections
    if (positions.size() < 100) {
        for (size_t i = 0; i < positions.size(); ++i) {
            const auto& voxelPos = positions[i];
            glm::vec3 voxelCenter = glm::vec3(voxelPos.x, voxelPos.y, voxelPos.z) + glm::vec3(0.5f);
            glm::vec3 voxelSize = glm::vec3(1.0f);
            
            // Use preview color with specified alpha for ghost effect
            glm::vec4 ghostColor = previewColor;
            ghostColor.a = alpha * 0.5f; // Even more transparent for individual voxels
            
            m_debugRenderer->drawBox(voxelCenter, voxelSize, ghostColor, 0.0f);
        }
    }

    // Optionally, log preview information
    static int framesSinceLastLog = 0;
    if (++framesSinceLastLog > 60) { // Log every 60 frames
        Logger::getInstance().info("Paste preview active with " + 
                                    std::to_string(positions.size()) + " voxels", "SelectionRenderer");
        framesSinceLastLog = 0;
    }
}

} // namespace fresh
