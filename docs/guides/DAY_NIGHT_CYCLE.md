# Day/Night Cycle System - Usage Guide

## Overview

The Fresh Voxel Engine includes a comprehensive day/night cycle system that manages time progression, sun/moon positioning, and dynamic lighting. The TimeManager is fully integrated with the engine and exposed to Lua for easy scripting.

## Features

- **Time Progression**: Realistic 24-hour day/night cycle
- **Sun/Moon Calculations**: Accurate celestial body positioning
- **Dynamic Lighting**: Smooth transitions between day, sunrise, sunset, and night
- **Sky Colors**: Beautiful color gradients based on time of day
- **Time Control**: Pause, accelerate, or set specific times
- **Event Callbacks**: React to sunrise, sunset, and day changes
- **Lua Integration**: Full API access from Lua scripts

## C++ Integration

### Engine Integration

The TimeManager is automatically initialized in the Engine:

```cpp
// In Engine::initialize()
if (!m_timeManager) {
    m_timeManager = std::make_unique<TimeManager>();
}

// In Engine::update(float deltaTime)
if (m_timeManager) {
    m_timeManager->update(deltaTime);
}
```

### Direct Usage

```cpp
#include "gameplay/TimeManager.h"

// Create time manager
TimeManager timeManager;

// Set time
timeManager.setTime(12000);  // Set to noon (12000 ticks)
timeManager.setTimeOfDay(TimeManager::TimeOfDay::Dawn);

// Get time information
int ticks = timeManager.getTime();              // 0-24000
float hours = timeManager.getTimeInHours();     // 0.0-24.0
std::string formatted = timeManager.getFormattedTime();  // "12:30 PM"

// Control time flow
timeManager.pause();
timeManager.resume();
timeManager.setTimeRate(2.0f);  // 2x speed

// Query sun/moon
glm::vec3 sunDir = timeManager.getSunDirection();
float sunElevation = timeManager.getSunElevation();
bool isDay = timeManager.isDaytime();
bool isSunrise = timeManager.isSunrise();

// Get lighting information
float ambient = timeManager.getAmbientLightIntensity();
glm::vec3 skyColor = timeManager.getSkyColor();
glm::vec3 sunColor = timeManager.getSunLightColor();
glm::vec3 moonColor = timeManager.getMoonLightColor();

// Event callbacks
timeManager.setOnDayChange([](int day) {
    std::cout << "New day: " << day << std::endl;
});

timeManager.setOnSunrise([]() {
    std::cout << "Sunrise!" << std::endl;
});

timeManager.setOnSunset([]() {
    std::cout << "Sunset!" << std::endl;
});
```

## Lua API Reference

### Time Control

```lua
-- Set time directly (0-24000 ticks)
Time.setTime(12000)  -- Noon

-- Set time using presets
Time.setTimeOfDay(Time.TimeOfDay.Dawn)      -- 6:00 AM
Time.setTimeOfDay(Time.TimeOfDay.Noon)      -- 12:00 PM
Time.setTimeOfDay(Time.TimeOfDay.Dusk)      -- 6:00 PM
Time.setTimeOfDay(Time.TimeOfDay.Midnight)  -- 12:00 AM

-- Get current time
local ticks = Time.getTime()              -- 0-24000
local hours = Time.getTimeInHours()       -- 0.0-24.0
local formatted = Time.getFormattedTime() -- "03:45 PM"

-- Control time flow
Time.pause()          -- Pause time progression
Time.resume()         -- Resume time progression
Time.togglePause()    -- Toggle pause state
local paused = Time.isPaused()  -- Check if paused

-- Adjust time speed
Time.setTimeRate(10.0)  -- 10x speed
local rate = Time.getTimeRate()
```

### Sun/Moon Queries

```lua
-- Sun position
local sunDir = Time.getSunDirection()  -- Returns Vec3Result
print(sunDir.x, sunDir.y, sunDir.z)

-- Moon position (opposite to sun)
local moonDir = Time.getMoonDirection()

-- Sun elevation
local elevation = Time.getSunElevation()  -- Degrees above horizon

-- Time of day checks
if Time.isDaytime() then
    print("It's daytime!")
end

if Time.isNighttime() then
    print("It's nighttime!")
end

if Time.isSunrise() then
    print("Beautiful sunrise!")
end

if Time.isSunset() then
    print("Beautiful sunset!")
end
```

### Lighting Queries

```lua
-- Ambient light intensity (0.0-1.0)
local ambient = Time.getAmbientLightIntensity()

-- Sky color
local skyColor = Time.getSkyColor()  -- Vec3Result (R, G, B)
print("Sky:", skyColor.x, skyColor.y, skyColor.z)

-- Horizon color (for sunrise/sunset)
local horizonColor = Time.getHorizonColor()

-- Sun light color
local sunColor = Time.getSunLightColor()

-- Moon light color
local moonColor = Time.getMoonLightColor()
```

### Day Tracking

```lua
-- Get current day number
local day = Time.getCurrentDay()
print("Day " .. day .. " in the world")
```

## Time System Details

### Time Units

- **1 Full Day** = 24,000 ticks
- **1 Real-Time Minute** = 20 ticks (at 1x speed)
- **1 Game Day** = 20 minutes real-time (at 1x speed)

### Time Ranges

| Time of Day | Ticks | Hours |
|------------|-------|-------|
| Midnight | 0 | 0:00 |
| Dawn Start | 5000 | ~5:00 AM |
| Sunrise | 5000-7000 | 5:00-7:00 AM |
| Noon | 12000 | 12:00 PM |
| Sunset | 17000-19000 | 5:00-7:00 PM |
| Dusk End | 19000 | ~7:00 PM |

### Lighting Calculations

The TimeManager automatically calculates:

1. **Ambient Light Intensity**
   - Day: 0.8-1.0
   - Night: 0.1-0.2
   - Smooth transitions during sunrise/sunset

2. **Sky Colors**
   - Day: Bright blue (0.53, 0.81, 0.92)
   - Sunrise/Sunset: Orange gradient (1.0, 0.5, 0.2)
   - Night: Dark blue/black (0.05, 0.05, 0.15)

3. **Sun Light Color**
   - Midday: White-yellow (1.0, 1.0, 0.95)
   - Sunset: Orange (1.0, 0.6, 0.3)
   - Night: No sun light (0, 0, 0)

4. **Moon Light Color**
   - Night: Blue-white (0.7, 0.7, 0.9)
   - Day: No moon light (0, 0, 0)

## Complete Example Scripts

### Basic Time Display

```lua
function on_update(dt)
    local time = Time.getFormattedTime()
    local day = Time.getCurrentDay()
    
    print("Day " .. day .. " - " .. time)
    
    if Time.isSunrise() then
        print("🌅 Sunrise!")
    elseif Time.isSunset() then
        print("🌇 Sunset!")
    end
end
```

### Time-Based Events

```lua
local lastPeriod = "day"

function on_update(dt)
    local currentPeriod = Time.isDaytime() and "day" or "night"
    
    if currentPeriod ~= lastPeriod then
        if currentPeriod == "day" then
            onDayStart()
        else
            onNightStart()
        end
        lastPeriod = currentPeriod
    end
end

function onDayStart()
    print("☀️  Good morning! Day has begun.")
    -- Spawn daytime creatures
    -- Play morning music
end

function onNightStart()
    print("🌙 Good evening! Night has fallen.")
    -- Spawn nighttime creatures
    -- Play night music
end
```

### Dynamic Lighting

```lua
function on_update(dt)
    -- Get lighting information
    local ambient = Time.getAmbientLightIntensity()
    local skyColor = Time.getSkyColor()
    local sunColor = Time.getSunLightColor()
    
    -- Apply to renderer
    -- renderer:setAmbientLight(ambient)
    -- renderer:setSkyColor(skyColor.x, skyColor.y, skyColor.z)
    -- renderer:setSunColor(sunColor.x, sunColor.y, sunColor.z)
    
    -- Update sun direction
    local sunDir = Time.getSunDirection()
    -- renderer:setDirectionalLightDirection(sunDir.x, sunDir.y, sunDir.z)
end
```

### Time Lapse Photography

```lua
-- Accelerate time for time-lapse
Time.setTimeRate(100.0)  -- 100x speed

-- Take screenshots at intervals
local lastScreenshotTime = 0
local screenshotInterval = 1.0  -- Every 1 second

function on_update(dt)
    lastScreenshotTime = lastScreenshotTime + dt
    
    if lastScreenshotTime >= screenshotInterval then
        -- takeScreenshot("timelapse_" .. Time.getTime() .. ".png")
        lastScreenshotTime = 0
    end
end
```

## Integration with Rendering

To integrate TimeManager with your rendering system:

```cpp
// In your render loop
void Engine::render() {
    if (m_timeManager && m_renderer) {
        // Update directional light (sun)
        glm::vec3 sunDir = m_timeManager->getSunDirection();
        glm::vec3 sunColor = m_timeManager->getSunLightColor();
        m_renderer->setDirectionalLight(sunDir, sunColor);
        
        // Update ambient light
        float ambient = m_timeManager->getAmbientLightIntensity();
        m_renderer->setAmbientLight(ambient);
        
        // Update sky color (clear color or skybox)
        glm::vec3 skyColor = m_timeManager->getSkyColor();
        m_renderer->setClearColor(skyColor.x, skyColor.y, skyColor.z);
    }
}
```

## Best Practices

1. **Time Rate**
   - Use 1.0x for normal gameplay
   - Use 10x-100x for testing/demos
   - Use 0.5x for slow-motion cinematics

2. **Performance**
   - TimeManager::update() is very efficient
   - Lighting calculations are cached and only update when needed
   - Safe to query multiple times per frame

3. **Events**
   - Use callbacks for important transitions (sunrise, sunset)
   - Don't poll isDaytime() every frame if you only need events
   - Event callbacks are optional

4. **Scripting**
   - Prefer Lua API for gameplay logic
   - Use C++ API for core engine systems
   - Hot-reload scripts to iterate quickly

## Troubleshooting

### Time Not Progressing

```lua
-- Check if time is paused
if Time.isPaused() then
    Time.resume()
end

-- Check time rate
local rate = Time.getTimeRate()
if rate == 0 then
    Time.setTimeRate(1.0)
end
```

### Lighting Not Updating

Make sure your rendering system is reading TimeManager values:
- Check that TimeManager is being updated in Engine::update()
- Verify renderer is applying lighting values
- Ensure shaders support dynamic lighting

### Events Not Firing

Event callbacks are set on the C++ TimeManager instance:
- For Lua, poll isDaytime() and check for transitions
- For C++, use setOnSunrise/setOnSunset callbacks

## See Also

- `examples/scripts/day_night_demo.lua` - Interactive demo script
- `engine/gameplay/TimeManager.h` - Full C++ API reference
- `engine/gameplay/TimeManager.cpp` - Implementation details

## Future Enhancements

Potential additions to the time system:
- Seasons integration (already has SeasonManager)
- Weather integration (already has WeatherManager)
- Moon phases
- Solar/lunar eclipses
- Customizable day length
- Time zones (for multiplayer)
- Calendar system (years, months, weeks)

---

**Status:** ✅ Fully Implemented and Integrated  
**Last Updated:** 2026-01-03  
**Version:** 1.0.0
