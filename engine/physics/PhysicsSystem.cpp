#include "physics/PhysicsSystem.h"

namespace fresh
{

PhysicsSystem::PhysicsSystem() {}

PhysicsSystem::~PhysicsSystem() {}

bool PhysicsSystem::initialize()
{
    return true;
}

void PhysicsSystem::update(float deltaTime)
{
    (void)deltaTime; // Unused - stub implementation
    // Stub implementation
}

} // namespace fresh
