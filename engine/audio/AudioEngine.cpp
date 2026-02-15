#include "audio/AudioEngine.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

#include "core/ResourceManager.h"

#ifdef FRESH_OPENAL_AVAILABLE
    #include <AL/al.h>
    #include <AL/alc.h>
#endif

#ifdef FRESH_VORBIS_AVAILABLE
    #include <vorbis/codec.h>
    #include <vorbis/vorbisfile.h>
#endif

namespace fresh
{

#ifdef FRESH_OPENAL_AVAILABLE
// OpenAL context variables
static ALCdevice* g_audioDevice = nullptr;
static ALCcontext* g_audioContext = nullptr;
#endif

AudioEngine& AudioEngine::getInstance()
{
    static AudioEngine instance;
    return instance;
}

bool AudioEngine::initialize()
{
    if (initialized)
        return true;

    std::cout << "Initializing AudioEngine..." << std::endl;

#ifdef FRESH_OPENAL_AVAILABLE
    // Open the default audio device
    g_audioDevice = alcOpenDevice(nullptr);
    if (!g_audioDevice) {
        std::cerr << "Failed to open OpenAL audio device" << std::endl;
        return false;
    }

    // Create audio context
    g_audioContext = alcCreateContext(g_audioDevice, nullptr);
    if (!g_audioContext) {
        std::cerr << "Failed to create OpenAL context" << std::endl;
        alcCloseDevice(g_audioDevice);
        g_audioDevice = nullptr;
        return false;
    }

    // Make the context current
    if (!alcMakeContextCurrent(g_audioContext)) {
        std::cerr << "Failed to make OpenAL context current" << std::endl;
        alcDestroyContext(g_audioContext);
        alcCloseDevice(g_audioDevice);
        g_audioContext = nullptr;
        g_audioDevice = nullptr;
        return false;
    }

    // Set default listener properties
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    ALfloat listenerOri[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
    alListenerfv(AL_ORIENTATION, listenerOri);

    initialized = true;
    std::cout << "AudioEngine initialized successfully with OpenAL" << std::endl;
    return true;
#else
    std::cout << "AudioEngine: OpenAL not available, running in stub mode" << std::endl;
    initialized = true;
    return true;
#endif
}

void AudioEngine::shutdown()
{
    if (!initialized)
        return;

    std::cout << "Shutting down AudioEngine..." << std::endl;

    stopAll();
    stopMusic();

#ifdef FRESH_OPENAL_AVAILABLE
    // Cleanup OpenAL resources
    if (g_audioContext) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(g_audioContext);
        g_audioContext = nullptr;
    }

    if (g_audioDevice) {
        alcCloseDevice(g_audioDevice);
        g_audioDevice = nullptr;
    }
#endif

    initialized = false;
}

void AudioEngine::update(float deltaTime)
{
    if (!initialized)
        return;

#ifdef FRESH_OPENAL_AVAILABLE
    // Update 3D audio based on listener position
    for (auto& [id, source] : activeSources) {
        if (source.is3D && source.isPlaying && source.sourceID != -1) {
            // Calculate distance-based attenuation
            float distance = glm::distance(listener.position, source.position);
            float attenuation = 1.0f;

            if (distance > source.minDistance) {
                attenuation = source.minDistance / std::min(distance, source.maxDistance);
            }

            // Apply attenuation to volume
            float effectiveVolume = source.volume * attenuation * effectsVolume * masterVolume;
            if (muted)
                effectiveVolume = 0.0f;

            // Update OpenAL source properties
            ALuint alSource = static_cast<ALuint>(source.sourceID);
            alSourcef(alSource, AL_GAIN, effectiveVolume);
            alSource3f(alSource, AL_POSITION, source.position.x, source.position.y,
                       source.position.z);

            // Check if source is still playing
            ALint state;
            alGetSourcei(alSource, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING) {
                source.isPlaying = false;
            }
        } else if (!source.is3D && source.isPlaying && source.sourceID != -1) {
            // Update 2D audio volume
            float effectiveVolume = source.volume * effectsVolume * masterVolume;
            if (muted)
                effectiveVolume = 0.0f;

            ALuint alSource = static_cast<ALuint>(source.sourceID);
            alSourcef(alSource, AL_GAIN, effectiveVolume);

            // Check if source is still playing
            ALint state;
            alGetSourcei(alSource, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING) {
                source.isPlaying = false;
            }
        }
    }
#else
    // Stub implementation for non-OpenAL builds
    for (auto& [id, source] : activeSources) {
        if (source.is3D && source.isPlaying) {
            float distance = glm::distance(listener.position, source.position);
            float attenuation = 1.0f;

            if (distance > source.minDistance) {
                attenuation = source.minDistance / std::min(distance, source.maxDistance);
            }

            float effectiveVolume = source.volume * attenuation * effectsVolume * masterVolume;
            if (muted)
                effectiveVolume = 0.0f;
            (void)effectiveVolume;
        }
    }
#endif

    // Handle music fading
    if (fadingMusic) {
        fadeTime += deltaTime;
        float t = fadeTime / fadeDuration;

        if (t >= 1.0f) {
            fadingMusic = false;
            if (!fadeIn) {
                stopMusic();
            }
        } else {
            float fadeVolume = fadeIn ? t : (1.0f - t);
            setMusicVolume(musicVolume * fadeVolume);
        }
    }

    // Cleanup finished sounds
    cleanupFinishedSources();
}

#ifdef FRESH_OPENAL_AVAILABLE
// Simple WAV file header structure
struct WAVHeader {
    char riff[4];           // "RIFF"
    uint32_t fileSize;      // File size - 8
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    uint32_t fmtSize;       // Format chunk size (16 for PCM)
    uint16_t audioFormat;   // Audio format (1 = PCM)
    uint16_t numChannels;   // Number of channels
    uint32_t sampleRate;    // Sample rate
    uint32_t byteRate;      // Byte rate
    uint16_t blockAlign;    // Block align
    uint16_t bitsPerSample; // Bits per sample
    char data[4];           // "data"
    uint32_t dataSize;      // Data size
};

// Helper function to load WAV file into OpenAL buffer
static ALuint loadWAVFile(const std::string& path)
{
    // Open the file in binary mode
    FILE* file = nullptr;
    #ifdef _WIN32
    // Use fopen_s on Windows to avoid C4996 warning
    errno_t err = fopen_s(&file, path.c_str(), "rb");
    if (err != 0 || !file) {
    #else
    file = fopen(path.c_str(), "rb");
    if (!file) {
    #endif
        std::cerr << "  Failed to open audio file: " << path << std::endl;
        return 0;
    }

    // Read WAV header
    WAVHeader header;
    size_t bytesRead = fread(&header, sizeof(WAVHeader), 1, file);
    if (bytesRead != 1) {
        std::cerr << "  Failed to read WAV header from: " << path << std::endl;
        fclose(file);
        return 0;
    }

    // Verify RIFF header
    if (header.riff[0] != 'R' || header.riff[1] != 'I' || header.riff[2] != 'F' ||
        header.riff[3] != 'F') {
        std::cerr << "  Not a valid RIFF file: " << path << std::endl;
        fclose(file);
        return 0;
    }

    // Verify WAVE format
    if (header.wave[0] != 'W' || header.wave[1] != 'A' || header.wave[2] != 'V' ||
        header.wave[3] != 'E') {
        std::cerr << "  Not a valid WAVE file: " << path << std::endl;
        fclose(file);
        return 0;
    }

    // Only support PCM format
    if (header.audioFormat != 1) {
        std::cerr << "  Only PCM format is supported (format: " << header.audioFormat << ")"
                  << std::endl;
        fclose(file);
        return 0;
    }

    // Allocate buffer for audio data
    std::vector<char> audioData(header.dataSize);
    bytesRead = fread(audioData.data(), 1, header.dataSize, file);
    fclose(file);

    if (bytesRead != header.dataSize) {
        std::cerr << "  Failed to read audio data (expected " << header.dataSize << ", got "
                  << bytesRead << ")" << std::endl;
        return 0;
    }

    // Determine OpenAL format
    ALenum format = 0;
    if (header.numChannels == 1) {
        format = (header.bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    } else if (header.numChannels == 2) {
        format = (header.bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    } else {
        std::cerr << "  Unsupported number of channels: " << header.numChannels << std::endl;
        return 0;
    }

    // Create OpenAL buffer
    ALuint buffer;
    alGenBuffers(1, &buffer);
    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "  Failed to generate OpenAL buffer" << std::endl;
        return 0;
    }

    // Upload audio data to buffer
    alBufferData(buffer, format, audioData.data(), header.dataSize, header.sampleRate);
    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "  Failed to upload audio data to buffer" << std::endl;
        alDeleteBuffers(1, &buffer);
        return 0;
    }

    std::cout << "  Loaded WAV file: " << path << " (" << header.sampleRate << " Hz, "
              << header.numChannels << " channel(s), " << header.bitsPerSample << " bits)"
              << std::endl;

    return buffer;
}

// Helper function to load OGG file into OpenAL buffer
#ifdef FRESH_VORBIS_AVAILABLE
static ALuint loadOGGFile(const std::string& path)
{
    // Open the OGG file
    OggVorbis_File oggFile;
    FILE* file = nullptr;
    
    #ifdef _WIN32
    errno_t err = fopen_s(&file, path.c_str(), "rb");
    if (err != 0 || !file) {
    #else
    file = fopen(path.c_str(), "rb");
    if (!file) {
    #endif
        std::cerr << "  Failed to open OGG file: " << path << std::endl;
        return 0;
    }
    
    // Initialize vorbis file for reading
    if (ov_open_callbacks(file, &oggFile, nullptr, 0, OV_CALLBACKS_DEFAULT) < 0) {
        std::cerr << "  Invalid OGG file: " << path << std::endl;
        fclose(file);
        return 0;
    }
    
    // Get file information
    vorbis_info* info = ov_info(&oggFile, -1);
    if (!info) {
        std::cerr << "  Failed to get OGG file info: " << path << std::endl;
        ov_clear(&oggFile);
        return 0;
    }
    
    // Determine OpenAL format
    ALenum format = 0;
    if (info->channels == 1) {
        format = AL_FORMAT_MONO16;
    } else if (info->channels == 2) {
        format = AL_FORMAT_STEREO16;
    } else {
        std::cerr << "  Unsupported number of channels in OGG: " << info->channels << std::endl;
        ov_clear(&oggFile);
        return 0;
    }
    
    // Decode entire file into memory
    std::vector<char> audioData;
    const size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    int bitstream;
    long bytesRead = 0;
    
    do {
        bytesRead = ov_read(&oggFile, buffer, BUFFER_SIZE, 0, 2, 1, &bitstream);
        if (bytesRead > 0) {
            audioData.insert(audioData.end(), buffer, buffer + bytesRead);
        }
    } while (bytesRead > 0);
    
    // Create OpenAL buffer
    ALuint alBuffer;
    alGenBuffers(1, &alBuffer);
    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "  Failed to generate OpenAL buffer for OGG" << std::endl;
        ov_clear(&oggFile);
        return 0;
    }
    
    // Upload audio data to buffer
    alBufferData(alBuffer, format, audioData.data(), static_cast<ALsizei>(audioData.size()), 
                 info->rate);
    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "  Failed to upload OGG audio data to buffer" << std::endl;
        alDeleteBuffers(1, &alBuffer);
        ov_clear(&oggFile);
        return 0;
    }
    
    std::cout << "  Loaded OGG file: " << path << " (" << info->rate << " Hz, "
              << info->channels << " channel(s))" << std::endl;
    
    // Clean up
    ov_clear(&oggFile); // This also closes the file
    
    return alBuffer;
}
#endif

#endif

// Helper function to load audio file (supports WAV and OGG)
static ALuint loadAudioFile(const std::string& path)
{
    // Determine file type by extension
    std::string ext;
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        ext = path.substr(dotPos + 1);
        // Convert to lowercase for comparison
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    }
    
    #ifdef FRESH_VORBIS_AVAILABLE
    if (ext == "ogg") {
        return loadOGGFile(path);
    }
    #endif
    
    // Default to WAV
    return loadWAVFile(path);
}

int AudioEngine::play2D(const std::string& path, float volume, bool loop)
{
    if (!initialized)
        return -1;

    std::cout << "Playing 2D audio: " << path << std::endl;

#ifdef FRESH_OPENAL_AVAILABLE
    // Load audio buffer (supports WAV and OGG formats)
    ALuint buffer = loadAudioFile(path);
    if (buffer == 0) {
        std::cerr << "  Failed to load audio file: " << path << std::endl;
        return -1;
    }

    // Generate OpenAL source
    ALuint alSource;
    alGenSources(1, &alSource);

    // Validate that source was created successfully
    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "  Failed to generate OpenAL source" << std::endl;
        return -1;
    }

    // Set source properties for 2D audio
    alSourcei(alSource, AL_BUFFER, buffer);
    alSourcef(alSource, AL_GAIN, volume * effectsVolume * masterVolume);
    alSourcef(alSource, AL_PITCH, 1.0f);
    alSourcei(alSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcei(alSource, AL_SOURCE_RELATIVE, AL_TRUE); // 2D audio
    alSource3f(alSource, AL_POSITION, 0.0f, 0.0f, 0.0f);

    // Play the source
    alSourcePlay(alSource);

    int id = getNextSourceID();
    AudioSource source;
    source.volume = volume;
    source.loop = loop;
    source.is3D = false;
    source.isPlaying = true;
    // Store ALuint as int - safe because OpenAL IDs are small positive integers
    source.sourceID = static_cast<int>(alSource);

    activeSources[id] = source;

    std::cout << "  Started 2D audio source (ID: " << id << ", AL source: " << alSource << ")"
              << std::endl;
    return id;
#else
    int id = getNextSourceID();
    AudioSource source;
    source.volume = volume;
    source.loop = loop;
    source.is3D = false;
    source.isPlaying = true;
    source.sourceID = id;

    activeSources[id] = source;
    std::cout << "  (Stub mode - OpenAL not available)" << std::endl;
    return id;
#endif
}

int AudioEngine::play3D(const std::string& path, const glm::vec3& position, float volume, bool loop)
{
    if (!initialized)
        return -1;

    std::cout << "Playing 3D audio: " << path << " at (" << position.x << ", " << position.y << ", "
              << position.z << ")" << std::endl;

#ifdef FRESH_OPENAL_AVAILABLE
    // Load audio buffer (supports WAV and OGG formats)
    ALuint buffer = loadAudioFile(path);
    if (buffer == 0) {
        std::cerr << "  Failed to load audio file: " << path << std::endl;
        return -1;
    }

    // Generate OpenAL source
    ALuint alSource;
    alGenSources(1, &alSource);

    // Validate that source was created successfully
    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "  Failed to generate OpenAL source" << std::endl;
        return -1;
    }

    // Set source properties for 3D audio
    alSourcei(alSource, AL_BUFFER, buffer);
    alSourcef(alSource, AL_GAIN, volume * effectsVolume * masterVolume);
    alSourcef(alSource, AL_PITCH, 1.0f);
    alSourcei(alSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcei(alSource, AL_SOURCE_RELATIVE, AL_FALSE); // 3D audio
    alSource3f(alSource, AL_POSITION, position.x, position.y, position.z);
    alSourcef(alSource, AL_REFERENCE_DISTANCE, 1.0f);
    alSourcef(alSource, AL_MAX_DISTANCE, 100.0f);
    alSourcef(alSource, AL_ROLLOFF_FACTOR, 1.0f);

    // Play the source
    alSourcePlay(alSource);

    int id = getNextSourceID();
    AudioSource source;
    source.position = position;
    source.volume = volume;
    source.loop = loop;
    source.is3D = true;
    source.isPlaying = true;
    // Store ALuint as int - safe because OpenAL IDs are small positive integers
    source.sourceID = static_cast<int>(alSource);

    activeSources[id] = source;

    std::cout << "  Started 3D audio source (ID: " << id << ", AL source: " << alSource << ")"
              << std::endl;
    return id;
#else
    int id = getNextSourceID();
    AudioSource source;
    source.position = position;
    source.volume = volume;
    source.loop = loop;
    source.is3D = true;
    source.isPlaying = true;
    source.sourceID = id;

    activeSources[id] = source;
    std::cout << "  (Stub mode - OpenAL not available)" << std::endl;
    return id;
#endif
}

void AudioEngine::stop(int soundID)
{
    auto it = activeSources.find(soundID);
    if (it != activeSources.end()) {
#ifdef FRESH_OPENAL_AVAILABLE
        if (it->second.sourceID != -1) {
            ALuint alSource = static_cast<ALuint>(it->second.sourceID);
            alSourceStop(alSource);
            alDeleteSources(1, &alSource);
        }
#endif
        it->second.isPlaying = false;
        activeSources.erase(it);
    }
}

void AudioEngine::stopAll()
{
#ifdef FRESH_OPENAL_AVAILABLE
    for (auto& [id, source] : activeSources) {
        if (source.sourceID != -1) {
            ALuint alSource = static_cast<ALuint>(source.sourceID);
            alSourceStop(alSource);
            alDeleteSources(1, &alSource);
        }
        source.isPlaying = false;
    }
#else
    for (auto& [id, source] : activeSources) {
        source.isPlaying = false;
    }
#endif
    activeSources.clear();
}

void AudioEngine::pause(int soundID)
{
    auto it = activeSources.find(soundID);
    if (it != activeSources.end()) {
#ifdef FRESH_OPENAL_AVAILABLE
        if (it->second.sourceID != -1) {
            ALuint alSource = static_cast<ALuint>(it->second.sourceID);
            alSourcePause(alSource);
        }
#endif
        it->second.isPlaying = false;
    }
}

void AudioEngine::resume(int soundID)
{
    auto it = activeSources.find(soundID);
    if (it != activeSources.end()) {
#ifdef FRESH_OPENAL_AVAILABLE
        if (it->second.sourceID != -1) {
            ALuint alSource = static_cast<ALuint>(it->second.sourceID);
            alSourcePlay(alSource);
        }
#endif
        it->second.isPlaying = true;
    }
}

void AudioEngine::setVolume(int soundID, float volume)
{
    auto it = activeSources.find(soundID);
    if (it != activeSources.end()) {
        it->second.volume = volume;
#ifdef FRESH_OPENAL_AVAILABLE
        if (it->second.sourceID != -1) {
            ALuint alSource = static_cast<ALuint>(it->second.sourceID);
            float effectiveVolume = volume * effectsVolume * masterVolume;
            if (muted)
                effectiveVolume = 0.0f;
            alSourcef(alSource, AL_GAIN, effectiveVolume);
        }
#endif
    }
}

bool AudioEngine::isPlaying(int soundID) const
{
    auto it = activeSources.find(soundID);
    return (it != activeSources.end()) && it->second.isPlaying;
}

void AudioEngine::playMusic(const std::string& path, float volume, bool loop)
{
    stopMusic();

    std::cout << "Playing music: " << path << std::endl;

#ifdef FRESH_OPENAL_AVAILABLE
    // Load audio buffer for music (supports WAV and OGG formats)
    ALuint buffer = loadAudioFile(path);
    if (buffer == 0) {
        std::cerr << "  Failed to load music file: " << path << std::endl;
        return;
    }

    // Generate OpenAL source for music
    ALuint alSource;
    alGenSources(1, &alSource);

    // Validate that source was created successfully
    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "  Failed to generate OpenAL source for music" << std::endl;
        return;
    }

    // Set source properties
    alSourcei(alSource, AL_BUFFER, buffer);
    alSourcef(alSource, AL_GAIN, volume * musicVolume * masterVolume);
    alSourcef(alSource, AL_PITCH, 1.0f);
    alSourcei(alSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcei(alSource, AL_SOURCE_RELATIVE, AL_TRUE); // Music is 2D
    alSource3f(alSource, AL_POSITION, 0.0f, 0.0f, 0.0f);

    // Play the music
    alSourcePlay(alSource);

    musicSource.volume = volume;
    musicSource.loop = loop;
    musicSource.isPlaying = true;
    // Store ALuint as int - safe because OpenAL IDs are small positive integers
    musicSource.sourceID = static_cast<int>(alSource);
    musicVolume = volume;

    std::cout << "  Started music source (AL source: " << alSource << ")" << std::endl;
#else
    musicSource.volume = volume;
    musicSource.loop = loop;
    musicSource.isPlaying = true;
    musicVolume = volume;
    std::cout << "  (Stub mode - OpenAL not available)" << std::endl;
#endif
}

void AudioEngine::stopMusic()
{
    if (musicSource.isPlaying) {
#ifdef FRESH_OPENAL_AVAILABLE
        if (musicSource.sourceID != -1) {
            ALuint alSource = static_cast<ALuint>(musicSource.sourceID);
            alSourceStop(alSource);
            alDeleteSources(1, &alSource);
        }
#endif
        musicSource.isPlaying = false;
        musicSource.sourceID = -1;
    }
}

void AudioEngine::setMusicVolume(float volume)
{
    musicVolume = volume;
#ifdef FRESH_OPENAL_AVAILABLE
    if (musicSource.isPlaying && musicSource.sourceID != -1) {
        ALuint alSource = static_cast<ALuint>(musicSource.sourceID);
        float effectiveVolume = volume * masterVolume;
        if (muted)
            effectiveVolume = 0.0f;
        alSourcef(alSource, AL_GAIN, effectiveVolume);
    }
#endif
}

void AudioEngine::fadeMusicIn(float duration)
{
    fadingMusic = true;
    fadeIn = true;
    fadeTime = 0.0f;
    fadeDuration = duration;
}

void AudioEngine::fadeMusicOut(float duration)
{
    fadingMusic = true;
    fadeIn = false;
    fadeTime = 0.0f;
    fadeDuration = duration;
}

void AudioEngine::setListenerPosition(const glm::vec3& position)
{
    listener.position = position;
#ifdef FRESH_OPENAL_AVAILABLE
    alListener3f(AL_POSITION, position.x, position.y, position.z);
#endif
}

void AudioEngine::setListenerVelocity(const glm::vec3& velocity)
{
    listener.velocity = velocity;
#ifdef FRESH_OPENAL_AVAILABLE
    alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
#endif
}

void AudioEngine::setListenerOrientation(const glm::vec3& forward, const glm::vec3& up)
{
    listener.forward = forward;
    listener.up = up;
#ifdef FRESH_OPENAL_AVAILABLE
    ALfloat orientation[] = {forward.x, forward.y, forward.z, up.x, up.y, up.z};
    alListenerfv(AL_ORIENTATION, orientation);
#endif
}

void AudioEngine::setMasterVolume(float volume)
{
    masterVolume = std::clamp(volume, 0.0f, 1.0f);
#ifdef FRESH_OPENAL_AVAILABLE
    // Update all active sources
    for (auto& [id, source] : activeSources) {
        if (source.sourceID != -1) {
            ALuint alSource = static_cast<ALuint>(source.sourceID);
            float effectiveVolume = source.volume * effectsVolume * masterVolume;
            if (muted)
                effectiveVolume = 0.0f;
            alSourcef(alSource, AL_GAIN, effectiveVolume);
        }
    }
    // Update music
    if (musicSource.isPlaying && musicSource.sourceID != -1) {
        ALuint alSource = static_cast<ALuint>(musicSource.sourceID);
        float effectiveVolume = musicVolume * masterVolume;
        if (muted)
            effectiveVolume = 0.0f;
        alSourcef(alSource, AL_GAIN, effectiveVolume);
    }
#endif
}

void AudioEngine::setMuted(bool isMuted)
{
    muted = isMuted;
#ifdef FRESH_OPENAL_AVAILABLE
    float volumeMultiplier = muted ? 0.0f : 1.0f;
    // Update all active sources
    for (auto& [id, source] : activeSources) {
        if (source.sourceID != -1) {
            ALuint alSource = static_cast<ALuint>(source.sourceID);
            float effectiveVolume = source.volume * effectsVolume * masterVolume * volumeMultiplier;
            alSourcef(alSource, AL_GAIN, effectiveVolume);
        }
    }
    // Update music
    if (musicSource.isPlaying && musicSource.sourceID != -1) {
        ALuint alSource = static_cast<ALuint>(musicSource.sourceID);
        float effectiveVolume = musicVolume * masterVolume * volumeMultiplier;
        alSourcef(alSource, AL_GAIN, effectiveVolume);
    }
#endif
}

int AudioEngine::getNextSourceID()
{
    return nextSourceID++;
}

void AudioEngine::cleanupFinishedSources()
{
    // Remove sources that have finished playing (non-looping)
    auto it = activeSources.begin();
    while (it != activeSources.end()) {
        if (!it->second.loop && !it->second.isPlaying) {
#ifdef FRESH_OPENAL_AVAILABLE
            if (it->second.sourceID != -1) {
                ALuint alSource = static_cast<ALuint>(it->second.sourceID);
                alDeleteSources(1, &alSource);
            }
#endif
            it = activeSources.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace fresh
