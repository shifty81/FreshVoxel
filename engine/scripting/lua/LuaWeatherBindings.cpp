#include "scripting/lua/LuaWeatherBindings.h"
#include "scripting/lua/LuaScriptingEngine.h"
#include "gameplay/WeatherManager.h"
#include "core/Logger.h"

#ifdef FRESH_LUA_AVAILABLE
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace fresh
{
namespace scripting
{

void LuaWeatherBindings::registerBindings(LuaScriptingEngine* engine, WeatherManager* weatherManager)
{
    if (!engine || !weatherManager) {
        LOG_ERROR_C("Cannot register weather bindings: null pointer", "LuaWeatherBindings");
        return;
    }

    sol::state* lua = engine->getLuaState();
    if (!lua) {
        LOG_ERROR_C("Cannot register weather bindings: null Lua state", "LuaWeatherBindings");
        return;
    }

    // Create Weather table
    sol::table weatherTable = lua->create_named_table("Weather");

    // ========================================================================
    // Weather Control
    // ========================================================================
    
    weatherTable["setWeather"] = [weatherManager](int weatherType) {
        setWeather(weatherManager, weatherType);
    };
    
    weatherTable["getWeather"] = [weatherManager]() -> int {
        return getWeather(weatherManager);
    };
    
    weatherTable["getWeatherName"] = [weatherManager]() -> std::string {
        return std::string(getWeatherName(weatherManager));
    };
    
    weatherTable["randomizeWeather"] = [weatherManager]() {
        randomizeWeather(weatherManager);
    };

    // ========================================================================
    // Auto-Change Control
    // ========================================================================
    
    weatherTable["setAutoChange"] = [weatherManager](bool enabled) {
        setAutoChange(weatherManager, enabled);
    };
    
    weatherTable["isAutoChangeEnabled"] = [weatherManager]() -> bool {
        return isAutoChangeEnabled(weatherManager);
    };
    
    weatherTable["setChangeInterval"] = [weatherManager](float seconds) {
        setChangeInterval(weatherManager, seconds);
    };
    
    weatherTable["getChangeInterval"] = [weatherManager]() -> float {
        return getChangeInterval(weatherManager);
    };

    // ========================================================================
    // Season Integration
    // ========================================================================
    
    weatherTable["setCurrentSeason"] = [weatherManager](int seasonIndex) {
        setCurrentSeason(weatherManager, seasonIndex);
    };

    // ========================================================================
    // Weather Effect Queries
    // ========================================================================
    
    weatherTable["getPrecipitationIntensity"] = [weatherManager]() -> float {
        return getPrecipitationIntensity(weatherManager);
    };
    
    weatherTable["isRaining"] = [weatherManager]() -> bool {
        return isRaining(weatherManager);
    };
    
    weatherTable["isSnowing"] = [weatherManager]() -> bool {
        return isSnowing(weatherManager);
    };
    
    weatherTable["getFogDensity"] = [weatherManager]() -> float {
        return getFogDensity(weatherManager);
    };
    
    weatherTable["getWindStrength"] = [weatherManager]() -> float {
        return getWindStrength(weatherManager);
    };
    
    weatherTable["getWindDirection"] = [weatherManager]() -> Vec3Result {
        return getWindDirection(weatherManager);
    };

    // ========================================================================
    // Lighting Effects
    // ========================================================================
    
    weatherTable["getAmbientLightModifier"] = [weatherManager]() -> float {
        return getAmbientLightModifier(weatherManager);
    };
    
    weatherTable["getSkyColorModifier"] = [weatherManager]() -> Vec3Result {
        return getSkyColorModifier(weatherManager);
    };
    
    weatherTable["getCloudCoverage"] = [weatherManager]() -> float {
        return getCloudCoverage(weatherManager);
    };

    // ========================================================================
    // Storm Effects
    // ========================================================================
    
    weatherTable["shouldTriggerLightning"] = [weatherManager](float deltaTime) -> bool {
        return shouldTriggerLightning(weatherManager, deltaTime);
    };

    // ========================================================================
    // Weather Type Constants
    // ========================================================================
    
    sol::table weatherTypeTable = lua->create_table();
    weatherTypeTable["Clear"] = 0;
    weatherTypeTable["Cloudy"] = 1;
    weatherTypeTable["Rain"] = 2;
    weatherTypeTable["Snow"] = 3;
    weatherTypeTable["Fog"] = 4;
    weatherTypeTable["Storm"] = 5;
    weatherTypeTable["Blizzard"] = 6;
    weatherTable["Type"] = weatherTypeTable;

    LOG_INFO_C("Weather bindings registered successfully", "LuaWeatherBindings");
}

// ============================================================================
// Implementation of helper functions
// ============================================================================

void LuaWeatherBindings::setWeather(WeatherManager* wm, int weatherType)
{
    WeatherType type;
    switch (weatherType) {
        case 0: type = WeatherType::Clear; break;
        case 1: type = WeatherType::Cloudy; break;
        case 2: type = WeatherType::Rain; break;
        case 3: type = WeatherType::Snow; break;
        case 4: type = WeatherType::Fog; break;
        case 5: type = WeatherType::Storm; break;
        case 6: type = WeatherType::Blizzard; break;
        default:
            LOG_WARN_C("Invalid weather type: " + std::to_string(weatherType), "LuaWeatherBindings");
            return;
    }
    wm->setWeather(type);
}

int LuaWeatherBindings::getWeather(WeatherManager* wm)
{
    switch (wm->getCurrentWeather()) {
        case WeatherType::Clear: return 0;
        case WeatherType::Cloudy: return 1;
        case WeatherType::Rain: return 2;
        case WeatherType::Snow: return 3;
        case WeatherType::Fog: return 4;
        case WeatherType::Storm: return 5;
        case WeatherType::Blizzard: return 6;
        default: return 0;
    }
}

const char* LuaWeatherBindings::getWeatherName(WeatherManager* wm)
{
    static std::string weatherStr;
    weatherStr = wm->getWeatherName();
    return weatherStr.c_str();
}

void LuaWeatherBindings::randomizeWeather(WeatherManager* wm)
{
    wm->randomizeWeather();
}

void LuaWeatherBindings::setAutoChange(WeatherManager* wm, bool enabled)
{
    wm->setAutoChange(enabled);
}

bool LuaWeatherBindings::isAutoChangeEnabled(WeatherManager* wm)
{
    return wm->isAutoChangeEnabled();
}

void LuaWeatherBindings::setChangeInterval(WeatherManager* wm, float seconds)
{
    wm->setChangeInterval(seconds);
}

float LuaWeatherBindings::getChangeInterval(WeatherManager* wm)
{
    return wm->getChangeInterval();
}

void LuaWeatherBindings::setCurrentSeason(WeatherManager* wm, int seasonIndex)
{
    wm->setCurrentSeason(seasonIndex);
}

float LuaWeatherBindings::getPrecipitationIntensity(WeatherManager* wm)
{
    return wm->getPrecipitationIntensity();
}

bool LuaWeatherBindings::isRaining(WeatherManager* wm)
{
    return wm->isRaining();
}

bool LuaWeatherBindings::isSnowing(WeatherManager* wm)
{
    return wm->isSnowing();
}

float LuaWeatherBindings::getFogDensity(WeatherManager* wm)
{
    return wm->getFogDensity();
}

float LuaWeatherBindings::getWindStrength(WeatherManager* wm)
{
    return wm->getWindStrength();
}

LuaWeatherBindings::Vec3Result LuaWeatherBindings::getWindDirection(WeatherManager* wm)
{
    glm::vec3 dir = wm->getWindDirection();
    return Vec3Result(dir.x, dir.y, dir.z);
}

float LuaWeatherBindings::getAmbientLightModifier(WeatherManager* wm)
{
    return wm->getAmbientLightModifier();
}

LuaWeatherBindings::Vec3Result LuaWeatherBindings::getSkyColorModifier(WeatherManager* wm)
{
    glm::vec3 color = wm->getSkyColorModifier();
    return Vec3Result(color.x, color.y, color.z);
}

float LuaWeatherBindings::getCloudCoverage(WeatherManager* wm)
{
    return wm->getCloudCoverage();
}

bool LuaWeatherBindings::shouldTriggerLightning(WeatherManager* wm, float deltaTime)
{
    return wm->shouldTriggerLightning(deltaTime);
}

} // namespace scripting
} // namespace fresh

#else // FRESH_LUA_AVAILABLE not defined

// Stub implementation when Lua is not available
#include "scripting/lua/LuaWeatherBindings.h"
#include "core/Logger.h"

namespace fresh
{
namespace scripting
{

void LuaWeatherBindings::registerBindings(LuaScriptingEngine* engine, WeatherManager* weatherManager)
{
    (void)engine;
    (void)weatherManager;
    LOG_INFO_C("Lua weather bindings not available (Lua support not compiled in)", "LuaWeatherBindings");
}

// Stub implementations
void LuaWeatherBindings::setWeather(WeatherManager* wm, int weatherType) { (void)wm; (void)weatherType; }
int LuaWeatherBindings::getWeather(WeatherManager* wm) { (void)wm; return 0; }
const char* LuaWeatherBindings::getWeatherName(WeatherManager* wm) { (void)wm; return "Clear"; }
void LuaWeatherBindings::randomizeWeather(WeatherManager* wm) { (void)wm; }
void LuaWeatherBindings::setAutoChange(WeatherManager* wm, bool enabled) { (void)wm; (void)enabled; }
bool LuaWeatherBindings::isAutoChangeEnabled(WeatherManager* wm) { (void)wm; return true; }
void LuaWeatherBindings::setChangeInterval(WeatherManager* wm, float seconds) { (void)wm; (void)seconds; }
float LuaWeatherBindings::getChangeInterval(WeatherManager* wm) { (void)wm; return 300.0f; }
void LuaWeatherBindings::setCurrentSeason(WeatherManager* wm, int seasonIndex) { (void)wm; (void)seasonIndex; }
float LuaWeatherBindings::getPrecipitationIntensity(WeatherManager* wm) { (void)wm; return 0.0f; }
bool LuaWeatherBindings::isRaining(WeatherManager* wm) { (void)wm; return false; }
bool LuaWeatherBindings::isSnowing(WeatherManager* wm) { (void)wm; return false; }
float LuaWeatherBindings::getFogDensity(WeatherManager* wm) { (void)wm; return 0.0f; }
float LuaWeatherBindings::getWindStrength(WeatherManager* wm) { (void)wm; return 0.1f; }

LuaWeatherBindings::Vec3Result LuaWeatherBindings::getWindDirection(WeatherManager* wm) { 
    (void)wm; 
    return Vec3Result(1.0f, 0.0f, 0.5f); 
}

float LuaWeatherBindings::getAmbientLightModifier(WeatherManager* wm) { (void)wm; return 1.0f; }

LuaWeatherBindings::Vec3Result LuaWeatherBindings::getSkyColorModifier(WeatherManager* wm) { 
    (void)wm; 
    return Vec3Result(1.0f, 1.0f, 1.0f); 
}

float LuaWeatherBindings::getCloudCoverage(WeatherManager* wm) { (void)wm; return 0.1f; }
bool LuaWeatherBindings::shouldTriggerLightning(WeatherManager* wm, float deltaTime) { 
    (void)wm; 
    (void)deltaTime; 
    return false; 
}

} // namespace scripting
} // namespace fresh

#endif // FRESH_LUA_AVAILABLE
