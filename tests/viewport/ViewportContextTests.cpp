/**
 * @file ViewportContextTests.cpp
 * @brief Tests for ViewportContext
 *
 * Validates the viewport system per ENGINE.md:
 * "The viewport is the atomic unit of the engine.
 *  Everything routes through ViewportContext."
 */

#include <gtest/gtest.h>
#include "viewport/ViewportContext.h"
#include "gameplay/Camera.h"

namespace fresh {

class ViewportContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        viewport = std::make_unique<ViewportContext>("Test Viewport", ViewportType::Perspective3D);
    }

    void TearDown() override {
        viewport.reset();
    }

    std::unique_ptr<ViewportContext> viewport;
};

// ============================================================================
// Construction Tests
// ============================================================================

TEST_F(ViewportContextTest, Constructor_SetsName) {
    EXPECT_EQ(viewport->getName(), "Test Viewport");
}

TEST_F(ViewportContextTest, Constructor_SetsType) {
    EXPECT_EQ(viewport->getType(), ViewportType::Perspective3D);
}

TEST_F(ViewportContextTest, Constructor_CreatesCamera) {
    EXPECT_NE(viewport->getCamera(), nullptr);
}

TEST_F(ViewportContextTest, Constructor_CameraInPerspectiveMode) {
    Camera* cam = viewport->getCamera();
    ASSERT_NE(cam, nullptr);
    EXPECT_EQ(cam->getCameraMode(), CameraMode::Perspective3D);
}

TEST_F(ViewportContextTest, Constructor_NotInitialized) {
    EXPECT_FALSE(viewport->isReady());
}

TEST_F(ViewportContextTest, Constructor_NoWorld) {
    EXPECT_EQ(viewport->getWorld(), nullptr);
}

TEST_F(ViewportContextTest, Constructor_NotFocused) {
    EXPECT_FALSE(viewport->isFocused());
}

// ============================================================================
// Viewport Type Tests
// ============================================================================

TEST_F(ViewportContextTest, TopDown2D_OrthoCamera) {
    auto topDown = std::make_unique<ViewportContext>("TopDown", ViewportType::TopDown2D);
    Camera* cam = topDown->getCamera();
    ASSERT_NE(cam, nullptr);
    EXPECT_EQ(cam->getCameraMode(), CameraMode::OrthographicTopDown);
}

TEST_F(ViewportContextTest, Isometric_OrthoCamera) {
    auto iso = std::make_unique<ViewportContext>("Iso", ViewportType::Isometric);
    Camera* cam = iso->getCamera();
    ASSERT_NE(cam, nullptr);
    EXPECT_EQ(cam->getCameraMode(), CameraMode::OrthographicTopDown);
    // Isometric should have rotated camera
    EXPECT_FLOAT_EQ(cam->getPitch(), -45.0f);
    EXPECT_FLOAT_EQ(cam->getYaw(), -45.0f);
}

TEST_F(ViewportContextTest, SetType_ChangesCamera) {
    viewport->setType(ViewportType::TopDown2D);
    EXPECT_EQ(viewport->getType(), ViewportType::TopDown2D);
    Camera* cam = viewport->getCamera();
    ASSERT_NE(cam, nullptr);
    EXPECT_EQ(cam->getCameraMode(), CameraMode::OrthographicTopDown);
}

// ============================================================================
// Initialize Tests (without real render context)
// ============================================================================

TEST_F(ViewportContextTest, InitializeWithNullContext_Fails) {
    bool result = viewport->initialize(nullptr, nullptr);
    EXPECT_FALSE(result);
    EXPECT_FALSE(viewport->isReady());
}

TEST_F(ViewportContextTest, BeginFrameWithoutInit_Fails) {
    EXPECT_FALSE(viewport->beginFrame());
}

TEST_F(ViewportContextTest, ResizeWithoutInit_Fails) {
    EXPECT_FALSE(viewport->resize(800, 600));
}

// ============================================================================
// World Reference Tests
// ============================================================================

TEST_F(ViewportContextTest, SetWorld_Accepted) {
    // Use a nullptr to test the setter (real world requires initialization)
    VoxelWorld* fakeWorld = reinterpret_cast<VoxelWorld*>(0x1234);
    viewport->setWorld(fakeWorld);
    EXPECT_EQ(viewport->getWorld(), fakeWorld);
}

TEST_F(ViewportContextTest, SetWorld_Null) {
    VoxelWorld* fakeWorld = reinterpret_cast<VoxelWorld*>(0x1234);
    viewport->setWorld(fakeWorld);
    viewport->setWorld(nullptr);
    EXPECT_EQ(viewport->getWorld(), nullptr);
}

// ============================================================================
// Focus Tests
// ============================================================================

TEST_F(ViewportContextTest, SetFocused_DelegatesToInputContext) {
    viewport->setFocused(true);
    EXPECT_TRUE(viewport->isFocused());
    EXPECT_TRUE(viewport->getInputContext().isFocused());
}

TEST_F(ViewportContextTest, SetFocused_False) {
    viewport->setFocused(true);
    viewport->setFocused(false);
    EXPECT_FALSE(viewport->isFocused());
}

// ============================================================================
// InputContext Access Tests
// ============================================================================

TEST_F(ViewportContextTest, InputContext_KeyEvents_RouteThrough) {
    viewport->setFocused(true);
    viewport->getInputContext().processKeyEvent(87, true); // 'W'
    EXPECT_TRUE(viewport->getInputContext().isKeyPressed(87));
}

TEST_F(ViewportContextTest, InputContext_NotFocused_IgnoresEvents) {
    viewport->setFocused(false);
    viewport->getInputContext().processKeyEvent(87, true);
    EXPECT_FALSE(viewport->getInputContext().isKeyPressed(87));
}

// ============================================================================
// Matrix Tests
// ============================================================================

TEST_F(ViewportContextTest, GetViewMatrix_ReturnsValidMatrix) {
    glm::mat4 view = viewport->getViewMatrix();
    // Should not be identity (camera has default position)
    EXPECT_NE(view, glm::mat4(0.0f));
}

TEST_F(ViewportContextTest, GetProjectionMatrix_ReturnsValidMatrix) {
    glm::mat4 proj = viewport->getProjectionMatrix();
    // Should not be zero matrix
    EXPECT_NE(proj, glm::mat4(0.0f));
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

TEST_F(ViewportContextTest, MoveConstructor_TransfersOwnership) {
    ViewportContext original("Original", ViewportType::Perspective3D);
    Camera* origCamera = original.getCamera();
    ASSERT_NE(origCamera, nullptr);

    ViewportContext moved(std::move(original));
    EXPECT_EQ(moved.getName(), "Original");
    EXPECT_EQ(moved.getType(), ViewportType::Perspective3D);
    EXPECT_NE(moved.getCamera(), nullptr);
}

TEST_F(ViewportContextTest, MoveAssignment_TransfersOwnership) {
    ViewportContext original("Original", ViewportType::Perspective3D);
    ViewportContext target("Target", ViewportType::TopDown2D);

    target = std::move(original);
    EXPECT_EQ(target.getName(), "Original");
    EXPECT_EQ(target.getType(), ViewportType::Perspective3D);
}

// ============================================================================
// Shutdown Tests
// ============================================================================

TEST_F(ViewportContextTest, Shutdown_WhenNotInitialized_NoOp) {
    viewport->shutdown();
    EXPECT_FALSE(viewport->isReady());
}

TEST_F(ViewportContextTest, EndFrame_WhenNotInitialized_NoOp) {
    // Should not crash
    viewport->endFrame();
}

} // namespace fresh
