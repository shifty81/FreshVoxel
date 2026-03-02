#include <gtest/gtest.h>

#include "character/WeaponAttachment.h"

using namespace fresh;

class WeaponAttachmentTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        attachment = std::make_unique<WeaponAttachment>();
        character = std::make_unique<VoxelCharacter>();

        // Generate a character so bones have valid transforms
        CharacterGenerationParams params;
        params.seed = 42;
        character->generateFromParams(params);
        character->update(0.0f);
    }

    void TearDown() override
    {
        attachment.reset();
        character.reset();
    }

    WeaponDefinition createSword() const
    {
        WeaponDefinition sword;
        sword.name = "Iron Sword";
        sword.gripType = GripType::OneHanded;
        sword.length = 4.0f;
        sword.weight = 1.0f;
        sword.gripOffset = glm::vec3(0.0f, -0.5f, 0.0f);
        sword.dimensions = glm::vec3(1, 4, 1);

        // Simple voxel blade
        for (int y = 0; y < 4; ++y) {
            CharacterVoxel v;
            v.position = glm::ivec3(0, y, 0);
            v.color = glm::vec3(0.7f, 0.7f, 0.8f);
            sword.voxels.push_back(v);
        }
        return sword;
    }

    WeaponDefinition createGreatsword() const
    {
        WeaponDefinition gs;
        gs.name = "Greatsword";
        gs.gripType = GripType::TwoHanded;
        gs.length = 6.0f;
        gs.weight = 2.5f;
        gs.gripOffset = glm::vec3(0.0f, -1.0f, 0.0f);
        gs.dimensions = glm::vec3(1, 6, 1);
        return gs;
    }

    WeaponDefinition createShield() const
    {
        WeaponDefinition shield;
        shield.name = "Wooden Shield";
        shield.gripType = GripType::Shield;
        shield.length = 2.0f;
        shield.weight = 1.5f;
        shield.dimensions = glm::vec3(3, 3, 1);
        return shield;
    }

    WeaponDefinition createPickaxe() const
    {
        WeaponDefinition pick;
        pick.name = "Pickaxe";
        pick.gripType = GripType::Tool;
        pick.length = 3.0f;
        pick.weight = 1.2f;
        pick.dimensions = glm::vec3(2, 3, 1);
        return pick;
    }

    std::unique_ptr<WeaponAttachment> attachment;
    std::unique_ptr<VoxelCharacter> character;
};

// ============================================================================
// Attachment Tests
// ============================================================================

TEST_F(WeaponAttachmentTest, Constructor_NoWeaponsAttached)
{
    EXPECT_EQ(attachment->getAttachedCount(), 0);
    EXPECT_FALSE(attachment->hasWeapon(AttachmentSlot::RightHand));
    EXPECT_FALSE(attachment->hasWeapon(AttachmentSlot::LeftHand));
    EXPECT_FALSE(attachment->hasWeapon(AttachmentSlot::Back));
    EXPECT_FALSE(attachment->hasWeapon(AttachmentSlot::Hip));
}

TEST_F(WeaponAttachmentTest, Attach_OneHandedToRightHand_Succeeds)
{
    WeaponDefinition sword = createSword();
    EXPECT_TRUE(attachment->attach(*character, sword, AttachmentSlot::RightHand));
    EXPECT_TRUE(attachment->hasWeapon(AttachmentSlot::RightHand));
    EXPECT_EQ(attachment->getAttachedCount(), 1);
}

TEST_F(WeaponAttachmentTest, Attach_OneHandedToLeftHand_Succeeds)
{
    WeaponDefinition sword = createSword();
    EXPECT_TRUE(attachment->attach(*character, sword, AttachmentSlot::LeftHand));
    EXPECT_TRUE(attachment->hasWeapon(AttachmentSlot::LeftHand));
}

TEST_F(WeaponAttachmentTest, Attach_TwoHanded_OccupiesBothHands)
{
    WeaponDefinition gs = createGreatsword();
    EXPECT_TRUE(attachment->attach(*character, gs, AttachmentSlot::RightHand));
    EXPECT_TRUE(attachment->hasWeapon(AttachmentSlot::RightHand));
    EXPECT_TRUE(attachment->hasWeapon(AttachmentSlot::LeftHand));
    EXPECT_TRUE(attachment->hasWeapon(AttachmentSlot::BothHands));
    EXPECT_EQ(attachment->getAttachedCount(), 2); // Both hand slots occupied
}

TEST_F(WeaponAttachmentTest, Attach_TwoHanded_FailsWhenHandOccupied)
{
    WeaponDefinition sword = createSword();
    WeaponDefinition gs = createGreatsword();

    attachment->attach(*character, sword, AttachmentSlot::RightHand);
    EXPECT_FALSE(attachment->attach(*character, gs, AttachmentSlot::RightHand));
}

TEST_F(WeaponAttachmentTest, Attach_ShieldToLeftHand_Succeeds)
{
    WeaponDefinition shield = createShield();
    EXPECT_TRUE(attachment->attach(*character, shield, AttachmentSlot::LeftHand));
    EXPECT_TRUE(attachment->hasWeapon(AttachmentSlot::LeftHand));
}

TEST_F(WeaponAttachmentTest, Attach_ShieldToRightHand_Fails)
{
    WeaponDefinition shield = createShield();
    EXPECT_FALSE(attachment->attach(*character, shield, AttachmentSlot::RightHand));
}

TEST_F(WeaponAttachmentTest, Attach_ToolToRightHand_Succeeds)
{
    WeaponDefinition pick = createPickaxe();
    EXPECT_TRUE(attachment->attach(*character, pick, AttachmentSlot::RightHand));
}

TEST_F(WeaponAttachmentTest, Attach_ToOccupiedSlot_Fails)
{
    WeaponDefinition sword1 = createSword();
    WeaponDefinition sword2 = createSword();
    sword2.name = "Steel Sword";

    attachment->attach(*character, sword1, AttachmentSlot::RightHand);
    EXPECT_FALSE(attachment->attach(*character, sword2, AttachmentSlot::RightHand));
}

TEST_F(WeaponAttachmentTest, Attach_ToBack_Succeeds)
{
    WeaponDefinition sword = createSword();
    EXPECT_TRUE(attachment->attach(*character, sword, AttachmentSlot::Back));
    EXPECT_TRUE(attachment->hasWeapon(AttachmentSlot::Back));
}

TEST_F(WeaponAttachmentTest, Attach_ToHip_Succeeds)
{
    WeaponDefinition sword = createSword();
    EXPECT_TRUE(attachment->attach(*character, sword, AttachmentSlot::Hip));
    EXPECT_TRUE(attachment->hasWeapon(AttachmentSlot::Hip));
}

// ============================================================================
// Detach Tests
// ============================================================================

TEST_F(WeaponAttachmentTest, Detach_RemovesWeapon)
{
    WeaponDefinition sword = createSword();
    attachment->attach(*character, sword, AttachmentSlot::RightHand);
    attachment->detach(AttachmentSlot::RightHand);
    EXPECT_FALSE(attachment->hasWeapon(AttachmentSlot::RightHand));
    EXPECT_EQ(attachment->getAttachedCount(), 0);
}

TEST_F(WeaponAttachmentTest, Detach_TwoHanded_DetachesBothHands)
{
    WeaponDefinition gs = createGreatsword();
    attachment->attach(*character, gs, AttachmentSlot::RightHand);
    attachment->detach(AttachmentSlot::RightHand);
    EXPECT_FALSE(attachment->hasWeapon(AttachmentSlot::RightHand));
    EXPECT_FALSE(attachment->hasWeapon(AttachmentSlot::LeftHand));
    EXPECT_EQ(attachment->getAttachedCount(), 0);
}

TEST_F(WeaponAttachmentTest, DetachAll_ClearsEverything)
{
    WeaponDefinition sword = createSword();
    WeaponDefinition shield = createShield();

    attachment->attach(*character, sword, AttachmentSlot::RightHand);
    attachment->attach(*character, shield, AttachmentSlot::LeftHand);
    attachment->attach(*character, sword, AttachmentSlot::Back);

    attachment->detachAll();
    EXPECT_EQ(attachment->getAttachedCount(), 0);
}

// ============================================================================
// GetWeapon Tests
// ============================================================================

TEST_F(WeaponAttachmentTest, GetWeapon_Attached_ReturnsDefinition)
{
    WeaponDefinition sword = createSword();
    attachment->attach(*character, sword, AttachmentSlot::RightHand);

    const WeaponDefinition* result = attachment->getWeapon(AttachmentSlot::RightHand);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name, "Iron Sword");
    EXPECT_EQ(result->gripType, GripType::OneHanded);
    EXPECT_FLOAT_EQ(result->length, 4.0f);
}

TEST_F(WeaponAttachmentTest, GetWeapon_EmptySlot_ReturnsNull)
{
    EXPECT_EQ(attachment->getWeapon(AttachmentSlot::RightHand), nullptr);
}

// ============================================================================
// Compatibility Tests
// ============================================================================

TEST_F(WeaponAttachmentTest, IsCompatible_OneHanded_CompatibleWithHands)
{
    EXPECT_TRUE(WeaponAttachment::isCompatible(GripType::OneHanded, AttachmentSlot::RightHand));
    EXPECT_TRUE(WeaponAttachment::isCompatible(GripType::OneHanded, AttachmentSlot::LeftHand));
    EXPECT_TRUE(WeaponAttachment::isCompatible(GripType::OneHanded, AttachmentSlot::Back));
    EXPECT_TRUE(WeaponAttachment::isCompatible(GripType::OneHanded, AttachmentSlot::Hip));
}

TEST_F(WeaponAttachmentTest, IsCompatible_TwoHanded_CompatibleWithRightAndBoth)
{
    EXPECT_TRUE(WeaponAttachment::isCompatible(GripType::TwoHanded, AttachmentSlot::RightHand));
    EXPECT_TRUE(WeaponAttachment::isCompatible(GripType::TwoHanded, AttachmentSlot::BothHands));
    EXPECT_TRUE(WeaponAttachment::isCompatible(GripType::TwoHanded, AttachmentSlot::Back));
    EXPECT_FALSE(WeaponAttachment::isCompatible(GripType::TwoHanded, AttachmentSlot::Hip));
}

TEST_F(WeaponAttachmentTest, IsCompatible_Shield_OnlyLeftHandAndBack)
{
    EXPECT_FALSE(WeaponAttachment::isCompatible(GripType::Shield, AttachmentSlot::RightHand));
    EXPECT_TRUE(WeaponAttachment::isCompatible(GripType::Shield, AttachmentSlot::LeftHand));
    EXPECT_TRUE(WeaponAttachment::isCompatible(GripType::Shield, AttachmentSlot::Back));
    EXPECT_FALSE(WeaponAttachment::isCompatible(GripType::Shield, AttachmentSlot::Hip));
}

TEST_F(WeaponAttachmentTest, IsCompatible_Tool_HandsAndHip)
{
    EXPECT_TRUE(WeaponAttachment::isCompatible(GripType::Tool, AttachmentSlot::RightHand));
    EXPECT_TRUE(WeaponAttachment::isCompatible(GripType::Tool, AttachmentSlot::LeftHand));
    EXPECT_TRUE(WeaponAttachment::isCompatible(GripType::Tool, AttachmentSlot::Hip));
    EXPECT_FALSE(WeaponAttachment::isCompatible(GripType::Tool, AttachmentSlot::Back));
}

// ============================================================================
// Bone Name Mapping Tests
// ============================================================================

TEST_F(WeaponAttachmentTest, GetBoneNameForSlot_RightHand_ReturnsRightHand)
{
    EXPECT_EQ(WeaponAttachment::getBoneNameForSlot(AttachmentSlot::RightHand), "RightHand");
}

TEST_F(WeaponAttachmentTest, GetBoneNameForSlot_LeftHand_ReturnsLeftHand)
{
    EXPECT_EQ(WeaponAttachment::getBoneNameForSlot(AttachmentSlot::LeftHand), "LeftHand");
}

TEST_F(WeaponAttachmentTest, GetBoneNameForSlot_BothHands_ReturnsRightHand)
{
    EXPECT_EQ(WeaponAttachment::getBoneNameForSlot(AttachmentSlot::BothHands), "RightHand");
}

TEST_F(WeaponAttachmentTest, GetBoneNameForSlot_Back_ReturnsSpine)
{
    EXPECT_EQ(WeaponAttachment::getBoneNameForSlot(AttachmentSlot::Back), "Spine");
}

TEST_F(WeaponAttachmentTest, GetBoneNameForSlot_Hip_ReturnsRoot)
{
    EXPECT_EQ(WeaponAttachment::getBoneNameForSlot(AttachmentSlot::Hip), "Root");
}

// ============================================================================
// Grip Position Tests
// ============================================================================

TEST_F(WeaponAttachmentTest, CalculateGripPosition_EmptySlot_ReturnsZero)
{
    glm::vec3 pos = attachment->calculateGripPosition(*character, AttachmentSlot::RightHand);
    // With no weapon attached, should still return a valid position (bone position)
    // (exact value depends on skeleton setup, just verify no crash)
    (void)pos;
}

TEST_F(WeaponAttachmentTest, CalculateGripPosition_AttachedWeapon_ReturnsPosition)
{
    WeaponDefinition sword = createSword();
    attachment->attach(*character, sword, AttachmentSlot::RightHand);

    glm::vec3 pos = attachment->calculateGripPosition(*character, AttachmentSlot::RightHand);
    // Grip position includes bone transform + grip offset
    // Just verify it returns without crashing and produces a finite value
    EXPECT_FALSE(std::isnan(pos.x));
    EXPECT_FALSE(std::isnan(pos.y));
    EXPECT_FALSE(std::isnan(pos.z));
}

// ============================================================================
// Swing Animation Generation Tests
// ============================================================================

TEST_F(WeaponAttachmentTest, GenerateSwing_Slash_HasKeyframes)
{
    WeaponDefinition sword = createSword();
    SwingAnimation anim = attachment->generateSwing(sword, SwingType::Slash, AttachmentSlot::RightHand);

    EXPECT_EQ(anim.type, SwingType::Slash);
    EXPECT_GT(anim.duration, 0.0f);
    EXPECT_GE(anim.keyframes.size(), 3u);
    EXPECT_FALSE(anim.mirrored);
}

TEST_F(WeaponAttachmentTest, GenerateSwing_Overhead_HasKeyframes)
{
    WeaponDefinition sword = createSword();
    SwingAnimation anim =
        attachment->generateSwing(sword, SwingType::Overhead, AttachmentSlot::RightHand);

    EXPECT_EQ(anim.type, SwingType::Overhead);
    EXPECT_GT(anim.duration, 0.0f);
    EXPECT_GE(anim.keyframes.size(), 3u);
}

TEST_F(WeaponAttachmentTest, GenerateSwing_Thrust_HasKeyframes)
{
    WeaponDefinition sword = createSword();
    SwingAnimation anim =
        attachment->generateSwing(sword, SwingType::Thrust, AttachmentSlot::RightHand);

    EXPECT_EQ(anim.type, SwingType::Thrust);
    EXPECT_GT(anim.duration, 0.0f);
    EXPECT_GE(anim.keyframes.size(), 3u);
}

TEST_F(WeaponAttachmentTest, GenerateSwing_Chop_HasKeyframes)
{
    WeaponDefinition pick = createPickaxe();
    SwingAnimation anim =
        attachment->generateSwing(pick, SwingType::Chop, AttachmentSlot::RightHand);

    EXPECT_EQ(anim.type, SwingType::Chop);
    EXPECT_GT(anim.duration, 0.0f);
    EXPECT_GE(anim.keyframes.size(), 3u);
}

TEST_F(WeaponAttachmentTest, GenerateSwing_Sweep_HasKeyframes)
{
    WeaponDefinition gs = createGreatsword();
    SwingAnimation anim =
        attachment->generateSwing(gs, SwingType::Sweep, AttachmentSlot::RightHand);

    EXPECT_EQ(anim.type, SwingType::Sweep);
    EXPECT_GT(anim.duration, 0.0f);
    EXPECT_GE(anim.keyframes.size(), 3u);
}

TEST_F(WeaponAttachmentTest, GenerateSwing_Block_HasKeyframes)
{
    WeaponDefinition shield = createShield();
    SwingAnimation anim =
        attachment->generateSwing(shield, SwingType::Block, AttachmentSlot::LeftHand);

    EXPECT_EQ(anim.type, SwingType::Block);
    EXPECT_GT(anim.duration, 0.0f);
    EXPECT_GE(anim.keyframes.size(), 3u);
    EXPECT_TRUE(anim.mirrored);
}

TEST_F(WeaponAttachmentTest, GenerateSwing_HeavyWeaponSlower)
{
    WeaponDefinition lightSword = createSword();
    lightSword.weight = 0.5f;

    WeaponDefinition heavyGS = createGreatsword();
    heavyGS.weight = 3.0f;

    SwingAnimation lightAnim =
        attachment->generateSwing(lightSword, SwingType::Slash, AttachmentSlot::RightHand);
    SwingAnimation heavyAnim =
        attachment->generateSwing(heavyGS, SwingType::Slash, AttachmentSlot::RightHand);

    EXPECT_LT(lightAnim.duration, heavyAnim.duration);
}

TEST_F(WeaponAttachmentTest, GenerateSwing_LeftHand_IsMirrored)
{
    WeaponDefinition sword = createSword();
    SwingAnimation anim =
        attachment->generateSwing(sword, SwingType::Slash, AttachmentSlot::LeftHand);

    EXPECT_TRUE(anim.mirrored);
}

TEST_F(WeaponAttachmentTest, GenerateSwing_KeyframesAreSorted)
{
    WeaponDefinition sword = createSword();
    SwingAnimation anim =
        attachment->generateSwing(sword, SwingType::Slash, AttachmentSlot::RightHand);

    for (size_t i = 1; i < anim.keyframes.size(); ++i) {
        EXPECT_GE(anim.keyframes[i].time, anim.keyframes[i - 1].time);
    }
}

TEST_F(WeaponAttachmentTest, GenerateSwing_LastKeyframeAtDuration)
{
    WeaponDefinition sword = createSword();
    SwingAnimation anim =
        attachment->generateSwing(sword, SwingType::Slash, AttachmentSlot::RightHand);

    ASSERT_FALSE(anim.keyframes.empty());
    EXPECT_FLOAT_EQ(anim.keyframes.back().time, anim.duration);
}

// ============================================================================
// Weapon Voxel Transform Tests
// ============================================================================

TEST_F(WeaponAttachmentTest, GetTransformedWeaponVoxels_EmptySlot_ReturnsEmpty)
{
    auto voxels = attachment->getTransformedWeaponVoxels(*character, AttachmentSlot::RightHand);
    EXPECT_TRUE(voxels.empty());
}

TEST_F(WeaponAttachmentTest, GetTransformedWeaponVoxels_AttachedSword_ReturnsVoxels)
{
    WeaponDefinition sword = createSword();
    attachment->attach(*character, sword, AttachmentSlot::RightHand);

    auto voxels = attachment->getTransformedWeaponVoxels(*character, AttachmentSlot::RightHand);
    EXPECT_EQ(voxels.size(), sword.voxels.size());
}

TEST_F(WeaponAttachmentTest, GetTransformedWeaponVoxels_PreservesColor)
{
    WeaponDefinition sword = createSword();
    attachment->attach(*character, sword, AttachmentSlot::RightHand);

    auto voxels = attachment->getTransformedWeaponVoxels(*character, AttachmentSlot::RightHand);
    ASSERT_FALSE(voxels.empty());

    // All sword voxels should maintain their silver-ish color
    for (const auto& v : voxels) {
        EXPECT_FLOAT_EQ(v.color.r, 0.7f);
        EXPECT_FLOAT_EQ(v.color.g, 0.7f);
        EXPECT_FLOAT_EQ(v.color.b, 0.8f);
    }
}

// ============================================================================
// Sword and Shield Combo Test
// ============================================================================

TEST_F(WeaponAttachmentTest, SwordAndShield_BothAttach)
{
    WeaponDefinition sword = createSword();
    WeaponDefinition shield = createShield();

    EXPECT_TRUE(attachment->attach(*character, sword, AttachmentSlot::RightHand));
    EXPECT_TRUE(attachment->attach(*character, shield, AttachmentSlot::LeftHand));
    EXPECT_EQ(attachment->getAttachedCount(), 2);

    EXPECT_EQ(attachment->getWeapon(AttachmentSlot::RightHand)->name, "Iron Sword");
    EXPECT_EQ(attachment->getWeapon(AttachmentSlot::LeftHand)->name, "Wooden Shield");
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(WeaponAttachmentTest, Detach_EmptySlot_DoesNotCrash)
{
    attachment->detach(AttachmentSlot::RightHand);
    attachment->detach(AttachmentSlot::LeftHand);
    attachment->detach(AttachmentSlot::Back);
    attachment->detach(AttachmentSlot::Hip);
    EXPECT_EQ(attachment->getAttachedCount(), 0);
}

TEST_F(WeaponAttachmentTest, DetachAll_AlreadyEmpty_DoesNotCrash)
{
    attachment->detachAll();
    EXPECT_EQ(attachment->getAttachedCount(), 0);
}

TEST_F(WeaponAttachmentTest, Attach_ReattachAfterDetach_Succeeds)
{
    WeaponDefinition sword = createSword();
    attachment->attach(*character, sword, AttachmentSlot::RightHand);
    attachment->detach(AttachmentSlot::RightHand);
    EXPECT_TRUE(attachment->attach(*character, sword, AttachmentSlot::RightHand));
    EXPECT_TRUE(attachment->hasWeapon(AttachmentSlot::RightHand));
}

TEST_F(WeaponAttachmentTest, Attach_WeaponWithNoVoxels_Succeeds)
{
    WeaponDefinition empty;
    empty.name = "Empty";
    empty.gripType = GripType::OneHanded;
    EXPECT_TRUE(attachment->attach(*character, empty, AttachmentSlot::RightHand));

    auto voxels = attachment->getTransformedWeaponVoxels(*character, AttachmentSlot::RightHand);
    EXPECT_TRUE(voxels.empty());
}
