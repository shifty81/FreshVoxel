#include "character/HumanoidSkeleton.h"
#include <glm/gtc/constants.hpp>

namespace fresh
{

HumanoidSkeleton::HumanoidSkeleton()
    : m_height(16.0f)
{
}

HumanoidSkeleton::~HumanoidSkeleton() = default;

void HumanoidSkeleton::initialize(float height)
{
    m_height = height;
    m_bones.clear();
    m_boneMap.clear();

    createBones(height);
}

void HumanoidSkeleton::createBones(float height)
{
    // Proportions based on typical humanoid character (height = 16 voxels)
    const float headHeight = height * 0.125f;      // 2 voxels
    const float chestHeight = height * 0.25f;      // 4 voxels
    const float spineHeight = height * 0.1875f;    // 3 voxels
    const float legHeight = height * 0.4375f;      // 7 voxels (total)
    const float upperLegHeight = height * 0.25f;   // 4 voxels
    const float lowerLegHeight = height * 0.1875f; // 3 voxels
    
    const float armLength = height * 0.4375f;      // 7 voxels (total)
    const float upperArmLength = height * 0.1875f; // 3 voxels
    const float lowerArmLength = height * 0.1875f; // 3 voxels
    const float handLength = height * 0.0625f;     // 1 voxel
    
    const float shoulderWidth = height * 0.375f;   // 6 voxels (total width)
    const float hipWidth = height * 0.25f;         // 4 voxels (total width)

    // Create root bone (pelvis/hips) at origin
    m_rootBone = std::make_unique<Bone>("root", nullptr);
    m_boneMap["root"] = m_rootBone.get();

    // Spine (upward from root)
    auto spinePtr = std::make_unique<Bone>("spine", m_rootBone.get());
    spinePtr->setLocalPosition(glm::vec3(0.0f, spineHeight, 0.0f));
    m_boneMap["spine"] = spinePtr.get();
    Bone* spine = spinePtr.get();
    m_bones.push_back(std::move(spinePtr));

    // Chest (upward from spine)
    auto chestPtr = std::make_unique<Bone>("chest", spine);
    chestPtr->setLocalPosition(glm::vec3(0.0f, chestHeight, 0.0f));
    m_boneMap["chest"] = chestPtr.get();
    Bone* chest = chestPtr.get();
    m_bones.push_back(std::move(chestPtr));

    // Head (upward from chest)
    auto headPtr = std::make_unique<Bone>("head", chest);
    headPtr->setLocalPosition(glm::vec3(0.0f, headHeight, 0.0f));
    m_boneMap["head"] = headPtr.get();
    m_bones.push_back(std::move(headPtr));

    // Left arm chain
    auto leftShoulderPtr = std::make_unique<Bone>("left_shoulder", chest);
    leftShoulderPtr->setLocalPosition(glm::vec3(-shoulderWidth / 2.0f, chestHeight * 0.75f, 0.0f));
    m_boneMap["left_shoulder"] = leftShoulderPtr.get();
    Bone* leftShoulder = leftShoulderPtr.get();
    m_bones.push_back(std::move(leftShoulderPtr));

    auto leftUpperArmPtr = std::make_unique<Bone>("left_upper_arm", leftShoulder);
    leftUpperArmPtr->setLocalPosition(glm::vec3(0.0f, -upperArmLength, 0.0f));
    m_boneMap["left_upper_arm"] = leftUpperArmPtr.get();
    Bone* leftUpperArm = leftUpperArmPtr.get();
    m_bones.push_back(std::move(leftUpperArmPtr));

    auto leftLowerArmPtr = std::make_unique<Bone>("left_lower_arm", leftUpperArm);
    leftLowerArmPtr->setLocalPosition(glm::vec3(0.0f, -lowerArmLength, 0.0f));
    m_boneMap["left_lower_arm"] = leftLowerArmPtr.get();
    Bone* leftLowerArm = leftLowerArmPtr.get();
    m_bones.push_back(std::move(leftLowerArmPtr));

    auto leftHandPtr = std::make_unique<Bone>("left_hand", leftLowerArm);
    leftHandPtr->setLocalPosition(glm::vec3(0.0f, -handLength, 0.0f));
    m_boneMap["left_hand"] = leftHandPtr.get();
    m_bones.push_back(std::move(leftHandPtr));

    // Right arm chain (mirror of left)
    auto rightShoulderPtr = std::make_unique<Bone>("right_shoulder", chest);
    rightShoulderPtr->setLocalPosition(glm::vec3(shoulderWidth / 2.0f, chestHeight * 0.75f, 0.0f));
    m_boneMap["right_shoulder"] = rightShoulderPtr.get();
    Bone* rightShoulder = rightShoulderPtr.get();
    m_bones.push_back(std::move(rightShoulderPtr));

    auto rightUpperArmPtr = std::make_unique<Bone>("right_upper_arm", rightShoulder);
    rightUpperArmPtr->setLocalPosition(glm::vec3(0.0f, -upperArmLength, 0.0f));
    m_boneMap["right_upper_arm"] = rightUpperArmPtr.get();
    Bone* rightUpperArm = rightUpperArmPtr.get();
    m_bones.push_back(std::move(rightUpperArmPtr));

    auto rightLowerArmPtr = std::make_unique<Bone>("right_lower_arm", rightUpperArm);
    rightLowerArmPtr->setLocalPosition(glm::vec3(0.0f, -lowerArmLength, 0.0f));
    m_boneMap["right_lower_arm"] = rightLowerArmPtr.get();
    Bone* rightLowerArm = rightLowerArmPtr.get();
    m_bones.push_back(std::move(rightLowerArmPtr));

    auto rightHandPtr = std::make_unique<Bone>("right_hand", rightLowerArm);
    rightHandPtr->setLocalPosition(glm::vec3(0.0f, -handLength, 0.0f));
    m_boneMap["right_hand"] = rightHandPtr.get();
    m_bones.push_back(std::move(rightHandPtr));

    // Left leg chain
    auto leftUpperLegPtr = std::make_unique<Bone>("left_upper_leg", m_rootBone.get());
    leftUpperLegPtr->setLocalPosition(glm::vec3(-hipWidth / 2.0f, -upperLegHeight, 0.0f));
    m_boneMap["left_upper_leg"] = leftUpperLegPtr.get();
    Bone* leftUpperLeg = leftUpperLegPtr.get();
    m_bones.push_back(std::move(leftUpperLegPtr));

    auto leftLowerLegPtr = std::make_unique<Bone>("left_lower_leg", leftUpperLeg);
    leftLowerLegPtr->setLocalPosition(glm::vec3(0.0f, -lowerLegHeight, 0.0f));
    m_boneMap["left_lower_leg"] = leftLowerLegPtr.get();
    Bone* leftLowerLeg = leftLowerLegPtr.get();
    m_bones.push_back(std::move(leftLowerLegPtr));

    auto leftFootPtr = std::make_unique<Bone>("left_foot", leftLowerLeg);
    leftFootPtr->setLocalPosition(glm::vec3(0.0f, -height * 0.0625f, height * 0.0625f)); // Foot forward
    m_boneMap["left_foot"] = leftFootPtr.get();
    m_bones.push_back(std::move(leftFootPtr));

    // Right leg chain (mirror of left)
    auto rightUpperLegPtr = std::make_unique<Bone>("right_upper_leg", m_rootBone.get());
    rightUpperLegPtr->setLocalPosition(glm::vec3(hipWidth / 2.0f, -upperLegHeight, 0.0f));
    m_boneMap["right_upper_leg"] = rightUpperLegPtr.get();
    Bone* rightUpperLeg = rightUpperLegPtr.get();
    m_bones.push_back(std::move(rightUpperLegPtr));

    auto rightLowerLegPtr = std::make_unique<Bone>("right_lower_leg", rightUpperLeg);
    rightLowerLegPtr->setLocalPosition(glm::vec3(0.0f, -lowerLegHeight, 0.0f));
    m_boneMap["right_lower_leg"] = rightLowerLegPtr.get();
    Bone* rightLowerLeg = rightLowerLegPtr.get();
    m_bones.push_back(std::move(rightLowerLegPtr));

    auto rightFootPtr = std::make_unique<Bone>("right_foot", rightLowerLeg);
    rightFootPtr->setLocalPosition(glm::vec3(0.0f, -height * 0.0625f, height * 0.0625f)); // Foot forward
    m_boneMap["right_foot"] = rightFootPtr.get();
    m_bones.push_back(std::move(rightFootPtr));
}

Bone* HumanoidSkeleton::getBone(BoneName name)
{
    return getBone(boneNameToString(name));
}

const Bone* HumanoidSkeleton::getBone(BoneName name) const
{
    return getBone(boneNameToString(name));
}

Bone* HumanoidSkeleton::getBone(const std::string& name)
{
    auto it = m_boneMap.find(name);
    return (it != m_boneMap.end()) ? it->second : nullptr;
}

const Bone* HumanoidSkeleton::getBone(const std::string& name) const
{
    auto it = m_boneMap.find(name);
    return (it != m_boneMap.end()) ? it->second : nullptr;
}

void HumanoidSkeleton::setProportions(const std::unordered_map<std::string, float>& proportions)
{
    for (const auto& [boneName, scale] : proportions) {
        Bone* bone = getBone(boneName);
        if (bone) {
            // Scale the local position from parent
            glm::vec3 pos = bone->getLocalPosition();
            bone->setLocalPosition(pos * scale);
        }
    }
}

void HumanoidSkeleton::resetToPose()
{
    // Reset all bone rotations to identity (T-pose)
    for (auto& [name, bone] : m_boneMap) {
        bone->setLocalRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    }
}

std::string HumanoidSkeleton::boneNameToString(BoneName name)
{
    switch (name) {
        case BoneName::Root: return "root";
        case BoneName::Spine: return "spine";
        case BoneName::Chest: return "chest";
        case BoneName::Head: return "head";
        case BoneName::LeftShoulder: return "left_shoulder";
        case BoneName::LeftUpperArm: return "left_upper_arm";
        case BoneName::LeftLowerArm: return "left_lower_arm";
        case BoneName::LeftHand: return "left_hand";
        case BoneName::RightShoulder: return "right_shoulder";
        case BoneName::RightUpperArm: return "right_upper_arm";
        case BoneName::RightLowerArm: return "right_lower_arm";
        case BoneName::RightHand: return "right_hand";
        case BoneName::LeftUpperLeg: return "left_upper_leg";
        case BoneName::LeftLowerLeg: return "left_lower_leg";
        case BoneName::LeftFoot: return "left_foot";
        case BoneName::RightUpperLeg: return "right_upper_leg";
        case BoneName::RightLowerLeg: return "right_lower_leg";
        case BoneName::RightFoot: return "right_foot";
        default: return "unknown";
    }
}

} // namespace fresh
