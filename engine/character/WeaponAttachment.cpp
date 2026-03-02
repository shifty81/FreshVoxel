#include "character/WeaponAttachment.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fresh
{

// Constants for swing animation generation
static constexpr float BASE_SWING_DURATION = 0.5f;
static constexpr float PI = glm::pi<float>();

WeaponAttachment::WeaponAttachment() = default;

WeaponAttachment::~WeaponAttachment() = default;

bool WeaponAttachment::attach(VoxelCharacter& character, const WeaponDefinition& weapon,
                              AttachmentSlot slot)
{
    // Validate compatibility
    if (!isCompatible(weapon.gripType, slot)) {
        return false;
    }

    // Verify the target bone exists on the character
    std::string boneName = getBoneNameForSlot(slot);
    if (character.getBoneIndex(boneName) < 0) {
        return false;
    }

    // For two-handed weapons, both hands must be free
    if (weapon.gripType == GripType::TwoHanded) {
        if (rightHand.occupied || leftHand.occupied) {
            return false;
        }
        // Attach to both hands
        rightHand.weapon = weapon;
        rightHand.slot = AttachmentSlot::RightHand;
        rightHand.occupied = true;

        leftHand.weapon = weapon;
        leftHand.slot = AttachmentSlot::LeftHand;
        leftHand.occupied = true;

        return true;
    }

    // Single-slot attachment
    AttachmentInfo* target = nullptr;
    switch (slot) {
    case AttachmentSlot::RightHand:
        target = &rightHand;
        break;
    case AttachmentSlot::LeftHand:
        target = &leftHand;
        break;
    case AttachmentSlot::BothHands:
        // BothHands requires two-handed weapon
        return false;
    case AttachmentSlot::Back:
        target = &back;
        break;
    case AttachmentSlot::Hip:
        target = &hip;
        break;
    }

    if (!target || target->occupied) {
        return false;
    }

    target->weapon = weapon;
    target->slot = slot;
    target->occupied = true;
    return true;
}

void WeaponAttachment::detach(AttachmentSlot slot)
{
    switch (slot) {
    case AttachmentSlot::RightHand:
        // If two-handed, detach from both hands
        if (rightHand.occupied && rightHand.weapon.gripType == GripType::TwoHanded) {
            leftHand.occupied = false;
        }
        rightHand.occupied = false;
        break;
    case AttachmentSlot::LeftHand:
        // If two-handed, detach from both hands
        if (leftHand.occupied && leftHand.weapon.gripType == GripType::TwoHanded) {
            rightHand.occupied = false;
        }
        leftHand.occupied = false;
        break;
    case AttachmentSlot::BothHands:
        rightHand.occupied = false;
        leftHand.occupied = false;
        break;
    case AttachmentSlot::Back:
        back.occupied = false;
        break;
    case AttachmentSlot::Hip:
        hip.occupied = false;
        break;
    }
}

void WeaponAttachment::detachAll()
{
    rightHand.occupied = false;
    leftHand.occupied = false;
    back.occupied = false;
    hip.occupied = false;
}

bool WeaponAttachment::hasWeapon(AttachmentSlot slot) const
{
    switch (slot) {
    case AttachmentSlot::RightHand:
        return rightHand.occupied;
    case AttachmentSlot::LeftHand:
        return leftHand.occupied;
    case AttachmentSlot::BothHands:
        return rightHand.occupied && leftHand.occupied;
    case AttachmentSlot::Back:
        return back.occupied;
    case AttachmentSlot::Hip:
        return hip.occupied;
    }
    return false;
}

const WeaponDefinition* WeaponAttachment::getWeapon(AttachmentSlot slot) const
{
    switch (slot) {
    case AttachmentSlot::RightHand:
        return rightHand.occupied ? &rightHand.weapon : nullptr;
    case AttachmentSlot::LeftHand:
        return leftHand.occupied ? &leftHand.weapon : nullptr;
    case AttachmentSlot::BothHands:
        return rightHand.occupied ? &rightHand.weapon : nullptr;
    case AttachmentSlot::Back:
        return back.occupied ? &back.weapon : nullptr;
    case AttachmentSlot::Hip:
        return hip.occupied ? &hip.weapon : nullptr;
    }
    return nullptr;
}

glm::vec3 WeaponAttachment::calculateGripPosition(const VoxelCharacter& character,
                                                   AttachmentSlot slot) const
{
    std::string boneName = getBoneNameForSlot(slot);
    int boneIndex = character.getBoneIndex(boneName);
    if (boneIndex < 0) {
        return glm::vec3(0.0f);
    }

    const auto& skeleton = character.getSkeleton();
    const auto& bone = skeleton[boneIndex];

    // Grip position is bone global position plus weapon grip offset
    glm::vec3 bonePos = glm::vec3(bone.globalTransform[3]);

    const AttachmentInfo* info = nullptr;
    switch (slot) {
    case AttachmentSlot::RightHand:
        info = rightHand.occupied ? &rightHand : nullptr;
        break;
    case AttachmentSlot::LeftHand:
        info = leftHand.occupied ? &leftHand : nullptr;
        break;
    case AttachmentSlot::BothHands:
        info = rightHand.occupied ? &rightHand : nullptr;
        break;
    case AttachmentSlot::Back:
        info = back.occupied ? &back : nullptr;
        break;
    case AttachmentSlot::Hip:
        info = hip.occupied ? &hip : nullptr;
        break;
    }

    if (info) {
        bonePos += info->weapon.gripOffset;
    }

    return bonePos;
}

SwingAnimation WeaponAttachment::generateSwing(const WeaponDefinition& weapon, SwingType swingType,
                                               AttachmentSlot slot) const
{
    switch (swingType) {
    case SwingType::Slash:
        return generateSlash(weapon, slot);
    case SwingType::Overhead:
        return generateOverhead(weapon, slot);
    case SwingType::Thrust:
        return generateThrust(weapon, slot);
    case SwingType::Chop:
        return generateChop(weapon, slot);
    case SwingType::Sweep:
        return generateSweep(weapon, slot);
    case SwingType::Block:
        return generateBlock(weapon, slot);
    }
    // Fallback to slash
    return generateSlash(weapon, slot);
}

std::vector<CharacterVoxel> WeaponAttachment::getTransformedWeaponVoxels(
    const VoxelCharacter& character, AttachmentSlot slot) const
{
    std::vector<CharacterVoxel> result;

    const WeaponDefinition* weapon = getWeapon(slot);
    if (!weapon) {
        return result;
    }

    std::string boneName = getBoneNameForSlot(slot);
    int boneIndex = character.getBoneIndex(boneName);
    if (boneIndex < 0) {
        return result;
    }

    const auto& skeleton = character.getSkeleton();
    const auto& bone = skeleton[boneIndex];
    glm::mat4 boneTransform = bone.globalTransform;

    // Transform each weapon voxel by the bone's global transform
    result.reserve(weapon->voxels.size());
    for (const auto& voxel : weapon->voxels) {
        CharacterVoxel transformed;
        glm::vec4 pos = boneTransform * glm::vec4(glm::vec3(voxel.position) + weapon->gripOffset,
                                                   1.0f);
        transformed.position = glm::ivec3(glm::round(glm::vec3(pos)));
        transformed.color = voxel.color;
        transformed.boneIndex = static_cast<uint8_t>(boneIndex);
        transformed.weight = 1.0f;
        result.push_back(transformed);
    }

    return result;
}

std::string WeaponAttachment::getBoneNameForSlot(AttachmentSlot slot)
{
    switch (slot) {
    case AttachmentSlot::RightHand:
        return "RightHand";
    case AttachmentSlot::LeftHand:
        return "LeftHand";
    case AttachmentSlot::BothHands:
        return "RightHand";
    case AttachmentSlot::Back:
        return "Spine";
    case AttachmentSlot::Hip:
        return "Root";
    }
    return "RightHand";
}

bool WeaponAttachment::isCompatible(GripType gripType, AttachmentSlot slot)
{
    switch (gripType) {
    case GripType::OneHanded:
        return slot == AttachmentSlot::RightHand || slot == AttachmentSlot::LeftHand ||
               slot == AttachmentSlot::Back || slot == AttachmentSlot::Hip;
    case GripType::TwoHanded:
        return slot == AttachmentSlot::RightHand || slot == AttachmentSlot::BothHands ||
               slot == AttachmentSlot::Back;
    case GripType::DualWield:
        return slot == AttachmentSlot::RightHand || slot == AttachmentSlot::LeftHand;
    case GripType::Shield:
        return slot == AttachmentSlot::LeftHand || slot == AttachmentSlot::Back;
    case GripType::Tool:
        return slot == AttachmentSlot::RightHand || slot == AttachmentSlot::LeftHand ||
               slot == AttachmentSlot::Hip;
    }
    return false;
}

int WeaponAttachment::getAttachedCount() const
{
    int count = 0;
    if (rightHand.occupied)
        count++;
    if (leftHand.occupied)
        count++;
    if (back.occupied)
        count++;
    if (hip.occupied)
        count++;
    return count;
}

// ============================================================================
// Procedural Swing Generation
// ============================================================================

float WeaponAttachment::calculateSwingSpeed(float weight)
{
    // Heavier weapons swing slower: duration = base * (0.7 + 0.3 * weight)
    return BASE_SWING_DURATION * (0.7f + 0.3f * std::clamp(weight, 0.5f, 3.0f));
}

SwingAnimation WeaponAttachment::generateSlash(const WeaponDefinition& weapon,
                                               AttachmentSlot slot) const
{
    SwingAnimation anim;
    anim.type = SwingType::Slash;
    anim.duration = calculateSwingSpeed(weapon.weight);
    anim.mirrored = (slot == AttachmentSlot::LeftHand);

    float mirror = anim.mirrored ? -1.0f : 1.0f;

    // Wind-up: arm raised to the side
    anim.keyframes.push_back(
        {0.0f, glm::vec3(0.0f, 0.0f, mirror * PI * 0.25f), glm::vec3(0.0f)});

    // Mid-swing: arm sweeping across
    anim.keyframes.push_back(
        {anim.duration * 0.4f, glm::vec3(0.0f, 0.0f, mirror * -PI * 0.5f), glm::vec3(0.0f)});

    // Follow-through: arm extended past center
    anim.keyframes.push_back(
        {anim.duration * 0.7f, glm::vec3(0.0f, 0.0f, mirror * -PI * 0.75f), glm::vec3(0.0f)});

    // Recovery: return to rest
    anim.keyframes.push_back({anim.duration, glm::vec3(0.0f), glm::vec3(0.0f)});

    return anim;
}

SwingAnimation WeaponAttachment::generateOverhead(const WeaponDefinition& weapon,
                                                  AttachmentSlot slot) const
{
    SwingAnimation anim;
    anim.type = SwingType::Overhead;
    anim.duration = calculateSwingSpeed(weapon.weight) * 1.2f; // Slightly slower
    anim.mirrored = (slot == AttachmentSlot::LeftHand);

    // Wind-up: arm raised above head
    anim.keyframes.push_back({0.0f, glm::vec3(-PI * 0.75f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)});

    // Peak: arm fully extended above
    anim.keyframes.push_back(
        {anim.duration * 0.3f, glm::vec3(-PI * 0.9f, 0.0f, 0.0f), glm::vec3(0.0f, 1.5f, 0.0f)});

    // Strike: arm swings down
    anim.keyframes.push_back(
        {anim.duration * 0.6f, glm::vec3(PI * 0.3f, 0.0f, 0.0f), glm::vec3(0.0f, -0.5f, 0.0f)});

    // Recovery: return to rest
    anim.keyframes.push_back({anim.duration, glm::vec3(0.0f), glm::vec3(0.0f)});

    return anim;
}

SwingAnimation WeaponAttachment::generateThrust(const WeaponDefinition& weapon,
                                                AttachmentSlot slot) const
{
    SwingAnimation anim;
    anim.type = SwingType::Thrust;
    anim.duration = calculateSwingSpeed(weapon.weight) * 0.8f; // Faster than slash
    anim.mirrored = (slot == AttachmentSlot::LeftHand);

    float reach = weapon.length * 0.5f;

    // Wind-up: arm pulled back
    anim.keyframes.push_back(
        {0.0f, glm::vec3(PI * 0.1f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -reach)});

    // Thrust forward
    anim.keyframes.push_back(
        {anim.duration * 0.4f, glm::vec3(-PI * 0.1f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, reach)});

    // Hold
    anim.keyframes.push_back(
        {anim.duration * 0.7f, glm::vec3(-PI * 0.1f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, reach * 0.8f)});

    // Recovery
    anim.keyframes.push_back({anim.duration, glm::vec3(0.0f), glm::vec3(0.0f)});

    return anim;
}

SwingAnimation WeaponAttachment::generateChop(const WeaponDefinition& weapon,
                                              AttachmentSlot slot) const
{
    SwingAnimation anim;
    anim.type = SwingType::Chop;
    anim.duration = calculateSwingSpeed(weapon.weight) * 1.1f;
    anim.mirrored = (slot == AttachmentSlot::LeftHand);

    // Wind-up: arm raised
    anim.keyframes.push_back(
        {0.0f, glm::vec3(-PI * 0.6f, 0.0f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f)});

    // Swing down
    anim.keyframes.push_back(
        {anim.duration * 0.5f, glm::vec3(PI * 0.4f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)});

    // Impact hold
    anim.keyframes.push_back(
        {anim.duration * 0.75f, glm::vec3(PI * 0.3f, 0.0f, 0.0f), glm::vec3(0.0f, -0.8f, 0.0f)});

    // Recovery
    anim.keyframes.push_back({anim.duration, glm::vec3(0.0f), glm::vec3(0.0f)});

    return anim;
}

SwingAnimation WeaponAttachment::generateSweep(const WeaponDefinition& weapon,
                                               AttachmentSlot slot) const
{
    SwingAnimation anim;
    anim.type = SwingType::Sweep;
    anim.duration = calculateSwingSpeed(weapon.weight) * 1.3f; // Wide sweep is slower
    anim.mirrored = (slot == AttachmentSlot::LeftHand);

    float mirror = anim.mirrored ? -1.0f : 1.0f;

    // Wind-up: arm far to side
    anim.keyframes.push_back(
        {0.0f, glm::vec3(0.0f, mirror * PI * 0.4f, mirror * PI * 0.3f), glm::vec3(0.0f)});

    // Sweep across body
    anim.keyframes.push_back(
        {anim.duration * 0.5f, glm::vec3(0.0f, mirror * -PI * 0.6f, mirror * -PI * 0.2f),
         glm::vec3(0.0f)});

    // Extended follow-through
    anim.keyframes.push_back(
        {anim.duration * 0.8f, glm::vec3(0.0f, mirror * -PI * 0.7f, 0.0f), glm::vec3(0.0f)});

    // Recovery
    anim.keyframes.push_back({anim.duration, glm::vec3(0.0f), glm::vec3(0.0f)});

    return anim;
}

SwingAnimation WeaponAttachment::generateBlock(const WeaponDefinition& weapon,
                                               AttachmentSlot slot) const
{
    SwingAnimation anim;
    anim.type = SwingType::Block;
    anim.duration = calculateSwingSpeed(weapon.weight) * 0.6f; // Quick to block
    anim.mirrored = (slot == AttachmentSlot::LeftHand);

    float mirror = anim.mirrored ? -1.0f : 1.0f;

    // Raise to blocking position
    anim.keyframes.push_back({0.0f, glm::vec3(0.0f), glm::vec3(0.0f)});

    // Block pose: arm raised in front
    anim.keyframes.push_back(
        {anim.duration * 0.3f, glm::vec3(-PI * 0.4f, mirror * PI * 0.1f, 0.0f),
         glm::vec3(0.0f, 0.5f, 0.5f)});

    // Hold block
    anim.keyframes.push_back(
        {anim.duration * 0.8f, glm::vec3(-PI * 0.4f, mirror * PI * 0.1f, 0.0f),
         glm::vec3(0.0f, 0.5f, 0.5f)});

    // Lower
    anim.keyframes.push_back({anim.duration, glm::vec3(0.0f), glm::vec3(0.0f)});

    return anim;
}

} // namespace fresh
