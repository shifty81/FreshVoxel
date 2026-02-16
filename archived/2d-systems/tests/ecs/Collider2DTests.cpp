/**
 * @file Collider2DTests.cpp
 * @brief Unit tests for Collider2D component
 */

#include <gtest/gtest.h>
#include "ecs/Collider2DComponent.h"
#include <glm/glm.hpp>

using namespace fresh::ecs;

/**
 * Test fixture for Collider2D tests
 */
class Collider2DTest : public ::testing::Test {
protected:
    void SetUp() override {
        collider = std::make_unique<Collider2DComponent>();
    }

    void TearDown() override {
        collider.reset();
    }

    std::unique_ptr<Collider2DComponent> collider;
};

/**
 * Test box collider creation
 */
TEST_F(Collider2DTest, CreateBox_InitializesCorrectly) {
    auto box = Collider2DComponent::createBox(glm::vec2(2.0f, 3.0f));
    
    EXPECT_EQ(box.shapeType, Collider2DComponent::ShapeType::Box);
    EXPECT_EQ(box.boxSize.x, 1.0f); // Half-extents
    EXPECT_EQ(box.boxSize.y, 1.5f);
    EXPECT_TRUE(box.enabled);
    EXPECT_FALSE(box.isTrigger);
}

/**
 * Test circle collider creation
 */
TEST_F(Collider2DTest, CreateCircle_InitializesCorrectly) {
    auto circle = Collider2DComponent::createCircle(5.0f);
    
    EXPECT_EQ(circle.shapeType, Collider2DComponent::ShapeType::Circle);
    EXPECT_FLOAT_EQ(circle.circleRadius, 5.0f);
    EXPECT_TRUE(circle.enabled);
}

/**
 * Test polygon collider creation
 */
TEST_F(Collider2DTest, CreatePolygon_InitializesCorrectly) {
    std::vector<glm::vec2> vertices = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f)
    };
    
    auto polygon = Collider2DComponent::createPolygon(vertices);
    
    EXPECT_EQ(polygon.shapeType, Collider2DComponent::ShapeType::Polygon);
    EXPECT_EQ(polygon.vertices.size(), 4);
    EXPECT_TRUE(polygon.enabled);
}

/**
 * Test edge collider creation
 */
TEST_F(Collider2DTest, CreateEdge_InitializesCorrectly) {
    glm::vec2 start(0.0f, 0.0f);
    glm::vec2 end(10.0f, 0.0f);
    auto edge = Collider2DComponent::createEdge(start, end);
    
    EXPECT_EQ(edge.shapeType, Collider2DComponent::ShapeType::Edge);
    EXPECT_EQ(edge.edgeStart, start);
    EXPECT_EQ(edge.edgeEnd, end);
    EXPECT_TRUE(edge.enabled);
}

/**
 * Test box AABB calculation
 */
TEST_F(Collider2DTest, BoxAABB_CalculatesCorrectly) {
    auto box = Collider2DComponent::createBox(glm::vec2(2.0f, 2.0f));
    glm::vec2 position(5.0f, 5.0f);
    float rotation = 0.0f;
    
    auto [min, max] = box.getAABB(position, rotation);
    
    EXPECT_FLOAT_EQ(min.x, 4.0f);
    EXPECT_FLOAT_EQ(min.y, 4.0f);
    EXPECT_FLOAT_EQ(max.x, 6.0f);
    EXPECT_FLOAT_EQ(max.y, 6.0f);
}

/**
 * Test circle AABB calculation
 */
TEST_F(Collider2DTest, CircleAABB_CalculatesCorrectly) {
    auto circle = Collider2DComponent::createCircle(3.0f);
    glm::vec2 position(10.0f, 10.0f);
    float rotation = 0.0f;
    
    auto [min, max] = circle.getAABB(position, rotation);
    
    EXPECT_FLOAT_EQ(min.x, 7.0f);
    EXPECT_FLOAT_EQ(min.y, 7.0f);
    EXPECT_FLOAT_EQ(max.x, 13.0f);
    EXPECT_FLOAT_EQ(max.y, 13.0f);
}

/**
 * Test box contains point
 */
TEST_F(Collider2DTest, BoxContainsPoint_DetectsCorrectly) {
    auto box = Collider2DComponent::createBox(glm::vec2(2.0f, 2.0f));
    glm::vec2 position(5.0f, 5.0f);
    float rotation = 0.0f;
    
    // Point inside
    EXPECT_TRUE(box.containsPoint(glm::vec2(5.0f, 5.0f), position, rotation));
    EXPECT_TRUE(box.containsPoint(glm::vec2(5.5f, 5.5f), position, rotation));
    
    // Point outside
    EXPECT_FALSE(box.containsPoint(glm::vec2(10.0f, 10.0f), position, rotation));
    EXPECT_FALSE(box.containsPoint(glm::vec2(0.0f, 0.0f), position, rotation));
}

/**
 * Test circle contains point
 */
TEST_F(Collider2DTest, CircleContainsPoint_DetectsCorrectly) {
    auto circle = Collider2DComponent::createCircle(5.0f);
    glm::vec2 position(10.0f, 10.0f);
    float rotation = 0.0f;
    
    // Point inside
    EXPECT_TRUE(circle.containsPoint(glm::vec2(10.0f, 10.0f), position, rotation));
    EXPECT_TRUE(circle.containsPoint(glm::vec2(12.0f, 10.0f), position, rotation));
    
    // Point outside
    EXPECT_FALSE(circle.containsPoint(glm::vec2(20.0f, 10.0f), position, rotation));
    EXPECT_FALSE(circle.containsPoint(glm::vec2(0.0f, 0.0f), position, rotation));
}

/**
 * Test collision layer masking
 */
TEST_F(Collider2DTest, CollisionMask_DefaultsCorrectly) {
    EXPECT_EQ(collider->layer, 0);
    EXPECT_EQ(collider->mask, 0xFFFFFFFF); // Should collide with all layers by default
}

/**
 * Test trigger flag
 */
TEST_F(Collider2DTest, TriggerFlag_WorksCorrectly) {
    EXPECT_FALSE(collider->isTrigger);
    
    collider->isTrigger = true;
    EXPECT_TRUE(collider->isTrigger);
}

/**
 * Test enabled flag
 */
TEST_F(Collider2DTest, EnabledFlag_WorksCorrectly) {
    EXPECT_TRUE(collider->enabled);
    
    collider->enabled = false;
    EXPECT_FALSE(collider->enabled);
}
