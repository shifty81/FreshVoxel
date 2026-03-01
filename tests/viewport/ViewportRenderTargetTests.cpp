/**
 * @file ViewportRenderTargetTests.cpp
 * @brief Tests for ViewportRenderTarget
 *
 * Validates the render target abstraction per ENGINE.md:
 * "Nothing renders unless a ViewportRenderTarget is bound."
 */

#include <gtest/gtest.h>
#include "viewport/ViewportRenderTarget.h"

namespace fresh {

class ViewportRenderTargetTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderTarget = std::make_unique<ViewportRenderTarget>();
    }

    void TearDown() override {
        renderTarget.reset();
    }

    std::unique_ptr<ViewportRenderTarget> renderTarget;
};

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(ViewportRenderTargetTest, DefaultState_Uninitialized) {
    EXPECT_EQ(renderTarget->getState(), RenderTargetState::Uninitialized);
    EXPECT_FALSE(renderTarget->isReady());
}

TEST_F(ViewportRenderTargetTest, DefaultState_ZeroDimensions) {
    EXPECT_EQ(renderTarget->getWidth(), 0);
    EXPECT_EQ(renderTarget->getHeight(), 0);
}

TEST_F(ViewportRenderTargetTest, DefaultState_NullHandles) {
    EXPECT_EQ(renderTarget->getRenderContext(), nullptr);
    EXPECT_EQ(renderTarget->getWindowHandle(), nullptr);
}

TEST_F(ViewportRenderTargetTest, InitializeWithNullContext_Fails) {
    bool result = renderTarget->initialize(nullptr, nullptr);
    EXPECT_FALSE(result);
    EXPECT_EQ(renderTarget->getState(), RenderTargetState::Invalid);
}

TEST_F(ViewportRenderTargetTest, DefaultAspectRatio_16by9) {
    // With zero height, should return default 16:9
    float aspect = renderTarget->getAspectRatio();
    EXPECT_FLOAT_EQ(aspect, 16.0f / 9.0f);
}

// ============================================================================
// Bind/Unbind Tests (without real render context)
// ============================================================================

TEST_F(ViewportRenderTargetTest, BindWhenUninitialized_Fails) {
    bool result = renderTarget->bind();
    EXPECT_FALSE(result);
}

TEST_F(ViewportRenderTargetTest, ResizeWithInvalidDimensions_Fails) {
    EXPECT_FALSE(renderTarget->resize(0, 100));
    EXPECT_FALSE(renderTarget->resize(100, 0));
    EXPECT_FALSE(renderTarget->resize(-1, 100));
    EXPECT_FALSE(renderTarget->resize(100, -1));
}

TEST_F(ViewportRenderTargetTest, ResizeWithoutContext_Fails) {
    EXPECT_FALSE(renderTarget->resize(800, 600));
}

// ============================================================================
// Shutdown Tests
// ============================================================================

TEST_F(ViewportRenderTargetTest, ShutdownWhenUninitialized_NoOp) {
    // Should not crash
    renderTarget->shutdown();
    EXPECT_EQ(renderTarget->getState(), RenderTargetState::Uninitialized);
}

TEST_F(ViewportRenderTargetTest, PresentWithoutContext_NoOp) {
    // Should not crash
    renderTarget->present();
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

TEST_F(ViewportRenderTargetTest, MoveConstructor_TransfersState) {
    // Create and move a render target
    ViewportRenderTarget original;
    // original is uninitialized, so move should work cleanly
    ViewportRenderTarget moved(std::move(original));
    EXPECT_EQ(moved.getState(), RenderTargetState::Uninitialized);
}

TEST_F(ViewportRenderTargetTest, MoveAssignment_TransfersState) {
    ViewportRenderTarget original;
    ViewportRenderTarget target;
    target = std::move(original);
    EXPECT_EQ(target.getState(), RenderTargetState::Uninitialized);
}

} // namespace fresh
