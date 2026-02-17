/**
 * @file SelectionManagerTests.cpp
 * @brief Comprehensive tests for SelectionManager
 * 
 * Tests the voxel selection system including box selection, clipboard operations,
 * manipulation, and edge cases.
 */

#include <gtest/gtest.h>
#include "editor/SelectionManager.h"
#include "voxel/VoxelWorld.h"
#include <glm/glm.hpp>

namespace fresh {

/**
 * @brief Test fixture for SelectionManager tests
 */
class SelectionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        selectionManager = std::make_unique<SelectionManager>();
        voxelWorld = std::make_unique<VoxelWorld>();
        voxelWorld->initialize();
        
        // Create a test chunk with some voxels
        createTestChunk();
    }

    void TearDown() override {
        selectionManager.reset();
        voxelWorld.reset();
    }
    
    void createTestChunk() {
        // Load a chunk at origin
        auto* chunk = voxelWorld->loadChunk(ChunkPos(0, 0));
        if (chunk) {
            // Fill with some test voxels (stone blocks in a 10x10x10 cube)
            for (int x = 0; x < 10; ++x) {
                for (int y = 0; y < 10; ++y) {
                    for (int z = 0; z < 10; ++z) {
                        voxelWorld->setVoxel(WorldPos(x, y, z), Voxel(VoxelType::Stone));
                    }
                }
            }
        }
    }

    std::unique_ptr<SelectionManager> selectionManager;
    std::unique_ptr<VoxelWorld> voxelWorld;
};

// ============================================================================
// Constructor and Initialization Tests
// ============================================================================

TEST_F(SelectionManagerTest, Constructor_InitializesEmpty) {
    // GIVEN: A newly constructed SelectionManager
    // THEN: It should have no selection
    EXPECT_FALSE(selectionManager->hasSelection());
    EXPECT_FALSE(selectionManager->isSelecting());
    EXPECT_EQ(0u, selectionManager->getSelectionSize());
}

TEST_F(SelectionManagerTest, Constructor_NoClipboardData) {
    // GIVEN: A newly constructed SelectionManager
    // THEN: Clipboard should be empty
    EXPECT_FALSE(selectionManager->hasClipboardData());
}

// ============================================================================
// Box Selection Tests
// ============================================================================

TEST_F(SelectionManagerTest, StartSelection_SetsSelectingMode) {
    // WHEN: Starting a selection
    selectionManager->startSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    
    // THEN: Should be in selecting mode
    EXPECT_TRUE(selectionManager->isSelecting());
    EXPECT_TRUE(selectionManager->hasSelection());
}

TEST_F(SelectionManagerTest, StartSelection_ClearsPreviousSelection) {
    // GIVEN: A previous selection exists
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(2.0f, 2.0f, 2.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    size_t previousSize = selectionManager->getSelectionSize();
    EXPECT_GT(previousSize, 0u);
    
    // WHEN: Starting a new selection
    selectionManager->startSelection(glm::vec3(5.0f, 5.0f, 5.0f));
    
    // THEN: Previous selection should be cleared
    EXPECT_TRUE(selectionManager->isSelecting());
}

TEST_F(SelectionManagerTest, UpdateSelection_WhileSelecting) {
    // GIVEN: Selection has been started
    selectionManager->startSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    
    // WHEN: Updating selection
    selectionManager->updateSelection(glm::vec3(3.0f, 3.0f, 3.0f));
    
    // THEN: Should still be selecting
    EXPECT_TRUE(selectionManager->isSelecting());
}

TEST_F(SelectionManagerTest, UpdateSelection_WithoutStarting_DoesNothing) {
    // WHEN: Updating without starting
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    
    // THEN: Should not be selecting
    EXPECT_FALSE(selectionManager->isSelecting());
}

TEST_F(SelectionManagerTest, FinalizeSelection_CreatesSelectionBox) {
    // GIVEN: A selection box is drawn
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(2.0f, 2.0f, 2.0f));
    
    // WHEN: Finalizing selection
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // THEN: Should have selected voxels and no longer be selecting
    EXPECT_FALSE(selectionManager->isSelecting());
    EXPECT_TRUE(selectionManager->hasSelection());
    EXPECT_GT(selectionManager->getSelectionSize(), 0u);
}

TEST_F(SelectionManagerTest, FinalizeSelection_SelectsOnlySolidVoxels) {
    // GIVEN: A region with some air blocks
    // Clear a few voxels
    voxelWorld->setVoxel(WorldPos(0, 0, 0), Voxel(VoxelType::Air));
    voxelWorld->setVoxel(WorldPos(1, 1, 1), Voxel(VoxelType::Air));
    
    // WHEN: Selecting a region that includes air
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(2.0f, 2.0f, 2.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // THEN: Should only select solid voxels (3x3x3 = 27, minus 2 air = 25)
    EXPECT_EQ(25u, selectionManager->getSelectionSize());
}

TEST_F(SelectionManagerTest, FinalizeSelection_WithoutWorld_DoesNothing) {
    // GIVEN: A selection is started
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(2.0f, 2.0f, 2.0f));
    
    // WHEN: Finalizing without a world
    selectionManager->finalizeSelection(nullptr);
    
    // THEN: Should not be selecting but also have no selection
    EXPECT_FALSE(selectionManager->isSelecting());
    EXPECT_EQ(0u, selectionManager->getSelectionSize());
}

// ============================================================================
// Selection Bounds Tests
// ============================================================================

TEST_F(SelectionManagerTest, GetSelectionBounds_WhileSelecting_ReturnsBox) {
    // GIVEN: Currently selecting
    selectionManager->startSelection(glm::vec3(1.0f, 2.0f, 3.0f));
    selectionManager->updateSelection(glm::vec3(4.0f, 5.0f, 6.0f));
    
    // WHEN: Getting bounds
    glm::ivec3 min, max;
    bool hasBounds = selectionManager->getSelectionBounds(min, max);
    
    // THEN: Should return valid bounds
    EXPECT_TRUE(hasBounds);
    EXPECT_EQ(glm::ivec3(1, 2, 3), min);
    EXPECT_EQ(glm::ivec3(4, 5, 6), max);
}

TEST_F(SelectionManagerTest, GetSelectionBounds_FinalizedSelection_ReturnsBounds) {
    // GIVEN: A finalized selection
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(2.0f, 2.0f, 2.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // WHEN: Getting bounds
    glm::ivec3 min, max;
    bool hasBounds = selectionManager->getSelectionBounds(min, max);
    
    // THEN: Should return valid bounds
    EXPECT_TRUE(hasBounds);
    EXPECT_EQ(glm::ivec3(0, 0, 0), min);
    EXPECT_EQ(glm::ivec3(2, 2, 2), max);
}

TEST_F(SelectionManagerTest, GetSelectionBounds_NoSelection_ReturnsFalse) {
    // WHEN: Getting bounds with no selection
    glm::ivec3 min, max;
    bool hasBounds = selectionManager->getSelectionBounds(min, max);
    
    // THEN: Should return false
    EXPECT_FALSE(hasBounds);
}

TEST_F(SelectionManagerTest, GetSelectionBounds_HandlesInvertedBox) {
    // GIVEN: Selection with end before start (drag in negative direction)
    selectionManager->startSelection(glm::vec3(5.0f, 5.0f, 5.0f));
    selectionManager->updateSelection(glm::vec3(2.0f, 2.0f, 2.0f));
    
    // WHEN: Getting bounds
    glm::ivec3 min, max;
    bool hasBounds = selectionManager->getSelectionBounds(min, max);
    
    // THEN: Should return correct min/max regardless of drag direction
    EXPECT_TRUE(hasBounds);
    EXPECT_EQ(glm::ivec3(2, 2, 2), min);
    EXPECT_EQ(glm::ivec3(5, 5, 5), max);
}

// ============================================================================
// Delete Selected Tests
// ============================================================================

TEST_F(SelectionManagerTest, DeleteSelected_RemovesVoxels) {
    // GIVEN: A selection of voxels
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // WHEN: Deleting selected voxels
    selectionManager->deleteSelected(voxelWorld.get());
    
    // THEN: Voxels should be air and selection should be cleared
    EXPECT_FALSE(selectionManager->hasSelection());
    
    auto* voxel = voxelWorld->getVoxel(WorldPos(0, 0, 0));
    ASSERT_NE(nullptr, voxel);
    EXPECT_EQ(VoxelType::Air, voxel->type);
}

TEST_F(SelectionManagerTest, DeleteSelected_WithNullWorld_DoesNothing) {
    // GIVEN: A selection
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // WHEN: Deleting with null world
    selectionManager->deleteSelected(nullptr);
    
    // THEN: Should not crash (and original voxels should still exist)
    auto* voxel = voxelWorld->getVoxel(WorldPos(0, 0, 0));
    ASSERT_NE(nullptr, voxel);
    EXPECT_EQ(VoxelType::Stone, voxel->type);
}

TEST_F(SelectionManagerTest, DeleteSelected_EmptySelection_DoesNothing) {
    // WHEN: Deleting with no selection
    selectionManager->deleteSelected(voxelWorld.get());
    
    // THEN: Should not crash
    EXPECT_FALSE(selectionManager->hasSelection());
}

// ============================================================================
// Move Selection Tests
// ============================================================================

TEST_F(SelectionManagerTest, MoveSelection_MovesVoxels) {
    // GIVEN: A selection at (0,0,0) to (1,1,1)
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // WHEN: Moving by (2, 0, 0)
    selectionManager->moveSelection(glm::ivec3(2, 0, 0), voxelWorld.get());
    
    // THEN: Old position should be air, new position should have voxels
    auto* oldVoxel = voxelWorld->getVoxel(WorldPos(0, 0, 0));
    auto* newVoxel = voxelWorld->getVoxel(WorldPos(2, 0, 0));
    
    ASSERT_NE(nullptr, oldVoxel);
    ASSERT_NE(nullptr, newVoxel);
    EXPECT_EQ(VoxelType::Air, oldVoxel->type);
    EXPECT_EQ(VoxelType::Stone, newVoxel->type);
}

TEST_F(SelectionManagerTest, MoveSelection_UpdatesSelectionBounds) {
    // GIVEN: A selection
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // WHEN: Moving selection
    glm::ivec3 delta(3, 3, 3);
    selectionManager->moveSelection(delta, voxelWorld.get());
    
    // THEN: Bounds should be updated
    glm::ivec3 min, max;
    bool hasBounds = selectionManager->getSelectionBounds(min, max);
    
    EXPECT_TRUE(hasBounds);
    EXPECT_EQ(glm::ivec3(3, 3, 3), min);
    EXPECT_EQ(glm::ivec3(4, 4, 4), max);
}

TEST_F(SelectionManagerTest, MoveSelection_WithNullWorld_DoesNothing) {
    // GIVEN: A selection
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // WHEN: Moving with null world
    selectionManager->moveSelection(glm::ivec3(1, 1, 1), nullptr);
    
    // THEN: Should not crash
    EXPECT_TRUE(selectionManager->hasSelection());
}

// ============================================================================
// Copy to Clipboard Tests
// ============================================================================

TEST_F(SelectionManagerTest, CopyToClipboard_CopiesSelection) {
    // GIVEN: A selection
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    size_t selectionSize = selectionManager->getSelectionSize();
    
    // WHEN: Copying to clipboard
    selectionManager->copyToClipboard(voxelWorld.get());
    
    // THEN: Clipboard should have data
    EXPECT_TRUE(selectionManager->hasClipboardData());
    EXPECT_EQ(selectionSize, selectionManager->getClipboardData().size());
}

TEST_F(SelectionManagerTest, CopyToClipboard_PreservesOriginalSelection) {
    // GIVEN: A selection
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // WHEN: Copying to clipboard
    selectionManager->copyToClipboard(voxelWorld.get());
    
    // THEN: Original selection should still exist
    EXPECT_TRUE(selectionManager->hasSelection());
    EXPECT_GT(selectionManager->getSelectionSize(), 0u);
    
    // AND: Original voxels should still be there
    auto* voxel = voxelWorld->getVoxel(WorldPos(0, 0, 0));
    ASSERT_NE(nullptr, voxel);
    EXPECT_EQ(VoxelType::Stone, voxel->type);
}

TEST_F(SelectionManagerTest, CopyToClipboard_WithNullWorld_DoesNothing) {
    // GIVEN: A selection
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // WHEN: Copying with null world
    selectionManager->copyToClipboard(nullptr);
    
    // THEN: Clipboard should remain empty
    EXPECT_FALSE(selectionManager->hasClipboardData());
}

TEST_F(SelectionManagerTest, CopyToClipboard_EmptySelection_DoesNothing) {
    // WHEN: Copying empty selection
    selectionManager->copyToClipboard(voxelWorld.get());
    
    // THEN: Clipboard should remain empty
    EXPECT_FALSE(selectionManager->hasClipboardData());
}

// ============================================================================
// Cut to Clipboard Tests
// ============================================================================

TEST_F(SelectionManagerTest, CutToClipboard_CopiesAndDeletes) {
    // GIVEN: A selection
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // WHEN: Cutting to clipboard
    selectionManager->cutToClipboard(voxelWorld.get());
    
    // THEN: Clipboard should have data
    EXPECT_TRUE(selectionManager->hasClipboardData());
    
    // AND: Original voxels should be air
    auto* voxel = voxelWorld->getVoxel(WorldPos(0, 0, 0));
    ASSERT_NE(nullptr, voxel);
    EXPECT_EQ(VoxelType::Air, voxel->type);
    
    // AND: Selection should be cleared
    EXPECT_FALSE(selectionManager->hasSelection());
}

TEST_F(SelectionManagerTest, CutToClipboard_WithNullWorld_DoesNothing) {
    // GIVEN: A selection
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // WHEN: Cutting with null world
    selectionManager->cutToClipboard(nullptr);
    
    // THEN: Should not crash and original voxels should remain
    auto* voxel = voxelWorld->getVoxel(WorldPos(0, 0, 0));
    ASSERT_NE(nullptr, voxel);
    EXPECT_EQ(VoxelType::Stone, voxel->type);
}

// ============================================================================
// Paste from Clipboard Tests
// ============================================================================

TEST_F(SelectionManagerTest, PasteFromClipboard_PlacesVoxels) {
    // GIVEN: Something in clipboard
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    selectionManager->copyToClipboard(voxelWorld.get());
    
    // Clear the area where we'll paste
    voxelWorld->setVoxel(WorldPos(5, 5, 5), Voxel(VoxelType::Air));
    
    // WHEN: Pasting at new location
    selectionManager->pasteFromClipboard(glm::ivec3(5, 5, 5), voxelWorld.get());
    
    // THEN: New location should have voxels
    auto* voxel = voxelWorld->getVoxel(WorldPos(5, 5, 5));
    ASSERT_NE(nullptr, voxel);
    EXPECT_EQ(VoxelType::Stone, voxel->type);
}

TEST_F(SelectionManagerTest, PasteFromClipboard_WithOffset_PlacesCorrectly) {
    // GIVEN: A 2x2x2 selection copied
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    selectionManager->copyToClipboard(voxelWorld.get());
    
    // WHEN: Pasting at offset
    selectionManager->pasteFromClipboard(glm::ivec3(3, 3, 3), voxelWorld.get());
    
    // THEN: All voxels should be placed with correct offset
    auto* voxel1 = voxelWorld->getVoxel(WorldPos(3, 3, 3));
    auto* voxel2 = voxelWorld->getVoxel(WorldPos(4, 4, 4));
    
    ASSERT_NE(nullptr, voxel1);
    ASSERT_NE(nullptr, voxel2);
    EXPECT_EQ(VoxelType::Stone, voxel1->type);
    EXPECT_EQ(VoxelType::Stone, voxel2->type);
}

TEST_F(SelectionManagerTest, PasteFromClipboard_EmptyClipboard_DoesNothing) {
    // WHEN: Pasting with empty clipboard
    selectionManager->pasteFromClipboard(glm::ivec3(5, 5, 5), voxelWorld.get());
    
    // THEN: Should not crash
    auto* voxel = voxelWorld->getVoxel(WorldPos(5, 5, 5));
    ASSERT_NE(nullptr, voxel);
    // Voxel at (5,5,5) should still be stone from initial setup
    EXPECT_EQ(VoxelType::Stone, voxel->type);
}

TEST_F(SelectionManagerTest, PasteFromClipboard_WithNullWorld_DoesNothing) {
    // GIVEN: Clipboard has data
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    selectionManager->copyToClipboard(voxelWorld.get());
    
    // WHEN: Pasting with null world
    selectionManager->pasteFromClipboard(glm::ivec3(5, 5, 5), nullptr);
    
    // THEN: Should not crash
    EXPECT_TRUE(selectionManager->hasClipboardData());
}

// ============================================================================
// Clear Selection Tests
// ============================================================================

TEST_F(SelectionManagerTest, ClearSelection_RemovesSelection) {
    // GIVEN: A selection exists
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    EXPECT_TRUE(selectionManager->hasSelection());
    
    // WHEN: Clearing selection
    selectionManager->clearSelection();
    
    // THEN: Selection should be gone
    EXPECT_FALSE(selectionManager->hasSelection());
    EXPECT_FALSE(selectionManager->isSelecting());
    EXPECT_EQ(0u, selectionManager->getSelectionSize());
}

TEST_F(SelectionManagerTest, ClearSelection_WhileSelecting_CancelsSelection) {
    // GIVEN: Currently selecting
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    
    EXPECT_TRUE(selectionManager->isSelecting());
    
    // WHEN: Clearing selection
    selectionManager->clearSelection();
    
    // THEN: Should not be selecting
    EXPECT_FALSE(selectionManager->isSelecting());
    EXPECT_FALSE(selectionManager->hasSelection());
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST_F(SelectionManagerTest, EdgeCase_SingleVoxelSelection) {
    // WHEN: Selecting a single voxel (start == end)
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // THEN: Should select exactly 1 voxel
    EXPECT_EQ(1u, selectionManager->getSelectionSize());
}

TEST_F(SelectionManagerTest, EdgeCase_LargeSelection) {
    // WHEN: Selecting all 10x10x10 voxels
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(9.0f, 9.0f, 9.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // THEN: Should select all 1000 voxels
    EXPECT_EQ(1000u, selectionManager->getSelectionSize());
}

TEST_F(SelectionManagerTest, EdgeCase_NegativeCoordinates) {
    // GIVEN: Place some voxels at negative coordinates
    // Need to load the chunk first
    voxelWorld->loadChunk(ChunkPos(-1, -1));
    voxelWorld->setVoxel(WorldPos(-5, 5, -5), Voxel(VoxelType::Stone));
    voxelWorld->setVoxel(WorldPos(-4, 5, -4), Voxel(VoxelType::Stone));
    
    // WHEN: Selecting at negative coordinates
    selectionManager->startSelection(glm::vec3(-6.0f, 4.0f, -6.0f));
    selectionManager->updateSelection(glm::vec3(-3.0f, 6.0f, -3.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // THEN: Should select voxels at negative coordinates
    EXPECT_GT(selectionManager->getSelectionSize(), 0u);
}

TEST_F(SelectionManagerTest, EdgeCase_SelectionAcrossChunks) {
    // GIVEN: Voxels in multiple chunks
    voxelWorld->setVoxel(WorldPos(20, 0, 0), Voxel(VoxelType::Stone));
    
    // WHEN: Selecting across chunk boundaries (chunk size is 16)
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(20.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // THEN: Should select voxels from both chunks
    EXPECT_GT(selectionManager->getSelectionSize(), 0u);
}

TEST_F(SelectionManagerTest, EdgeCase_EmptyRegionSelection) {
    // GIVEN: A region with only air
    for (int x = 15; x < 20; ++x) {
        for (int y = 15; y < 20; ++y) {
            for (int z = 15; z < 20; ++z) {
                voxelWorld->setVoxel(WorldPos(x, y, z), Voxel(VoxelType::Air));
            }
        }
    }
    
    // WHEN: Selecting the empty region
    selectionManager->startSelection(glm::vec3(15.0f, 15.0f, 15.0f));
    selectionManager->updateSelection(glm::vec3(19.0f, 19.0f, 19.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // THEN: Should have 0 selected voxels (air is not selected)
    EXPECT_EQ(0u, selectionManager->getSelectionSize());
}

TEST_F(SelectionManagerTest, EdgeCase_MultipleOperations) {
    // Test a sequence of operations to ensure state management is correct
    
    // 1. Create selection
    selectionManager->startSelection(glm::vec3(0.0f, 0.0f, 0.0f));
    selectionManager->updateSelection(glm::vec3(1.0f, 1.0f, 1.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    EXPECT_TRUE(selectionManager->hasSelection());
    
    // 2. Copy to clipboard
    selectionManager->copyToClipboard(voxelWorld.get());
    EXPECT_TRUE(selectionManager->hasClipboardData());
    
    // 3. Clear selection
    selectionManager->clearSelection();
    EXPECT_FALSE(selectionManager->hasSelection());
    EXPECT_TRUE(selectionManager->hasClipboardData()); // Clipboard should persist
    
    // 4. Paste
    selectionManager->pasteFromClipboard(glm::ivec3(5, 5, 5), voxelWorld.get());
    auto* voxel = voxelWorld->getVoxel(WorldPos(5, 5, 5));
    ASSERT_NE(nullptr, voxel);
    EXPECT_EQ(VoxelType::Stone, voxel->type);
}

// ============================================================================
// Selection Mode Tests
// ============================================================================

TEST_F(SelectionManagerTest, DefaultSelectionMode_IsBox) {
    EXPECT_EQ(SelectionMode::Box, selectionManager->getSelectionMode());
}

TEST_F(SelectionManagerTest, SetSelectionMode_Brush) {
    selectionManager->setSelectionMode(SelectionMode::Brush);
    EXPECT_EQ(SelectionMode::Brush, selectionManager->getSelectionMode());
}

TEST_F(SelectionManagerTest, SetSelectionMode_Wand) {
    selectionManager->setSelectionMode(SelectionMode::Wand);
    EXPECT_EQ(SelectionMode::Wand, selectionManager->getSelectionMode());
}

TEST_F(SelectionManagerTest, BrushRadius_Default) {
    EXPECT_EQ(3, selectionManager->getBrushRadius());
}

TEST_F(SelectionManagerTest, BrushRadius_SetValid) {
    selectionManager->setBrushRadius(5);
    EXPECT_EQ(5, selectionManager->getBrushRadius());
}

TEST_F(SelectionManagerTest, BrushRadius_ClampedMin) {
    selectionManager->setBrushRadius(0);
    EXPECT_EQ(1, selectionManager->getBrushRadius());
}

TEST_F(SelectionManagerTest, BrushRadius_ClampedMax) {
    selectionManager->setBrushRadius(100);
    EXPECT_EQ(16, selectionManager->getBrushRadius());
}

TEST_F(SelectionManagerTest, WandLimit_Default) {
    EXPECT_EQ(1000, selectionManager->getWandLimit());
}

TEST_F(SelectionManagerTest, WandLimit_SetValid) {
    selectionManager->setWandLimit(500);
    EXPECT_EQ(500, selectionManager->getWandLimit());
}

TEST_F(SelectionManagerTest, WandLimit_ClampedMin) {
    selectionManager->setWandLimit(0);
    EXPECT_EQ(1, selectionManager->getWandLimit());
}

TEST_F(SelectionManagerTest, WandLimit_ClampedMax) {
    selectionManager->setWandLimit(20000);
    EXPECT_EQ(10000, selectionManager->getWandLimit());
}

TEST_F(SelectionManagerTest, BrushSelection_SelectsVoxelsInSphere) {
    selectionManager->setSelectionMode(SelectionMode::Brush);
    selectionManager->setBrushRadius(2);
    
    // Select around center of the test voxel block
    selectionManager->startSelection(glm::vec3(5.0f, 5.0f, 5.0f));
    selectionManager->updateSelection(glm::vec3(5.0f, 5.0f, 5.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // Should have selected voxels in a sphere of radius 2
    EXPECT_GT(selectionManager->getSelectionSize(), 0u);
    // Sphere of radius 2 in a filled volume should have ~33 voxels (4/3*pi*2^3 ≈ 33.5)
    // But many are at fractional boundaries, so check a reasonable range
    EXPECT_LE(selectionManager->getSelectionSize(), 100u);
}

TEST_F(SelectionManagerTest, BrushSelection_EmptyArea) {
    selectionManager->setSelectionMode(SelectionMode::Brush);
    selectionManager->setBrushRadius(2);
    
    // Select in an area with no voxels
    selectionManager->startSelection(glm::vec3(50.0f, 50.0f, 50.0f));
    selectionManager->updateSelection(glm::vec3(50.0f, 50.0f, 50.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    EXPECT_EQ(0u, selectionManager->getSelectionSize());
}

TEST_F(SelectionManagerTest, WandSelection_SelectsConnectedSameType) {
    selectionManager->setSelectionMode(SelectionMode::Wand);
    
    // Select starting from a stone voxel - should flood fill to all connected stone
    selectionManager->startSelection(glm::vec3(5.0f, 5.0f, 5.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // All 1000 stone voxels in the 10x10x10 cube should be connected
    // But wand limit defaults to 1000, so it will select up to 1000
    EXPECT_GT(selectionManager->getSelectionSize(), 0u);
    EXPECT_LE(selectionManager->getSelectionSize(), 1000u);
}

TEST_F(SelectionManagerTest, WandSelection_EmptyArea) {
    selectionManager->setSelectionMode(SelectionMode::Wand);
    
    // Select in an area with no voxels (air)
    selectionManager->startSelection(glm::vec3(50.0f, 50.0f, 50.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    EXPECT_EQ(0u, selectionManager->getSelectionSize());
}

TEST_F(SelectionManagerTest, WandSelection_RespectsLimit) {
    selectionManager->setSelectionMode(SelectionMode::Wand);
    selectionManager->setWandLimit(5);
    
    // Select starting from a stone voxel
    selectionManager->startSelection(glm::vec3(5.0f, 5.0f, 5.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // Should select at most 5 voxels
    EXPECT_LE(selectionManager->getSelectionSize(), 5u);
    EXPECT_GT(selectionManager->getSelectionSize(), 0u);
}

TEST_F(SelectionManagerTest, WandSelection_OnlySelectsSameType) {
    // Place a different type voxel to create a boundary
    voxelWorld->setVoxel(WorldPos(5, 5, 5), Voxel(VoxelType::Dirt));
    
    selectionManager->setSelectionMode(SelectionMode::Wand);
    
    // Select starting from the dirt voxel - should only get that one
    // since it's surrounded by stone (different type)
    selectionManager->startSelection(glm::vec3(5.0f, 5.0f, 5.0f));
    selectionManager->finalizeSelection(voxelWorld.get());
    
    // The dirt voxel is surrounded by stone, so only 1 should be selected
    EXPECT_EQ(1u, selectionManager->getSelectionSize());
}

} // namespace fresh
