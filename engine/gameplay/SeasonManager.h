#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <string>

namespace fresh
{

/**
 * @brief Season enumeration for the game world
 */
enum class Season {
    Spring,  // Lush green, moderate temperature, occasional rain
    Summer,  // Bright colors, hot, clear skies
    Fall,    // Orange/red foliage, cooler, windy
    Winter   // Snow, ice, cold, shorter days
};

/**
 * @brief Manages seasonal changes in the game world
 * 
 * Controls season progression, visual effects, and environmental changes
 * based on the current season. Integrates with TimeManager for day-based
 * season progression.
 */
class SeasonManager
{
public:
    SeasonManager();
    ~SeasonManager() = default;

    /**
     * @brief Update season progression
     * @param deltaTime Time elapsed since last frame in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Set current season
     * @param season Season to set
     */
    void setSeason(Season season);

    /**
     * @brief Get current season
     * @return Current season
     */
    Season getCurrentSeason() const { return m_currentSeason; }

    /**
     * @brief Get season as string
     * @return Season name
     */
    std::string getSeasonName() const;

    /**
     * @brief Enable/disable automatic season progression
     * @param enabled True to enable automatic progression
     */
    void setAutoProgress(bool enabled) { m_autoProgress = enabled; }

    /**
     * @brief Check if auto-progression is enabled
     * @return True if enabled
     */
    bool isAutoProgressEnabled() const { return m_autoProgress; }

    /**
     * @brief Set days per season for auto-progression
     * @param days Number of in-game days per season
     */
    void setDaysPerSeason(int days) { m_daysPerSeason = days; }

    /**
     * @brief Get days per season
     * @return Days per season
     */
    int getDaysPerSeason() const { return m_daysPerSeason; }

    /**
     * @brief Manually advance to next season
     */
    void advanceToNextSeason();

    /**
     * @brief Notify season manager of a day change
     * @param currentDay Current day number
     */
    void onDayChange(int currentDay);

    // Visual effect modifiers based on season
    /**
     * @brief Get grass color tint for current season
     * @return RGB color multiplier for grass
     */
    glm::vec3 getGrassColorTint() const;

    /**
     * @brief Get foliage (leaves) color tint for current season
     * @return RGB color multiplier for leaves
     */
    glm::vec3 getFoliageColorTint() const;

    /**
     * @brief Get ambient temperature for current season
     * @return Temperature value (0.0 = freezing, 1.0 = hot)
     */
    float getAmbientTemperature() const;

    /**
     * @brief Get sky color modifier for current season
     * @return RGB color modifier for sky
     */
    glm::vec3 getSkyColorModifier() const;

    /**
     * @brief Check if snow should fall during current season
     * @return True if snow should fall
     */
    bool shouldSnow() const { return m_currentSeason == Season::Winter; }

    /**
     * @brief Check if leaves should fall during current season
     * @return True if leaves should fall (Fall season)
     */
    bool shouldLeavesfall() const { return m_currentSeason == Season::Fall; }

    /**
     * @brief Get day length modifier for current season
     * @return Multiplier for day length (1.0 = normal, < 1.0 = shorter days)
     */
    float getDayLengthModifier() const;

    /**
     * @brief Set callback for when season changes
     * @param callback Function to call when season changes
     */
    void setOnSeasonChange(std::function<void(Season)> callback) { m_onSeasonChange = callback; }

private:
    Season m_currentSeason;
    bool m_autoProgress;
    int m_daysPerSeason;
    int m_lastSeasonChangeDay;
    
    std::function<void(Season)> m_onSeasonChange;
};

} // namespace fresh
