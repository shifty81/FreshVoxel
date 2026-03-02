/**
 * @file LuaSeasonBindingsTests.cpp
 * @brief Unit tests for the Lua Season bindings
 */

#include <gtest/gtest.h>
#include "scripting/lua/LuaSeasonBindings.h"
#include "gameplay/SeasonManager.h"

using namespace fresh;
using namespace fresh::scripting;

/**
 * Tests for LuaSeasonBindings stub implementations.
 * These tests verify the stub functions work correctly when Lua is not available.
 * When Lua is available, the actual bindings are tested through integration tests.
 */
class LuaSeasonBindingsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        season = std::make_unique<SeasonManager>();
    }

    void TearDown() override
    {
        season.reset();
    }

    std::unique_ptr<SeasonManager> season;
};

// Test that binding registration doesn't crash with null pointers
TEST_F(LuaSeasonBindingsTest, RegisterBindings_NullEngine_DoesNotCrash)
{
    // Should not crash with nullptr engine
    LuaSeasonBindings::registerBindings(nullptr, season.get());
}

TEST_F(LuaSeasonBindingsTest, RegisterBindings_NullSeasonManager_DoesNotCrash)
{
    // Should not crash with nullptr season manager
    LuaSeasonBindings::registerBindings(nullptr, nullptr);
}

// Test that the SeasonManager has all the methods we're binding
TEST_F(LuaSeasonBindingsTest, SeasonManager_HasAllRequiredMethods)
{
    // Verify all the methods we're binding exist and work
    EXPECT_EQ(season->getCurrentSeason(), Season::Spring);
    EXPECT_EQ(season->getSeasonName(), "Spring");
    EXPECT_TRUE(season->isAutoProgressEnabled());
    EXPECT_EQ(season->getDaysPerSeason(), 7);  // Default: 7 in-game days per season
    
    season->setSeason(Season::Summer);
    EXPECT_EQ(season->getCurrentSeason(), Season::Summer);
    
    season->setAutoProgress(false);
    EXPECT_FALSE(season->isAutoProgressEnabled());
    
    season->setDaysPerSeason(60);
    EXPECT_EQ(season->getDaysPerSeason(), 60);
    
    season->advanceToNextSeason();
    EXPECT_EQ(season->getCurrentSeason(), Season::Fall);
    
    // Test visual effect queries
    glm::vec3 grass = season->getGrassColorTint();
    EXPECT_GE(grass.r, 0.0f);
    EXPECT_LE(grass.r, 1.0f);
    
    glm::vec3 foliage = season->getFoliageColorTint();
    EXPECT_GE(foliage.g, 0.0f);
    EXPECT_LE(foliage.g, 1.0f);
    
    glm::vec3 sky = season->getSkyColorModifier();
    EXPECT_GE(sky.b, 0.0f);
    EXPECT_LE(sky.b, 2.0f);  // Could be modified
    
    float temp = season->getAmbientTemperature();
    EXPECT_GE(temp, 0.0f);
    EXPECT_LE(temp, 1.0f);
    
    float dayLength = season->getDayLengthModifier();
    EXPECT_GT(dayLength, 0.0f);
    EXPECT_LE(dayLength, 1.5f);
}

// Test season type mapping matches binding expectations
TEST_F(LuaSeasonBindingsTest, SeasonTypeMapping_MatchesConstants)
{
    // Verify the enum values we use in bindings match actual Season enum
    season->setSeason(Season::Spring);
    EXPECT_EQ(static_cast<int>(season->getCurrentSeason()), 0);
    EXPECT_EQ(season->getSeasonName(), "Spring");
    EXPECT_FALSE(season->shouldSnow());
    EXPECT_FALSE(season->shouldLeavesfall());
    
    season->setSeason(Season::Summer);
    EXPECT_EQ(static_cast<int>(season->getCurrentSeason()), 1);
    EXPECT_EQ(season->getSeasonName(), "Summer");
    
    season->setSeason(Season::Fall);
    EXPECT_EQ(static_cast<int>(season->getCurrentSeason()), 2);
    EXPECT_EQ(season->getSeasonName(), "Fall");
    EXPECT_TRUE(season->shouldLeavesfall());
    
    season->setSeason(Season::Winter);
    EXPECT_EQ(static_cast<int>(season->getCurrentSeason()), 3);
    EXPECT_EQ(season->getSeasonName(), "Winter");
    EXPECT_TRUE(season->shouldSnow());
}

// Test full year cycle
TEST_F(LuaSeasonBindingsTest, FullYearCycle_AllSeasonsAccessible)
{
    season->setSeason(Season::Spring);
    EXPECT_EQ(season->getCurrentSeason(), Season::Spring);
    
    season->advanceToNextSeason();
    EXPECT_EQ(season->getCurrentSeason(), Season::Summer);
    
    season->advanceToNextSeason();
    EXPECT_EQ(season->getCurrentSeason(), Season::Fall);
    
    season->advanceToNextSeason();
    EXPECT_EQ(season->getCurrentSeason(), Season::Winter);
    
    season->advanceToNextSeason();
    EXPECT_EQ(season->getCurrentSeason(), Season::Spring);  // Full cycle
}

// Test visual effects vary by season
TEST_F(LuaSeasonBindingsTest, VisualEffects_VaryBySeason)
{
    // Spring - lush green
    season->setSeason(Season::Spring);
    glm::vec3 springGrass = season->getGrassColorTint();
    
    // Fall - yellower grass
    season->setSeason(Season::Fall);
    glm::vec3 fallGrass = season->getGrassColorTint();
    
    // Winter - dull grass
    season->setSeason(Season::Winter);
    glm::vec3 winterGrass = season->getGrassColorTint();
    
    // Grass should be different across seasons
    EXPECT_NE(springGrass.g, fallGrass.g);
    EXPECT_NE(springGrass.g, winterGrass.g);
}

// Test temperature varies by season
TEST_F(LuaSeasonBindingsTest, Temperature_VariesBySeason)
{
    season->setSeason(Season::Summer);
    float summerTemp = season->getAmbientTemperature();
    
    season->setSeason(Season::Winter);
    float winterTemp = season->getAmbientTemperature();
    
    // Summer should be warmer than winter
    EXPECT_GT(summerTemp, winterTemp);
}
