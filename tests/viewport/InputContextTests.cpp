/**
 * @file InputContextTests.cpp
 * @brief Tests for InputContext
 *
 * Validates the viewport-owned input routing system per ENGINE.md:
 * "Input is never global — Input routes through viewport-owned InputContext."
 */

#include <gtest/gtest.h>
#include "viewport/InputContext.h"
#include <GLFW/glfw3.h>

namespace fresh {

class InputContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        inputContext = std::make_unique<InputContext>();
        inputContext->initialize();
    }

    void TearDown() override {
        inputContext.reset();
    }

    std::unique_ptr<InputContext> inputContext;
};

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(InputContextTest, DefaultState_NotFocused) {
    EXPECT_FALSE(inputContext->isFocused());
}

TEST_F(InputContextTest, DefaultState_GameMode) {
    EXPECT_EQ(inputContext->getInputMode(), InputMode::GameMode);
}

TEST_F(InputContextTest, DefaultState_NoMouseDelta) {
    glm::vec2 delta = inputContext->getMouseDelta();
    EXPECT_FLOAT_EQ(delta.x, 0.0f);
    EXPECT_FLOAT_EQ(delta.y, 0.0f);
}

// ============================================================================
// Focus Tests
// ============================================================================

TEST_F(InputContextTest, SetFocused_True) {
    inputContext->setFocused(true);
    EXPECT_TRUE(inputContext->isFocused());
}

TEST_F(InputContextTest, SetFocused_False) {
    inputContext->setFocused(true);
    inputContext->setFocused(false);
    EXPECT_FALSE(inputContext->isFocused());
}

// ============================================================================
// Key Event Tests
// ============================================================================

TEST_F(InputContextTest, KeyPress_WhenFocused_Registered) {
    inputContext->setFocused(true);
    inputContext->processKeyEvent(GLFW_KEY_W, true);
    EXPECT_TRUE(inputContext->isKeyPressed(GLFW_KEY_W));
}

TEST_F(InputContextTest, KeyPress_WhenNotFocused_Ignored) {
    inputContext->setFocused(false);
    inputContext->processKeyEvent(GLFW_KEY_W, true);
    EXPECT_FALSE(inputContext->isKeyPressed(GLFW_KEY_W));
}

TEST_F(InputContextTest, KeyRelease_ClearsState) {
    inputContext->setFocused(true);
    inputContext->processKeyEvent(GLFW_KEY_W, true);
    inputContext->processKeyEvent(GLFW_KEY_W, false);
    EXPECT_FALSE(inputContext->isKeyPressed(GLFW_KEY_W));
}

TEST_F(InputContextTest, KeyJustPressed_TrueOnFirstFrame) {
    inputContext->setFocused(true);
    inputContext->processKeyEvent(GLFW_KEY_W, true);
    EXPECT_TRUE(inputContext->isKeyJustPressed(GLFW_KEY_W));
}

TEST_F(InputContextTest, KeyJustPressed_ClearedAfterUpdate) {
    inputContext->setFocused(true);
    inputContext->processKeyEvent(GLFW_KEY_W, true);
    EXPECT_TRUE(inputContext->isKeyJustPressed(GLFW_KEY_W));

    inputContext->update();
    EXPECT_FALSE(inputContext->isKeyJustPressed(GLFW_KEY_W));
    // Key should still be held
    EXPECT_TRUE(inputContext->isKeyPressed(GLFW_KEY_W));
}

// ============================================================================
// Mouse Event Tests
// ============================================================================

TEST_F(InputContextTest, MouseMove_WhenFocused_UpdatesPosition) {
    inputContext->setFocused(true);
    inputContext->processMouseMove(100.0f, 200.0f);
    glm::vec2 pos = inputContext->getMousePosition();
    EXPECT_FLOAT_EQ(pos.x, 100.0f);
    EXPECT_FLOAT_EQ(pos.y, 200.0f);
}

TEST_F(InputContextTest, MouseMove_WhenNotFocused_Ignored) {
    inputContext->setFocused(false);
    inputContext->processMouseMove(100.0f, 200.0f);
    glm::vec2 pos = inputContext->getMousePosition();
    EXPECT_FLOAT_EQ(pos.x, 0.0f);
    EXPECT_FLOAT_EQ(pos.y, 0.0f);
}

TEST_F(InputContextTest, MouseDelta_CalculatedCorrectly) {
    inputContext->setFocused(true);
    inputContext->processMouseMove(100.0f, 100.0f);
    inputContext->processMouseMove(150.0f, 120.0f);
    glm::vec2 delta = inputContext->getMouseDelta();
    EXPECT_FLOAT_EQ(delta.x, 50.0f);
    EXPECT_FLOAT_EQ(delta.y, 20.0f);
}

TEST_F(InputContextTest, MouseDelta_ClearedAfterUpdate) {
    inputContext->setFocused(true);
    inputContext->processMouseMove(100.0f, 100.0f);
    inputContext->processMouseMove(150.0f, 120.0f);
    inputContext->update();
    glm::vec2 delta = inputContext->getMouseDelta();
    EXPECT_FLOAT_EQ(delta.x, 0.0f);
    EXPECT_FLOAT_EQ(delta.y, 0.0f);
}

TEST_F(InputContextTest, MouseButton_WhenFocused_Registered) {
    inputContext->setFocused(true);
    inputContext->processMouseButton(0, true);
    EXPECT_TRUE(inputContext->isMouseButtonPressed(0));
}

TEST_F(InputContextTest, MouseButton_WhenNotFocused_Ignored) {
    inputContext->setFocused(false);
    inputContext->processMouseButton(0, true);
    EXPECT_FALSE(inputContext->isMouseButtonPressed(0));
}

TEST_F(InputContextTest, MouseButtonJustPressed_ClearedAfterUpdate) {
    inputContext->setFocused(true);
    inputContext->processMouseButton(0, true);
    EXPECT_TRUE(inputContext->isMouseButtonJustPressed(0));

    inputContext->update();
    EXPECT_FALSE(inputContext->isMouseButtonJustPressed(0));
    EXPECT_TRUE(inputContext->isMouseButtonPressed(0));
}

// ============================================================================
// Action Mapping Tests
// ============================================================================

TEST_F(InputContextTest, DefaultBindings_MoveForward) {
    inputContext->setFocused(true);
    inputContext->processKeyEvent(GLFW_KEY_W, true);
    EXPECT_TRUE(inputContext->isActionActive(InputAction::MoveForward));
}

TEST_F(InputContextTest, DefaultBindings_Jump) {
    inputContext->setFocused(true);
    inputContext->processKeyEvent(GLFW_KEY_SPACE, true);
    EXPECT_TRUE(inputContext->isActionActive(InputAction::Jump));
}

TEST_F(InputContextTest, CustomBinding_OverridesDefault) {
    inputContext->setFocused(true);
    inputContext->setKeyBinding(InputAction::MoveForward, GLFW_KEY_UP);
    inputContext->processKeyEvent(GLFW_KEY_UP, true);
    EXPECT_TRUE(inputContext->isActionActive(InputAction::MoveForward));
}

TEST_F(InputContextTest, ActionJustPressed_WorksWithBindings) {
    inputContext->setFocused(true);
    inputContext->processKeyEvent(GLFW_KEY_W, true);
    EXPECT_TRUE(inputContext->isActionJustPressed(InputAction::MoveForward));

    inputContext->update();
    EXPECT_FALSE(inputContext->isActionJustPressed(InputAction::MoveForward));
    EXPECT_TRUE(inputContext->isActionActive(InputAction::MoveForward));
}

// ============================================================================
// Input Mode Tests
// ============================================================================

TEST_F(InputContextTest, SetInputMode_UIMode) {
    inputContext->setInputMode(InputMode::UIMode);
    EXPECT_EQ(inputContext->getInputMode(), InputMode::UIMode);
}

TEST_F(InputContextTest, SetInputMode_BuildMode) {
    inputContext->setInputMode(InputMode::BuildMode);
    EXPECT_EQ(inputContext->getInputMode(), InputMode::BuildMode);
}

} // namespace fresh
