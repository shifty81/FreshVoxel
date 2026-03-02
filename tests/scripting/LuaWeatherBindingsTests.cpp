/**
 * @file LuaWeatherBindingsTests.cpp
 * @brief Unit tests for the Lua Weather bindings
 */

#include <gtest/gtest.h>
#include "scripting/lua/LuaWeatherBindings.h"
#include "gameplay/WeatherManager.h"

using namespace fresh;
using namespace fresh::scripting;

/**
 * Tests for LuaWeatherBindings stub implementations.
 * These tests verify the stub functions work correctly when Lua is not available.
 * When Lua is available, the actual bindings are tested through integration tests.
 */
class LuaWeatherBindingsTest : public ::testing::Test
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

// Test that binding registration doesn't crash with null pointers
TEST_F(LuaWeatherBindingsTest, RegisterBindings_NullEngine_DoesNotCrash)
{
    // Should not crash with nullptr engine
    LuaWeatherBindings::registerBindings(nullptr, weather.get());
}

TEST_F(LuaWeatherBindingsTest, RegisterBindings_NullWeatherManager_DoesNotCrash)
{
    // Should not crash with nullptr weather manager
    LuaWeatherBindings::registerBindings(nullptr, nullptr);
}

// Test that the WeatherManager has all the methods we're binding
TEST_F(LuaWeatherBindingsTest, WeatherManager_HasAllRequiredMethods)
{
    // Verify all the methods we're binding exist and work
    EXPECT_EQ(weather->getCurrentWeather(), WeatherType::Clear);
    EXPECT_EQ(weather->getWeatherName(), "Clear");
    EXPECT_TRUE(weather->isAutoChangeEnabled());
    EXPECT_EQ(weather->getChangeInterval(), 300.0f);
    
    weather->setWeather(WeatherType::Rain);
    // Complete transition
    for (int i = 0; i < 20; i++) {
        weather->update(1.0f);
    }
    EXPECT_EQ(weather->getCurrentWeather(), WeatherType::Rain);
    
    weather->setAutoChange(false);
    EXPECT_FALSE(weather->isAutoChangeEnabled());
    
    weather->setChangeInterval(120.0f);
    EXPECT_EQ(weather->getChangeInterval(), 120.0f);
    
    weather->setCurrentSeason(3); // Winter
    weather->randomizeWeather();
    
    // Test query methods
    float precip = weather->getPrecipitationIntensity();
    EXPECT_GE(precip, 0.0f);
    EXPECT_LE(precip, 1.0f);
    
    float fog = weather->getFogDensity();
    EXPECT_GE(fog, 0.0f);
    EXPECT_LE(fog, 1.0f);
    
    float wind = weather->getWindStrength();
    EXPECT_GE(wind, 0.0f);
    EXPECT_LE(wind, 1.0f);
    
    glm::vec3 windDir = weather->getWindDirection();
    float length = glm::length(windDir);
    EXPECT_NEAR(length, 1.0f, 0.01f);
    
    float lightMod = weather->getAmbientLightModifier();
    EXPECT_GT(lightMod, 0.0f);
    EXPECT_LE(lightMod, 1.0f);
    
    glm::vec3 skyMod = weather->getSkyColorModifier();
    EXPECT_GE(skyMod.r, 0.0f);
    EXPECT_LE(skyMod.r, 1.0f);
    
    float clouds = weather->getCloudCoverage();
    EXPECT_GE(clouds, 0.0f);
    EXPECT_LE(clouds, 1.0f);
}

// Test weather type mapping matches binding expectations
TEST_F(LuaWeatherBindingsTest, WeatherTypeMapping_MatchesConstants)
{
    // Verify the enum values we use in bindings match actual WeatherType
    weather->setWeather(WeatherType::Clear);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_EQ(static_cast<int>(weather->getCurrentWeather()), 0);
    
    weather->setWeather(WeatherType::Cloudy);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_EQ(weather->getWeatherName(), "Cloudy");
    
    weather->setWeather(WeatherType::Rain);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_TRUE(weather->isRaining());
    
    weather->setWeather(WeatherType::Snow);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_TRUE(weather->isSnowing());
    
    weather->setWeather(WeatherType::Fog);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_GT(weather->getFogDensity(), 0.5f);
    
    weather->setWeather(WeatherType::Storm);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_TRUE(weather->isRaining());
    EXPECT_GT(weather->getWindStrength(), 0.5f);
    
    weather->setWeather(WeatherType::Blizzard);
    for (int i = 0; i < 20; i++) weather->update(1.0f);
    EXPECT_TRUE(weather->isSnowing());
}
