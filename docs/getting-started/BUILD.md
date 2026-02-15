# Building Fresh Voxel Engine

This guide provides clear, step-by-step instructions for building Fresh Voxel Engine from source on Windows 10/11.

**Fresh Voxel Engine is now Windows-exclusive** for optimal gaming performance and professional game development with our Unreal Engine-like editor.

---

## Windows Build (Visual Studio 2022)

### 🚀 Quick Start: Automated Build

**For the fastest and easiest setup, use the automated PowerShell script:**

```powershell
# Clone repository
git clone https://github.com/shifty81/fresh.git
cd fresh

# Run automated setup and build
.\setup-and-build.ps1
```

This single command handles everything: checking prerequisites, setting up vcpkg, generating the solution, building, and opening Visual Studio.

**See [AUTOMATED_BUILD.md](AUTOMATED_BUILD.md) for complete documentation of the automated script.**

---

## Manual Build Instructions

The rest of this guide covers the manual build process. Use this if you prefer manual control or need to troubleshoot specific steps.

## Prerequisites

Before you begin, ensure you have the following installed:

### 1. Visual Studio 2022
- **Download:** https://visualstudio.microsoft.com/downloads/
- **Edition:** Community (free), Professional, or Enterprise
- **Required Workloads:**
  - "Desktop development with C++"
  - ".NET desktop development" (for optional C# bindings)
- **Note:** This installation includes the Windows SDK needed for DirectX 11/12

### 2. CMake
- **Version:** 3.20 or higher (tested with 3.20-3.31)
- **⚠️ Important:** Use stable releases only (e.g., 3.31.7)
- **❌ Avoid:** CMake 4.x or pre-release versions (rc, alpha, beta)
- **Download:** https://cmake.org/download/
- **Installation:** Select "Add CMake to system PATH" during installation
- **Verify:** Open Command Prompt and run `cmake --version`
  - Should show: `cmake version 3.X.Y` (where X is 20-31)
  
**Troubleshooting CMake Version Issues:**
- If you have CMake 4.x or a pre-release version (e.g., 4.2.0-rc2), uninstall it and install CMake 3.31.x
- CMake 4.x has breaking changes and may not work correctly with this project
- See [CMAKE_TROUBLESHOOTING.md](CMAKE_TROUBLESHOOTING.md) for detailed guidance

### 3. Git for Windows
- **Download:** https://git-scm.com/download/win
- **Required for:** Cloning the repository and vcpkg dependency management

### 4. .NET 9 SDK (Optional)
- **Download:** https://dotnet.microsoft.com/download/dotnet/9.0
- **Purpose:** Only needed if you want C# bindings/scripting support
- **Verify:** Run `dotnet --version` (should show 9.0.x)

## Step-by-Step Build Instructions

### Step 1: Clone the Repository

Open Command Prompt or PowerShell and run:

```batch
git clone https://github.com/shifty81/fresh.git
cd fresh
```

### Step 2: Set Up vcpkg for Dependencies

Fresh Engine uses vcpkg to manage C++ dependencies (GLM, ImGui, OpenAL). Windows builds use native Win32 APIs for windowing and input (GLFW is only used for Linux CI testing). We recommend installing vcpkg in the project root directory for simplicity.

**Option A: Install vcpkg in Project Directory (Recommended)**

```batch
# From project root (fresh directory)
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
cd ..
```

**Expected result:** vcpkg is now installed at `.\vcpkg\vcpkg.exe`

**Option B: Install vcpkg in Parent Directory (Alternative)**

If you prefer to share vcpkg across multiple projects:

```batch
# Navigate to parent directory
cd ..

# Clone vcpkg
git clone https://github.com/microsoft/vcpkg.git

# Bootstrap vcpkg
cd vcpkg
bootstrap-vcpkg.bat

# Return to project directory
cd fresh
```

**Expected result:** vcpkg is now installed at `..\vcpkg\vcpkg.exe`

### Step 3: Generate Visual Studio Solution

Now we'll use CMake to generate the Visual Studio 2022 solution. The vcpkg toolchain will automatically install all required dependencies.

**Using the automated script (Easiest):**

```batch
# Run the generation script
generate_vs2022.bat
```

The script will automatically detect vcpkg in either the project or parent directory.

**Manual CMake configuration:**

**If you installed vcpkg in project directory:**

```batch
# Create build directory
mkdir build
cd build

# Generate Visual Studio 2022 solution with vcpkg
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake ..
```

**If you installed vcpkg in parent directory:**

```batch
# Create build directory
mkdir build
cd build

# Generate Visual Studio 2022 solution with vcpkg
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=..\..\vcpkg\scripts\buildsystems\vcpkg.cmake ..
```

**What happens during this step:**
- CMake configures the project for Visual Studio 2022
- vcpkg automatically downloads and builds dependencies (GLM, ImGui, GLEW, OpenAL, etc.)
- This may take 5-15 minutes on first run (dependencies are cached for future use)
- A Visual Studio solution file is created: `build\FreshVoxelEngine.sln`

**Troubleshooting:**
- If you get "vcpkg not found", verify the path matches your installation location
- For project directory: `..\vcpkg\scripts\buildsystems\vcpkg.cmake`
- For parent directory: `..\..\vcpkg\scripts\buildsystems\vcpkg.cmake`
- If CMake fails, check that all prerequisites are installed and in your PATH

### Step 4: Build the Engine

You have two options to build:

#### Option A: Build from Command Line (Recommended for first build)

```batch
# Build Release configuration
cmake --build . --config Release

# Or build Debug configuration
cmake --build . --config Debug
```

**Build time:** 2-5 minutes depending on your system

#### Option B: Build from Visual Studio

1. Open `build\FreshVoxelEngine.sln` in Visual Studio 2022
2. In Solution Explorer, right-click the **FreshVoxelEngine** project
3. Select **"Set as Startup Project"** (the project name will become bold)
4. Select configuration from the toolbar:
   - **Debug** - For development and debugging
   - **Release** - For optimized builds
5. Press **F7** or select **Build > Build Solution**

### Step 5: Run the Engine

After a successful build:

#### From Command Line:
```batch
# Run Release build
build\Release\FreshVoxelEngine.exe

# Run Debug build
build\Debug\FreshVoxelEngine.exe
```

#### From Visual Studio:
1. Ensure **FreshVoxelEngine** is set as the startup project (bold in Solution Explorer)
2. Press **F5** (run with debugging) or **Ctrl+F5** (run without debugging)

## Optional: Build .NET 9 Bindings

If you want C# scripting support, build the managed wrapper:

```batch
# From repository root
cd dotnet
dotnet build -c Release
cd ..
```

The managed assembly will be located at: `dotnet\bin\Release\net9.0-windows\FreshEngine.Managed.dll`

## Quick Reference

### Common CMake Commands

```batch
# Regenerate solution (from build directory)
cmake ..

# Clean and rebuild
cmake --build . --config Release --clean-first

# Build with multiple processors (faster)
cmake --build . --config Release -- /m

# Install to specific location
cmake --install . --prefix C:\FreshEngine
```

### Visual Studio Keyboard Shortcuts

- **F5** - Build and run with debugging
- **Ctrl+F5** - Build and run without debugging
- **F7** - Build solution
- **Ctrl+Shift+B** - Build solution (alternative)
- **Ctrl+Alt+F7** - Rebuild solution
- **F9** - Toggle breakpoint
- **F10** - Step over
- **F11** - Step into

## Project Structure

```
fresh/
├── build/                          # Build output (created by CMake)
│   ├── FreshVoxelEngine.sln       # Visual Studio solution
│   ├── Debug/                      # Debug binaries
│   └── Release/                    # Release binaries
├── CMakeLists.txt                  # Build configuration
├── vcpkg.json                      # Dependency manifest
├── engine/                        # Source and header files
│   ├── core/                      # Engine core
│   ├── renderer/                  # Rendering system (DirectX 11/12)
│   ├── voxel/                     # Voxel engine
│   └── ...
├── shaders/                       # HLSL/GLSL shaders
└── dotnet/                        # .NET 9 bindings (optional)
```

## Build Configurations

### Debug
- **Optimization:** Disabled (`/Od`)
- **Debug Info:** Full (`/Zi`)
- **Runtime Checks:** Enabled
- **Use for:** Development, debugging, testing
- **Output:** `build\Debug\FreshVoxelEngine.exe`

### Release
- **Optimization:** Maximum speed (`/O2`)
- **Debug Info:** None or minimal
- **Whole Program Optimization:** Enabled (`/GL`, `/LTCG`)
- **Use for:** Performance testing, distribution
- **Output:** `build\Release\FreshVoxelEngine.exe`

## Graphics API Support

Fresh Engine uses DirectX as the primary graphics API for Windows gaming:

- **DirectX 12** - Primary renderer (Windows 10/11)
- **DirectX 11** - Compatibility fallback (Windows 7+)
- **OpenGL 4.5+** - Optional compatibility backend

No configuration required - the engine automatically selects DirectX 12 on Windows 10/11, with automatic fallback to DirectX 11 if needed.

## Troubleshooting

### "CMake not found" or "cmake is not recognized"
**Solution:** 
- Reinstall CMake and ensure "Add to PATH" is selected
- Or manually add CMake to PATH: `C:\Program Files\CMake\bin`
- Restart your command prompt

### "Visual Studio 2022 not found"
**Solution:**
- Ensure Visual Studio 2022 is installed (Community, Professional, or Enterprise)
- Install the "Desktop development with C++" workload
- Restart your command prompt/terminal

### "vcpkg.cmake not found"
**Solution:**
- Verify vcpkg is cloned in the parent directory: `..\vcpkg\`
- Verify the bootstrap script has been run: `..\vcpkg\vcpkg.exe` should exist
- Check the CMake command uses the correct path: `..\..\vcpkg\scripts\buildsystems\vcpkg.cmake`

### Build fails with dependency errors
**Solution:**
- This means vcpkg didn't run properly during CMake generation
- Delete the `build` directory
- Re-run Step 3 with the correct vcpkg toolchain path
- vcpkg will automatically download and build all dependencies
- Note: On Windows, native Win32 APIs are used for windowing (GLFW is not required)

### "Cannot open include file: 'd3d11.h'" or DirectX errors
**Solution:**
- Install Windows SDK via Visual Studio Installer
- Ensure "Desktop development with C++" workload is installed
- This includes the Windows 10/11 SDK with DirectX headers

### "Cannot open include file: 'lua.h'" or Lua-related errors
**Solution:**
- **Lua is optional** - The engine can build and run without it
- If you see warnings about "Lua not found" during CMake configuration, this is expected and safe to ignore
- Asset management Lua configuration features will use stub implementations when Lua is not installed
- **To enable Lua support:**
  - Windows: `vcpkg install luajit:x64-windows` or `vcpkg install lua:x64-windows`
  - Linux: `sudo apt-get install liblua5.4-dev`
  - macOS: `brew install lua`
- After installing Lua, reconfigure with CMake to enable Lua features

### Build succeeds but "ALL_BUILD cannot be started" when pressing F5
**Solution:**
- In Visual Studio Solution Explorer, right-click **FreshVoxelEngine** project
- Select **"Set as Startup Project"**
- The project name should appear in bold
- Now press F5

### Runtime error: "DLL not found" or missing dependencies
**Solution:**
- vcpkg automatically copies required DLLs to the output directory
- If DLLs are missing, rebuild with: `cmake --build . --config Release --clean-first`
- Ensure you're running the executable from the correct directory (`build\Release\` or `build\Debug\`)

## Advanced Options

### Building with Tests

```batch
# Configure with tests enabled
cmake -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTS=ON -DCMAKE_TOOLCHAIN_FILE=..\..\vcpkg\scripts\buildsystems\vcpkg.cmake ..

# Build and run tests
cmake --build . --config Release
ctest -C Release
```

### Custom Install Location

```batch
# Configure with custom install prefix
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX=C:\FreshEngine -DCMAKE_TOOLCHAIN_FILE=..\..\vcpkg\scripts\buildsystems\vcpkg.cmake ..

# Build and install
cmake --build . --config Release
cmake --install . --config Release
```

### Using vcpkg in Project Directory (Alternative)

If you prefer to keep vcpkg within the project:

```batch
# Clone vcpkg into project directory
cd fresh
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
cd ..

# Generate solution (note the different path)
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake ..
```

## Getting Help

- **Documentation:** See the `docs/` directory
- **Issues:** https://github.com/shifty81/fresh/issues
- **Contributing:** See [CONTRIBUTING.md](CONTRIBUTING.md)

## Next Steps

After successfully building:

1. **Explore the codebase** - See [ARCHITECTURE.md](ARCHITECTURE.md)
2. **Try the examples** - Check `examples/` directory  
3. **Read the documentation** - See [DOCUMENTATION_INDEX.md](DOCUMENTATION_INDEX.md)
4. **Start developing** - See [DEVELOPER_SETUP.md](DEVELOPER_SETUP.md) for more details

## Summary

**Quick Build from Scratch (Parent Directory vcpkg):**
```batch
# 1. Clone repository
git clone https://github.com/shifty81/fresh.git
cd fresh

# 2. Set up vcpkg (one-time setup) - Parent directory
cd ..
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
cd ..\fresh

# 3. Generate and build
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=..\..\vcpkg\scripts\buildsystems\vcpkg.cmake ..
cmake --build . --config Release

# 4. Run
Release\FreshVoxelEngine.exe
```

**Alternative: Using generate_vs2022.bat (Recommended)**
```batch
# After setting up vcpkg (either in parent or project directory)
generate_vs2022.bat
cd build
cmake --build . --config Release
Release\FreshVoxelEngine.exe
```

The `generate_vs2022.bat` script automatically detects vcpkg in either the parent or project directory.

That's it! You now have a fully built Fresh Voxel Engine ready for development.

---

## Additional Resources

- **Automated Build**: See [AUTOMATED_BUILD.md](AUTOMATED_BUILD.md)
- **Getting Started Guide**: See [GETTING_STARTED.md](GETTING_STARTED.md)
- **Architecture Overview**: See [ARCHITECTURE.md](ARCHITECTURE.md)
- **Development Guidelines**: See [CONTRIBUTING.md](CONTRIBUTING.md)
- **Windows Integration**: See [WINDOWS_INTEGRATION.md](WINDOWS_INTEGRATION.md)
- **NPC Editor Guide**: Coming soon - Easy NPC creation and behavior configuration

---

## Archived Cross-Platform Support

**Note:** Fresh Voxel Engine is now Windows-exclusive for optimal gaming performance and professional game development.

Previously supported platforms (Linux and macOS) have been archived. If you need cross-platform support, see the archived documentation and build scripts in the `archived/cross-platform/` directory.

---

**Questions or Issues?**
- Open an issue: https://github.com/shifty81/fresh/issues
- Join discussions: https://github.com/shifty81/fresh/discussions
