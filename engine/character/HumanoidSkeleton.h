#pragma once

#include "character/Bone.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace fresh
{

/**
 * @brief Standard humanoid skeleton with 15 bones
 * 
 * Bone hierarchy:
 * - Root (pelvis/hips)
 *   - Spine
 *     - Chest
 *       - Head
 *       - LeftShoulder
 *         - LeftUpperArm
 *           - LeftLowerArm
 *             - LeftHand
 *       - RightShoulder
 *         - RightUpperArm
 *           - RightLowerArm
 *             - RightHand
 *   - LeftUpperLeg
 *     - LeftLowerLeg
 *       - LeftFoot
 *   - RightUpperLeg
 *     - RightLowerLeg
 *       - RightFoot
 */
class HumanoidSkeleton
{
public:
    /**
     * @brief Standard bone names in the humanoid skeleton
     */
    enum class BoneName
    {
        Root,           // Pelvis/hips (origin point)
        Spine,          // Lower spine
        Chest,          // Upper spine/chest
        Head,           // Head/neck
        LeftShoulder,   // Left shoulder joint
        LeftUpperArm,   // Left upper arm
        LeftLowerArm,   // Left forearm
        LeftHand,       // Left hand
        RightShoulder,  // Right shoulder joint
        RightUpperArm,  // Right upper arm
        RightLowerArm,  // Right forearm
        RightHand,      // Right hand
        LeftUpperLeg,   // Left thigh
        LeftLowerLeg,   // Left shin
        LeftFoot,       // Left foot
        RightUpperLeg,  // Right thigh
        RightLowerLeg,  // Right shin
        RightFoot       // Right foot
    };

    HumanoidSkeleton();
    ~HumanoidSkeleton();

    /**
     * @brief Initialize the skeleton with default proportions
     * @param height Character height in voxels (default: 16)
     */
    void initialize(float height = 16.0f);

    /**
     * @brief Get a bone by its standard name
     * @param name Standard bone name from BoneName enum
     * @return Pointer to bone or nullptr if not found
     */
    Bone* getBone(BoneName name);
    const Bone* getBone(BoneName name) const;

    /**
     * @brief Get a bone by string name
     * @param name String name of the bone (e.g., "left_upper_arm")
     * @return Pointer to bone or nullptr if not found
     */
    Bone* getBone(const std::string& name);
    const Bone* getBone(const std::string& name) const;

    /**
     * @brief Get the root bone (pelvis)
     */
    Bone* getRootBone() { return m_rootBone.get(); }
    const Bone* getRootBone() const { return m_rootBone.get(); }

    /**
     * @brief Get all bones in the skeleton
     */
    const std::unordered_map<std::string, Bone*>& getAllBones() const { return m_boneMap; }

    /**
     * @brief Set custom proportions for the skeleton
     * @param proportions Map of bone names to scale factors
     */
    void setProportions(const std::unordered_map<std::string, float>& proportions);

    /**
     * @brief Reset all bones to their default pose (T-pose)
     */
    void resetToPose();

private:
    /**
     * @brief Create all bones and set up hierarchy
     */
    void createBones(float height);

    /**
     * @brief Convert BoneName enum to string
     */
    static std::string boneNameToString(BoneName name);

    // Bone storage
    std::unique_ptr<Bone> m_rootBone;
    std::vector<std::unique_ptr<Bone>> m_bones;
    std::unordered_map<std::string, Bone*> m_boneMap;

    // Default proportions (relative to height)
    float m_height;
};

} // namespace fresh
