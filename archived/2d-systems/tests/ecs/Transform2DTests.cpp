/**
 * @file Transform2DTests.cpp
 * @brief Unit tests for Transform2D component
 */

#include <gtest/gtest.h>
#include "ecs/Transform2DComponent.h"
#include <glm/glm.hpp>
#include <cmath>

using namespace fresh::ecs;

/**
 * Test fixture for Transform2D tests
 */
class Transform2DTest : public ::testing::Test {
protected:
    void SetUp() override {
        transform = std::make_unique<Transform2DComponent>();
    }

    void TearDown() override {
        transform.reset();
    }

    std::unique_ptr<Transform2DComponent> transform;
};

/**
 * Test default initialization
 */
TEST_F(Transform2DTest, DefaultConstructor_InitializesCorrectly) {
    EXPECT_EQ(transform->position.x, 0.0f);
    EXPECT_EQ(transform->position.y, 0.0f);
    EXPECT_EQ(transform->rotation, 0.0f);
    EXPECT_EQ(transform->scale.x, 1.0f);
    EXPECT_EQ(transform->scale.y, 1.0f);
    EXPECT_EQ(transform->layer, 0);
    EXPECT_EQ(transform->depth, 0.0f);
}

/**
 * Test parametrized constructor
 */
TEST_F(Transform2DTest, ParameterizedConstructor_SetsValuesCorrectly) {
    glm::vec2 pos(10.0f, 20.0f);
    float rot = glm::radians(45.0f);
    glm::vec2 scl(2.0f, 3.0f);
    
    Transform2DComponent t(pos, rot, scl);
    
    EXPECT_EQ(t.position, pos);
    EXPECT_FLOAT_EQ(t.rotation, rot);
    EXPECT_EQ(t.scale, scl);
}

/**
 * Test rotation degrees conversion
 */
TEST_F(Transform2DTest, RotationDegrees_ConversionWorks) {
    transform->setRotationDegrees(90.0f);
    
    EXPECT_NEAR(transform->getRotationDegrees(), 90.0f, 0.001f);
    EXPECT_NEAR(transform->rotation, glm::radians(90.0f), 0.001f);
}

/**
 * Test forward vector calculation
 */
TEST_F(Transform2DTest, GetForward_CalculatesCorrectly) {
    // Rotation = 0 should point right (1, 0)
    transform->rotation = 0.0f;
    glm::vec2 forward = transform->getForward();
    EXPECT_NEAR(forward.x, 1.0f, 0.001f);
    EXPECT_NEAR(forward.y, 0.0f, 0.001f);
    
    // Rotation = 90 degrees should point up (0, 1)
    transform->rotation = glm::radians(90.0f);
    forward = transform->getForward();
    EXPECT_NEAR(forward.x, 0.0f, 0.001f);
    EXPECT_NEAR(forward.y, 1.0f, 0.001f);
}

/**
 * Test right vector calculation
 */
TEST_F(Transform2DTest, GetRight_CalculatesCorrectly) {
    // Rotation = 0 should have right pointing down (0, -1)
    transform->rotation = 0.0f;
    glm::vec2 right = transform->getRight();
    EXPECT_NEAR(right.x, 0.0f, 0.001f);
    EXPECT_NEAR(right.y, -1.0f, 0.001f);
}

/**
 * Test transform matrix generation
 */
TEST_F(Transform2DTest, GetTransformMatrix_GeneratesValidMatrix) {
    transform->position = glm::vec2(10.0f, 20.0f);
    transform->rotation = glm::radians(45.0f);
    transform->scale = glm::vec2(2.0f, 3.0f);
    
    glm::mat3 matrix = transform->getTransformMatrix();
    
    // Matrix should not be identity
    EXPECT_NE(matrix[0][0], 1.0f);
}

/**
 * Test point transformation
 */
TEST_F(Transform2DTest, TransformPoint_TransformsCorrectly) {
    transform->position = glm::vec2(10.0f, 20.0f);
    transform->rotation = 0.0f;
    transform->scale = glm::vec2(1.0f, 1.0f);
    
    glm::vec2 localPoint(5.0f, 0.0f);
    glm::vec2 worldPoint = transform->transformPoint(localPoint);
    
    EXPECT_NEAR(worldPoint.x, 15.0f, 0.001f);
    EXPECT_NEAR(worldPoint.y, 20.0f, 0.001f);
}

/**
 * Test inverse transformation
 */
TEST_F(Transform2DTest, InverseTransformPoint_ReversesTransform) {
    transform->position = glm::vec2(10.0f, 20.0f);
    transform->rotation = 0.0f;
    transform->scale = glm::vec2(1.0f, 1.0f);
    
    glm::vec2 localPoint(5.0f, 3.0f);
    glm::vec2 worldPoint = transform->transformPoint(localPoint);
    glm::vec2 backToLocal = transform->inverseTransformPoint(worldPoint);
    
    EXPECT_NEAR(backToLocal.x, localPoint.x, 0.001f);
    EXPECT_NEAR(backToLocal.y, localPoint.y, 0.001f);
}

/**
 * Test cloning
 */
TEST_F(Transform2DTest, Clone_CreatesIdenticalCopy) {
    transform->position = glm::vec2(10.0f, 20.0f);
    transform->rotation = glm::radians(45.0f);
    transform->scale = glm::vec2(2.0f, 3.0f);
    transform->layer = 5;
    transform->depth = 1.5f;
    
    auto* cloned = dynamic_cast<Transform2DComponent*>(transform->clone());
    ASSERT_NE(cloned, nullptr);
    
    EXPECT_EQ(cloned->position, transform->position);
    EXPECT_FLOAT_EQ(cloned->rotation, transform->rotation);
    EXPECT_EQ(cloned->scale, transform->scale);
    EXPECT_EQ(cloned->layer, transform->layer);
    EXPECT_FLOAT_EQ(cloned->depth, transform->depth);
    
    delete cloned;
}
