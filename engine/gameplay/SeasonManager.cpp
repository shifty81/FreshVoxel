#include "gameplay/SeasonManager.h"

namespace fresh
{

SeasonManager::SeasonManager()
    : m_currentSeason(Season::Spring)
    , m_autoProgress(true)
    , m_daysPerSeason(7)  // 7 in-game days per season by default
    , m_lastSeasonChangeDay(0)
{
}

void SeasonManager::update(float deltaTime)
{
    // Update is called each frame but season changes are day-based
    // Actual progression happens in onDayChange()
    (void)deltaTime;
}

void SeasonManager::setSeason(Season season)
{
    if (m_currentSeason != season) {
        m_currentSeason = season;
        
        if (m_onSeasonChange) {
            m_onSeasonChange(season);
        }
    }
}

std::string SeasonManager::getSeasonName() const
{
    switch (m_currentSeason) {
        case Season::Spring: return "Spring";
        case Season::Summer: return "Summer";
        case Season::Fall:   return "Fall";
        case Season::Winter: return "Winter";
        default:             return "Unknown";
    }
}

void SeasonManager::advanceToNextSeason()
{
    Season nextSeason;
    switch (m_currentSeason) {
        case Season::Spring: nextSeason = Season::Summer; break;
        case Season::Summer: nextSeason = Season::Fall; break;
        case Season::Fall:   nextSeason = Season::Winter; break;
        case Season::Winter: nextSeason = Season::Spring; break;
        default:             nextSeason = Season::Spring; break;
    }
    
    setSeason(nextSeason);
}

void SeasonManager::onDayChange(int currentDay)
{
    if (!m_autoProgress) {
        return;
    }
    
    // Check if enough days have passed to change season
    int daysSinceLastChange = currentDay - m_lastSeasonChangeDay;
    if (daysSinceLastChange >= m_daysPerSeason) {
        advanceToNextSeason();
        m_lastSeasonChangeDay = currentDay;
    }
}

glm::vec3 SeasonManager::getGrassColorTint() const
{
    switch (m_currentSeason) {
        case Season::Spring:
            return glm::vec3(0.7f, 1.0f, 0.7f);  // Lush green
        case Season::Summer:
            return glm::vec3(1.0f, 1.0f, 0.8f);  // Bright, slightly yellow
        case Season::Fall:
            return glm::vec3(0.8f, 0.7f, 0.4f);  // Brown/yellow
        case Season::Winter:
            return glm::vec3(0.9f, 0.9f, 0.9f);  // Covered with snow (white tint)
        default:
            return glm::vec3(1.0f, 1.0f, 1.0f);
    }
}

glm::vec3 SeasonManager::getFoliageColorTint() const
{
    switch (m_currentSeason) {
        case Season::Spring:
            return glm::vec3(0.6f, 1.0f, 0.6f);  // Fresh green
        case Season::Summer:
            return glm::vec3(0.5f, 0.9f, 0.5f);  // Dark green
        case Season::Fall:
            return glm::vec3(1.0f, 0.5f, 0.2f);  // Orange/red
        case Season::Winter:
            return glm::vec3(0.3f, 0.3f, 0.3f);  // Dead/bare (dark)
        default:
            return glm::vec3(1.0f, 1.0f, 1.0f);
    }
}

float SeasonManager::getAmbientTemperature() const
{
    switch (m_currentSeason) {
        case Season::Spring: return 0.6f;  // Mild
        case Season::Summer: return 0.9f;  // Hot
        case Season::Fall:   return 0.5f;  // Cool
        case Season::Winter: return 0.2f;  // Cold
        default:             return 0.5f;
    }
}

glm::vec3 SeasonManager::getSkyColorModifier() const
{
    switch (m_currentSeason) {
        case Season::Spring:
            return glm::vec3(1.0f, 1.0f, 1.1f);  // Bright blue
        case Season::Summer:
            return glm::vec3(1.1f, 1.05f, 1.0f); // Warm, slightly hazy
        case Season::Fall:
            return glm::vec3(1.0f, 0.95f, 0.9f); // Slightly orange tint
        case Season::Winter:
            return glm::vec3(0.9f, 0.95f, 1.0f); // Cool blue tint
        default:
            return glm::vec3(1.0f, 1.0f, 1.0f);
    }
}

float SeasonManager::getDayLengthModifier() const
{
    switch (m_currentSeason) {
        case Season::Spring: return 1.0f;  // Normal day length
        case Season::Summer: return 1.2f;  // Longer days
        case Season::Fall:   return 1.0f;  // Normal day length
        case Season::Winter: return 0.8f;  // Shorter days
        default:             return 1.0f;
    }
}

} // namespace fresh
