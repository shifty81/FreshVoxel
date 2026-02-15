#include "editor/WorldEditor.h"

#include <iostream>

#include "editor/TerraformingSystem.h"
#include "voxel/VoxelWorld.h"

namespace fresh
{

WorldEditor::WorldEditor() : m_world(nullptr), m_enabled(false) {}

WorldEditor::~WorldEditor() {}

bool WorldEditor::initialize(VoxelWorld* world)
{
    if (!world) {
        std::cerr << "Cannot initialize WorldEditor with null world" << std::endl;
        return false;
    }

    m_world = world;

    // Create terraforming system
    m_terraformingSystem = std::make_unique<TerraformingSystem>(m_world);
    if (!m_terraformingSystem->initialize()) {
        std::cerr << "Failed to initialize terraforming system" << std::endl;
        return false;
    }

    std::cout << "World editor initialized" << std::endl;
    return true;
}

void WorldEditor::update(float deltaTime)
{
    (void)deltaTime; // Unused - placeholder for future implementation
    if (!m_enabled) {
        return;
    }

    // Editor update logic would go here
    // e.g., handle keyboard/mouse input for terraforming
}

} // namespace fresh
