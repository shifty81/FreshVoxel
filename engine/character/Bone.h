#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace fresh
{

/**
 * @brief Represents a single bone in a character skeleton
 * 
 * Bones form a hierarchical structure where each bone can have a parent
 * and multiple children. This forms the skeleton's bone hierarchy.
 */
class Bone
{
public:
    /**
     * @brief Construct a new Bone
     * @param name Bone identifier (e.g., "spine", "left_arm")
     * @param parent Pointer to parent bone (nullptr for root)
     */
    Bone(const std::string& name, Bone* parent = nullptr);

    // Getters
    const std::string& getName() const { return m_name; }
    Bone* getParent() { return m_parent; }
    const Bone* getParent() const { return m_parent; }
    const std::vector<Bone*>& getChildren() const { return m_children; }
    
    /**
     * @brief Get local position relative to parent
     */
    const glm::vec3& getLocalPosition() const { return m_localPosition; }
    
    /**
     * @brief Get local rotation relative to parent
     */
    const glm::quat& getLocalRotation() const { return m_localRotation; }
    
    /**
     * @brief Get local scale relative to parent
     */
    const glm::vec3& getLocalScale() const { return m_localScale; }
    
    /**
     * @brief Get world position (calculated from hierarchy)
     */
    glm::vec3 getWorldPosition() const;
    
    /**
     * @brief Get world rotation (calculated from hierarchy)
     */
    glm::quat getWorldRotation() const;
    
    /**
     * @brief Get local transform matrix
     */
    glm::mat4 getLocalTransform() const;
    
    /**
     * @brief Get world transform matrix (calculated from hierarchy)
     */
    glm::mat4 getWorldTransform() const;

    // Setters
    void setLocalPosition(const glm::vec3& position) { m_localPosition = position; }
    void setLocalRotation(const glm::quat& rotation) { m_localRotation = rotation; }
    void setLocalScale(const glm::vec3& scale) { m_localScale = scale; }

    /**
     * @brief Add a child bone
     * @param child Pointer to child bone to add
     */
    void addChild(Bone* child);
    
    /**
     * @brief Remove a child bone
     * @param child Pointer to child bone to remove
     */
    void removeChild(Bone* child);

    /**
     * @brief Get bone length (distance to first child or default length)
     */
    float getLength() const;

private:
    std::string m_name;
    Bone* m_parent;
    std::vector<Bone*> m_children;
    
    // Local transform (relative to parent)
    glm::vec3 m_localPosition;
    glm::quat m_localRotation;
    glm::vec3 m_localScale;
};

} // namespace fresh
