#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <string>
#include <random>

namespace fresh
{

/**
 * @brief Weather type enumeration
 */
enum class WeatherType {
    Clear,      // Sunny, no precipitation
    Cloudy,     // Overcast but no rain
    Rain,       // Rainfall with clouds
    Snow,       // Snowfall (primarily in winter)
    Fog,        // Heavy fog reducing visibility
    Storm,      // Thunderstorm with heavy rain
    Blizzard    // Heavy snow with wind (winter)
};

/**
 * @brief Manages dynamic weather system
 * 
 * Controls weather changes, precipitation effects, visibility,
 * and lighting adjustments based on current weather conditions.
 */
class WeatherManager
{
public:
    WeatherManager();
    ~WeatherManager() = default;

    /**
     * @brief Update weather system
     * @param deltaTime Time elapsed since last frame in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Set current weather
     * @param weather Weather type to set
     */
    void setWeather(WeatherType weather);

    /**
     * @brief Get current weather
     * @return Current weather type
     */
    WeatherType getCurrentWeather() const { return m_currentWeather; }

    /**
     * @brief Get weather as string
     * @return Weather type name
     */
    std::string getWeatherName() const;

    /**
     * @brief Enable/disable automatic weather changes
     * @param enabled True to enable random weather changes
     */
    void setAutoChange(bool enabled) { m_autoChange = enabled; }

    /**
     * @brief Check if auto-change is enabled
     * @return True if enabled
     */
    bool isAutoChangeEnabled() const { return m_autoChange; }

    /**
     * @brief Set weather change interval
     * @param seconds Time in seconds between potential weather changes
     */
    void setChangeInterval(float seconds) { m_changeInterval = seconds; }

    /**
     * @brief Get weather change interval
     * @return Interval in seconds
     */
    float getChangeInterval() const { return m_changeInterval; }

    /**
     * @brief Set current season for weather probability adjustments
     * @param seasonIndex 0=Spring, 1=Summer, 2=Fall, 3=Winter
     */
    void setCurrentSeason(int seasonIndex) { m_currentSeason = seasonIndex; }

    /**
     * @brief Trigger random weather change
     */
    void randomizeWeather();

    // Weather effect parameters
    /**
     * @brief Get precipitation intensity (0.0 = none, 1.0 = heavy)
     * @return Precipitation intensity
     */
    float getPrecipitationIntensity() const;

    /**
     * @brief Check if it's raining
     * @return True if weather includes rain
     */
    bool isRaining() const;

    /**
     * @brief Check if it's snowing
     * @return True if weather includes snow
     */
    bool isSnowing() const;

    /**
     * @brief Get fog density (0.0 = clear, 1.0 = heavy fog)
     * @return Fog density value
     */
    float getFogDensity() const;

    /**
     * @brief Get wind strength (0.0 = calm, 1.0 = strong wind)
     * @return Wind strength
     */
    float getWindStrength() const;

    /**
     * @brief Get wind direction
     * @return Wind direction vector (normalized)
     */
    glm::vec3 getWindDirection() const;

    /**
     * @brief Get ambient light modifier for weather
     * @return Light intensity multiplier (0.0-1.0)
     */
    float getAmbientLightModifier() const;

    /**
     * @brief Get sky color modifier for weather
     * @return RGB color modifier
     */
    glm::vec3 getSkyColorModifier() const;

    /**
     * @brief Get cloud coverage (0.0 = clear, 1.0 = overcast)
     * @return Cloud coverage value
     */
    float getCloudCoverage() const;

    /**
     * @brief Check if lightning should occur (for storms)
     * @param deltaTime Time since last frame
     * @return True if lightning flash should occur
     */
    bool shouldTriggerLightning(float deltaTime);

    /**
     * @brief Set callback for when weather changes
     * @param callback Function to call when weather changes
     */
    void setOnWeatherChange(std::function<void(WeatherType)> callback) { 
        m_onWeatherChange = callback; 
    }

private:
    /**
     * @brief Select new random weather based on current season
     * @return New weather type
     */
    WeatherType selectRandomWeather();

    /**
     * @brief Calculate transition between weather states
     */
    void updateTransition(float deltaTime);

private:
    WeatherType m_currentWeather;
    WeatherType m_targetWeather;      // For smooth transitions
    float m_transitionProgress;        // 0.0 to 1.0
    bool m_autoChange;
    float m_changeInterval;            // Seconds between weather checks
    float m_timeSinceLastChange;
    int m_currentSeason;               // 0=Spring, 1=Summer, 2=Fall, 3=Winter
    
    // Lightning timing for storms
    float m_lightningTimer;
    float m_nextLightningTime;
    
    std::mt19937 m_randomEngine;
    std::uniform_real_distribution<float> m_distribution;
    
    std::function<void(WeatherType)> m_onWeatherChange;
};

} // namespace fresh
