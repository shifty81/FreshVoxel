#include <gtest/gtest.h>
#include "gameplay/TimeManager.h"
#include <cmath>

using namespace fresh;

class TimeManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        timeManager = std::make_unique<TimeManager>();
    }

    void TearDown() override
    {
        timeManager.reset();
    }

    std::unique_ptr<TimeManager> timeManager;
};

// Basic time management tests
TEST_F(TimeManagerTest, InitialTime)
{
    // Should start at dawn (6000 ticks)
    EXPECT_EQ(timeManager->getTime(), 6000);
    EXPECT_EQ(timeManager->getCurrentDay(), 0);
}

TEST_F(TimeManagerTest, SetTime)
{
    timeManager->setTime(12000);  // Noon
    EXPECT_EQ(timeManager->getTime(), 12000);
    
    timeManager->setTime(0);  // Midnight
    EXPECT_EQ(timeManager->getTime(), 0);
    
    timeManager->setTime(18000);  // Dusk
    EXPECT_EQ(timeManager->getTime(), 18000);
}

TEST_F(TimeManagerTest, SetTimeOfDay)
{
    timeManager->setTimeOfDay(TimeManager::TimeOfDay::Dawn);
    EXPECT_EQ(timeManager->getTime(), 6000);
    
    timeManager->setTimeOfDay(TimeManager::TimeOfDay::Noon);
    EXPECT_EQ(timeManager->getTime(), 12000);
    
    timeManager->setTimeOfDay(TimeManager::TimeOfDay::Dusk);
    EXPECT_EQ(timeManager->getTime(), 18000);
    
    timeManager->setTimeOfDay(TimeManager::TimeOfDay::Midnight);
    EXPECT_EQ(timeManager->getTime(), 0);
}

TEST_F(TimeManagerTest, TimeProgression)
{
    timeManager->setTime(0);
    
    // Update by 1 second at 1x speed
    // Should advance by ~20 ticks (TICKS_PER_SECOND)
    timeManager->update(1.0f);
    
    EXPECT_NEAR(timeManager->getTime(), 20, 1);
}

TEST_F(TimeManagerTest, TimeNormalization)
{
    // Set time past 24000 and update
    timeManager->setTime(23990);
    timeManager->update(1.0f);  // Should roll over to next day
    
    EXPECT_LT(timeManager->getTime(), 24000);
    EXPECT_GE(timeManager->getCurrentDay(), 1);
}

TEST_F(TimeManagerTest, PauseResume)
{
    timeManager->setTime(0);
    int initialTime = timeManager->getTime();
    
    timeManager->pause();
    EXPECT_TRUE(timeManager->isPaused());
    
    timeManager->update(1.0f);
    EXPECT_EQ(timeManager->getTime(), initialTime);  // Time shouldn't progress
    
    timeManager->resume();
    EXPECT_FALSE(timeManager->isPaused());
    
    timeManager->update(1.0f);
    EXPECT_GT(timeManager->getTime(), initialTime);  // Time should progress
}

TEST_F(TimeManagerTest, TogglePause)
{
    EXPECT_FALSE(timeManager->isPaused());
    
    timeManager->togglePause();
    EXPECT_TRUE(timeManager->isPaused());
    
    timeManager->togglePause();
    EXPECT_FALSE(timeManager->isPaused());
}

TEST_F(TimeManagerTest, TimeRate)
{
    timeManager->setTime(0);
    
    // Test 2x speed
    timeManager->setTimeRate(2.0f);
    EXPECT_FLOAT_EQ(timeManager->getTimeRate(), 2.0f);
    
    timeManager->update(1.0f);
    EXPECT_NEAR(timeManager->getTime(), 40, 2);  // Should be ~40 ticks (2x normal)
    
    // Test 0.5x speed
    timeManager->setTime(0);
    timeManager->setTimeRate(0.5f);
    timeManager->update(1.0f);
    EXPECT_NEAR(timeManager->getTime(), 10, 1);  // Should be ~10 ticks (0.5x normal)
}

TEST_F(TimeManagerTest, NegativeTimeRatePrevention)
{
    timeManager->setTimeRate(-1.0f);
    EXPECT_GE(timeManager->getTimeRate(), 0.0f);  // Should be clamped to 0
}

// Time conversion tests
TEST_F(TimeManagerTest, GetTimeInHours)
{
    timeManager->setTime(0);  // Midnight
    EXPECT_FLOAT_EQ(timeManager->getTimeInHours(), 0.0f);
    
    timeManager->setTime(6000);  // 6 AM
    EXPECT_FLOAT_EQ(timeManager->getTimeInHours(), 6.0f);
    
    timeManager->setTime(12000);  // Noon
    EXPECT_FLOAT_EQ(timeManager->getTimeInHours(), 12.0f);
    
    timeManager->setTime(18000);  // 6 PM
    EXPECT_FLOAT_EQ(timeManager->getTimeInHours(), 18.0f);
}

TEST_F(TimeManagerTest, FormattedTime)
{
    timeManager->setTime(0);  // Midnight
    EXPECT_EQ(timeManager->getFormattedTime(), "12:00 AM");
    
    timeManager->setTime(6000);  // 6 AM
    EXPECT_EQ(timeManager->getFormattedTime(), "06:00 AM");
    
    timeManager->setTime(12000);  // Noon
    EXPECT_EQ(timeManager->getFormattedTime(), "12:00 PM");
    
    timeManager->setTime(18000);  // 6 PM
    EXPECT_EQ(timeManager->getFormattedTime(), "06:00 PM");
}

// Day/Night cycle tests
TEST_F(TimeManagerTest, IsDaytime)
{
    timeManager->setTime(12000);  // Noon
    EXPECT_TRUE(timeManager->isDaytime());
    EXPECT_FALSE(timeManager->isNighttime());
    
    timeManager->setTime(0);  // Midnight
    EXPECT_FALSE(timeManager->isDaytime());
    EXPECT_TRUE(timeManager->isNighttime());
}

TEST_F(TimeManagerTest, IsSunrise)
{
    timeManager->setTime(6000);  // Dawn
    EXPECT_TRUE(timeManager->isSunrise());
    EXPECT_FALSE(timeManager->isSunset());
    
    timeManager->setTime(12000);  // Noon
    EXPECT_FALSE(timeManager->isSunrise());
}

TEST_F(TimeManagerTest, IsSunset)
{
    timeManager->setTime(18000);  // Dusk
    EXPECT_TRUE(timeManager->isSunset());
    EXPECT_FALSE(timeManager->isSunrise());
    
    timeManager->setTime(12000);  // Noon
    EXPECT_FALSE(timeManager->isSunset());
}

// Sun/Moon positioning tests
TEST_F(TimeManagerTest, SunDirection)
{
    // At noon, sun should be high in sky (positive y)
    timeManager->setTime(12000);
    glm::vec3 sunDir = timeManager->getSunDirection();
    EXPECT_GT(sunDir.y, 0.9f);  // Should be nearly vertical
    
    // At midnight, sun should be below horizon (negative y)
    timeManager->setTime(0);
    sunDir = timeManager->getSunDirection();
    EXPECT_LT(sunDir.y, -0.9f);
}

TEST_F(TimeManagerTest, MoonDirection)
{
    // Moon should be opposite to sun
    timeManager->setTime(12000);  // Noon
    glm::vec3 sunDir = timeManager->getSunDirection();
    glm::vec3 moonDir = timeManager->getMoonDirection();
    
    // Moon should be nearly opposite (dot product close to -1)
    float dot = glm::dot(sunDir, moonDir);
    EXPECT_NEAR(dot, -1.0f, 0.1f);
}

TEST_F(TimeManagerTest, SunElevation)
{
    // At noon, elevation should be high
    timeManager->setTime(12000);
    float elevation = timeManager->getSunElevation();
    EXPECT_GT(elevation, 80.0f);  // Close to 90 degrees
    
    // At midnight, elevation should be negative
    timeManager->setTime(0);
    elevation = timeManager->getSunElevation();
    EXPECT_LT(elevation, -80.0f);
    
    // At sunrise/sunset, elevation should be near 0
    timeManager->setTime(6000);
    elevation = timeManager->getSunElevation();
    EXPECT_NEAR(elevation, 0.0f, 10.0f);
}

// Lighting tests
TEST_F(TimeManagerTest, AmbientLightIntensity)
{
    // Daytime should have high ambient light
    timeManager->setTime(12000);  // Noon
    float intensity = timeManager->getAmbientLightIntensity();
    EXPECT_GT(intensity, 0.8f);
    EXPECT_LE(intensity, 1.0f);
    
    // Nighttime should have low ambient light
    timeManager->setTime(0);  // Midnight
    intensity = timeManager->getAmbientLightIntensity();
    EXPECT_LT(intensity, 0.3f);
    EXPECT_GE(intensity, 0.1f);
}

TEST_F(TimeManagerTest, SkyColor)
{
    // Daytime sky should be blue
    timeManager->setTime(12000);
    glm::vec3 skyColor = timeManager->getSkyColor();
    EXPECT_GT(skyColor.b, skyColor.r);  // More blue than red
    
    // Nighttime sky should be dark
    timeManager->setTime(0);
    skyColor = timeManager->getSkyColor();
    EXPECT_LT(skyColor.r + skyColor.g + skyColor.b, 1.0f);  // Dark overall
}

TEST_F(TimeManagerTest, SunLightColor)
{
    // Daytime sun should be white/yellow
    timeManager->setTime(12000);
    glm::vec3 sunColor = timeManager->getSunLightColor();
    EXPECT_GT(sunColor.r, 0.9f);
    EXPECT_GT(sunColor.g, 0.9f);
    
    // Nighttime sun should be off (black)
    timeManager->setTime(0);
    sunColor = timeManager->getSunLightColor();
    EXPECT_FLOAT_EQ(sunColor.r, 0.0f);
    EXPECT_FLOAT_EQ(sunColor.g, 0.0f);
    EXPECT_FLOAT_EQ(sunColor.b, 0.0f);
}

TEST_F(TimeManagerTest, MoonLightColor)
{
    // Nighttime moon should have blue-ish color
    timeManager->setTime(0);
    glm::vec3 moonColor = timeManager->getMoonLightColor();
    EXPECT_GT(moonColor.b, moonColor.r);  // More blue than red
    
    // Daytime moon should be off
    timeManager->setTime(12000);
    moonColor = timeManager->getMoonLightColor();
    EXPECT_FLOAT_EQ(moonColor.r, 0.0f);
    EXPECT_FLOAT_EQ(moonColor.g, 0.0f);
    EXPECT_FLOAT_EQ(moonColor.b, 0.0f);
}

// Event callback tests
TEST_F(TimeManagerTest, OnDayChangeCallback)
{
    int dayChangedTo = -1;
    timeManager->setOnDayChange([&dayChangedTo](int day) {
        dayChangedTo = day;
    });
    
    timeManager->setTime(23990);
    timeManager->update(1.0f);  // Should trigger day change
    
    EXPECT_EQ(dayChangedTo, 1);
}

TEST_F(TimeManagerTest, OnSunriseCallback)
{
    bool sunriseTriggered = false;
    timeManager->setOnSunrise([&sunriseTriggered]() {
        sunriseTriggered = true;
    });
    
    // Start at night, advance to day
    timeManager->setTime(0);
    timeManager->update(0.1f);  // Still night
    EXPECT_FALSE(sunriseTriggered);
    
    timeManager->setTime(7000);  // Jump to day
    timeManager->update(0.1f);
    EXPECT_TRUE(sunriseTriggered);
}

TEST_F(TimeManagerTest, OnSunsetCallback)
{
    bool sunsetTriggered = false;
    timeManager->setOnSunset([&sunsetTriggered]() {
        sunsetTriggered = true;
    });
    
    // Start at day, advance to night
    timeManager->setTime(12000);
    timeManager->update(0.1f);  // Still day
    EXPECT_FALSE(sunsetTriggered);
    
    timeManager->setTime(0);  // Jump to night
    timeManager->update(0.1f);
    EXPECT_TRUE(sunsetTriggered);
}

// ============================================================================
// Star Field Tests
// ============================================================================

TEST_F(TimeManagerTest, StarField_EmptyByDefault)
{
    EXPECT_TRUE(timeManager->getStarField().empty());
}

TEST_F(TimeManagerTest, StarField_GenerateCreatesStars)
{
    timeManager->generateStarField(100, 42);
    EXPECT_EQ(100u, timeManager->getStarField().size());
}

TEST_F(TimeManagerTest, StarField_DeterministicWithSameSeed)
{
    timeManager->generateStarField(50, 12345);
    auto stars1 = timeManager->getStarField();

    timeManager->generateStarField(50, 12345);
    auto stars2 = timeManager->getStarField();

    ASSERT_EQ(stars1.size(), stars2.size());
    for (size_t i = 0; i < stars1.size(); ++i) {
        EXPECT_FLOAT_EQ(stars1[i].direction.x, stars2[i].direction.x);
        EXPECT_FLOAT_EQ(stars1[i].direction.y, stars2[i].direction.y);
        EXPECT_FLOAT_EQ(stars1[i].direction.z, stars2[i].direction.z);
        EXPECT_FLOAT_EQ(stars1[i].brightness, stars2[i].brightness);
        EXPECT_FLOAT_EQ(stars1[i].size, stars2[i].size);
    }
}

TEST_F(TimeManagerTest, StarField_DifferentSeedsDifferentResults)
{
    timeManager->generateStarField(50, 42);
    auto stars1 = timeManager->getStarField();

    timeManager->generateStarField(50, 99);
    auto stars2 = timeManager->getStarField();

    // At least one star should differ
    bool allSame = true;
    for (size_t i = 0; i < stars1.size(); ++i) {
        if (stars1[i].direction.x != stars2[i].direction.x) {
            allSame = false;
            break;
        }
    }
    EXPECT_FALSE(allSame);
}

TEST_F(TimeManagerTest, StarField_CountClamped)
{
    timeManager->generateStarField(0, 42);
    EXPECT_EQ(1u, timeManager->getStarField().size());

    timeManager->generateStarField(10000, 42);
    EXPECT_EQ(5000u, timeManager->getStarField().size());
}

TEST_F(TimeManagerTest, StarField_StarsInUpperHemisphere)
{
    timeManager->generateStarField(200, 42);
    for (const auto& star : timeManager->getStarField()) {
        EXPECT_GE(star.direction.y, 0.0f);
    }
}

TEST_F(TimeManagerTest, StarField_BrightnessInRange)
{
    timeManager->generateStarField(200, 42);
    for (const auto& star : timeManager->getStarField()) {
        EXPECT_GE(star.brightness, 0.3f);
        EXPECT_LE(star.brightness, 1.0f);
    }
}

TEST_F(TimeManagerTest, StarField_SizeInRange)
{
    timeManager->generateStarField(200, 42);
    for (const auto& star : timeManager->getStarField()) {
        EXPECT_GE(star.size, 0.5f);
        EXPECT_LE(star.size, 2.0f);
    }
}

TEST_F(TimeManagerTest, StarVisibility_ZeroDuringDay)
{
    timeManager->setTime(12000); // Noon
    EXPECT_FLOAT_EQ(0.0f, timeManager->getStarVisibility());
}

TEST_F(TimeManagerTest, StarVisibility_FullDuringNight)
{
    timeManager->setTime(0); // Midnight
    EXPECT_FLOAT_EQ(1.0f, timeManager->getStarVisibility());
}

TEST_F(TimeManagerTest, StarVisibility_FadesAtSunrise)
{
    // Middle of sunrise
    int midSunrise = (5000 + 7000) / 2;
    timeManager->setTime(midSunrise);
    float visibility = timeManager->getStarVisibility();
    EXPECT_GT(visibility, 0.0f);
    EXPECT_LT(visibility, 1.0f);
}

TEST_F(TimeManagerTest, StarVisibility_FadesAtSunset)
{
    // Middle of sunset
    int midSunset = (17000 + 19000) / 2;
    timeManager->setTime(midSunset);
    float visibility = timeManager->getStarVisibility();
    EXPECT_GT(visibility, 0.0f);
    EXPECT_LT(visibility, 1.0f);
}

TEST_F(TimeManagerTest, StarField_RegenerateReplacesOld)
{
    timeManager->generateStarField(100, 42);
    EXPECT_EQ(100u, timeManager->getStarField().size());
    
    timeManager->generateStarField(50, 99);
    EXPECT_EQ(50u, timeManager->getStarField().size());
}
