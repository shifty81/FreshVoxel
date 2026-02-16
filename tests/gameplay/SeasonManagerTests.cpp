#include <gtest/gtest.h>
#include "gameplay/SeasonManager.h"

using namespace fresh;

class SeasonManagerTest : public ::testing::Test
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

// Basic initialization tests
TEST_F(SeasonManagerTest, InitialSeason)
{
    EXPECT_EQ(season->getCurrentSeason(), Season::Spring);
    EXPECT_EQ(season->getSeasonName(), "Spring");
}

TEST_F(SeasonManagerTest, InitialAutoProgress)
{
    EXPECT_TRUE(season->isAutoProgressEnabled());
    EXPECT_EQ(season->getDaysPerSeason(), 7);
}

// Set season tests
TEST_F(SeasonManagerTest, SetSeason_AllSeasons)
{
    season->setSeason(Season::Spring);
    EXPECT_EQ(season->getCurrentSeason(), Season::Spring);
    EXPECT_EQ(season->getSeasonName(), "Spring");

    season->setSeason(Season::Summer);
    EXPECT_EQ(season->getCurrentSeason(), Season::Summer);
    EXPECT_EQ(season->getSeasonName(), "Summer");

    season->setSeason(Season::Fall);
    EXPECT_EQ(season->getCurrentSeason(), Season::Fall);
    EXPECT_EQ(season->getSeasonName(), "Fall");

    season->setSeason(Season::Winter);
    EXPECT_EQ(season->getCurrentSeason(), Season::Winter);
    EXPECT_EQ(season->getSeasonName(), "Winter");
}

TEST_F(SeasonManagerTest, SetSeason_SameSeason_NoCallback)
{
    bool callbackCalled = false;
    season->setOnSeasonChange([&callbackCalled](Season) {
        callbackCalled = true;
    });

    // Setting same season should not trigger callback
    season->setSeason(Season::Spring);  // Already Spring
    EXPECT_FALSE(callbackCalled);
}

// Season progression tests
TEST_F(SeasonManagerTest, AdvanceToNextSeason)
{
    EXPECT_EQ(season->getCurrentSeason(), Season::Spring);

    season->advanceToNextSeason();
    EXPECT_EQ(season->getCurrentSeason(), Season::Summer);

    season->advanceToNextSeason();
    EXPECT_EQ(season->getCurrentSeason(), Season::Fall);

    season->advanceToNextSeason();
    EXPECT_EQ(season->getCurrentSeason(), Season::Winter);

    season->advanceToNextSeason();
    EXPECT_EQ(season->getCurrentSeason(), Season::Spring);
}

TEST_F(SeasonManagerTest, FullYearCycle)
{
    // Test a complete year cycle
    for (int year = 0; year < 2; year++) {
        EXPECT_EQ(season->getCurrentSeason(), Season::Spring);
        season->advanceToNextSeason();
        EXPECT_EQ(season->getCurrentSeason(), Season::Summer);
        season->advanceToNextSeason();
        EXPECT_EQ(season->getCurrentSeason(), Season::Fall);
        season->advanceToNextSeason();
        EXPECT_EQ(season->getCurrentSeason(), Season::Winter);
        season->advanceToNextSeason();
    }
}

// Auto-progression tests
TEST_F(SeasonManagerTest, AutoProgression_ChangesSeasonAfterDays)
{
    season->setDaysPerSeason(7);

    // Simulate 7 day changes
    for (int day = 1; day <= 6; day++) {
        season->onDayChange(day);
        EXPECT_EQ(season->getCurrentSeason(), Season::Spring);
    }

    // On day 7, season should advance
    season->onDayChange(7);
    EXPECT_EQ(season->getCurrentSeason(), Season::Summer);
}

TEST_F(SeasonManagerTest, AutoProgression_Disabled)
{
    season->setAutoProgress(false);
    EXPECT_FALSE(season->isAutoProgressEnabled());

    // Simulate many day changes
    for (int day = 1; day <= 100; day++) {
        season->onDayChange(day);
    }

    // Season should not have changed
    EXPECT_EQ(season->getCurrentSeason(), Season::Spring);
}

TEST_F(SeasonManagerTest, SetDaysPerSeason)
{
    season->setDaysPerSeason(14);
    EXPECT_EQ(season->getDaysPerSeason(), 14);

    // Should not change at day 7
    for (int day = 1; day <= 13; day++) {
        season->onDayChange(day);
    }
    EXPECT_EQ(season->getCurrentSeason(), Season::Spring);

    // Should change at day 14
    season->onDayChange(14);
    EXPECT_EQ(season->getCurrentSeason(), Season::Summer);
}

// Visual effects - Grass color
TEST_F(SeasonManagerTest, GrassColorTint_Spring)
{
    season->setSeason(Season::Spring);
    glm::vec3 color = season->getGrassColorTint();
    // Spring grass should be lush green
    EXPECT_GT(color.g, color.r);
}

TEST_F(SeasonManagerTest, GrassColorTint_Fall)
{
    season->setSeason(Season::Fall);
    glm::vec3 color = season->getGrassColorTint();
    // Fall grass should be brownish/yellow
    EXPECT_GT(color.r, color.b);
}

TEST_F(SeasonManagerTest, GrassColorTint_Winter)
{
    season->setSeason(Season::Winter);
    glm::vec3 color = season->getGrassColorTint();
    // Winter grass should be whitish (snow covered)
    EXPECT_GT(color.r, 0.8f);
    EXPECT_GT(color.g, 0.8f);
    EXPECT_GT(color.b, 0.8f);
}

// Visual effects - Foliage color
TEST_F(SeasonManagerTest, FoliageColorTint_Spring)
{
    season->setSeason(Season::Spring);
    glm::vec3 color = season->getFoliageColorTint();
    // Spring foliage should be fresh green
    EXPECT_GT(color.g, color.r);
    EXPECT_GT(color.g, color.b);
}

TEST_F(SeasonManagerTest, FoliageColorTint_Fall)
{
    season->setSeason(Season::Fall);
    glm::vec3 color = season->getFoliageColorTint();
    // Fall foliage should be orange/red
    EXPECT_GT(color.r, color.g);
    EXPECT_GT(color.r, color.b);
}

TEST_F(SeasonManagerTest, FoliageColorTint_Winter)
{
    season->setSeason(Season::Winter);
    glm::vec3 color = season->getFoliageColorTint();
    // Winter foliage should be dark (bare)
    EXPECT_LT(color.r, 0.5f);
    EXPECT_LT(color.g, 0.5f);
    EXPECT_LT(color.b, 0.5f);
}

// Temperature tests
TEST_F(SeasonManagerTest, AmbientTemperature)
{
    season->setSeason(Season::Summer);
    float summerTemp = season->getAmbientTemperature();

    season->setSeason(Season::Winter);
    float winterTemp = season->getAmbientTemperature();

    // Summer should be hotter than winter
    EXPECT_GT(summerTemp, winterTemp);

    // All temperatures should be in valid range
    for (auto s : {Season::Spring, Season::Summer, Season::Fall, Season::Winter}) {
        season->setSeason(s);
        float temp = season->getAmbientTemperature();
        EXPECT_GE(temp, 0.0f);
        EXPECT_LE(temp, 1.0f);
    }
}

// Sky color modifier tests
TEST_F(SeasonManagerTest, SkyColorModifier_AllSeasons)
{
    for (auto s : {Season::Spring, Season::Summer, Season::Fall, Season::Winter}) {
        season->setSeason(s);
        glm::vec3 modifier = season->getSkyColorModifier();
        // All components should be positive
        EXPECT_GT(modifier.r, 0.0f);
        EXPECT_GT(modifier.g, 0.0f);
        EXPECT_GT(modifier.b, 0.0f);
    }
}

// Snow and leaf fall conditions
TEST_F(SeasonManagerTest, ShouldSnow)
{
    season->setSeason(Season::Winter);
    EXPECT_TRUE(season->shouldSnow());

    season->setSeason(Season::Summer);
    EXPECT_FALSE(season->shouldSnow());

    season->setSeason(Season::Spring);
    EXPECT_FALSE(season->shouldSnow());

    season->setSeason(Season::Fall);
    EXPECT_FALSE(season->shouldSnow());
}

TEST_F(SeasonManagerTest, ShouldLeavesFall)
{
    season->setSeason(Season::Fall);
    EXPECT_TRUE(season->shouldLeavesfall());

    season->setSeason(Season::Spring);
    EXPECT_FALSE(season->shouldLeavesfall());

    season->setSeason(Season::Summer);
    EXPECT_FALSE(season->shouldLeavesfall());

    season->setSeason(Season::Winter);
    EXPECT_FALSE(season->shouldLeavesfall());
}

// Day length modifier tests
TEST_F(SeasonManagerTest, DayLengthModifier)
{
    season->setSeason(Season::Summer);
    float summerLength = season->getDayLengthModifier();

    season->setSeason(Season::Winter);
    float winterLength = season->getDayLengthModifier();

    // Summer days should be longer than winter days
    EXPECT_GT(summerLength, winterLength);

    // All modifiers should be positive
    for (auto s : {Season::Spring, Season::Summer, Season::Fall, Season::Winter}) {
        season->setSeason(s);
        EXPECT_GT(season->getDayLengthModifier(), 0.0f);
    }
}

// Callback tests
TEST_F(SeasonManagerTest, OnSeasonChangeCallback)
{
    Season changedTo = Season::Spring;
    int callCount = 0;
    season->setOnSeasonChange([&changedTo, &callCount](Season newSeason) {
        changedTo = newSeason;
        callCount++;
    });

    season->advanceToNextSeason();
    EXPECT_EQ(changedTo, Season::Summer);
    EXPECT_EQ(callCount, 1);

    season->advanceToNextSeason();
    EXPECT_EQ(changedTo, Season::Fall);
    EXPECT_EQ(callCount, 2);
}

TEST_F(SeasonManagerTest, OnSeasonChangeCallback_FromDayChange)
{
    Season changedTo = Season::Spring;
    season->setOnSeasonChange([&changedTo](Season newSeason) {
        changedTo = newSeason;
    });

    season->setDaysPerSeason(3);
    
    // Trigger season change via day progression
    season->onDayChange(1);
    season->onDayChange(2);
    EXPECT_EQ(changedTo, Season::Spring);  // Not changed yet
    
    season->onDayChange(3);
    EXPECT_EQ(changedTo, Season::Summer);  // Changed!
}

// Update test (should not crash)
TEST_F(SeasonManagerTest, Update_DoesNotCrash)
{
    for (int i = 0; i < 100; i++) {
        season->update(0.016f);  // ~60 FPS
    }
    // Season manager update is a no-op for now (day-based), but should not crash
    EXPECT_EQ(season->getCurrentSeason(), Season::Spring);
}
