# Audio Format Support

## Overview

The Fresh Voxel Engine AudioEngine supports multiple audio formats for sound effects and background music.

## Supported Formats

### WAV (Always Available)
- **Format**: PCM (Pulse Code Modulation)
- **Channels**: Mono or Stereo
- **Bit Depth**: 8-bit or 16-bit
- **Codec**: Uncompressed
- **Use Case**: Short sound effects
- **Pros**: No decoding overhead, widely supported
- **Cons**: Large file sizes

### OGG Vorbis (Optional)
- **Format**: Ogg Vorbis
- **Channels**: Mono or Stereo
- **Bit Depth**: 16-bit (decoded to PCM)
- **Codec**: Lossy compression
- **Use Case**: Background music, ambient sounds
- **Pros**: Small file sizes (~10-20% of WAV), good quality
- **Cons**: Requires libvorbis library, CPU decoding cost

## Installation

### Windows (vcpkg)
```powershell
# Install libvorbis for OGG support
vcpkg install libvorbis:x64-windows

# Then rebuild the project
cmake --build build --config Release
```

### Verification
Check the CMake output for:
```
-- libvorbis enabled for OGG audio support
```

If you see a warning instead, OGG support is disabled but WAV will still work.

## Usage

### Playing Audio

The AudioEngine automatically detects the format by file extension:

```cpp
#include "audio/AudioEngine.h"

AudioEngine& audio = AudioEngine::getInstance();
audio.initialize();

// Play WAV sound effect (2D)
int soundID = audio.play2D("sounds/click.wav", 1.0f, false);

// Play OGG background music (2D, looping)
audio.playMusic("music/ambient.ogg", 0.8f, true);

// Play WAV sound at 3D position
glm::vec3 position(10.0f, 0.0f, 5.0f);
int sound3D = audio.play3D("sounds/explosion.wav", position, 1.0f, false);
```

### Format Auto-Detection

- `.wav` → WAV decoder
- `.ogg` → OGG decoder (if available)
- Unknown → WAV decoder (default)

Case-insensitive: `.WAV`, `.Wav`, `.wav` all work

## Performance Considerations

### WAV Files
- **Load Time**: Fast (direct file read)
- **Memory**: High (uncompressed data)
- **CPU**: None (already PCM)
- **Best For**: Short, frequently-played sounds

### OGG Files
- **Load Time**: Slower (decode entire file)
- **Memory**: Lower (decoded PCM still smaller than large WAV)
- **CPU**: Moderate (one-time decode at load)
- **Best For**: Long music tracks, ambient loops

## File Size Comparison

Example: 3-minute music track at 44.1kHz stereo:
- **WAV (16-bit)**: ~30 MB
- **OGG (Q5, 160kbps)**: ~3.5 MB
- **Savings**: ~88% smaller

## Future Formats

### Planned
- **MP3**: Widely used, but requires licensing considerations
- **FLAC**: Lossless compression, good for high-quality music

### Streaming Audio
Current implementation loads entire file into memory. For very long tracks (>5 minutes), streaming support may be added:
- Reduce memory footprint
- Faster startup (play while loading)
- Requires buffer management

## Troubleshooting

### "Failed to load audio file"
1. Check file exists at the specified path
2. Verify file extension is `.wav` or `.ogg`
3. For OGG: Ensure libvorbis is installed

### "Invalid OGG file"
- File may be corrupted
- May not be actual Ogg Vorbis format
- Try re-encoding with a tool like Audacity

### "Unsupported number of channels"
- Only mono (1) and stereo (2) are supported
- Convert files to stereo using audio editing software

### Build Warnings
```
-- libvorbis not found. OGG audio support will be disabled.
```
Solution: Install libvorbis via vcpkg (see Installation section)

## Technical Details

### WAV Loading
1. Read and validate RIFF/WAVE header
2. Verify PCM format (format code 1)
3. Read audio data into buffer
4. Upload to OpenAL buffer
5. Determine OpenAL format (MONO8, MONO16, STEREO8, STEREO16)

### OGG Loading
1. Open file and initialize vorbis with `ov_open_callbacks()`
2. Get file info with `ov_info()` (sample rate, channels)
3. Decode entire file with `ov_read()` loop into PCM buffer
4. Upload decoded PCM to OpenAL buffer
5. Clean up with `ov_clear()`

### OpenAL Buffer Management
- Each loaded audio file creates one OpenAL buffer
- Buffers are shared across multiple sources (instances)
- TODO: Add buffer caching to avoid reloading same file
- TODO: Implement buffer cleanup/garbage collection

## Code References

- **AudioEngine**: `engine/audio/AudioEngine.cpp`
- **WAV Loader**: `loadWAVFile()` function
- **OGG Loader**: `loadOGGFile()` function (when FRESH_VORBIS_AVAILABLE)
- **Universal Loader**: `loadAudioFile()` function
- **CMake Config**: `CMakeLists.txt` lines 120-127, 552-558

## Testing

### Sample Audio Files
Create test files in `sounds/` directory:

```bash
# Generate test WAV (1 second, 440Hz tone)
ffmpeg -f lavfi -i "sine=frequency=440:duration=1" -ar 44100 sounds/test.wav

# Convert to OGG
ffmpeg -i sounds/test.wav -c:a libvorbis -q:a 5 sounds/test.ogg
```

### Unit Test Example
```cpp
#include "audio/AudioEngine.h"

TEST(AudioEngineTest, LoadWAVFile) {
    AudioEngine& audio = AudioEngine::getInstance();
    ASSERT_TRUE(audio.initialize());
    
    int id = audio.play2D("sounds/test.wav", 1.0f, false);
    EXPECT_NE(id, -1);
}

TEST(AudioEngineTest, LoadOGGFile) {
    AudioEngine& audio = AudioEngine::getInstance();
    ASSERT_TRUE(audio.initialize());
    
    int id = audio.play2D("sounds/test.ogg", 1.0f, false);
    EXPECT_NE(id, -1);
}
```

## License

The Fresh Voxel Engine uses MIT License.

**Dependencies**:
- **OpenAL-Soft**: LGPL or commercial license
- **libvorbis**: BSD-style license (free for all uses)
- **Ogg**: BSD-style license (free for all uses)

All dependencies are permissively licensed and safe for commercial use.

---

**Last Updated**: 2025-11-16  
**Version**: 0.2.0  
**Status**: Complete ✅
