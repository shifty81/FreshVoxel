#pragma once
#include <map>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{

class VoxelWorld;

/**
 * @brief Axis-Aligned Bounding Box
 */
struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB() : min(0.0f), max(0.0f) {}
    AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

    [[nodiscard]] glm::vec3 getCenter() const noexcept
    {
        return (min + max) * 0.5f;
    }
    [[nodiscard]] glm::vec3 getExtents() const noexcept
    {
        return (max - min) * 0.5f;
    }
    [[nodiscard]] glm::vec3 getSize() const noexcept
    {
        return max - min;
    }

    [[nodiscard]] bool contains(const glm::vec3& point) const noexcept;
    [[nodiscard]] bool intersects(const AABB& other) const noexcept;
};

/**
 * @brief Bounding Sphere
 */
struct BoundingSphere {
    glm::vec3 center;
    float radius;

    BoundingSphere() : center(0.0f), radius(0.0f) {}
    BoundingSphere(const glm::vec3& center, float radius) : center(center), radius(radius) {}

    [[nodiscard]] bool contains(const glm::vec3& point) const noexcept;
    [[nodiscard]] bool intersects(const BoundingSphere& other) const noexcept;
    [[nodiscard]] bool intersects(const AABB& aabb) const noexcept;
};

/**
 * @brief Ray for raycasting
 */
struct Ray {
    glm::vec3 origin;
    glm::vec3 direction; // Should be normalized

    Ray() : origin(0.0f), direction(0.0f, 0.0f, -1.0f) {}
    Ray(const glm::vec3& origin, const glm::vec3& direction)
        : origin(origin), direction(glm::normalize(direction))
    {
    }

    [[nodiscard]] glm::vec3 getPoint(float distance) const noexcept
    {
        return origin + direction * distance;
    }
};

/**
 * @brief Ray hit information
 */
struct RayHit {
    bool hit = false;
    float distance = 0.0f;
    glm::vec3 point{0.0f};
    glm::vec3 normal{0.0f};
    int blockX = 0, blockY = 0, blockZ = 0;
};

/**
 * @brief Collision detection system
 *
 * Provides various collision detection methods for voxels,
 * AABBs, spheres, and rays.
 */
class CollisionDetection
{
public:
    /**
     * @brief Check AABB vs AABB collision
     */
    static bool testAABB(const AABB& a, const AABB& b);

    /**
     * @brief Check sphere vs sphere collision
     */
    static bool testSphere(const BoundingSphere& a, const BoundingSphere& b);

    /**
     * @brief Check AABB vs sphere collision
     */
    static bool testAABBSphere(const AABB& aabb, const BoundingSphere& sphere);

    /**
     * @brief Cast ray against AABB
     * @return true if hit, sets distance and normal
     */
    static bool raycastAABB(const Ray& ray, const AABB& aabb, float& distance, glm::vec3& normal);

    /**
     * @brief Cast ray against sphere
     */
    static bool raycastSphere(const Ray& ray, const BoundingSphere& sphere, float& distance,
                              glm::vec3& normal);

    /**
     * @brief Cast ray against voxel world (DDA algorithm)
     * @param maxDistance Maximum ray distance
     * @return RayHit with detailed hit information
     */
    static RayHit raycastVoxel(const Ray& ray, VoxelWorld* world, float maxDistance = 100.0f);

    /**
     * @brief Check if AABB collides with voxel world
     * @return true if collision detected
     */
    static bool testAABBVoxelWorld(const AABB& aabb, VoxelWorld* world);

    /**
     * @brief Sweep AABB through voxel world
     * @param velocity Movement velocity
     * @return Adjusted velocity after collision response
     */
    static glm::vec3 sweepAABBVoxelWorld(const AABB& aabb, const glm::vec3& velocity,
                                         VoxelWorld* world);

    /**
     * @brief Get closest point on AABB to a point
     */
    static glm::vec3 closestPointOnAABB(const AABB& aabb, const glm::vec3& point);

    /**
     * @brief Calculate penetration depth and normal between two AABBs
     */
    static bool calculatePenetration(const AABB& a, const AABB& b, glm::vec3& normal, float& depth);
};

/**
 * @brief Simple physics body for collision
 */
struct CollisionBody {
    AABB aabb;
    glm::vec3 velocity{0.0f};
    bool isStatic = false;
    bool isTrigger = false; // No physical response, just detection

    void* userData = nullptr; // For game object reference
};

/**
 * @brief Collision pair
 */
struct CollisionPair {
    CollisionBody* a;
    CollisionBody* b;
    glm::vec3 normal;
    float penetration;
};

/**
 * @brief Broad phase collision detection using spatial hashing
 */
class SpatialHash
{
public:
    SpatialHash(float cellSize = 16.0f);

    void clear();
    void insert(CollisionBody* body);
    void remove(CollisionBody* body);

    std::vector<CollisionBody*> query(const AABB& aabb);
    std::vector<CollisionPair> findCollisions();

private:
    struct Cell {
        std::vector<CollisionBody*> bodies;
    };

    int hashPosition(const glm::vec3& pos) const;
    glm::ivec3 getCellCoords(const glm::vec3& pos) const;

    float cellSize;
    std::map<int, Cell> cells;
};

} // namespace fresh
