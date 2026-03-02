#pragma once

namespace fresh
{

class SeasonManager;

namespace scripting
{

class LuaScriptingEngine;

/**
 * @brief Lua bindings for the SeasonManager (seasonal changes system)
 * 
 * Exposes season control, visual modifiers, and environmental effects
 * to Lua scripts. Allows scripts to:
 * - Get/set current season
 * - Query visual tints (grass, foliage, sky)
 * - Access temperature and day length modifiers
 * - Control automatic season progression
 */
class LuaSeasonBindings
{
public:
    /**
     * @brief Register all season-related bindings with the Lua scripting engine
     * @param engine Pointer to the Lua scripting engine
     * @param seasonManager Pointer to the SeasonManager instance
     */
    static void registerBindings(LuaScriptingEngine* engine, SeasonManager* seasonManager);

private:
    // Helper struct for returning 3D vectors to Lua
    struct Vec3Result {
        float x, y, z;
        Vec3Result(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    };

    // Season control
    static void setSeason(SeasonManager* sm, int seasonType);
    static int getSeason(SeasonManager* sm);
    static const char* getSeasonName(SeasonManager* sm);
    static void advanceToNextSeason(SeasonManager* sm);
    
    // Auto-progression control
    static void setAutoProgress(SeasonManager* sm, bool enabled);
    static bool isAutoProgressEnabled(SeasonManager* sm);
    static void setDaysPerSeason(SeasonManager* sm, int days);
    static int getDaysPerSeason(SeasonManager* sm);
    
    // Visual effect queries
    static Vec3Result getGrassColorTint(SeasonManager* sm);
    static Vec3Result getFoliageColorTint(SeasonManager* sm);
    static Vec3Result getSkyColorModifier(SeasonManager* sm);
    
    // Environmental queries
    static float getAmbientTemperature(SeasonManager* sm);
    static float getDayLengthModifier(SeasonManager* sm);
    static bool shouldSnow(SeasonManager* sm);
    static bool shouldLeavesFall(SeasonManager* sm);
};

} // namespace scripting
} // namespace fresh
