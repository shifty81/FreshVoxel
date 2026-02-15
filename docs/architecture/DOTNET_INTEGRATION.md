# .NET 9 Integration Guide

This guide explains how to use .NET 9 with the Fresh Voxel Engine, which is the **primary and recommended development method** for the project.

## Overview

The Fresh Voxel Engine uses .NET 9 as its primary development platform, allowing you to:

- Write game logic in C# while leveraging native C++ performance (Recommended approach)
- Use modern .NET features and libraries
- Seamlessly interop between managed and native code
- Deploy on Windows with full DirectX support
- Benefit from memory safety and easier debugging

## Why .NET 9 First?

### Primary Benefits

1. **Modern Language Features** - Use C# 12 with latest language improvements
2. **Safer Development** - Memory safety and automatic garbage collection
3. **Faster Iteration** - Hot reload and rapid prototyping
4. **Rich Ecosystem** - Access thousands of NuGet packages
5. **Better Tooling** - Superior IDE support and debugging experience
6. **Easier Learning Curve** - More approachable for new developers

### When to Use Native C++

- When you need absolute maximum performance
- When working on low-level engine internals
- When implementing new graphics features
- When porting to non-Windows platforms (future)

## Architecture

```
┌──────────────────────────────────────────────┐
│          Your C# Game Application            │
│     (Game logic, AI, scripting, etc.)        │
└──────────────────────────────────────────────┘
                     ↓
┌──────────────────────────────────────────────┐
│        FreshEngine.Managed.dll               │
│    (P/Invoke wrappers, managed API)          │
└──────────────────────────────────────────────┘
                     ↓ P/Invoke
┌──────────────────────────────────────────────┐
│        FreshVoxelEngine.dll                  │
│    (Native C++ engine, core systems)         │
└──────────────────────────────────────────────┘
                     ↓
┌──────────────────────────────────────────────┐
│           DirectX 11/12 Runtime              │
│      (GPU rendering, Windows platform)       │
└──────────────────────────────────────────────┘
```

## Requirements

### Software Requirements

- **Visual Studio 2022** (17.8 or later)
  - "Desktop development with C++" workload
  - ".NET desktop development" workload
  
- **.NET 9 SDK**
  - Download: https://dotnet.microsoft.com/download/dotnet/9.0
  - Verify: `dotnet --version` (should show 9.0.x)

- **Windows 10/11 (x64)**
  - DirectX 12 (Windows 10+) or DirectX 11

### Build Requirements

- CMake 3.20+
- MSVC v143 (Visual Studio 2022 compiler)
- Windows SDK 10.0.19041.0 or higher

## Quick Start

### 1. Install .NET 9 SDK

First, install the .NET 9 SDK (required):

**Download:** https://dotnet.microsoft.com/download/dotnet/9.0

**Verify installation:**
```batch
dotnet --version
```
Should show 9.0.x or higher.

### 2. Run the Automated Installer

The main installer checks for .NET 9 first and builds the managed wrapper:

```batch
cd fresh
install.bat
```

The installer will:
1. Check for .NET 9 SDK (required)
2. Build the managed wrapper (FreshEngine.Managed.dll)
3. Install native engine dependencies
4. Build the native engine backend

### 3. Create Your First C# Game

Create a new C# console application:

```csharp
using FreshEngine.Managed;

namespace MyVoxelGame
{
    class Program
    {
        static void Main(string[] args)
        {
            using var engine = new Engine();
            
            if (engine.Initialize())
            {
                engine.WindowTitle = "My Voxel Game";
                engine.SetWindowSize(1920, 1080);
                
                Console.WriteLine("Engine initialized successfully!");
                
                // Run the game loop
                engine.Run();
                
                // Cleanup
                engine.Shutdown();
            }
            else
            {
                Console.WriteLine("Failed to initialize engine");
            }
        }
    }
}
```

### 4. Reference the Managed Library

Add a reference to the built managed wrapper:

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>Exe</OutputType>
    <TargetFramework>net9.0-windows</TargetFramework>
    <PlatformTarget>x64</PlatformTarget>
  </PropertyGroup>

  <ItemGroup>
    <Reference Include="FreshEngine.Managed">
      <HintPath>path\to\fresh\dotnet\bin\Release\net9.0-windows\FreshEngine.Managed.dll</HintPath>
    </Reference>
  </ItemGroup>
</Project>
```

## Current Implementation Status

### ✅ Available Features

- Engine lifecycle management (Create, Initialize, Run, Shutdown)
- Window management (Title, Size)
- P/Invoke infrastructure

### 🔄 In Progress

The native engine currently outputs an `.exe` file. To enable full .NET integration, the following changes are planned:

1. **Create DLL Export** - Modify CMakeLists.txt to build a shared library
2. **C API Layer** - Add C-compatible exports for key engine functions
3. **Extended Bindings** - Expose more engine features to managed code

### 📋 Planned Features

- Scene management
- Voxel world manipulation
- Camera control
- Input handling
- Resource loading
- Event system callbacks
- Rendering customization

## Enabling Full .NET Integration

To enable full .NET integration, follow these steps:

### Step 1: Modify CMakeLists.txt

Add a shared library target for the engine:

```cmake
# Add shared library for .NET interop
add_library(${PROJECT_NAME}_dll SHARED ${ENGINE_SOURCES})

# Configure DLL output
set_target_properties(${PROJECT_NAME}_dll PROPERTIES
    OUTPUT_NAME "FreshVoxelEngine"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    POSITION_INDEPENDENT_CODE ON
)

# Copy dependencies
target_link_libraries(${PROJECT_NAME}_dll PRIVATE
    glfw
    glm::glm
    imgui::imgui
    d3d11.lib
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
)
```

### Step 2: Create C API Exports

Create a new header file `engine/core/EngineAPI.h`:

```cpp
#pragma once

#ifdef FRESH_ENGINE_EXPORTS
#define FRESH_API __declspec(dllexport)
#else
#define FRESH_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Engine lifecycle
FRESH_API void* Engine_Create();
FRESH_API void Engine_Destroy(void* engine);
FRESH_API bool Engine_Initialize(void* engine);
FRESH_API void Engine_Run(void* engine);
FRESH_API void Engine_Shutdown(void* engine);

// Window management
FRESH_API void Engine_SetWindowTitle(void* engine, const char* title);
FRESH_API void Engine_SetWindowSize(void* engine, int width, int height);

#ifdef __cplusplus
}
#endif
```

### Step 3: Implement C API Exports

Create `engine/core/EngineAPI.cpp`:

```cpp
#include "core/EngineAPI.h"
#include "core/Engine.h"

extern "C" {

FRESH_API void* Engine_Create() {
    return new fresh::Engine();
}

FRESH_API void Engine_Destroy(void* engine) {
    delete static_cast<fresh::Engine*>(engine);
}

FRESH_API bool Engine_Initialize(void* engine) {
    return static_cast<fresh::Engine*>(engine)->initialize();
}

FRESH_API void Engine_Run(void* engine) {
    static_cast<fresh::Engine*>(engine)->run();
}

FRESH_API void Engine_Shutdown(void* engine) {
    static_cast<fresh::Engine*>(engine)->shutdown();
}

FRESH_API void Engine_SetWindowTitle(void* engine, const char* title) {
    static_cast<fresh::Engine*>(engine)->setWindowTitle(title);
}

FRESH_API void Engine_SetWindowSize(void* engine, int width, int height) {
    static_cast<fresh::Engine*>(engine)->setWindowSize(width, height);
}

}
```

### Step 4: Update P/Invoke Declarations

The P/Invoke declarations in `dotnet/EngineInterop.cs` are already set up to use the exported functions.

### Step 5: Build and Test

```batch
# Build native DLL
cmake --build build --config Release --target FreshVoxelEngine_dll

# Build managed wrapper
cd dotnet
dotnet build -c Release

# Test the integration
dotnet run --project ../examples/ManagedGame/ManagedGame.csproj
```

## Advanced Usage

### Custom Game Loop

```csharp
using FreshEngine.Managed;

public class CustomGameLoop
{
    private readonly Engine _engine;
    
    public CustomGameLoop()
    {
        _engine = new Engine();
    }
    
    public void Run()
    {
        if (!_engine.Initialize())
            return;
            
        _engine.WindowTitle = "Custom Game Loop";
        _engine.SetWindowSize(1920, 1080);
        
        // Custom initialization
        LoadContent();
        
        // Run game loop
        _engine.Run();
        
        // Cleanup
        UnloadContent();
        _engine.Shutdown();
    }
    
    private void LoadContent()
    {
        // Load textures, models, etc.
    }
    
    private void UnloadContent()
    {
        // Cleanup resources
    }
}
```

### Marshaling Complex Data

For passing complex data structures between C# and C++:

```csharp
[StructLayout(LayoutKind.Sequential)]
public struct Vector3
{
    public float X;
    public float Y;
    public float Z;
    
    public Vector3(float x, float y, float z)
    {
        X = x;
        Y = y;
        Z = z;
    }
}

[DllImport("FreshVoxelEngine.dll")]
private static extern void Engine_SetCameraPosition(IntPtr engine, Vector3 position);
```

### Event Callbacks

```csharp
// Define callback delegate
[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void RenderCallback(IntPtr userData);

// Register callback
[DllImport("FreshVoxelEngine.dll")]
private static extern void Engine_SetRenderCallback(IntPtr engine, RenderCallback callback);

// Use callback
private void OnRender(IntPtr userData)
{
    // Custom rendering logic
    Console.WriteLine("Frame rendered");
}

_renderCallback = OnRender; // Keep reference to prevent GC
NativeMethods.Engine_SetRenderCallback(_handle, _renderCallback);
```

## Performance Considerations

### P/Invoke Overhead

- Each P/Invoke call has ~10-20ns overhead
- Batch operations when possible
- Cache frequently used values in managed code
- Use unsafe code for high-frequency operations

### Memory Management

- **Native Resources**: Always use `IDisposable` pattern
- **Pinned Memory**: Use `GCHandle.Alloc(data, GCHandleType.Pinned)` for large data transfers
- **String Marshaling**: Consider using `StringBuilder` for output strings
- **Array Marshaling**: Use `fixed` statements for array access

### Example - Efficient Bulk Operations

```csharp
// Instead of multiple calls:
for (int i = 0; i < 1000; i++)
{
    engine.SetVoxel(i, 0, 0, VoxelType.Stone); // 1000 P/Invoke calls
}

// Batch the operation:
var voxels = new VoxelData[1000];
// Fill voxels array...
engine.SetVoxelsBulk(voxels); // 1 P/Invoke call
```

## Debugging

### Mixed-Mode Debugging

Enable mixed-mode debugging in Visual Studio:

1. Right-click project → Properties
2. Debug → Enable native code debugging
3. Set breakpoints in both C# and C++ code
4. Step through across the interop boundary

### Common Issues

**Issue**: `DllNotFoundException`
- **Solution**: Ensure `FreshVoxelEngine.dll` is in the output directory
- Copy native DLL to bin folder or add to PATH

**Issue**: `BadImageFormatException`
- **Solution**: Ensure both managed and native code are x64
- Check platform target settings

**Issue**: Access Violation
- **Solution**: Verify P/Invoke signatures match native function signatures
- Check pointer validity and lifetime

## Best Practices

1. **Always dispose native resources** - Use `using` statements
2. **Cache P/Invoke delegates** - Don't create them in hot paths
3. **Batch operations** - Minimize boundary crossings
4. **Keep native objects alive** - Prevent premature garbage collection
5. **Thread safety** - Ensure thread-safe access to native objects
6. **Error handling** - Check return values and handle exceptions

## Examples

See the `examples/` directory for complete C# examples:

- `examples/BasicGame/` - Simple game loop
- `examples/VoxelEditor/` - Voxel editing application
- `examples/TerrainGenerator/` - Procedural terrain generation

## Contributing

When extending the .NET bindings:

1. Add C API export in native code
2. Update P/Invoke declarations in `EngineInterop.cs`
3. Create managed wrapper in appropriate managed class
4. Add XML documentation comments
5. Update this guide with usage examples

## License

Same as the main Fresh Voxel Engine project (MIT License).
