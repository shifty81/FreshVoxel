#pragma once
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "character/VoxelCharacter.h"

namespace fresh
{

/**
 * @brief Type of weapon grip
 */
enum class GripType {
    OneHanded,  // Sword, axe, wand
    TwoHanded,  // Greatsword, staff, bow
    DualWield,  // Two one-handed weapons
    Shield,     // Shield in off-hand
    Tool        // Pickaxe, shovel, hammer
};

/**
 * @brief Which hand(s) hold the weapon
 */
enum class AttachmentSlot {
    RightHand,
    LeftHand,
    BothHands,
    Back,   // Sheathed on back
    Hip     // Sheathed on hip
};

/**
 * @brief Type of swing/use animation
 */
enum class SwingType {
    Slash,      // Horizontal arc
    Overhead,   // Vertical overhead
    Thrust,     // Forward stab
    Chop,       // Downward chop (axes)
    Sweep,      // Wide horizontal sweep
    Block       // Raise to block
};

/**
 * @brief Definition of a weapon or tool
 */
struct WeaponDefinition {
    std::string name;
    GripType gripType = GripType::OneHanded;
    float length = 1.0f;       // Weapon length in voxel units
    float weight = 1.0f;       // Affects swing speed (0.5 light - 3.0 heavy)
    glm::vec3 gripOffset{0.0f}; // Offset from hand bone to grip point
    glm::vec3 dimensions{1, 4, 1}; // Weapon bounding box (width, height, depth)

    // Voxel representation of the weapon
    std::vector<CharacterVoxel> voxels;
};

/**
 * @brief Keyframe for a procedural swing animation
 */
struct SwingKeyframe {
    float time;              // Time in seconds
    glm::vec3 handRotation;  // Hand bone rotation (Euler, radians)
    glm::vec3 handOffset;    // Additional hand position offset
};

/**
 * @brief A generated swing animation sequence
 */
struct SwingAnimation {
    SwingType type;
    float duration;               // Total duration in seconds
    std::vector<SwingKeyframe> keyframes;
    bool mirrored = false;        // True if generated for left hand
};

/**
 * @brief System for attaching weapons/tools to character bones and generating swing animations
 *
 * Manages weapon attachment points, grip calculations, and procedural
 * swing animation generation for character-held items.
 */
class WeaponAttachment
{
public:
    WeaponAttachment();
    ~WeaponAttachment();

    /**
     * @brief Attach a weapon to a character
     * @param character Character to attach weapon to
     * @param weapon Weapon definition
     * @param slot Where to attach the weapon
     * @return true if attachment succeeded
     */
    bool attach(VoxelCharacter& character, const WeaponDefinition& weapon, AttachmentSlot slot);

    /**
     * @brief Detach weapon from a slot
     * @param slot Slot to detach from
     */
    void detach(AttachmentSlot slot);

    /**
     * @brief Detach all weapons
     */
    void detachAll();

    /**
     * @brief Check if a slot has a weapon attached
     * @param slot Slot to check
     * @return true if weapon is attached
     */
    bool hasWeapon(AttachmentSlot slot) const;

    /**
     * @brief Get the weapon in a slot
     * @param slot Slot to query
     * @return Pointer to weapon definition, or nullptr if empty
     */
    const WeaponDefinition* getWeapon(AttachmentSlot slot) const;

    /**
     * @brief Calculate grip position in world space for a hand
     * @param character Character holding the weapon
     * @param slot Attachment slot
     * @return World-space grip position
     */
    glm::vec3 calculateGripPosition(const VoxelCharacter& character, AttachmentSlot slot) const;

    /**
     * @brief Generate a procedural swing animation
     * @param weapon Weapon to swing
     * @param swingType Type of swing
     * @param slot Which hand is swinging
     * @return Generated swing animation
     */
    SwingAnimation generateSwing(const WeaponDefinition& weapon, SwingType swingType,
                                 AttachmentSlot slot) const;

    /**
     * @brief Get transformed weapon voxels in world space
     * @param character Character holding the weapon
     * @param slot Slot to get voxels for
     * @return Weapon voxels positioned relative to character
     */
    std::vector<CharacterVoxel> getTransformedWeaponVoxels(const VoxelCharacter& character,
                                                           AttachmentSlot slot) const;

    /**
     * @brief Get the bone name for an attachment slot
     * @param slot Attachment slot
     * @return Bone name string
     */
    static std::string getBoneNameForSlot(AttachmentSlot slot);

    /**
     * @brief Check if a weapon grip type is compatible with a slot
     * @param gripType Weapon grip type
     * @param slot Target attachment slot
     * @return true if compatible
     */
    static bool isCompatible(GripType gripType, AttachmentSlot slot);

    /**
     * @brief Get the number of currently attached weapons
     */
    int getAttachedCount() const;

private:
    /**
     * @brief Generate a slash swing animation
     */
    SwingAnimation generateSlash(const WeaponDefinition& weapon, AttachmentSlot slot) const;

    /**
     * @brief Generate an overhead swing animation
     */
    SwingAnimation generateOverhead(const WeaponDefinition& weapon, AttachmentSlot slot) const;

    /**
     * @brief Generate a thrust animation
     */
    SwingAnimation generateThrust(const WeaponDefinition& weapon, AttachmentSlot slot) const;

    /**
     * @brief Generate a chop animation
     */
    SwingAnimation generateChop(const WeaponDefinition& weapon, AttachmentSlot slot) const;

    /**
     * @brief Generate a sweep animation
     */
    SwingAnimation generateSweep(const WeaponDefinition& weapon, AttachmentSlot slot) const;

    /**
     * @brief Generate a block animation
     */
    SwingAnimation generateBlock(const WeaponDefinition& weapon, AttachmentSlot slot) const;

    /**
     * @brief Calculate swing speed based on weapon weight
     * @param weight Weapon weight
     * @return Duration multiplier (heavier = slower)
     */
    static float calculateSwingSpeed(float weight);

    struct AttachmentInfo {
        WeaponDefinition weapon;
        AttachmentSlot slot;
        bool occupied = false;
    };

    // Attachment slots
    AttachmentInfo rightHand;
    AttachmentInfo leftHand;
    AttachmentInfo back;
    AttachmentInfo hip;
};

} // namespace fresh
