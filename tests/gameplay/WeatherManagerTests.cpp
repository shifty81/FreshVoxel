#include <gtest/gtest.h>
#include "gameplay/WeatherManager.h"

using namespace fresh;

class WeatherManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        weather = std::make_unique<WeatherManager>();
    }

    void TearDown() override
    {
        weather.reset();
    }

    std::unique_ptr<WeatherManager> weather;
};

// Basic initialization tests
TEST_F(WeatherManagerTest, InitialWeather)
{
    EXPECT_EQ(weather->getCurrentWeather(), WeatherType::Clear);
    EXPECT_EQ(weather->getWeatherName(), "Clear");
}

TEST_F(WeatherManagerTest, InitialAutoChange)
{
    EXPECT_TRUE(weather->isAutoChangeEnabled());
    EXPECT_EQ(weather->getChangeInterval(), 300.0f);
}

// Set weather tests
TEST_F(WeatherManagerTest, SetWeather_ChangesWeatherType)
{
    weather->setWeather(WeatherType::Rain);
    // Weather starts transitioning; after transition completes, it should be Rain
    // Let's advance the transition
    for (int i = 0; i < 20; i++) {
        weather->update(1.0f);
    }
    EXPECT_EQ(weather->getCurrentWeather(), WeatherType::Rain);
    EXPECT_EQ(weather->getWeatherName(), "Rain");
}

TEST_F(WeatherManagerTest, SetWeather_AllTypes)
{
    struct {
        WeatherType type;
        std::string name;
    } testCases[] = {
        {WeatherType::Clear, "Clear"},
        {WeatherType::Cloudy, "Cloudy"},
        {WeatherType::Rain, "Rain"},
        {WeatherType::Snow, "Snow"},
        {WeatherType::Fog, "Fog"},
        {WeatherType::Storm, "Storm"},
        {WeatherType::Blizzard, "Blizzard"},
    };

    for (const auto& tc : testCases) {
        weather->setWeather(tc.type);
        // Complete transition
        for (int i = 0; i < 20; i++) {
            weather->update(1.0f);
        }
        EXPECT_EQ(weather->getCurrentWeather(), tc.type);
        EXPECT_EQ(weather->getWeatherName(), tc.name);
    }
}

TEST_F(WeatherManagerTest, SetWeather_SameWeather_NoTransition)
{
    // Setting weather to same type should not trigger a change
    weather->setWeather(WeatherType::Clear);
    EXPECT_EQ(weather->getCurrentWeather(), WeatherType::Clear);
}

// Auto-change control tests
TEST_F(WeatherManagerTest, DisableAutoChange)
{
    weather->setAutoChange(false);
    EXPECT_FALSE(weather->isAutoChangeEnabled());
    
    // Advance beyond change interval
    WeatherType initial = weather->getCurrentWeather();
    weather->update(500.0f);  // Longer than default interval
    
    // Weather should not have changed since auto-change is disabled
    EXPECT_EQ(weather->getCurrentWeather(), initial);
}

TEST_F(WeatherManagerTest, SetChangeInterval)
{
    weather->setChangeInterval(120.0f);
    EXPECT_EQ(weather->getChangeInterval(), 120.0f);
}

// Season integration tests
TEST_F(WeatherManagerTest, SetCurrentSeason)
{
    weather->setCurrentSeason(3);  // Winter
    // Should not crash; randomize should use winter probabilities
    weather->randomizeWeather();
    // Weather should be one of the valid types
    WeatherType w = weather->getCurrentWeather();
    EXPECT_TRUE(
        w == WeatherType::Clear || w == WeatherType::Cloudy ||
        w == WeatherType::Snow || w == WeatherType::Blizzard ||
        w == WeatherType::Fog || w == WeatherType::Rain ||
        w == WeatherType::Storm
    );
}

// Precipitation tests
TEST_F(WeatherManagerTest, PrecipitationIntensity_Clear)
{
    // Clear weather has no precipitation
    EXPECT_FLOAT_EQ(weather->getPrecipitationIntensity(), 0.0f);
}

TEST_F(WeatherManagerTest, PrecipitationIntensity_Rain)
{
    weather->setWeather(WeatherType::Rain);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_GT(weather->getPrecipitationIntensity(), 0.0f);
}

TEST_F(WeatherManagerTest, PrecipitationIntensity_Storm)
{
    weather->setWeather(WeatherType::Storm);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    float stormIntensity = weather->getPrecipitationIntensity();
    EXPECT_GT(stormIntensity, 0.5f);
}

// Rain/Snow detection
TEST_F(WeatherManagerTest, IsRaining)
{
    EXPECT_FALSE(weather->isRaining());
    
    weather->setWeather(WeatherType::Rain);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_TRUE(weather->isRaining());
    
    weather->setWeather(WeatherType::Storm);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_TRUE(weather->isRaining());
}

TEST_F(WeatherManagerTest, IsSnowing)
{
    EXPECT_FALSE(weather->isSnowing());
    
    weather->setWeather(WeatherType::Snow);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_TRUE(weather->isSnowing());
    
    weather->setWeather(WeatherType::Blizzard);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_TRUE(weather->isSnowing());
}

// Fog density tests
TEST_F(WeatherManagerTest, FogDensity_Clear)
{
    EXPECT_FLOAT_EQ(weather->getFogDensity(), 0.0f);
}

TEST_F(WeatherManagerTest, FogDensity_Fog)
{
    weather->setWeather(WeatherType::Fog);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_GT(weather->getFogDensity(), 0.5f);
}

// Wind tests
TEST_F(WeatherManagerTest, WindStrength_Clear)
{
    // Clear weather has light wind
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    float wind = weather->getWindStrength();
    EXPECT_GE(wind, 0.0f);
    EXPECT_LE(wind, 0.3f);
}

TEST_F(WeatherManagerTest, WindStrength_Storm)
{
    weather->setWeather(WeatherType::Storm);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_GT(weather->getWindStrength(), 0.5f);
}

TEST_F(WeatherManagerTest, WindDirection_IsNormalized)
{
    glm::vec3 dir = weather->getWindDirection();
    float length = glm::length(dir);
    EXPECT_NEAR(length, 1.0f, 0.01f);
}

// Light modifier tests
TEST_F(WeatherManagerTest, AmbientLightModifier_Clear)
{
    EXPECT_FLOAT_EQ(weather->getAmbientLightModifier(), 1.0f);
}

TEST_F(WeatherManagerTest, AmbientLightModifier_Storm)
{
    weather->setWeather(WeatherType::Storm);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_LT(weather->getAmbientLightModifier(), 1.0f);
}

// Sky color modifier tests
TEST_F(WeatherManagerTest, SkyColorModifier_Clear)
{
    glm::vec3 modifier = weather->getSkyColorModifier();
    EXPECT_FLOAT_EQ(modifier.r, 1.0f);
    EXPECT_FLOAT_EQ(modifier.g, 1.0f);
    EXPECT_FLOAT_EQ(modifier.b, 1.0f);
}

// Cloud coverage tests
TEST_F(WeatherManagerTest, CloudCoverage_Clear)
{
    // Clear has low cloud coverage (starts fully transitioned)
    EXPECT_LE(weather->getCloudCoverage(), 0.2f);
}

TEST_F(WeatherManagerTest, CloudCoverage_Storm)
{
    weather->setWeather(WeatherType::Storm);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_GT(weather->getCloudCoverage(), 0.8f);
}

// Lightning tests
TEST_F(WeatherManagerTest, Lightning_NotInClear)
{
    EXPECT_FALSE(weather->shouldTriggerLightning(1.0f));
}

TEST_F(WeatherManagerTest, Lightning_PossibleInStorm)
{
    weather->setWeather(WeatherType::Storm);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    
    // Lightning should eventually trigger in a storm
    bool lightningTriggered = false;
    for (int i = 0; i < 100; i++) {
        weather->update(0.5f);
        if (weather->shouldTriggerLightning(0.5f)) {
            lightningTriggered = true;
            break;
        }
    }
    EXPECT_TRUE(lightningTriggered);
}

// Callback tests
TEST_F(WeatherManagerTest, OnWeatherChangeCallback)
{
    WeatherType changedTo = WeatherType::Clear;
    weather->setOnWeatherChange([&changedTo](WeatherType newWeather) {
        changedTo = newWeather;
    });
    
    weather->setWeather(WeatherType::Rain);
    EXPECT_EQ(changedTo, WeatherType::Rain);
}

// Randomize weather test
TEST_F(WeatherManagerTest, RandomizeWeather_ProducesValidWeather)
{
    weather->setAutoChange(false);
    
    for (int i = 0; i < 10; i++) {
        weather->randomizeWeather();
        WeatherType w = weather->getCurrentWeather();
        EXPECT_TRUE(
            w == WeatherType::Clear || w == WeatherType::Cloudy ||
            w == WeatherType::Rain || w == WeatherType::Snow ||
            w == WeatherType::Fog || w == WeatherType::Storm ||
            w == WeatherType::Blizzard
        );
    }
}
