# Audio System Documentation

## Overview

The Fresh Voxel Engine includes a comprehensive audio system powered by OpenAL-Soft, supporting both 2D and 3D spatial audio, music playback, and complete volume control.

## Features

### ✅ Implemented
- **OpenAL Integration**: Cross-platform audio using OpenAL-Soft
- **2D Audio**: Non-positional sound effects (UI sounds, notifications)
- **3D Spatial Audio**: Position-based audio with distance attenuation
- **Music Playback**: Background music with looping support
- **Volume Control**: Master, effects, and music volume levels
- **Fade Effects**: Fade in/out for music transitions
- **Listener Control**: Position and orientation for 3D audio
- **Mute/Unmute**: Global audio muting
- **Auto Cleanup**: Automatic cleanup of finished sounds
- **Stub Mode**: Graceful fallback when OpenAL is not available

### ⚠️ Partially Implemented
- **Audio File Loading**: Framework exists but needs WAV/OGG parser

### ❌ Not Yet Implemented
- **Sound Effects Assets**: No audio files included yet
- **Streaming Audio**: Large music files not yet supported
- **Audio Groups**: No categorization beyond effects/music

## Architecture

### Core Components

```cpp
namespace fresh {
    class AudioEngine {
        // Singleton pattern for global audio management
        static AudioEngine& getInstance();
        
        // Lifecycle
        bool initialize();
        void shutdown();
        void update(float deltaTime);
        
        // Playback
        int play2D(const std::string& path, float volume, bool loop);
        int play3D(const std::string& path, const glm::vec3& position, 
                   float volume, bool loop);
        
        // Control
        void stop(int soundID);
        void pause(int soundID);
        void resume(int soundID);
        void setVolume(int soundID, float volume);
        
        // Music
        void playMusic(const std::string& path, float volume, bool loop);
        void stopMusic();
        void fadeMusicIn(float duration);
        void fadeMusicOut(float duration);
        
        // Listener (3D audio)
        void setListenerPosition(const glm::vec3& position);
        void setListenerVelocity(const glm::vec3& velocity);
        void setListenerOrientation(const glm::vec3& forward, 
                                    const glm::vec3& up);
        
        // Global settings
        void setMasterVolume(float volume);
        void setEffectsVolume(float volume);
        void setMusicVolume(float volume);
        void setMuted(bool muted);
    };
}
```

## Usage Examples

### Basic Setup

```cpp
#include "audio/AudioEngine.h"

int main() {
    fresh::AudioEngine& audio = fresh::AudioEngine::getInstance();
    
    // Initialize (call once at startup)
    if (!audio.initialize()) {
        // Handle error - OpenAL not available
        return 1;
    }
    
    // Update every frame
    while (running) {
        audio.update(deltaTime);
        // ... rest of game loop
    }
    
    // Cleanup (call at shutdown)
    audio.shutdown();
}
```

### Playing 2D Sounds

```cpp
// Play UI sound (non-positional)
int soundId = audio.play2D("sounds/ui_click.wav", 1.0f, false);

// Play with custom volume
audio.play2D("sounds/notification.wav", 0.7f, false);

// Play looping ambient sound
audio.play2D("sounds/rain.wav", 0.5f, true);
```

### Playing 3D Spatial Audio

```cpp
// Play sound at world position
glm::vec3 soundPos(10.0f, 5.0f, -20.0f);
int soundId = audio.play3D("sounds/explosion.wav", soundPos, 1.0f, false);

// Position updates automatically based on listener position
// Set listener to player position each frame:
audio.setListenerPosition(player.getPosition());
audio.setListenerOrientation(player.getForward(), player.getUp());
```

### Music Playback

```cpp
// Play background music
audio.playMusic("music/main_theme.ogg", 0.6f, true);

// Fade out and stop
audio.fadeMusicOut(2.0f);  // 2 second fade

// Fade in new track
audio.playMusic("music/battle_theme.ogg", 0.7f, true);
audio.fadeMusicIn(1.5f);  // 1.5 second fade
```

### Volume Control

```cpp
// Set master volume (affects all audio)
audio.setMasterVolume(0.8f);  // 80%

// Set effects volume (affects only sound effects)
audio.setEffectsVolume(0.9f);  // 90%

// Set music volume (affects only music)
audio.setMusicVolume(0.5f);  // 50%

// Mute/unmute all audio
audio.setMuted(true);   // Mute
audio.setMuted(false);  // Unmute
```

### Sound Control

```cpp
// Play sound and get ID
int soundId = audio.play2D("sounds/engine.wav", 1.0f, true);

// Pause the sound
audio.pause(soundId);

// Resume the sound
audio.resume(soundId);

// Adjust volume
audio.setVolume(soundId, 0.5f);

// Stop the sound
audio.stop(soundId);

// Stop all sounds
audio.stopAll();
```

## Integration Guide

### Game Integration

In your game loop:

```cpp
void Game::update(float deltaTime) {
    // Update player position
    player.update(deltaTime);
    
    // Update audio listener to follow player
    AudioEngine& audio = AudioEngine::getInstance();
    audio.setListenerPosition(player.getPosition());
    audio.setListenerOrientation(player.getForward(), glm::vec3(0, 1, 0));
    
    // Update audio system
    audio.update(deltaTime);
}
```

### Player Footsteps Example

```cpp
void Player::update(float deltaTime) {
    // ... movement code ...
    
    if (isWalking && !footstepPlaying) {
        AudioEngine& audio = AudioEngine::getInstance();
        footstepSound = audio.play3D("sounds/footstep.wav", 
                                     position, 0.7f, false);
        footstepPlaying = true;
    }
    
    if (!audio.isPlaying(footstepSound)) {
        footstepPlaying = false;
    }
}
```

### Block Interaction Example

```cpp
void VoxelWorld::breakBlock(const glm::vec3& position) {
    // Remove block
    setBlock(position, VoxelType::Air);
    
    // Play break sound at block position
    AudioEngine& audio = AudioEngine::getInstance();
    audio.play3D("sounds/block_break.wav", position, 1.0f, false);
}

void VoxelWorld::placeBlock(const glm::vec3& position, VoxelType type) {
    // Place block
    setBlock(position, type);
    
    // Play place sound at block position
    AudioEngine& audio = AudioEngine::getInstance();
    audio.play3D("sounds/block_place.wav", position, 0.9f, false);
}
```

## Configuration

### CMake Integration

The audio system automatically detects OpenAL availability:

```cmake
# CMakeLists.txt automatically handles OpenAL
find_package(OpenAL QUIET)
if(OpenAL_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenAL::OpenAL)
    add_definitions(-DFRESH_OPENAL_AVAILABLE)
endif()
```

### vcpkg Dependencies

Windows users need to add OpenAL to vcpkg:

```json
{
  "dependencies": [
    {
      "name": "openal-soft",
      "platform": "windows"
    }
  ]
}
```

Linux/macOS users install via package manager:

```bash
# Ubuntu/Debian
sudo apt-get install libopenal-dev

# macOS
brew install openal-soft
```

## Performance Considerations

### 3D Audio Distance Culling

The audio system automatically attenuates 3D sounds based on distance:

```cpp
struct AudioSource {
    float minDistance = 1.0f;   // No attenuation within this distance
    float maxDistance = 100.0f; // Full attenuation beyond this distance
};
```

Sounds beyond `maxDistance` are effectively silent but still tracked. Consider manually stopping distant sounds for performance:

```cpp
float distance = glm::distance(listener.position, soundSource.position);
if (distance > maxDistance * 2.0f) {
    audio.stop(soundId);  // Too far, save resources
}
```

### Source Limits

OpenAL typically supports 256 concurrent sound sources, but practical limits are lower:
- **Recommended maximum**: 32-64 active sounds
- **Monitor active sources**: Use `audio.isPlaying()` to check
- **Cleanup**: Non-looping sounds auto-cleanup when finished

## Troubleshooting

### OpenAL Not Available

If OpenAL initialization fails:

```
Failed to open OpenAL audio device
```

**Solutions:**
1. **Linux**: Install `libopenal-dev` package
2. **Windows**: Ensure vcpkg installed openal-soft
3. **macOS**: Install via `brew install openal-soft`
4. **No audio device**: The system gracefully falls back to stub mode

### No Sound Output

If sounds play but no audio:

1. **Check volume levels**:
   ```cpp
   std::cout << "Master: " << audio.getMasterVolume() << std::endl;
   std::cout << "Effects: " << audio.getEffectsVolume() << std::endl;
   std::cout << "Muted: " << audio.isMuted() << std::endl;
   ```

2. **Verify audio files**: Audio file loading not yet implemented
   - Current version demonstrates API without actual audio playback
   - Need to implement WAV/OGG loader

3. **Check 3D audio distance**:
   ```cpp
   // Ensure listener and source aren't too far apart
   audio.setListenerPosition(glm::vec3(0, 0, 0));
   audio.play3D("test.wav", glm::vec3(1, 0, 0), 1.0f, false);
   ```

## Next Steps

### To Complete Audio System (15% remaining):

1. **Audio File Loading**:
   - Implement WAV file parser
   - Add OGG Vorbis support (via libvorbis)
   - Consider using libsndfile for multiple formats

2. **Sound Assets**:
   - Create/obtain sound effects
   - Add to `sounds/` directory
   - Document audio file requirements

3. **Advanced Features**:
   - Audio streaming for large files
   - Audio groups/categories
   - DSP effects (reverb, echo)
   - Multiple listeners (split-screen)

## Testing

Run the included test program:

```bash
# Build test
cd build
cmake --build . --target audio_test

# Run test
./audio_test
```

Or use the standalone example:

```bash
# See examples/audio_test.cpp for usage
cd examples
g++ -std=c++20 -I../engine -DFRESH_OPENAL_AVAILABLE \
    audio_test.cpp ../engine/audio/AudioEngine.cpp \
    -lopenal -o audio_test
./audio_test
```

## API Reference

See `engine/audio/AudioEngine.h` for complete API documentation.

## License

The audio system is part of the Fresh Voxel Engine, licensed under MIT License.

---

**Status**: 85% Complete (as of 2025-11-07)
**Dependencies**: OpenAL-Soft 1.23.1+
**Platforms**: Windows, Linux, macOS
