/**
 * @file RigidBody2DTests.cpp
 * @brief Unit tests for RigidBody2D component
 */

#include <gtest/gtest.h>
#include "ecs/RigidBody2DComponent.h"
#include <glm/glm.hpp>

using namespace fresh::ecs;

/**
 * Test fixture for RigidBody2D tests
 */
class RigidBody2DTest : public ::testing::Test {
protected:
    void SetUp() override {
        rigidbody = std::make_unique<RigidBody2DComponent>();
    }

    void TearDown() override {
        rigidbody.reset();
    }

    std::unique_ptr<RigidBody2DComponent> rigidbody;
};

/**
 * Test default initialization
 */
TEST_F(RigidBody2DTest, DefaultConstructor_InitializesCorrectly) {
    EXPECT_EQ(rigidbody->bodyType, RigidBody2DComponent::BodyType::Dynamic);
    EXPECT_EQ(rigidbody->velocity, glm::vec2(0.0f));
    EXPECT_FLOAT_EQ(rigidbody->angularVelocity, 0.0f);
    EXPECT_FLOAT_EQ(rigidbody->mass, 1.0f);
    EXPECT_FLOAT_EQ(rigidbody->inverseMass, 1.0f);
    EXPECT_TRUE(rigidbody->useGravity);
    EXPECT_FALSE(rigidbody->freezeRotation);
}

/**
 * Test adding force
 */
TEST_F(RigidBody2DTest, AddForce_AccumulatesForces) {
    glm::vec2 force1(10.0f, 0.0f);
    glm::vec2 force2(0.0f, 5.0f);
    
    rigidbody->addForce(force1);
    rigidbody->addForce(force2);
    
    EXPECT_EQ(rigidbody->force, glm::vec2(10.0f, 5.0f));
}

/**
 * Test adding impulse
 */
TEST_F(RigidBody2DTest, AddImpulse_ChangesVelocity) {
    rigidbody->mass = 2.0f;
    rigidbody->inverseMass = 0.5f;
    glm::vec2 impulse(10.0f, 0.0f);
    
    rigidbody->addImpulse(impulse);
    
    EXPECT_EQ(rigidbody->velocity, glm::vec2(5.0f, 0.0f)); // impulse * inverseMass
}

/**
 * Test impulse doesn't affect static bodies
 */
TEST_F(RigidBody2DTest, AddImpulse_DoesNotAffectStaticBody) {
    rigidbody->bodyType = RigidBody2DComponent::BodyType::Static;
    glm::vec2 impulse(10.0f, 0.0f);
    
    rigidbody->addImpulse(impulse);
    
    EXPECT_EQ(rigidbody->velocity, glm::vec2(0.0f));
}

/**
 * Test adding torque
 */
TEST_F(RigidBody2DTest, AddTorque_AccumulatesTorque) {
    rigidbody->addTorque(5.0f);
    rigidbody->addTorque(3.0f);
    
    EXPECT_FLOAT_EQ(rigidbody->torque, 8.0f);
}

/**
 * Test set mass updates inverse mass
 */
TEST_F(RigidBody2DTest, SetMass_UpdatesInverseMass) {
    rigidbody->setMass(2.0f);
    
    EXPECT_FLOAT_EQ(rigidbody->mass, 2.0f);
    EXPECT_FLOAT_EQ(rigidbody->inverseMass, 0.5f);
}

/**
 * Test static body has zero inverse mass
 */
TEST_F(RigidBody2DTest, SetMass_StaticBodyHasZeroInverseMass) {
    rigidbody->bodyType = RigidBody2DComponent::BodyType::Static;
    rigidbody->setMass(10.0f);
    
    EXPECT_FLOAT_EQ(rigidbody->inverseMass, 0.0f);
}

/**
 * Test clearing forces
 */
TEST_F(RigidBody2DTest, ClearForces_ResetsForces) {
    rigidbody->addForce(glm::vec2(10.0f, 10.0f));
    rigidbody->addTorque(5.0f);
    
    rigidbody->clearForces();
    
    EXPECT_EQ(rigidbody->force, glm::vec2(0.0f));
    EXPECT_FLOAT_EQ(rigidbody->torque, 0.0f);
}

/**
 * Test kinetic energy calculation
 */
TEST_F(RigidBody2DTest, GetKineticEnergy_CalculatesCorrectly) {
    rigidbody->mass = 2.0f;
    rigidbody->velocity = glm::vec2(3.0f, 4.0f); // magnitude = 5
    rigidbody->angularVelocity = 2.0f;
    
    float ke = rigidbody->getKineticEnergy();
    
    // Linear KE = 0.5 * 2 * 25 = 25
    // Rotational KE = 0.5 * 2 * 4 = 4
    // Total = 29
    EXPECT_FLOAT_EQ(ke, 29.0f);
}

/**
 * Test drag properties
 */
TEST_F(RigidBody2DTest, DragProperties_DefaultCorrectly) {
    EXPECT_GT(rigidbody->drag, 0.0f);
    EXPECT_LT(rigidbody->drag, 1.0f);
    EXPECT_GT(rigidbody->angularDrag, 0.0f);
    EXPECT_LT(rigidbody->angularDrag, 1.0f);
}

/**
 * Test restitution and friction properties
 */
TEST_F(RigidBody2DTest, PhysicsProperties_DefaultCorrectly) {
    EXPECT_GE(rigidbody->restitution, 0.0f);
    EXPECT_LE(rigidbody->restitution, 1.0f);
    EXPECT_GE(rigidbody->friction, 0.0f);
    EXPECT_LE(rigidbody->friction, 1.0f);
}

/**
 * Test gravity scale
 */
TEST_F(RigidBody2DTest, GravityScale_DefaultsToOne) {
    EXPECT_EQ(rigidbody->gravityScale, glm::vec2(1.0f));
}

/**
 * Test body types
 */
TEST_F(RigidBody2DTest, BodyTypes_CanBeSet) {
    rigidbody->bodyType = RigidBody2DComponent::BodyType::Static;
    EXPECT_EQ(rigidbody->bodyType, RigidBody2DComponent::BodyType::Static);
    
    rigidbody->bodyType = RigidBody2DComponent::BodyType::Kinematic;
    EXPECT_EQ(rigidbody->bodyType, RigidBody2DComponent::BodyType::Kinematic);
    
    rigidbody->bodyType = RigidBody2DComponent::BodyType::Dynamic;
    EXPECT_EQ(rigidbody->bodyType, RigidBody2DComponent::BodyType::Dynamic);
}
