#pragma once

namespace fresh
{

class TimeManager;

namespace scripting
{

class LuaScriptingEngine;

/**
 * @brief Lua bindings for the TimeManager (day/night cycle system)
 * 
 * Exposes time management, sun/moon queries, lighting calculations,
 * and time-based event callbacks to Lua scripts.
 */
class LuaTimeBindings
{
public:
    /**
     * @brief Register all time-related bindings with the Lua scripting engine
     * @param engine Pointer to the Lua scripting engine
     * @param timeManager Pointer to the TimeManager instance
     */
    static void registerBindings(LuaScriptingEngine* engine, TimeManager* timeManager);

private:
    // Helper functions for bindings
    
    // Time control
    static void setTime(TimeManager* tm, int ticks);
    static void setTimeOfDay(TimeManager* tm, int preset);
    static int getTime(TimeManager* tm);
    static float getTimeInHours(TimeManager* tm);
    static const char* getFormattedTime(TimeManager* tm);
    static void pause(TimeManager* tm);
    static void resume(TimeManager* tm);
    static void togglePause(TimeManager* tm);
    static bool isPaused(TimeManager* tm);
    static void setTimeRate(TimeManager* tm, float rate);
    static float getTimeRate(TimeManager* tm);
    
    // Sun/Moon queries
    struct Vec3Result {
        float x, y, z;
        Vec3Result(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    };
    
    static Vec3Result getSunDirection(TimeManager* tm);
    static Vec3Result getMoonDirection(TimeManager* tm);
    static float getSunElevation(TimeManager* tm);
    static bool isDaytime(TimeManager* tm);
    static bool isNighttime(TimeManager* tm);
    static bool isSunrise(TimeManager* tm);
    static bool isSunset(TimeManager* tm);
    
    // Lighting queries
    static float getAmbientLightIntensity(TimeManager* tm);
    static Vec3Result getSkyColor(TimeManager* tm);
    static Vec3Result getHorizonColor(TimeManager* tm);
    static Vec3Result getSunLightColor(TimeManager* tm);
    static Vec3Result getMoonLightColor(TimeManager* tm);
    
    // Day tracking
    static int getCurrentDay(TimeManager* tm);
};

} // namespace scripting
} // namespace fresh
