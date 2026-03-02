#include "scripting/lua/LuaSeasonBindings.h"
#include "scripting/lua/LuaScriptingEngine.h"
#include "gameplay/SeasonManager.h"
#include "core/Logger.h"

#ifdef FRESH_LUA_AVAILABLE
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace fresh
{
namespace scripting
{

void LuaSeasonBindings::registerBindings(LuaScriptingEngine* engine, SeasonManager* seasonManager)
{
    if (!engine || !seasonManager) {
        LOG_ERROR_C("Cannot register season bindings: null pointer", "LuaSeasonBindings");
        return;
    }

    sol::state* lua = engine->getLuaState();
    if (!lua) {
        LOG_ERROR_C("Cannot register season bindings: null Lua state", "LuaSeasonBindings");
        return;
    }

    // Create Season table
    sol::table seasonTable = lua->create_named_table("Season");

    // ========================================================================
    // Season Control
    // ========================================================================
    
    seasonTable["setSeason"] = [seasonManager](int seasonType) {
        setSeason(seasonManager, seasonType);
    };
    
    seasonTable["getSeason"] = [seasonManager]() -> int {
        return getSeason(seasonManager);
    };
    
    seasonTable["getSeasonName"] = [seasonManager]() -> std::string {
        return std::string(getSeasonName(seasonManager));
    };
    
    seasonTable["advanceToNextSeason"] = [seasonManager]() {
        advanceToNextSeason(seasonManager);
    };

    // ========================================================================
    // Auto-Progression Control
    // ========================================================================
    
    seasonTable["setAutoProgress"] = [seasonManager](bool enabled) {
        setAutoProgress(seasonManager, enabled);
    };
    
    seasonTable["isAutoProgressEnabled"] = [seasonManager]() -> bool {
        return isAutoProgressEnabled(seasonManager);
    };
    
    seasonTable["setDaysPerSeason"] = [seasonManager](int days) {
        setDaysPerSeason(seasonManager, days);
    };
    
    seasonTable["getDaysPerSeason"] = [seasonManager]() -> int {
        return getDaysPerSeason(seasonManager);
    };

    // ========================================================================
    // Visual Effect Queries
    // ========================================================================
    
    seasonTable["getGrassColorTint"] = [seasonManager]() -> Vec3Result {
        return getGrassColorTint(seasonManager);
    };
    
    seasonTable["getFoliageColorTint"] = [seasonManager]() -> Vec3Result {
        return getFoliageColorTint(seasonManager);
    };
    
    seasonTable["getSkyColorModifier"] = [seasonManager]() -> Vec3Result {
        return getSkyColorModifier(seasonManager);
    };

    // ========================================================================
    // Environmental Queries
    // ========================================================================
    
    seasonTable["getAmbientTemperature"] = [seasonManager]() -> float {
        return getAmbientTemperature(seasonManager);
    };
    
    seasonTable["getDayLengthModifier"] = [seasonManager]() -> float {
        return getDayLengthModifier(seasonManager);
    };
    
    seasonTable["shouldSnow"] = [seasonManager]() -> bool {
        return shouldSnow(seasonManager);
    };
    
    seasonTable["shouldLeavesFall"] = [seasonManager]() -> bool {
        return shouldLeavesFall(seasonManager);
    };

    // ========================================================================
    // Season Type Constants
    // ========================================================================
    
    sol::table seasonTypeTable = lua->create_table();
    seasonTypeTable["Spring"] = 0;
    seasonTypeTable["Summer"] = 1;
    seasonTypeTable["Fall"] = 2;
    seasonTypeTable["Winter"] = 3;
    seasonTable["Type"] = seasonTypeTable;

    LOG_INFO_C("Season bindings registered successfully", "LuaSeasonBindings");
}

// ============================================================================
// Implementation of helper functions
// ============================================================================

void LuaSeasonBindings::setSeason(SeasonManager* sm, int seasonType)
{
    Season season;
    switch (seasonType) {
        case 0: season = Season::Spring; break;
        case 1: season = Season::Summer; break;
        case 2: season = Season::Fall; break;
        case 3: season = Season::Winter; break;
        default:
            LOG_WARN_C("Invalid season type: " + std::to_string(seasonType), "LuaSeasonBindings");
            return;
    }
    sm->setSeason(season);
}

int LuaSeasonBindings::getSeason(SeasonManager* sm)
{
    switch (sm->getCurrentSeason()) {
        case Season::Spring: return 0;
        case Season::Summer: return 1;
        case Season::Fall: return 2;
        case Season::Winter: return 3;
        default: return 0;
    }
}

const char* LuaSeasonBindings::getSeasonName(SeasonManager* sm)
{
    static std::string seasonStr;
    seasonStr = sm->getSeasonName();
    return seasonStr.c_str();
}

void LuaSeasonBindings::advanceToNextSeason(SeasonManager* sm)
{
    sm->advanceToNextSeason();
}

void LuaSeasonBindings::setAutoProgress(SeasonManager* sm, bool enabled)
{
    sm->setAutoProgress(enabled);
}

bool LuaSeasonBindings::isAutoProgressEnabled(SeasonManager* sm)
{
    return sm->isAutoProgressEnabled();
}

void LuaSeasonBindings::setDaysPerSeason(SeasonManager* sm, int days)
{
    sm->setDaysPerSeason(days);
}

int LuaSeasonBindings::getDaysPerSeason(SeasonManager* sm)
{
    return sm->getDaysPerSeason();
}

LuaSeasonBindings::Vec3Result LuaSeasonBindings::getGrassColorTint(SeasonManager* sm)
{
    glm::vec3 color = sm->getGrassColorTint();
    return Vec3Result(color.x, color.y, color.z);
}

LuaSeasonBindings::Vec3Result LuaSeasonBindings::getFoliageColorTint(SeasonManager* sm)
{
    glm::vec3 color = sm->getFoliageColorTint();
    return Vec3Result(color.x, color.y, color.z);
}

LuaSeasonBindings::Vec3Result LuaSeasonBindings::getSkyColorModifier(SeasonManager* sm)
{
    glm::vec3 color = sm->getSkyColorModifier();
    return Vec3Result(color.x, color.y, color.z);
}

float LuaSeasonBindings::getAmbientTemperature(SeasonManager* sm)
{
    return sm->getAmbientTemperature();
}

float LuaSeasonBindings::getDayLengthModifier(SeasonManager* sm)
{
    return sm->getDayLengthModifier();
}

bool LuaSeasonBindings::shouldSnow(SeasonManager* sm)
{
    return sm->shouldSnow();
}

bool LuaSeasonBindings::shouldLeavesFall(SeasonManager* sm)
{
    return sm->shouldLeavesfall();
}

} // namespace scripting
} // namespace fresh

#else // FRESH_LUA_AVAILABLE not defined

// Stub implementation when Lua is not available
#include "scripting/lua/LuaSeasonBindings.h"
#include "core/Logger.h"

namespace fresh
{
namespace scripting
{

void LuaSeasonBindings::registerBindings(LuaScriptingEngine* engine, SeasonManager* seasonManager)
{
    (void)engine;
    (void)seasonManager;
    LOG_INFO_C("Lua season bindings not available (Lua support not compiled in)", "LuaSeasonBindings");
}

// Stub implementations
void LuaSeasonBindings::setSeason(SeasonManager* sm, int seasonType) { (void)sm; (void)seasonType; }
int LuaSeasonBindings::getSeason(SeasonManager* sm) { (void)sm; return 0; }
const char* LuaSeasonBindings::getSeasonName(SeasonManager* sm) { (void)sm; return "Spring"; }
void LuaSeasonBindings::advanceToNextSeason(SeasonManager* sm) { (void)sm; }
void LuaSeasonBindings::setAutoProgress(SeasonManager* sm, bool enabled) { (void)sm; (void)enabled; }
bool LuaSeasonBindings::isAutoProgressEnabled(SeasonManager* sm) { (void)sm; return true; }
void LuaSeasonBindings::setDaysPerSeason(SeasonManager* sm, int days) { (void)sm; (void)days; }
int LuaSeasonBindings::getDaysPerSeason(SeasonManager* sm) { (void)sm; return 7; }

LuaSeasonBindings::Vec3Result LuaSeasonBindings::getGrassColorTint(SeasonManager* sm) { 
    (void)sm; 
    return Vec3Result(0.4f, 0.8f, 0.3f); 
}

LuaSeasonBindings::Vec3Result LuaSeasonBindings::getFoliageColorTint(SeasonManager* sm) { 
    (void)sm; 
    return Vec3Result(0.2f, 0.6f, 0.2f); 
}

LuaSeasonBindings::Vec3Result LuaSeasonBindings::getSkyColorModifier(SeasonManager* sm) { 
    (void)sm; 
    return Vec3Result(1.0f, 1.0f, 1.0f); 
}

float LuaSeasonBindings::getAmbientTemperature(SeasonManager* sm) { (void)sm; return 0.6f; }
float LuaSeasonBindings::getDayLengthModifier(SeasonManager* sm) { (void)sm; return 1.0f; }
bool LuaSeasonBindings::shouldSnow(SeasonManager* sm) { (void)sm; return false; }
bool LuaSeasonBindings::shouldLeavesFall(SeasonManager* sm) { (void)sm; return false; }

} // namespace scripting
} // namespace fresh

#endif // FRESH_LUA_AVAILABLE
