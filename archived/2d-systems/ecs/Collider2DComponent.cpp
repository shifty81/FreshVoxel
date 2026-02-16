#include "ecs/Collider2DComponent.h"
#include <cmath>
#include <algorithm>

namespace fresh
{
namespace ecs
{

Collider2DComponent::Collider2DComponent()
    : shapeType(ShapeType::Box)
    , boxSize(0.5f, 0.5f)
    , circleRadius(0.5f)
    , edgeStart(0.0f, 0.0f)
    , edgeEnd(1.0f, 0.0f)
    , offset(0.0f, 0.0f)
    , isTrigger(false)
    , enabled(true)
    , layer(0)
    , mask(0xFFFFFFFF)
{
}

Collider2DComponent Collider2DComponent::createBox(const glm::vec2& size)
{
    Collider2DComponent collider;
    collider.shapeType = ShapeType::Box;
    collider.boxSize = size * 0.5f; // Store as half-extents
    return collider;
}

Collider2DComponent Collider2DComponent::createCircle(float radius)
{
    Collider2DComponent collider;
    collider.shapeType = ShapeType::Circle;
    collider.circleRadius = radius;
    return collider;
}

Collider2DComponent Collider2DComponent::createPolygon(const std::vector<glm::vec2>& points)
{
    Collider2DComponent collider;
    collider.shapeType = ShapeType::Polygon;
    collider.vertices = points;
    return collider;
}

Collider2DComponent Collider2DComponent::createEdge(const glm::vec2& start, const glm::vec2& end)
{
    Collider2DComponent collider;
    collider.shapeType = ShapeType::Edge;
    collider.edgeStart = start;
    collider.edgeEnd = end;
    return collider;
}

std::pair<glm::vec2, glm::vec2> Collider2DComponent::getAABB(const glm::vec2& position, float rotation) const
{
    glm::vec2 worldPos = position + offset;
    
    switch (shapeType)
    {
        case ShapeType::Box:
        {
            // For simplicity, we'll compute AABB of rotated box
            // Could be optimized with a tighter fit
            float cos_r = std::cos(rotation);
            float sin_r = std::sin(rotation);
            
            // Transform all 4 corners
            glm::vec2 corners[4] = {
                glm::vec2(-boxSize.x, -boxSize.y),
                glm::vec2( boxSize.x, -boxSize.y),
                glm::vec2( boxSize.x,  boxSize.y),
                glm::vec2(-boxSize.x,  boxSize.y)
            };
            
            glm::vec2 min(std::numeric_limits<float>::max());
            glm::vec2 max(std::numeric_limits<float>::lowest());
            
            for (int i = 0; i < 4; i++)
            {
                glm::vec2 rotated(
                    corners[i].x * cos_r - corners[i].y * sin_r,
                    corners[i].x * sin_r + corners[i].y * cos_r
                );
                glm::vec2 world = worldPos + rotated;
                min = glm::min(min, world);
                max = glm::max(max, world);
            }
            
            return {min, max};
        }
        
        case ShapeType::Circle:
        {
            glm::vec2 min = worldPos - glm::vec2(circleRadius);
            glm::vec2 max = worldPos + glm::vec2(circleRadius);
            return {min, max};
        }
        
        case ShapeType::Polygon:
        {
            if (vertices.empty())
            {
                return {worldPos, worldPos};
            }
            
            float cos_r = std::cos(rotation);
            float sin_r = std::sin(rotation);
            
            glm::vec2 min(std::numeric_limits<float>::max());
            glm::vec2 max(std::numeric_limits<float>::lowest());
            
            for (const auto& vertex : vertices)
            {
                glm::vec2 rotated(
                    vertex.x * cos_r - vertex.y * sin_r,
                    vertex.x * sin_r + vertex.y * cos_r
                );
                glm::vec2 world = worldPos + rotated;
                min = glm::min(min, world);
                max = glm::max(max, world);
            }
            
            return {min, max};
        }
        
        case ShapeType::Edge:
        {
            float cos_r = std::cos(rotation);
            float sin_r = std::sin(rotation);
            
            glm::vec2 start(
                edgeStart.x * cos_r - edgeStart.y * sin_r,
                edgeStart.x * sin_r + edgeStart.y * cos_r
            );
            glm::vec2 end(
                edgeEnd.x * cos_r - edgeEnd.y * sin_r,
                edgeEnd.x * sin_r + edgeEnd.y * cos_r
            );
            
            glm::vec2 worldStart = worldPos + start;
            glm::vec2 worldEnd = worldPos + end;
            
            glm::vec2 min = glm::min(worldStart, worldEnd);
            glm::vec2 max = glm::max(worldStart, worldEnd);
            
            return {min, max};
        }
    }
    
    return {worldPos, worldPos};
}

bool Collider2DComponent::containsPoint(const glm::vec2& point, const glm::vec2& position, float rotation) const
{
    glm::vec2 worldPos = position + offset;
    
    // Transform point to local space
    float cos_r = std::cos(-rotation);
    float sin_r = std::sin(-rotation);
    glm::vec2 localPoint = point - worldPos;
    glm::vec2 rotatedPoint(
        localPoint.x * cos_r - localPoint.y * sin_r,
        localPoint.x * sin_r + localPoint.y * cos_r
    );
    
    switch (shapeType)
    {
        case ShapeType::Box:
            return std::abs(rotatedPoint.x) <= boxSize.x && std::abs(rotatedPoint.y) <= boxSize.y;
            
        case ShapeType::Circle:
            return glm::length(localPoint) <= circleRadius;
            
        case ShapeType::Polygon:
        {
            // Ray casting algorithm for point in polygon
            if (vertices.size() < 3)
                return false;
                
            bool inside = false;
            for (size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++)
            {
                const glm::vec2& vi = vertices[i];
                const glm::vec2& vj = vertices[j];
                
                if (((vi.y > rotatedPoint.y) != (vj.y > rotatedPoint.y)) &&
                    (rotatedPoint.x < (vj.x - vi.x) * (rotatedPoint.y - vi.y) / (vj.y - vi.y) + vi.x))
                {
                    inside = !inside;
                }
            }
            return inside;
        }
        
        case ShapeType::Edge:
        {
            // Check if point is close to edge (within small threshold)
            glm::vec2 edge = edgeEnd - edgeStart;
            float edgeLength = glm::length(edge);
            if (edgeLength < 0.001f)
                return false;
                
            glm::vec2 edgeDir = edge / edgeLength;
            glm::vec2 toPoint = rotatedPoint - edgeStart;
            float projection = glm::dot(toPoint, edgeDir);
            
            if (projection < 0.0f || projection > edgeLength)
                return false;
                
            glm::vec2 closest = edgeStart + edgeDir * projection;
            float distance = glm::length(rotatedPoint - closest);
            return distance < 0.1f; // Small threshold
        }
    }
    
    return false;
}

} // namespace ecs
} // namespace fresh
