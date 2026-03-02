#pragma once

namespace fresh
{

class WeatherManager;

namespace scripting
{

class LuaScriptingEngine;

/**
 * @brief Lua bindings for the WeatherManager (weather system)
 * 
 * Exposes weather control, weather queries, and environmental effects
 * to Lua scripts. Allows scripts to:
 * - Get/set current weather type
 * - Query precipitation, fog, wind effects
 * - Control automatic weather changes
 * - React to weather change events
 */
class LuaWeatherBindings
{
public:
    /**
     * @brief Register all weather-related bindings with the Lua scripting engine
     * @param engine Pointer to the Lua scripting engine
     * @param weatherManager Pointer to the WeatherManager instance
     */
    static void registerBindings(LuaScriptingEngine* engine, WeatherManager* weatherManager);

private:
    // Helper struct for returning 3D vectors to Lua
    struct Vec3Result {
        float x, y, z;
        Vec3Result(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    };

    // Weather control
    static void setWeather(WeatherManager* wm, int weatherType);
    static int getWeather(WeatherManager* wm);
    static const char* getWeatherName(WeatherManager* wm);
    static void randomizeWeather(WeatherManager* wm);
    
    // Auto-change control
    static void setAutoChange(WeatherManager* wm, bool enabled);
    static bool isAutoChangeEnabled(WeatherManager* wm);
    static void setChangeInterval(WeatherManager* wm, float seconds);
    static float getChangeInterval(WeatherManager* wm);
    
    // Season integration
    static void setCurrentSeason(WeatherManager* wm, int seasonIndex);
    
    // Weather effect queries
    static float getPrecipitationIntensity(WeatherManager* wm);
    static bool isRaining(WeatherManager* wm);
    static bool isSnowing(WeatherManager* wm);
    static float getFogDensity(WeatherManager* wm);
    static float getWindStrength(WeatherManager* wm);
    static Vec3Result getWindDirection(WeatherManager* wm);
    
    // Lighting effects
    static float getAmbientLightModifier(WeatherManager* wm);
    static Vec3Result getSkyColorModifier(WeatherManager* wm);
    static float getCloudCoverage(WeatherManager* wm);
    
    // Storm effects
    static bool shouldTriggerLightning(WeatherManager* wm, float deltaTime);
};

} // namespace scripting
} // namespace fresh
