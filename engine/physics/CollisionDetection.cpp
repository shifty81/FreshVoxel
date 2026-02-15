#include "physics/CollisionDetection.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "voxel/VoxelTypes.h"
#include "voxel/VoxelWorld.h"

namespace fresh
{

// AABB implementation
bool AABB::contains(const glm::vec3& point) const noexcept
{
    return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y &&
           point.z >= min.z && point.z <= max.z;
}

bool AABB::intersects(const AABB& other) const noexcept
{
    return (min.x <= other.max.x && max.x >= other.min.x) &&
           (min.y <= other.max.y && max.y >= other.min.y) &&
           (min.z <= other.max.z && max.z >= other.min.z);
}

// BoundingSphere implementation
bool BoundingSphere::contains(const glm::vec3& point) const noexcept
{
    return glm::distance(center, point) <= radius;
}

bool BoundingSphere::intersects(const BoundingSphere& other) const noexcept
{
    float distance = glm::distance(center, other.center);
    return distance <= (radius + other.radius);
}

bool BoundingSphere::intersects(const AABB& aabb) const noexcept
{
    glm::vec3 closest = glm::clamp(center, aabb.min, aabb.max);
    float distance = glm::distance(center, closest);
    return distance <= radius;
}

// CollisionDetection implementation
bool CollisionDetection::testAABB(const AABB& a, const AABB& b)
{
    return a.intersects(b);
}

bool CollisionDetection::testSphere(const BoundingSphere& a, const BoundingSphere& b)
{
    return a.intersects(b);
}

bool CollisionDetection::testAABBSphere(const AABB& aabb, const BoundingSphere& sphere)
{
    return sphere.intersects(aabb);
}

bool CollisionDetection::raycastAABB(const Ray& ray, const AABB& aabb, float& distance,
                                     glm::vec3& normal)
{
    glm::vec3 invDir = 1.0f / ray.direction;
    glm::vec3 t0 = (aabb.min - ray.origin) * invDir;
    glm::vec3 t1 = (aabb.max - ray.origin) * invDir;

    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);

    float tEnter = std::max(std::max(tmin.x, tmin.y), tmin.z);
    float tExit = std::min(std::min(tmax.x, tmax.y), tmax.z);

    if (tEnter > tExit || tExit < 0.0f) {
        return false;
    }

    distance = tEnter >= 0.0f ? tEnter : tExit;

    // Calculate normal
    const float epsilon = 0.0001f;
    glm::vec3 hitPoint = ray.origin + ray.direction * tEnter;
    normal = glm::vec3(0.0f);

    if (std::abs(hitPoint.x - aabb.min.x) < epsilon)
        normal = glm::vec3(-1, 0, 0);
    else if (std::abs(hitPoint.x - aabb.max.x) < epsilon)
        normal = glm::vec3(1, 0, 0);
    else if (std::abs(hitPoint.y - aabb.min.y) < epsilon)
        normal = glm::vec3(0, -1, 0);
    else if (std::abs(hitPoint.y - aabb.max.y) < epsilon)
        normal = glm::vec3(0, 1, 0);
    else if (std::abs(hitPoint.z - aabb.min.z) < epsilon)
        normal = glm::vec3(0, 0, -1);
    else if (std::abs(hitPoint.z - aabb.max.z) < epsilon)
        normal = glm::vec3(0, 0, 1);

    return true;
}

bool CollisionDetection::raycastSphere(const Ray& ray, const BoundingSphere& sphere,
                                       float& distance, glm::vec3& normal)
{
    glm::vec3 oc = ray.origin - sphere.center;
    float b = glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - c;

    if (discriminant < 0.0f) {
        return false;
    }

    float sqrt_discriminant = std::sqrt(discriminant);
    float t0 = -b - sqrt_discriminant;
    float t1 = -b + sqrt_discriminant;

    distance = (t0 >= 0.0f) ? t0 : t1;
    if (distance < 0.0f)
        return false;

    glm::vec3 hitPoint = ray.getPoint(distance);
    normal = glm::normalize(hitPoint - sphere.center);

    return true;
}

RayHit CollisionDetection::raycastVoxel(const Ray& ray, VoxelWorld* world, float maxDistance)
{
    RayHit hit;

    if (!world)
        return hit;

    // DDA (Digital Differential Analyzer) algorithm
    glm::vec3 pos = ray.origin;
    glm::ivec3 mapPos(std::floor(pos.x), std::floor(pos.y), std::floor(pos.z));

    glm::vec3 deltaDist(std::abs(1.0f / ray.direction.x), std::abs(1.0f / ray.direction.y),
                        std::abs(1.0f / ray.direction.z));

    glm::ivec3 step(ray.direction.x < 0 ? -1 : 1, ray.direction.y < 0 ? -1 : 1,
                    ray.direction.z < 0 ? -1 : 1);

    glm::vec3 sideDist((ray.direction.x < 0) ? (pos.x - mapPos.x) * deltaDist.x
                                             : (mapPos.x + 1.0f - pos.x) * deltaDist.x,
                       (ray.direction.y < 0) ? (pos.y - mapPos.y) * deltaDist.y
                                             : (mapPos.y + 1.0f - pos.y) * deltaDist.y,
                       (ray.direction.z < 0) ? (pos.z - mapPos.z) * deltaDist.z
                                             : (mapPos.z + 1.0f - pos.z) * deltaDist.z);

    glm::ivec3 normal(0);
    float distance = 0.0f;

    const int maxSteps = static_cast<int>(maxDistance * 2);
    for (int i = 0; i < maxSteps; ++i) {
        // Check current voxel
        Voxel* voxelPtr = world->getVoxel(WorldPos(mapPos.x, mapPos.y, mapPos.z));
        VoxelType voxel = voxelPtr ? voxelPtr->type : VoxelType::Air;
        if (voxel != VoxelType::Air && voxel != VoxelType::Water) {
            hit.hit = true;
            hit.distance = distance;
            hit.point = ray.getPoint(distance);
            hit.normal = glm::vec3(normal);
            hit.blockX = mapPos.x;
            hit.blockY = mapPos.y;
            hit.blockZ = mapPos.z;
            break;
        }

        // Step to next voxel
        if (sideDist.x < sideDist.y) {
            if (sideDist.x < sideDist.z) {
                sideDist.x += deltaDist.x;
                mapPos.x += step.x;
                normal = glm::ivec3(-step.x, 0, 0);
                distance = sideDist.x;
            } else {
                sideDist.z += deltaDist.z;
                mapPos.z += step.z;
                normal = glm::ivec3(0, 0, -step.z);
                distance = sideDist.z;
            }
        } else {
            if (sideDist.y < sideDist.z) {
                sideDist.y += deltaDist.y;
                mapPos.y += step.y;
                normal = glm::ivec3(0, -step.y, 0);
                distance = sideDist.y;
            } else {
                sideDist.z += deltaDist.z;
                mapPos.z += step.z;
                normal = glm::ivec3(0, 0, -step.z);
                distance = sideDist.z;
            }
        }

        if (distance > maxDistance)
            break;
    }

    return hit;
}

bool CollisionDetection::testAABBVoxelWorld(const AABB& aabb, VoxelWorld* world)
{
    if (!world)
        return false;

    // Sample voxels within AABB
    int minX = static_cast<int>(std::floor(aabb.min.x));
    int minY = static_cast<int>(std::floor(aabb.min.y));
    int minZ = static_cast<int>(std::floor(aabb.min.z));
    int maxX = static_cast<int>(std::ceil(aabb.max.x));
    int maxY = static_cast<int>(std::ceil(aabb.max.y));
    int maxZ = static_cast<int>(std::ceil(aabb.max.z));

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                Voxel* voxel = world->getVoxel(WorldPos(x, y, z));
                if (voxel && voxel->type != VoxelType::Air && voxel->type != VoxelType::Water) {
                    return true;
                }
            }
        }
    }

    return false;
}

glm::vec3 CollisionDetection::sweepAABBVoxelWorld(const AABB& aabb, const glm::vec3& velocity,
                                                  VoxelWorld* world)
{
    if (!world)
        return velocity;

    // Simple sweep - test each axis separately
    glm::vec3 adjustedVelocity = velocity;

    // X axis
    AABB testAABB = aabb;
    testAABB.min.x += velocity.x;
    testAABB.max.x += velocity.x;
    if (testAABBVoxelWorld(testAABB, world)) {
        adjustedVelocity.x = 0.0f;
    }

    // Y axis
    testAABB = aabb;
    testAABB.min.y += velocity.y;
    testAABB.max.y += velocity.y;
    if (testAABBVoxelWorld(testAABB, world)) {
        adjustedVelocity.y = 0.0f;
    }

    // Z axis
    testAABB = aabb;
    testAABB.min.z += velocity.z;
    testAABB.max.z += velocity.z;
    if (testAABBVoxelWorld(testAABB, world)) {
        adjustedVelocity.z = 0.0f;
    }

    return adjustedVelocity;
}

glm::vec3 CollisionDetection::closestPointOnAABB(const AABB& aabb, const glm::vec3& point)
{
    return glm::clamp(point, aabb.min, aabb.max);
}

bool CollisionDetection::calculatePenetration(const AABB& a, const AABB& b, glm::vec3& normal,
                                              float& depth)
{
    if (!a.intersects(b))
        return false;

    // Calculate overlap on each axis
    float overlapX = std::min(a.max.x - b.min.x, b.max.x - a.min.x);
    float overlapY = std::min(a.max.y - b.min.y, b.max.y - a.min.y);
    float overlapZ = std::min(a.max.z - b.min.z, b.max.z - a.min.z);

    // Find axis with minimum overlap (separation axis)
    if (overlapX < overlapY && overlapX < overlapZ) {
        depth = overlapX;
        normal = (a.getCenter().x < b.getCenter().x) ? glm::vec3(-1, 0, 0) : glm::vec3(1, 0, 0);
    } else if (overlapY < overlapZ) {
        depth = overlapY;
        normal = (a.getCenter().y < b.getCenter().y) ? glm::vec3(0, -1, 0) : glm::vec3(0, 1, 0);
    } else {
        depth = overlapZ;
        normal = (a.getCenter().z < b.getCenter().z) ? glm::vec3(0, 0, -1) : glm::vec3(0, 0, 1);
    }

    return true;
}

// SpatialHash implementation
SpatialHash::SpatialHash(float cellSize) : cellSize(cellSize) {}

void SpatialHash::clear()
{
    cells.clear();
}

void SpatialHash::insert(CollisionBody* body)
{
    if (!body)
        return;

    // Insert into all cells the AABB overlaps
    glm::ivec3 minCell = getCellCoords(body->aabb.min);
    glm::ivec3 maxCell = getCellCoords(body->aabb.max);

    for (int x = minCell.x; x <= maxCell.x; ++x) {
        for (int y = minCell.y; y <= maxCell.y; ++y) {
            for (int z = minCell.z; z <= maxCell.z; ++z) {
                int hash = x * 73856093 ^ y * 19349663 ^ z * 83492791;
                cells[hash].bodies.push_back(body);
            }
        }
    }
}

void SpatialHash::remove(CollisionBody* body)
{
    if (!body)
        return;

    // Remove from all cells the AABB overlaps
    glm::ivec3 minCell = getCellCoords(body->aabb.min);
    glm::ivec3 maxCell = getCellCoords(body->aabb.max);

    for (int x = minCell.x; x <= maxCell.x; ++x) {
        for (int y = minCell.y; y <= maxCell.y; ++y) {
            for (int z = minCell.z; z <= maxCell.z; ++z) {
                int hash = x * 73856093 ^ y * 19349663 ^ z * 83492791;
                auto it = cells.find(hash);
                if (it != cells.end()) {
                    auto& bodies = it->second.bodies;
                    bodies.erase(std::remove(bodies.begin(), bodies.end(), body), bodies.end());
                    
                    // Remove empty cells to save memory
                    if (bodies.empty()) {
                        cells.erase(it);
                    }
                }
            }
        }
    }
}

std::vector<CollisionBody*> SpatialHash::query(const AABB& aabb)
{
    std::vector<CollisionBody*> result;
    std::set<CollisionBody*> uniqueBodies;

    glm::ivec3 minCell = getCellCoords(aabb.min);
    glm::ivec3 maxCell = getCellCoords(aabb.max);

    for (int x = minCell.x; x <= maxCell.x; ++x) {
        for (int y = minCell.y; y <= maxCell.y; ++y) {
            for (int z = minCell.z; z <= maxCell.z; ++z) {
                int hash = x * 73856093 ^ y * 19349663 ^ z * 83492791;
                auto it = cells.find(hash);
                if (it != cells.end()) {
                    for (auto* body : it->second.bodies) {
                        if (uniqueBodies.insert(body).second) {
                            result.push_back(body);
                        }
                    }
                }
            }
        }
    }

    return result;
}

std::vector<CollisionPair> SpatialHash::findCollisions()
{
    std::vector<CollisionPair> pairs;

    // Find all colliding pairs (simplified)
    for (auto& [hash, cell] : cells) {
        for (size_t i = 0; i < cell.bodies.size(); ++i) {
            for (size_t j = i + 1; j < cell.bodies.size(); ++j) {
                auto* a = cell.bodies[i];
                auto* b = cell.bodies[j];

                if (a->aabb.intersects(b->aabb)) {
                    CollisionPair pair;
                    pair.a = a;
                    pair.b = b;
                    CollisionDetection::calculatePenetration(a->aabb, b->aabb, pair.normal,
                                                             pair.penetration);
                    pairs.push_back(pair);
                }
            }
        }
    }

    return pairs;
}

int SpatialHash::hashPosition(const glm::vec3& pos) const
{
    glm::ivec3 cell = getCellCoords(pos);
    return cell.x * 73856093 ^ cell.y * 19349663 ^ cell.z * 83492791;
}

glm::ivec3 SpatialHash::getCellCoords(const glm::vec3& pos) const
{
    return glm::ivec3(static_cast<int>(std::floor(pos.x / cellSize)),
                      static_cast<int>(std::floor(pos.y / cellSize)),
                      static_cast<int>(std::floor(pos.z / cellSize)));
}

} // namespace fresh
