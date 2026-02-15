#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{

class AudioClipResource;

/**
 * @brief Audio source properties
 */
struct AudioSource {
    std::shared_ptr<AudioClipResource> clip;
    glm::vec3 position{0.0f};
    float volume = 1.0f;
    float pitch = 1.0f;
    float minDistance = 1.0f;
    float maxDistance = 100.0f;
    bool loop = false;
    bool is3D = false;
    bool isPlaying = false;
    int sourceID = -1; // Platform-specific audio source ID
};

/**
 * @brief Audio listener (typically attached to player/camera)
 */
struct AudioListener {
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    glm::vec3 forward{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
};

/**
 * @brief Audio engine for sound effects and music
 *
 * Manages 2D and 3D spatial audio, background music,
 * and sound effect playback. Designed for easy integration
 * with OpenAL or other audio libraries.
 */
class AudioEngine
{
public:
    static AudioEngine& getInstance();

    /**
     * @brief Initialize audio system
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Shutdown audio system
     */
    void shutdown();

    /**
     * @brief Update audio (call every frame for 3D audio)
     */
    void update(float deltaTime);

    // Playback control

    /**
     * @brief Play 2D sound effect
     * @param path Path to audio file
     * @param volume Volume (0.0 to 1.0)
     * @param loop Whether to loop
     * @return Sound ID for controlling playback
     */
    int play2D(const std::string& path, float volume = 1.0f, bool loop = false);

    /**
     * @brief Play 3D spatial sound
     * @param path Path to audio file
     * @param position 3D position in world
     * @param volume Volume (0.0 to 1.0)
     * @param loop Whether to loop
     * @return Sound ID for controlling playback
     */
    int play3D(const std::string& path, const glm::vec3& position, float volume = 1.0f,
               bool loop = false);

    /**
     * @brief Stop sound by ID
     */
    void stop(int soundID);

    /**
     * @brief Stop all sounds
     */
    void stopAll();

    /**
     * @brief Pause/resume sound
     */
    void pause(int soundID);
    void resume(int soundID);

    /**
     * @brief Set sound volume
     */
    void setVolume(int soundID, float volume);

    /**
     * @brief Check if sound is playing
     */
    bool isPlaying(int soundID) const;

    // Music playback

    /**
     * @brief Play background music
     */
    void playMusic(const std::string& path, float volume = 0.5f, bool loop = true);

    /**
     * @brief Stop music
     */
    void stopMusic();

    /**
     * @brief Set music volume
     */
    void setMusicVolume(float volume);

    /**
     * @brief Fade music in/out
     */
    void fadeMusicIn(float duration);
    void fadeMusicOut(float duration);

    // Listener control (for 3D audio)

    /**
     * @brief Set listener position and orientation
     */
    void setListenerPosition(const glm::vec3& position);
    void setListenerVelocity(const glm::vec3& velocity);
    void setListenerOrientation(const glm::vec3& forward, const glm::vec3& up);

    // Global settings

    /**
     * @brief Set master volume (affects all audio)
     */
    void setMasterVolume(float volume);
    float getMasterVolume() const
    {
        return masterVolume;
    }

    /**
     * @brief Set effects volume multiplier
     */
    void setEffectsVolume(float volume)
    {
        effectsVolume = volume;
    }
    float getEffectsVolume() const
    {
        return effectsVolume;
    }

    /**
     * @brief Mute/unmute all audio
     */
    void setMuted(bool muted);
    bool isMuted() const
    {
        return muted;
    }

private:
    AudioEngine() = default;
    ~AudioEngine() = default;
    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    int getNextSourceID();
    void cleanupFinishedSources();

    bool initialized = false;
    int nextSourceID = 1;

    std::map<int, AudioSource> activeSources;
    AudioSource musicSource;
    AudioListener listener;

    float masterVolume = 1.0f;
    float effectsVolume = 1.0f;
    float musicVolume = 0.5f;
    bool muted = false;

    // Music fade
    bool fadingMusic = false;
    float fadeTime = 0.0f;
    float fadeDuration = 0.0f;
    bool fadeIn = false;
};

} // namespace fresh
