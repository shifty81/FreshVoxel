#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "ecs/IComponent.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief Component for 2D collision shapes
 *
 * Supports various 2D collision shapes including box, circle, and polygon.
 */
class Collider2DComponent : public IComponent
{
public:
    /**
     * @brief Type of collision shape
     */
    enum class ShapeType
    {
        Box,        ///< Axis-aligned bounding box
        Circle,     ///< Circle shape
        Polygon,    ///< Convex polygon
        Edge        ///< Line segment
    };

    Collider2DComponent();
    virtual ~Collider2DComponent() = default;

    COMPONENT_TYPE(Collider2DComponent)

    // Shape properties
    ShapeType shapeType;
    
    // Box shape
    glm::vec2 boxSize;              ///< Size of box (half-extents)
    
    // Circle shape
    float circleRadius;             ///< Radius of circle
    
    // Polygon shape
    std::vector<glm::vec2> vertices; ///< Vertices for polygon (local space)
    
    // Edge shape
    glm::vec2 edgeStart;            ///< Start point of edge
    glm::vec2 edgeEnd;              ///< End point of edge
    
    // Common properties
    glm::vec2 offset;               ///< Offset from entity position
    bool isTrigger;                 ///< If true, doesn't apply physics but detects overlaps
    bool enabled;                   ///< Is collider active?
    
    // Collision layers/masks for filtering
    unsigned int layer;             ///< Which layer this collider is on
    unsigned int mask;              ///< Which layers this collider collides with

    /**
     * @brief Create a box collider
     */
    static Collider2DComponent createBox(const glm::vec2& size);

    /**
     * @brief Create a circle collider
     */
    static Collider2DComponent createCircle(float radius);

    /**
     * @brief Create a polygon collider
     */
    static Collider2DComponent createPolygon(const std::vector<glm::vec2>& points);

    /**
     * @brief Create an edge collider
     */
    static Collider2DComponent createEdge(const glm::vec2& start, const glm::vec2& end);

    /**
     * @brief Get axis-aligned bounding box (AABB)
     * @param transform World transform to apply
     * @return min and max corners of AABB
     */
    std::pair<glm::vec2, glm::vec2> getAABB(const glm::vec2& position, float rotation) const;

    /**
     * @brief Check if a point is inside this collider
     */
    bool containsPoint(const glm::vec2& point, const glm::vec2& position, float rotation) const;
};

} // namespace ecs
} // namespace fresh
