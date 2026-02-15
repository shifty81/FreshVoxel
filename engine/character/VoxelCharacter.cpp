#include "character/VoxelCharacter.h"

#include <cmath>
#include <random>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace fresh
{

VoxelCharacter::VoxelCharacter()
{
    createHumanoidSkeleton();
}

VoxelCharacter::~VoxelCharacter() = default;

void VoxelCharacter::generateFromParams(const CharacterGenerationParams& params)
{
    generationParams = params;

    // Clear existing voxels
    voxels.clear();

    // Generate character voxels algorithmically
    generateVoxelsAlgorithmic(params);

    // Update bone transforms
    updateBoneTransforms(0, glm::mat4(1.0f));
}

void VoxelCharacter::assembleFromParts(const std::vector<BodyPartTemplate>& parts)
{
    voxels.clear();

    // Combine all body part voxels
    for (const auto& part : parts) {
        voxels.insert(voxels.end(), part.voxels.begin(), part.voxels.end());
    }

    // Update transforms
    updateBoneTransforms(0, glm::mat4(1.0f));
}

void VoxelCharacter::update(float deltaTime)
{
    // Suppress unused parameter warning
    (void)deltaTime;

    // Update bone transforms based on current rotations
    updateBoneTransforms(0, glm::mat4(1.0f));
}

std::vector<CharacterVoxel> VoxelCharacter::getTransformedVoxels() const
{
    std::vector<CharacterVoxel> transformed = voxels;

    // Apply bone transforms to voxels
    for (auto& voxel : transformed) {
        if (voxel.boneIndex < skeleton.size()) {
            const glm::mat4& boneTransform = skeleton[voxel.boneIndex].globalTransform;
            glm::vec4 transformedPos = boneTransform * glm::vec4(voxel.position, 1.0f);
            voxel.position = glm::ivec3(transformedPos);
        }
    }

    return transformed;
}

void VoxelCharacter::setBoneRotation(int boneIndex, const glm::vec3& boneRotation)
{
    if (boneIndex >= 0 && boneIndex < static_cast<int>(skeleton.size())) {
        skeleton[boneIndex].localRotation = boneRotation;
        updateBoneTransforms(0, glm::mat4(1.0f));
    }
}

int VoxelCharacter::getBoneIndex(const std::string& name) const
{
    for (size_t i = 0; i < skeleton.size(); ++i) {
        if (skeleton[i].name == name) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void VoxelCharacter::createHumanoidSkeleton()
{
    skeleton.clear();

    // Standard humanoid skeleton (15 bones)
    // Index 0: Root (Hips)
    skeleton.push_back({"Root",
                        -1, // No parent
                        {0.0f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 1: Spine
    skeleton.push_back({"Spine",
                        0, // Parent: Root
                        {0.0f, 2.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 2: Neck
    skeleton.push_back({"Neck",
                        1, // Parent: Spine
                        {0.0f, 4.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 3: Head
    skeleton.push_back({"Head",
                        2, // Parent: Neck
                        {0.0f, 1.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 4: LeftShoulder
    skeleton.push_back({"LeftShoulder",
                        1, // Parent: Spine
                        {-1.0f, 3.5f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 5: LeftArm
    skeleton.push_back({"LeftArm",
                        4, // Parent: LeftShoulder
                        {-1.5f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 6: LeftHand
    skeleton.push_back({"LeftHand",
                        5, // Parent: LeftArm
                        {-1.5f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 7: RightShoulder
    skeleton.push_back({"RightShoulder",
                        1, // Parent: Spine
                        {1.0f, 3.5f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 8: RightArm
    skeleton.push_back({"RightArm",
                        7, // Parent: RightShoulder
                        {1.5f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 9: RightHand
    skeleton.push_back({"RightHand",
                        8, // Parent: RightArm
                        {1.5f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 10: LeftHip
    skeleton.push_back({"LeftHip",
                        0, // Parent: Root
                        {-0.5f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 11: LeftLeg
    skeleton.push_back({"LeftLeg",
                        10, // Parent: LeftHip
                        {0.0f, -2.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 12: LeftFoot
    skeleton.push_back({"LeftFoot",
                        11, // Parent: LeftLeg
                        {0.0f, -2.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 13: RightHip
    skeleton.push_back({"RightHip",
                        0, // Parent: Root
                        {0.5f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 14: RightLeg
    skeleton.push_back({"RightLeg",
                        13, // Parent: RightHip
                        {0.0f, -2.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});

    // Index 15: RightFoot
    skeleton.push_back({"RightFoot",
                        14, // Parent: RightLeg
                        {0.0f, -2.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 1.0f}});
}

void VoxelCharacter::generateVoxelsAlgorithmic(const CharacterGenerationParams& params)
{
    std::mt19937 rng(params.seed);
    std::uniform_real_distribution<float> colorDist(0.9f, 1.1f);

    const int baseHeight = params.baseHeight;
    const float headScale = params.headScale;
    const float torsoScale = params.torsoScale;
    const float limbScale = params.limbScale;

    // Head (attached to bone 3 - Head)
    const int headSize = static_cast<int>(4 * headScale);
    const int headHeight = baseHeight - 2;
    for (int x = -headSize / 2; x < headSize / 2; ++x) {
        for (int y = 0; y < headSize; ++y) {
            for (int z = -headSize / 2; z < headSize / 2; ++z) {
                // Spherical head shape
                float dist = std::sqrt(static_cast<float>(x * x + y * y + z * z));
                if (dist < headSize / 2.0f) {
                    CharacterVoxel voxel;
                    voxel.position = {x, headHeight + y, z};
                    voxel.color = params.skinColor * colorDist(rng);
                    voxel.boneIndex = 3; // Head bone
                    voxel.weight = 1.0f;
                    voxels.push_back(voxel);
                }
            }
        }
    }

    // Torso (attached to bone 1 - Spine)
    const int torsoWidth = static_cast<int>(4 * torsoScale);
    const int torsoDepth = static_cast<int>(2 * torsoScale);
    const int torsoHeight = static_cast<int>(6 * torsoScale);
    const int torsoY = baseHeight - 8;
    for (int x = -torsoWidth / 2; x < torsoWidth / 2; ++x) {
        for (int y = 0; y < torsoHeight; ++y) {
            for (int z = -torsoDepth / 2; z < torsoDepth / 2; ++z) {
                CharacterVoxel voxel;
                voxel.position = {x, torsoY + y, z};
                voxel.color = params.primaryColor * colorDist(rng);
                voxel.boneIndex = 1; // Spine bone
                voxel.weight = 1.0f;
                voxels.push_back(voxel);
            }
        }
    }

    // Left Arm (attached to bone 5 - LeftArm)
    const int armWidth = static_cast<int>(2 * limbScale);
    const int armLength = static_cast<int>(4 * limbScale);
    const int leftArmY = torsoY + torsoHeight - 2;
    const int leftArmX = -torsoWidth / 2 - 1;
    for (int x = 0; x < armWidth; ++x) {
        for (int y = 0; y < armLength; ++y) {
            for (int z = -armWidth / 2; z < armWidth / 2; ++z) {
                CharacterVoxel voxel;
                voxel.position = {leftArmX - x, leftArmY - y, z};
                voxel.color = params.skinColor * colorDist(rng);
                voxel.boneIndex = 5; // LeftArm bone
                voxel.weight = 1.0f;
                voxels.push_back(voxel);
            }
        }
    }

    // Right Arm (attached to bone 8 - RightArm)
    const int rightArmX = torsoWidth / 2 + 1;
    for (int x = 0; x < armWidth; ++x) {
        for (int y = 0; y < armLength; ++y) {
            for (int z = -armWidth / 2; z < armWidth / 2; ++z) {
                CharacterVoxel voxel;
                voxel.position = {rightArmX + x, leftArmY - y, z};
                voxel.color = params.skinColor * colorDist(rng);
                voxel.boneIndex = 8; // RightArm bone
                voxel.weight = 1.0f;
                voxels.push_back(voxel);
            }
        }
    }

    // Left Leg (attached to bone 11 - LeftLeg)
    const int legWidth = static_cast<int>(2 * limbScale);
    const int legLength = static_cast<int>(4 * limbScale);
    const int leftLegX = -1;
    const int legY = torsoY;
    for (int x = 0; x < legWidth; ++x) {
        for (int y = 0; y < legLength; ++y) {
            for (int z = -legWidth / 2; z < legWidth / 2; ++z) {
                CharacterVoxel voxel;
                voxel.position = {leftLegX - x, legY - y, z};
                voxel.color = params.secondaryColor * colorDist(rng);
                voxel.boneIndex = 11; // LeftLeg bone
                voxel.weight = 1.0f;
                voxels.push_back(voxel);
            }
        }
    }

    // Right Leg (attached to bone 14 - RightLeg)
    const int rightLegX = 1;
    for (int x = 0; x < legWidth; ++x) {
        for (int y = 0; y < legLength; ++y) {
            for (int z = -legWidth / 2; z < legWidth / 2; ++z) {
                CharacterVoxel voxel;
                voxel.position = {rightLegX + x, legY - y, z};
                voxel.color = params.secondaryColor * colorDist(rng);
                voxel.boneIndex = 14; // RightLeg bone
                voxel.weight = 1.0f;
                voxels.push_back(voxel);
            }
        }
    }

    // Calculate dimensions
    glm::ivec3 minPos(INT_MAX);
    glm::ivec3 maxPos(INT_MIN);
    for (const auto& voxel : voxels) {
        minPos = glm::min(minPos, voxel.position);
        maxPos = glm::max(maxPos, voxel.position);
    }
    dimensions = maxPos - minPos + glm::ivec3(1);
}

void VoxelCharacter::updateBoneTransforms(int boneIndex, const glm::mat4& parentTransform)
{
    if (boneIndex < 0 || boneIndex >= static_cast<int>(skeleton.size())) {
        return;
    }

    Bone& bone = skeleton[boneIndex];

    // Calculate local transform
    bone.localTransform = calculateBoneTransform(bone);

    // Calculate global transform
    bone.globalTransform = parentTransform * bone.localTransform;

    // Update children recursively
    for (size_t i = 0; i < skeleton.size(); ++i) {
        if (skeleton[i].parentIndex == boneIndex) {
            updateBoneTransforms(static_cast<int>(i), bone.globalTransform);
        }
    }
}

glm::mat4 VoxelCharacter::calculateBoneTransform(const Bone& bone) const
{
    glm::mat4 transform(1.0f);

    // Apply translation
    transform = glm::translate(transform, bone.localPosition);

    // Apply rotation (Euler angles)
    transform = glm::rotate(transform, bone.localRotation.x, glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, bone.localRotation.y, glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, bone.localRotation.z, glm::vec3(0, 0, 1));

    // Apply scale
    transform = glm::scale(transform, bone.localScale);

    return transform;
}

} // namespace fresh
