#include "scripting/lua/LuaTimeBindings.h"
#include "scripting/lua/ScriptingEngine.h"
#include "gameplay/TimeManager.h"
#include "core/Logger.h"

#ifdef FRESH_LUA_AVAILABLE
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace fresh
{
namespace scripting
{

void LuaTimeBindings::registerBindings(LuaScriptingEngine* engine, TimeManager* timeManager)
{
    if (!engine || !timeManager) {
        LOG_ERROR_C("Cannot register time bindings: null pointer", "LuaTimeBindings");
        return;
    }

    sol::state* lua = engine->getLuaState();
    if (!lua) {
        LOG_ERROR_C("Cannot register time bindings: null Lua state", "LuaTimeBindings");
        return;
    }

    // Create Time table
    sol::table timeTable = lua->create_named_table("Time");

    // ========================================================================
    // Time Control
    // ========================================================================
    
    timeTable["setTime"] = [timeManager](int ticks) {
        setTime(timeManager, ticks);
    };
    
    timeTable["setTimeOfDay"] = [timeManager](int preset) {
        setTimeOfDay(timeManager, preset);
    };
    
    timeTable["getTime"] = [timeManager]() -> int {
        return getTime(timeManager);
    };
    
    timeTable["getTimeInHours"] = [timeManager]() -> float {
        return getTimeInHours(timeManager);
    };
    
    timeTable["getFormattedTime"] = [timeManager]() -> std::string {
        return std::string(getFormattedTime(timeManager));
    };
    
    timeTable["pause"] = [timeManager]() {
        pause(timeManager);
    };
    
    timeTable["resume"] = [timeManager]() {
        resume(timeManager);
    };
    
    timeTable["togglePause"] = [timeManager]() {
        togglePause(timeManager);
    };
    
    timeTable["isPaused"] = [timeManager]() -> bool {
        return isPaused(timeManager);
    };
    
    timeTable["setTimeRate"] = [timeManager](float rate) {
        setTimeRate(timeManager, rate);
    };
    
    timeTable["getTimeRate"] = [timeManager]() -> float {
        return getTimeRate(timeManager);
    };

    // ========================================================================
    // Sun/Moon Queries
    // ========================================================================
    
    timeTable["getSunDirection"] = [timeManager]() -> Vec3Result {
        return getSunDirection(timeManager);
    };
    
    timeTable["getMoonDirection"] = [timeManager]() -> Vec3Result {
        return getMoonDirection(timeManager);
    };
    
    timeTable["getSunElevation"] = [timeManager]() -> float {
        return getSunElevation(timeManager);
    };
    
    timeTable["isDaytime"] = [timeManager]() -> bool {
        return isDaytime(timeManager);
    };
    
    timeTable["isNighttime"] = [timeManager]() -> bool {
        return isNighttime(timeManager);
    };
    
    timeTable["isSunrise"] = [timeManager]() -> bool {
        return isSunrise(timeManager);
    };
    
    timeTable["isSunset"] = [timeManager]() -> bool {
        return isSunset(timeManager);
    };

    // ========================================================================
    // Lighting Queries
    // ========================================================================
    
    timeTable["getAmbientLightIntensity"] = [timeManager]() -> float {
        return getAmbientLightIntensity(timeManager);
    };
    
    timeTable["getSkyColor"] = [timeManager]() -> Vec3Result {
        return getSkyColor(timeManager);
    };
    
    timeTable["getHorizonColor"] = [timeManager]() -> Vec3Result {
        return getHorizonColor(timeManager);
    };
    
    timeTable["getSunLightColor"] = [timeManager]() -> Vec3Result {
        return getSunLightColor(timeManager);
    };
    
    timeTable["getMoonLightColor"] = [timeManager]() -> Vec3Result {
        return getMoonLightColor(timeManager);
    };

    // ========================================================================
    // Day Tracking
    // ========================================================================
    
    timeTable["getCurrentDay"] = [timeManager]() -> int {
        return getCurrentDay(timeManager);
    };

    // ========================================================================
    // Time of Day Presets (Constants)
    // ========================================================================
    
    sol::table timeOfDayTable = lua->create_table();
    timeOfDayTable["Dawn"] = 0;      // 6:00 AM
    timeOfDayTable["Noon"] = 1;      // 12:00 PM
    timeOfDayTable["Dusk"] = 2;      // 6:00 PM
    timeOfDayTable["Midnight"] = 3;  // 12:00 AM
    timeTable["TimeOfDay"] = timeOfDayTable;

    // ========================================================================
    // Vec3Result usertype (for returning 3D vectors)
    // ========================================================================
    
    lua->new_usertype<Vec3Result>("Vec3Result",
        "x", &Vec3Result::x,
        "y", &Vec3Result::y,
        "z", &Vec3Result::z
    );

    LOG_INFO_C("Time bindings registered successfully", "LuaTimeBindings");
}

// ============================================================================
// Implementation of helper functions
// ============================================================================

void LuaTimeBindings::setTime(TimeManager* tm, int ticks)
{
    tm->setTime(ticks);
}

void LuaTimeBindings::setTimeOfDay(TimeManager* tm, int preset)
{
    TimeManager::TimeOfDay tod;
    switch (preset) {
        case 0: tod = TimeManager::TimeOfDay::Dawn; break;
        case 1: tod = TimeManager::TimeOfDay::Noon; break;
        case 2: tod = TimeManager::TimeOfDay::Dusk; break;
        case 3: tod = TimeManager::TimeOfDay::Midnight; break;
        default:
            LOG_WARN_C("Invalid time of day preset: " + std::to_string(preset), "LuaTimeBindings");
            return;
    }
    tm->setTimeOfDay(tod);
}

int LuaTimeBindings::getTime(TimeManager* tm)
{
    return tm->getTime();
}

float LuaTimeBindings::getTimeInHours(TimeManager* tm)
{
    return tm->getTimeInHours();
}

const char* LuaTimeBindings::getFormattedTime(TimeManager* tm)
{
    static std::string timeStr;
    timeStr = tm->getFormattedTime();
    return timeStr.c_str();
}

void LuaTimeBindings::pause(TimeManager* tm)
{
    tm->pause();
}

void LuaTimeBindings::resume(TimeManager* tm)
{
    tm->resume();
}

void LuaTimeBindings::togglePause(TimeManager* tm)
{
    tm->togglePause();
}

bool LuaTimeBindings::isPaused(TimeManager* tm)
{
    return tm->isPaused();
}

void LuaTimeBindings::setTimeRate(TimeManager* tm, float rate)
{
    tm->setTimeRate(rate);
}

float LuaTimeBindings::getTimeRate(TimeManager* tm)
{
    return tm->getTimeRate();
}

LuaTimeBindings::Vec3Result LuaTimeBindings::getSunDirection(TimeManager* tm)
{
    glm::vec3 dir = tm->getSunDirection();
    return Vec3Result(dir.x, dir.y, dir.z);
}

LuaTimeBindings::Vec3Result LuaTimeBindings::getMoonDirection(TimeManager* tm)
{
    glm::vec3 dir = tm->getMoonDirection();
    return Vec3Result(dir.x, dir.y, dir.z);
}

float LuaTimeBindings::getSunElevation(TimeManager* tm)
{
    return tm->getSunElevation();
}

bool LuaTimeBindings::isDaytime(TimeManager* tm)
{
    return tm->isDaytime();
}

bool LuaTimeBindings::isNighttime(TimeManager* tm)
{
    return tm->isNighttime();
}

bool LuaTimeBindings::isSunrise(TimeManager* tm)
{
    return tm->isSunrise();
}

bool LuaTimeBindings::isSunset(TimeManager* tm)
{
    return tm->isSunset();
}

float LuaTimeBindings::getAmbientLightIntensity(TimeManager* tm)
{
    return tm->getAmbientLightIntensity();
}

LuaTimeBindings::Vec3Result LuaTimeBindings::getSkyColor(TimeManager* tm)
{
    glm::vec3 color = tm->getSkyColor();
    return Vec3Result(color.x, color.y, color.z);
}

LuaTimeBindings::Vec3Result LuaTimeBindings::getHorizonColor(TimeManager* tm)
{
    glm::vec3 color = tm->getHorizonColor();
    return Vec3Result(color.x, color.y, color.z);
}

LuaTimeBindings::Vec3Result LuaTimeBindings::getSunLightColor(TimeManager* tm)
{
    glm::vec3 color = tm->getSunLightColor();
    return Vec3Result(color.x, color.y, color.z);
}

LuaTimeBindings::Vec3Result LuaTimeBindings::getMoonLightColor(TimeManager* tm)
{
    glm::vec3 color = tm->getMoonLightColor();
    return Vec3Result(color.x, color.y, color.z);
}

int LuaTimeBindings::getCurrentDay(TimeManager* tm)
{
    return tm->getCurrentDay();
}

} // namespace scripting
} // namespace fresh

#else // FRESH_LUA_AVAILABLE not defined

// Stub implementation when Lua is not available
#include "scripting/lua/LuaTimeBindings.h"
#include "core/Logger.h"

namespace fresh
{
namespace scripting
{

void LuaTimeBindings::registerBindings(LuaScriptingEngine* engine, TimeManager* timeManager)
{
    (void)engine;
    (void)timeManager;
    LOG_INFO_C("Lua time bindings not available (Lua support not compiled in)", "LuaTimeBindings");
}

// Stub implementations for all helper functions
void LuaTimeBindings::setTime(TimeManager* tm, int ticks) { (void)tm; (void)ticks; }
void LuaTimeBindings::setTimeOfDay(TimeManager* tm, int preset) { (void)tm; (void)preset; }
int LuaTimeBindings::getTime(TimeManager* tm) { (void)tm; return 0; }
float LuaTimeBindings::getTimeInHours(TimeManager* tm) { (void)tm; return 0.0f; }
const char* LuaTimeBindings::getFormattedTime(TimeManager* tm) { (void)tm; return "00:00"; }
void LuaTimeBindings::pause(TimeManager* tm) { (void)tm; }
void LuaTimeBindings::resume(TimeManager* tm) { (void)tm; }
void LuaTimeBindings::togglePause(TimeManager* tm) { (void)tm; }
bool LuaTimeBindings::isPaused(TimeManager* tm) { (void)tm; return false; }
void LuaTimeBindings::setTimeRate(TimeManager* tm, float rate) { (void)tm; (void)rate; }
float LuaTimeBindings::getTimeRate(TimeManager* tm) { (void)tm; return 1.0f; }

LuaTimeBindings::Vec3Result LuaTimeBindings::getSunDirection(TimeManager* tm) { 
    (void)tm; 
    return Vec3Result(0.0f, 1.0f, 0.0f); 
}

LuaTimeBindings::Vec3Result LuaTimeBindings::getMoonDirection(TimeManager* tm) { 
    (void)tm; 
    return Vec3Result(0.0f, -1.0f, 0.0f); 
}

float LuaTimeBindings::getSunElevation(TimeManager* tm) { (void)tm; return 0.0f; }
bool LuaTimeBindings::isDaytime(TimeManager* tm) { (void)tm; return true; }
bool LuaTimeBindings::isNighttime(TimeManager* tm) { (void)tm; return false; }
bool LuaTimeBindings::isSunrise(TimeManager* tm) { (void)tm; return false; }
bool LuaTimeBindings::isSunset(TimeManager* tm) { (void)tm; return false; }
float LuaTimeBindings::getAmbientLightIntensity(TimeManager* tm) { (void)tm; return 1.0f; }

LuaTimeBindings::Vec3Result LuaTimeBindings::getSkyColor(TimeManager* tm) { 
    (void)tm; 
    return Vec3Result(0.5f, 0.7f, 1.0f); 
}

LuaTimeBindings::Vec3Result LuaTimeBindings::getHorizonColor(TimeManager* tm) { 
    (void)tm; 
    return Vec3Result(0.8f, 0.9f, 1.0f); 
}

LuaTimeBindings::Vec3Result LuaTimeBindings::getSunLightColor(TimeManager* tm) { 
    (void)tm; 
    return Vec3Result(1.0f, 1.0f, 0.9f); 
}

LuaTimeBindings::Vec3Result LuaTimeBindings::getMoonLightColor(TimeManager* tm) { 
    (void)tm; 
    return Vec3Result(0.6f, 0.6f, 0.8f); 
}

int LuaTimeBindings::getCurrentDay(TimeManager* tm) { (void)tm; return 1; }

} // namespace scripting
} // namespace fresh

#endif // FRESH_LUA_AVAILABLE
