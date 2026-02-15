#pragma once
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{

// Forward declarations
class VoxelWorld;

/**
 * @brief Represents a single voxel in a character model
 */
struct CharacterVoxel {
    glm::ivec3 position;   // Position relative to character origin
    glm::vec3 color;       // RGB color (0-1 range)
    uint8_t boneIndex = 0; // Index of bone this voxel is attached to
    float weight = 1.0f;   // Bone weight (for smooth deformation)
};

/**
 * @brief Represents a bone in the character skeleton
 */
struct Bone {
    std::string name;
    int parentIndex = -1;          // Index of parent bone (-1 for root)
    glm::vec3 localPosition{0.0f}; // Position relative to parent
    glm::vec3 localRotation{0.0f}; // Euler angles (radians)
    glm::vec3 localScale{1.0f};    // Scale

    // Calculated transforms (updated during animation)
    glm::mat4 localTransform{1.0f};
    glm::mat4 globalTransform{1.0f};
};

/**
 * @brief Character body part type for modular generation
 */
enum class BodyPartType {
    Head,
    Torso,
    LeftArm,
    RightArm,
    LeftLeg,
    RightLeg,
    LeftHand,
    RightHand,
    LeftFoot,
    RightFoot
};

/**
 * @brief Template for a character body part
 */
struct BodyPartTemplate {
    BodyPartType type;
    std::vector<CharacterVoxel> voxels;
    glm::vec3 attachmentPoint{0.0f}; // Where this part connects to parent
    glm::ivec3 dimensions{0};        // Bounding box dimensions
};

/**
 * @brief Parameters for procedural character generation
 */
struct CharacterGenerationParams {
    // Body proportions (multipliers)
    float headScale = 1.0f;  // 0.5 - 1.5
    float torsoScale = 1.0f; // 0.8 - 1.2
    float limbScale = 1.0f;  // 0.8 - 1.2

    // Height (in voxels)
    int baseHeight = 16; // 12 - 20 voxels

    // Color palette
    glm::vec3 skinColor{0.8f, 0.6f, 0.5f};
    glm::vec3 primaryColor{0.2f, 0.4f, 0.8f};   // Clothing/armor
    glm::vec3 secondaryColor{0.3f, 0.3f, 0.3f}; // Accent color

    // Style
    bool bulky = false;   // Muscular vs slim build
    bool armored = false; // Add armor voxels

    // Random seed for generation
    uint32_t seed = 0;
};

/**
 * @brief A procedurally generated voxel character
 *
 * Represents a character made of voxels with a skeletal rig for animation.
 * Supports modular assembly and algorithmic generation.
 */
class VoxelCharacter
{
public:
    VoxelCharacter();
    ~VoxelCharacter();

    /**
     * @brief Generate character from parameters
     * @param params Generation parameters
     */
    void generateFromParams(const CharacterGenerationParams& params);

    /**
     * @brief Assemble character from body part templates
     * @param parts Map of body part templates to use
     */
    void assembleFromParts(const std::vector<BodyPartTemplate>& parts);

    /**
     * @brief Update character animation and transforms
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);

    /**
     * @brief Get all voxels with world-space positions
     * @return Vector of voxels in world space
     */
    std::vector<CharacterVoxel> getTransformedVoxels() const;

    /**
     * @brief Get character skeleton
     */
    const std::vector<Bone>& getSkeleton() const
    {
        return skeleton;
    }
    std::vector<Bone>& getSkeleton()
    {
        return skeleton;
    }

    /**
     * @brief Set bone transform (for animation)
     * @param boneIndex Index of bone to modify
     * @param boneRotation Euler angles in radians
     */
    void setBoneRotation(int boneIndex, const glm::vec3& boneRotation);

    /**
     * @brief Get bone by name
     * @param name Bone name
     * @return Bone index or -1 if not found
     */
    int getBoneIndex(const std::string& name) const;

    // Position and orientation
    void setPosition(const glm::vec3& pos)
    {
        position = pos;
    }
    glm::vec3 getPosition() const
    {
        return position;
    }

    void setRotation(const glm::vec3& rot)
    {
        rotation = rot;
    }
    glm::vec3 getRotation() const
    {
        return rotation;
    }

    // Dimensions
    glm::ivec3 getDimensions() const
    {
        return dimensions;
    }

private:
    /**
     * @brief Initialize standard humanoid skeleton
     */
    void createHumanoidSkeleton();

    /**
     * @brief Generate voxels algorithmically
     * @param params Generation parameters
     */
    void generateVoxelsAlgorithmic(const CharacterGenerationParams& params);

    /**
     * @brief Update bone transforms recursively
     * @param boneIndex Current bone
     * @param parentTransform Parent's global transform
     */
    void updateBoneTransforms(int boneIndex, const glm::mat4& parentTransform);

    /**
     * @brief Calculate local transform matrix for a bone
     * @param bone Bone to calculate for
     * @return Transform matrix
     */
    glm::mat4 calculateBoneTransform(const Bone& bone) const;

    std::vector<CharacterVoxel> voxels;
    std::vector<Bone> skeleton;

    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f}; // Euler angles
    glm::ivec3 dimensions{0}; // Bounding box

    CharacterGenerationParams generationParams;
};

} // namespace fresh
