#include "gameplay/TimeManager.h"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace fresh
{

TimeManager::TimeManager()
    : m_currentTime(6000)  // Start at dawn
    , m_currentDay(0)
    , m_timeRate(1.0f)
    , m_paused(false)
    , m_wasDay(true)
    , m_ambientIntensity(0.8f)
    , m_skyColor(0.53f, 0.81f, 0.92f)  // Default day sky
    , m_horizonColor(1.0f, 0.9f, 0.7f)
    , m_sunColor(1.0f, 1.0f, 0.9f)
    , m_moonColor(0.7f, 0.7f, 0.9f)
{
    updateLighting();
}

void TimeManager::update(float deltaTime)
{
    if (m_paused) {
        return;
    }

    // Progress time
    float ticksDelta = TICKS_PER_SECOND * m_timeRate * deltaTime;
    m_currentTime += static_cast<int>(ticksDelta);

    // Handle day rollover
    normalizeTime();

    // Update lighting
    updateLighting();

    // Check for events
    checkEvents();
}

void TimeManager::setTime(int ticks)
{
    m_currentTime = ticks;
    normalizeTime();
    updateLighting();
}

void TimeManager::setTimeOfDay(TimeOfDay timeOfDay)
{
    switch (timeOfDay) {
    case TimeOfDay::Dawn:
        m_currentTime = 6000;
        break;
    case TimeOfDay::Noon:
        m_currentTime = 12000;
        break;
    case TimeOfDay::Dusk:
        m_currentTime = 18000;
        break;
    case TimeOfDay::Midnight:
        m_currentTime = 0;
        break;
    }
    updateLighting();
}

float TimeManager::getTimeInHours() const
{
    return (m_currentTime / 24000.0f) * 24.0f;
}

std::string TimeManager::getFormattedTime() const
{
    float hours = getTimeInHours();
    int hour = static_cast<int>(hours);
    int minute = static_cast<int>((hours - hour) * 60);

    // Convert to 12-hour format
    bool isPM = hour >= 12;
    int displayHour = hour % 12;
    if (displayHour == 0) displayHour = 12;

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << displayHour << ":"
        << std::setfill('0') << std::setw(2) << minute
        << (isPM ? " PM" : " AM");
    return oss.str();
}

void TimeManager::setTimeRate(float rate)
{
    m_timeRate = std::max(0.0f, rate);  // Prevent negative time
}

glm::vec3 TimeManager::getSunDirection() const
{
    // Sun moves from east to west
    // At noon (12000 ticks), sun is directly overhead
    // Convert time to angle (0-360 degrees)
    float angle = (m_currentTime / 24000.0f) * 360.0f;
    
    // Offset so noon is at top (90 degrees)
    angle = angle - 90.0f;
    
    float radians = glm::radians(angle);
    
    return glm::normalize(glm::vec3(
        std::cos(radians),
        std::sin(radians),
        0.0f
    ));
}

glm::vec3 TimeManager::getMoonDirection() const
{
    // Moon is opposite to sun
    return -getSunDirection();
}

float TimeManager::getSunElevation() const
{
    glm::vec3 sunDir = getSunDirection();
    // Calculate angle from horizon (y component)
    float elevation = std::asin(sunDir.y);
    return glm::degrees(elevation);
}

bool TimeManager::isDaytime() const
{
    float elevation = getSunElevation();
    return elevation > 0.0f;
}

bool TimeManager::isSunrise() const
{
    return m_currentTime >= SUNRISE_START && m_currentTime <= SUNRISE_END;
}

bool TimeManager::isSunset() const
{
    return m_currentTime >= SUNSET_START && m_currentTime <= SUNSET_END;
}

float TimeManager::getAmbientLightIntensity() const
{
    return m_ambientIntensity;
}

glm::vec3 TimeManager::getSkyColor() const
{
    return m_skyColor;
}

glm::vec3 TimeManager::getHorizonColor() const
{
    return m_horizonColor;
}

glm::vec3 TimeManager::getSunLightColor() const
{
    return m_sunColor;
}

glm::vec3 TimeManager::getMoonLightColor() const
{
    return m_moonColor;
}

void TimeManager::updateLighting()
{
    float elevation = getSunElevation();
    
    // Calculate ambient light intensity based on sun elevation
    // Day: 0.8-1.0, Night: 0.1-0.2, Transitions: smooth interpolation
    if (elevation > 0.0f) {
        // Daytime
        m_ambientIntensity = 0.8f + (elevation / 90.0f) * 0.2f;
    } else {
        // Nighttime
        float nightFactor = std::max(0.0f, (elevation + 30.0f) / 30.0f);
        m_ambientIntensity = 0.1f + nightFactor * 0.2f;
    }

    // Calculate sky color
    if (isSunrise() || isSunset()) {
        // Sunrise/Sunset: gradient from orange to blue
        float transitionProgress;
        if (isSunrise()) {
            transitionProgress = static_cast<float>(m_currentTime - SUNRISE_START) / 
                                static_cast<float>(SUNRISE_END - SUNRISE_START);
        } else {
            transitionProgress = 1.0f - (static_cast<float>(m_currentTime - SUNSET_START) / 
                                        static_cast<float>(SUNSET_END - SUNSET_START));
        }
        
        glm::vec3 sunsetColor(1.0f, 0.5f, 0.2f);    // Orange
        glm::vec3 dayColor(0.53f, 0.81f, 0.92f);    // Sky blue
        m_skyColor = glm::mix(sunsetColor, dayColor, transitionProgress);
        m_horizonColor = glm::vec3(1.0f, 0.7f, 0.4f);  // Warm horizon
    } else if (isDaytime()) {
        // Daytime: blue sky
        m_skyColor = glm::vec3(0.53f, 0.81f, 0.92f);
        m_horizonColor = glm::vec3(0.8f, 0.9f, 1.0f);
    } else {
        // Nighttime: dark blue/black
        float nightProgress = std::max(0.0f, std::min(1.0f, 
            (19000.0f - m_currentTime) / 5000.0f));
        if (m_currentTime < 5000) {
            nightProgress = std::max(0.0f, std::min(1.0f, 
                (5000.0f - m_currentTime) / 5000.0f));
        }
        m_skyColor = glm::mix(
            glm::vec3(0.05f, 0.05f, 0.15f),  // Deep night
            glm::vec3(0.1f, 0.1f, 0.3f),     // Early night
            nightProgress
        );
        m_horizonColor = glm::vec3(0.1f, 0.1f, 0.2f);
    }

    // Sun color: warm during day, orange at sunset
    if (isSunset() || isSunrise()) {
        m_sunColor = glm::vec3(1.0f, 0.6f, 0.3f);  // Orange sun
    } else if (isDaytime()) {
        m_sunColor = glm::vec3(1.0f, 1.0f, 0.95f); // White-yellow sun
    } else {
        m_sunColor = glm::vec3(0.0f, 0.0f, 0.0f);  // No sun at night
    }

    // Moon color: blue-white at night
    if (isNighttime()) {
        m_moonColor = glm::vec3(0.7f, 0.7f, 0.9f);
    } else {
        m_moonColor = glm::vec3(0.0f, 0.0f, 0.0f); // No moon during day
    }
}

void TimeManager::checkEvents()
{
    bool isDay = isDaytime();
    
    // Check for day/night transition
    if (isDay != m_wasDay) {
        if (isDay && m_onSunrise) {
            m_onSunrise();
        } else if (!isDay && m_onSunset) {
            m_onSunset();
        }
        m_wasDay = isDay;
    }
}

void TimeManager::normalizeTime()
{
    if (m_currentTime >= 24000) {
        m_currentTime -= 24000;
        m_currentDay++;
        if (m_onDayChange) {
            m_onDayChange(m_currentDay);
        }
    } else if (m_currentTime < 0) {
        m_currentTime += 24000;
        m_currentDay = std::max(0, m_currentDay - 1);
    }
}

} // namespace fresh
