#include "gameplay/WeatherManager.h"
#include <cmath>

namespace fresh
{

WeatherManager::WeatherManager()
    : m_currentWeather(WeatherType::Clear)
    , m_targetWeather(WeatherType::Clear)
    , m_transitionProgress(1.0f)
    , m_autoChange(true)
    , m_changeInterval(300.0f)  // 5 minutes default
    , m_timeSinceLastChange(0.0f)
    , m_currentSeason(0)
    , m_lightningTimer(0.0f)
    , m_nextLightningTime(0.0f)
    , m_randomEngine(std::random_device{}())
    , m_distribution(0.0f, 1.0f)
{
}

void WeatherManager::update(float deltaTime)
{
    // Update transition between weather states
    if (m_transitionProgress < 1.0f) {
        updateTransition(deltaTime);
    }
    
    // Check for automatic weather changes
    if (m_autoChange) {
        m_timeSinceLastChange += deltaTime;
        
        if (m_timeSinceLastChange >= m_changeInterval) {
            randomizeWeather();
            m_timeSinceLastChange = 0.0f;
        }
    }
    
    // Update lightning timing for storms
    if (m_currentWeather == WeatherType::Storm) {
        m_lightningTimer += deltaTime;
    }
}

void WeatherManager::setWeather(WeatherType weather)
{
    if (m_currentWeather != weather) {
        m_targetWeather = weather;
        m_transitionProgress = 0.0f;  // Start transition
        
        if (m_onWeatherChange) {
            m_onWeatherChange(weather);
        }
    }
}

std::string WeatherManager::getWeatherName() const
{
    switch (m_currentWeather) {
        case WeatherType::Clear:    return "Clear";
        case WeatherType::Cloudy:   return "Cloudy";
        case WeatherType::Rain:     return "Rain";
        case WeatherType::Snow:     return "Snow";
        case WeatherType::Fog:      return "Fog";
        case WeatherType::Storm:    return "Storm";
        case WeatherType::Blizzard: return "Blizzard";
        default:                    return "Unknown";
    }
}

void WeatherManager::randomizeWeather()
{
    WeatherType newWeather = selectRandomWeather();
    setWeather(newWeather);
}

WeatherType WeatherManager::selectRandomWeather()
{
    float roll = m_distribution(m_randomEngine);
    
    // Weather probabilities based on season
    // Spring: Rain common, occasional storms
    // Summer: Clear common, occasional storms
    // Fall: Cloudy and rain common
    // Winter: Snow and blizzard common
    
    switch (m_currentSeason) {
        case 0: // Spring
            if (roll < 0.3f) return WeatherType::Clear;
            if (roll < 0.5f) return WeatherType::Cloudy;
            if (roll < 0.8f) return WeatherType::Rain;
            if (roll < 0.9f) return WeatherType::Storm;
            return WeatherType::Fog;
            
        case 1: // Summer
            if (roll < 0.6f) return WeatherType::Clear;
            if (roll < 0.8f) return WeatherType::Cloudy;
            if (roll < 0.95f) return WeatherType::Rain;
            return WeatherType::Storm;
            
        case 2: // Fall
            if (roll < 0.25f) return WeatherType::Clear;
            if (roll < 0.5f) return WeatherType::Cloudy;
            if (roll < 0.75f) return WeatherType::Rain;
            if (roll < 0.85f) return WeatherType::Fog;
            return WeatherType::Storm;
            
        case 3: // Winter
            if (roll < 0.2f) return WeatherType::Clear;
            if (roll < 0.35f) return WeatherType::Cloudy;
            if (roll < 0.7f) return WeatherType::Snow;
            if (roll < 0.85f) return WeatherType::Blizzard;
            return WeatherType::Fog;
            
        default:
            return WeatherType::Clear;
    }
}

void WeatherManager::updateTransition(float deltaTime)
{
    // Smooth transition over 10 seconds
    m_transitionProgress += deltaTime / 10.0f;
    
    if (m_transitionProgress >= 1.0f) {
        m_transitionProgress = 1.0f;
        m_currentWeather = m_targetWeather;
    }
}

float WeatherManager::getPrecipitationIntensity() const
{
    float intensity = 0.0f;
    
    switch (m_currentWeather) {
        case WeatherType::Rain:
            intensity = 0.6f;
            break;
        case WeatherType::Snow:
            intensity = 0.5f;
            break;
        case WeatherType::Storm:
            intensity = 1.0f;
            break;
        case WeatherType::Blizzard:
            intensity = 0.9f;
            break;
        default:
            intensity = 0.0f;
            break;
    }
    
    // Apply transition
    return intensity * m_transitionProgress;
}

bool WeatherManager::isRaining() const
{
    return m_currentWeather == WeatherType::Rain || 
           m_currentWeather == WeatherType::Storm;
}

bool WeatherManager::isSnowing() const
{
    return m_currentWeather == WeatherType::Snow || 
           m_currentWeather == WeatherType::Blizzard;
}

float WeatherManager::getFogDensity() const
{
    float density = 0.0f;
    
    switch (m_currentWeather) {
        case WeatherType::Fog:
            density = 0.8f;
            break;
        case WeatherType::Cloudy:
            density = 0.2f;
            break;
        case WeatherType::Rain:
            density = 0.3f;
            break;
        case WeatherType::Storm:
            density = 0.5f;
            break;
        case WeatherType::Blizzard:
            density = 0.7f;
            break;
        default:
            density = 0.0f;
            break;
    }
    
    return density * m_transitionProgress;
}

float WeatherManager::getWindStrength() const
{
    float strength = 0.0f;
    
    switch (m_currentWeather) {
        case WeatherType::Clear:
            strength = 0.1f;
            break;
        case WeatherType::Cloudy:
            strength = 0.2f;
            break;
        case WeatherType::Rain:
            strength = 0.4f;
            break;
        case WeatherType::Storm:
            strength = 0.8f;
            break;
        case WeatherType::Blizzard:
            strength = 1.0f;
            break;
        default:
            strength = 0.1f;
            break;
    }
    
    return strength * m_transitionProgress;
}

glm::vec3 WeatherManager::getWindDirection() const
{
    // For now, wind blows in a consistent direction
    // Could be enhanced with time-varying wind
    return glm::normalize(glm::vec3(1.0f, 0.0f, 0.5f));
}

float WeatherManager::getAmbientLightModifier() const
{
    float modifier = 1.0f;
    
    switch (m_currentWeather) {
        case WeatherType::Clear:
            modifier = 1.0f;
            break;
        case WeatherType::Cloudy:
            modifier = 0.8f;
            break;
        case WeatherType::Rain:
            modifier = 0.7f;
            break;
        case WeatherType::Snow:
            modifier = 0.85f;
            break;
        case WeatherType::Fog:
            modifier = 0.6f;
            break;
        case WeatherType::Storm:
            modifier = 0.5f;
            break;
        case WeatherType::Blizzard:
            modifier = 0.55f;
            break;
    }
    
    // Interpolate based on transition
    return 1.0f + (modifier - 1.0f) * m_transitionProgress;
}

glm::vec3 WeatherManager::getSkyColorModifier() const
{
    glm::vec3 modifier(1.0f);
    
    switch (m_currentWeather) {
        case WeatherType::Clear:
            modifier = glm::vec3(1.0f, 1.0f, 1.0f);
            break;
        case WeatherType::Cloudy:
            modifier = glm::vec3(0.7f, 0.7f, 0.75f);
            break;
        case WeatherType::Rain:
            modifier = glm::vec3(0.6f, 0.6f, 0.65f);
            break;
        case WeatherType::Snow:
            modifier = glm::vec3(0.9f, 0.9f, 0.95f);
            break;
        case WeatherType::Fog:
            modifier = glm::vec3(0.8f, 0.8f, 0.85f);
            break;
        case WeatherType::Storm:
            modifier = glm::vec3(0.4f, 0.4f, 0.45f);
            break;
        case WeatherType::Blizzard:
            modifier = glm::vec3(0.85f, 0.85f, 0.9f);
            break;
    }
    
    // Interpolate based on transition
    return glm::vec3(1.0f) + (modifier - glm::vec3(1.0f)) * m_transitionProgress;
}

float WeatherManager::getCloudCoverage() const
{
    float coverage = 0.0f;
    
    switch (m_currentWeather) {
        case WeatherType::Clear:
            coverage = 0.1f;
            break;
        case WeatherType::Cloudy:
            coverage = 0.7f;
            break;
        case WeatherType::Rain:
        case WeatherType::Snow:
            coverage = 0.9f;
            break;
        case WeatherType::Storm:
        case WeatherType::Blizzard:
            coverage = 1.0f;
            break;
        case WeatherType::Fog:
            coverage = 0.5f;
            break;
    }
    
    return coverage * m_transitionProgress;
}

bool WeatherManager::shouldTriggerLightning(float /*deltaTime*/)
{
    if (m_currentWeather != WeatherType::Storm) {
        return false;
    }
    
    // Check if it's time for next lightning strike
    if (m_lightningTimer >= m_nextLightningTime) {
        m_lightningTimer = 0.0f;
        // Random interval between 2-8 seconds
        m_nextLightningTime = 2.0f + m_distribution(m_randomEngine) * 6.0f;
        return true;
    }
    
    return false;
}

} // namespace fresh
