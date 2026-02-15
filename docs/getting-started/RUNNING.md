# Running Fresh Voxel Engine

This guide explains how to properly run the Fresh Voxel Engine executable after building.

## Important: Working Directory

The Fresh Voxel Engine executable **must be run from the build directory** where the `shaders/` folder exists.

### Why?

The engine loads shader files at runtime from the `shaders/` directory relative to the current working directory. During the build process, CMake automatically copies all shader files from the source `shaders/` directory to `<build_directory>/shaders/`.

## Running the Executable

### From Command Line

**Windows:**
```batch
cd build\Release
.\FreshVoxelEngine.exe
```

Or from Debug build:
```batch
cd build\Debug
.\FreshVoxelEngine.exe
```

**Linux/macOS:**
```bash
cd build
./FreshVoxelEngine
```

### From Visual Studio (Windows)

When running from Visual Studio (F5 or Ctrl+F5), the working directory is automatically set correctly by the IDE. No additional configuration is needed.

### From Other IDEs

If using another IDE (CLion, VS Code, etc.), ensure the working directory is set to the build directory:

**CLion:**
1. Run → Edit Configurations
2. Set "Working directory" to `$ProjectFileDir$/build`

**VS Code (launch.json):**
```json
{
    "cwd": "${workspaceFolder}/build"
}
```

## Troubleshooting

### Error: "Failed to open shader file: shaders/voxel.vert"

**Cause:** The executable is being run from the wrong directory.

**Solution:**
1. Navigate to your build directory: `cd build` (Linux/macOS) or `cd build\Release` (Windows)
2. Run the executable from there
3. Verify that `shaders/` folder exists in the current directory with `ls shaders/` or `dir shaders\`

### Error: "Failed to create voxel shader program"

**Cause:** Shader files couldn't be loaded or compiled.

**Solution:**
1. First, ensure you're running from the correct directory (see above)
2. Check that shader files exist in `<build_directory>/shaders/`:
   - `voxel.vert`
   - `voxel.frag`
   - `crosshair.vert`
   - `crosshair.frag`
3. If shader files are missing, rebuild the project: `cmake --build build`

### Shader Files Missing After Build

**Cause:** The CMake configure step may have failed to copy shaders.

**Solution:**
1. Re-run CMake configuration: `cmake -B build`
2. Rebuild: `cmake --build build`
3. Verify shaders were copied: `ls build/shaders/` or `dir build\shaders\`

## Directory Structure

After a successful build, your directory structure should look like:

```
fresh/
├── build/
│   ├── shaders/           ← Shader files copied here by CMake
│   │   ├── voxel.vert
│   │   ├── voxel.frag
│   │   ├── crosshair.vert
│   │   └── crosshair.frag
│   ├── FreshVoxelEngine   ← Linux/macOS executable
│   ├── Release/           ← Windows Release build
│   │   └── FreshVoxelEngine.exe
│   └── Debug/             ← Windows Debug build
│       └── FreshVoxelEngine.exe
├── shaders/               ← Source shader files (not used at runtime)
├── engine/
└── ...
```

## Alternative: Install Target

For a production deployment, you can use the CMake install target which copies both the executable and shaders to the correct locations:

```bash
cmake --build build --target install
```

This installs to the configured install prefix (default: `/usr/local/bin` on Linux, `C:\Program Files\FreshVoxelEngine` on Windows).
