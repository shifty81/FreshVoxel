#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <string>

namespace fresh
{

/**
 * @brief Manages game time and day/night cycle
 * 
 * Handles time progression, sun/moon positioning, and provides
 * callbacks for time-based events. Time is measured in ticks where
 * 24000 ticks = 1 full day (20 minutes real time at default speed).
 */
class TimeManager
{
public:
    /**
     * @brief Time of day presets for quick time setting
     */
    enum class TimeOfDay {
        Dawn,      // 6:00 AM (6000 ticks)
        Noon,      // 12:00 PM (12000 ticks)
        Dusk,      // 6:00 PM (18000 ticks)
        Midnight   // 12:00 AM (0 ticks)
    };

    TimeManager();
    ~TimeManager() = default;

    /**
     * @brief Update time progression
     * @param deltaTime Time elapsed since last frame in seconds
     */
    void update(float deltaTime);

    // Time control
    /**
     * @brief Set current time in ticks (0-24000)
     * @param ticks Time in ticks
     */
    void setTime(int ticks);

    /**
     * @brief Set time to a preset time of day
     * @param timeOfDay Preset time to set
     */
    void setTimeOfDay(TimeOfDay timeOfDay);

    /**
     * @brief Get current time in ticks (0-24000)
     * @return Current time in ticks
     */
    int getTime() const { return m_currentTime; }

    /**
     * @brief Get current time as hours (0.0-24.0)
     * @return Current time in hours
     */
    float getTimeInHours() const;

    /**
     * @brief Get formatted time string (HH:MM AM/PM)
     * @return Formatted time string
     */
    std::string getFormattedTime() const;

    /**
     * @brief Pause time progression
     */
    void pause() { m_paused = true; }

    /**
     * @brief Resume time progression
     */
    void resume() { m_paused = false; }

    /**
     * @brief Toggle pause state
     */
    void togglePause() { m_paused = !m_paused; }

    /**
     * @brief Check if time is paused
     * @return true if paused
     */
    bool isPaused() const { return m_paused; }

    /**
     * @brief Set time progression rate multiplier
     * @param rate Rate multiplier (1.0 = normal, 2.0 = 2x speed, etc.)
     */
    void setTimeRate(float rate);

    /**
     * @brief Get current time rate
     * @return Time rate multiplier
     */
    float getTimeRate() const { return m_timeRate; }

    // Sun/Moon calculations
    /**
     * @brief Get sun position in world space
     * @return Normalized direction vector for sun light
     */
    glm::vec3 getSunDirection() const;

    /**
     * @brief Get moon position in world space
     * @return Normalized direction vector for moon light
     */
    glm::vec3 getMoonDirection() const;

    /**
     * @brief Get sun elevation angle in degrees (0-180)
     * @return Sun elevation angle
     */
    float getSunElevation() const;

    /**
     * @brief Check if it's daytime
     * @return true if sun is above horizon
     */
    bool isDaytime() const;

    /**
     * @brief Check if it's nighttime
     * @return true if sun is below horizon
     */
    bool isNighttime() const { return !isDaytime(); }

    /**
     * @brief Check if it's sunrise period
     * @return true if in sunrise transition
     */
    bool isSunrise() const;

    /**
     * @brief Check if it's sunset period
     * @return true if in sunset transition
     */
    bool isSunset() const;

    // Lighting calculations
    /**
     * @brief Get ambient light intensity (0.0-1.0)
     * @return Ambient light intensity
     */
    float getAmbientLightIntensity() const;

    /**
     * @brief Get sky color for current time
     * @return RGB color for sky
     */
    glm::vec3 getSkyColor() const;

    /**
     * @brief Get horizon color for current time (for sunrise/sunset)
     * @return RGB color for horizon
     */
    glm::vec3 getHorizonColor() const;

    /**
     * @brief Get sun light color
     * @return RGB color for sun light
     */
    glm::vec3 getSunLightColor() const;

    /**
     * @brief Get moon light color
     * @return RGB color for moon light
     */
    glm::vec3 getMoonLightColor() const;

    // Event callbacks
    /**
     * @brief Set callback for when a new day starts
     * @param callback Function to call when day changes
     */
    void setOnDayChange(std::function<void(int)> callback) { m_onDayChange = callback; }

    /**
     * @brief Set callback for sunrise event
     * @param callback Function to call at sunrise
     */
    void setOnSunrise(std::function<void()> callback) { m_onSunrise = callback; }

    /**
     * @brief Set callback for sunset event
     * @param callback Function to call at sunset
     */
    void setOnSunset(std::function<void()> callback) { m_onSunset = callback; }

    /**
     * @brief Get current day number
     * @return Days elapsed since game start
     */
    int getCurrentDay() const { return m_currentDay; }

private:
    /**
     * @brief Calculate lighting colors based on sun elevation
     */
    void updateLighting();

    /**
     * @brief Check and trigger time-based events
     */
    void checkEvents();

    /**
     * @brief Normalize time to 0-24000 range
     */
    void normalizeTime();

private:
    // Time state
    int m_currentTime;      // 0-24000 ticks (24000 ticks = 1 day)
    int m_currentDay;       // Days elapsed
    float m_timeRate;       // Speed multiplier
    bool m_paused;          // Time progression paused

    // Ticks per real-time second at 1x speed
    // 24000 ticks per 20 minutes = 20 ticks per second
    static constexpr float TICKS_PER_SECOND = 20.0f;

    // Time of day ranges
    static constexpr int SUNRISE_START = 5000;  // ~5 AM
    static constexpr int SUNRISE_END = 7000;    // ~7 AM
    static constexpr int SUNSET_START = 17000;  // ~5 PM
    static constexpr int SUNSET_END = 19000;    // ~7 PM

    // Event tracking
    bool m_wasDay;          // Was it daytime last frame?
    std::function<void(int)> m_onDayChange;
    std::function<void()> m_onSunrise;
    std::function<void()> m_onSunset;

    // Cached lighting values (updated each frame)
    float m_ambientIntensity;
    glm::vec3 m_skyColor;
    glm::vec3 m_horizonColor;
    glm::vec3 m_sunColor;
    glm::vec3 m_moonColor;
};

} // namespace fresh
