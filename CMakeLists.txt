cmake_minimum_required(VERSION 3.20)
project(FreshVoxelEngine VERSION 0.1.0 LANGUAGES CXX)

# FreshVoxel Engine — Voxel-only game engine and editor
# Scope: 3D voxel games (isometric/2.5D to full 3D perspective)
# Future fork: Fresh2D for strictly 2D games

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# Platform Detection and Graphics API Configuration
# ==============================================================================

# FreshVoxel Engine is now Windows-only for voxel gaming and professional editor development
# Temporarily allow Linux builds for CI testing purposes
if(NOT WIN32)
    message(WARNING "FreshVoxel Engine is primarily designed for Windows 10/11 (x64).")
    message(WARNING "Building on Linux for testing purposes only - some features may not work.")
    set(ENABLE_DIRECTX OFF)
    set(ENABLE_OPENGL ON)
else()
    message(STATUS "Configuring FreshVoxel Engine for Windows Gaming Platform")
    message(STATUS "  - Platform: Windows 10/11 (x64)")
    message(STATUS "  - Primary Renderer: DirectX 12")
    message(STATUS "  - Fallback Renderer: DirectX 11")
    message(STATUS "  - Optional: OpenGL support (for compatibility)")
    message(STATUS "  - Editor: Windows-native Unreal-like game development environment")
    set(ENABLE_DIRECTX ON)
    set(ENABLE_OPENGL ON)
endif()

# Enable graphics backends based on platform
if(WIN32)
    add_definitions(-DFRESH_DIRECTX_SUPPORT)
    add_definitions(-DFRESH_WINDOWS_ONLY)
    add_definitions(-DNOMINMAX)  # Prevent Windows.h from defining min/max macros
    add_definitions(-DUNICODE -D_UNICODE)  # Use Unicode (wide character) Windows APIs
endif()
if(ENABLE_OPENGL)
    add_definitions(-DFRESH_OPENGL_SUPPORT)
endif()

# ==============================================================================
# Dependencies
# ==============================================================================

# Detect if we're using vcpkg for automatic dependency management
if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
    set(USING_VCPKG TRUE)
    message(STATUS "Using vcpkg for dependency management")
    message(STATUS "Dependencies will be automatically installed from vcpkg.json")
else()
    set(USING_VCPKG FALSE)
    message(STATUS "Not using vcpkg - dependencies must be pre-installed")
endif()

# Try to find GLFW (required for windowing)
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found via find_package, trying pkg-config")
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(GLFW QUIET glfw3)
    endif()
    
    if(NOT GLFW_FOUND)
        message(WARNING "GLFW not found. Please install GLFW 3.3+")
        message(WARNING "  Windows: vcpkg install glfw3:x64-windows")
    endif()
endif()

# GLM for math (header-only)
find_package(glm QUIET)
if(NOT glm_FOUND)
    message(STATUS "GLM not found, will use header-only include if available")
endif()

# OpenGL and GLEW (for OpenGL extension loading on non-Apple platforms)
if(ENABLE_OPENGL)
    find_package(OpenGL QUIET)
    if(OpenGL_FOUND)
        message(STATUS "OpenGL found")
    else()
        message(WARNING "OpenGL not found. OpenGL backend will not be available.")
    endif()
    
    # GLEW is needed on Windows for OpenGL extension loading (optional)
    find_package(GLEW QUIET)
    if(GLEW_FOUND)
        message(STATUS "GLEW found for OpenGL extension loading")
        add_definitions(-DFRESH_GLEW_AVAILABLE)
    else()
        message(WARNING "GLEW not found. OpenGL support will be limited.")
        message(WARNING "  Install via: vcpkg install glew:x64-windows")
    endif()
endif()

# OpenAL for audio
find_package(OpenAL QUIET)
if(NOT OpenAL_FOUND)
    message(WARNING "OpenAL not found. Audio features will be disabled.")
    message(WARNING "  vcpkg install openal-soft:x64-windows")
else()
    message(STATUS "OpenAL found successfully")
endif()

# Vorbis for OGG audio support
find_package(Vorbis QUIET CONFIG)
if(NOT Vorbis_FOUND)
    message(WARNING "libvorbis not found. OGG audio support will be disabled.")
    message(WARNING "  vcpkg install libvorbis:x64-windows")
else()
    message(STATUS "libvorbis found successfully")
endif()

# Lua for scripting
# On Windows with vcpkg, prefer CONFIG mode to get proper target
if(USING_VCPKG)
    # Extract vcpkg root path once for reuse in fallback logic
    if(DEFINED CMAKE_TOOLCHAIN_FILE AND NOT DEFINED VCPKG_ROOT)
        get_filename_component(VCPKG_ROOT "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
        get_filename_component(VCPKG_ROOT "${VCPKG_ROOT}" DIRECTORY)
        get_filename_component(VCPKG_ROOT "${VCPKG_ROOT}" DIRECTORY)
    endif()
    
    # Try vcpkg's CONFIG mode first on Windows
    # Try LuaJIT first (has better C linkage support on Windows)
    find_package(luajit CONFIG QUIET)
    if(luajit_FOUND OR LUAJIT_FOUND)
        set(Lua_FOUND TRUE)
        set(LUA_FOUND_VIA_CONFIG TRUE)
        set(USING_LUAJIT TRUE)
        message(STATUS "LuaJIT found via vcpkg CONFIG mode")
        
        # Get LuaJIT variables from the config
        if(NOT DEFINED LUA_INCLUDE_DIR AND DEFINED LUAJIT_INCLUDE_DIR)
            set(LUA_INCLUDE_DIR ${LUAJIT_INCLUDE_DIR})
        elseif(NOT DEFINED LUA_INCLUDE_DIR AND DEFINED luajit_INCLUDE_DIR)
            set(LUA_INCLUDE_DIR ${luajit_INCLUDE_DIR})
        endif()
        if(NOT DEFINED LUA_LIBRARIES AND DEFINED LUAJIT_LIBRARIES)
            set(LUA_LIBRARIES ${LUAJIT_LIBRARIES})
        elseif(NOT DEFINED LUA_LIBRARIES AND DEFINED luajit_LIBRARIES)
            set(LUA_LIBRARIES ${luajit_LIBRARIES})
        endif()
        
        # Try to extract include directory from target properties if not already set
        if(NOT DEFINED LUA_INCLUDE_DIR)
            if(TARGET luajit)
                get_target_property(LUAJIT_INCLUDE_DIRS luajit INTERFACE_INCLUDE_DIRECTORIES)
                if(LUAJIT_INCLUDE_DIRS)
                    set(LUA_INCLUDE_DIR ${LUAJIT_INCLUDE_DIRS})
                endif()
            elseif(TARGET luajit::luajit)
                get_target_property(LUAJIT_INCLUDE_DIRS luajit::luajit INTERFACE_INCLUDE_DIRECTORIES)
                if(LUAJIT_INCLUDE_DIRS)
                    set(LUA_INCLUDE_DIR ${LUAJIT_INCLUDE_DIRS})
                endif()
            endif()
        endif()
        
        # Final fallback: try to find lua.h in vcpkg installed directory
        if(NOT DEFINED LUA_INCLUDE_DIR)
            # Try with VCPKG_TARGET_TRIPLET if defined
            # Check CMAKE_BINARY_DIR first (build-specific vcpkg installation),
            # then VCPKG_ROOT (global vcpkg installation)
            if(DEFINED VCPKG_TARGET_TRIPLET)
                if(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/luajit.h")
                    set(LUA_INCLUDE_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include")
                    message(STATUS "  Found LuaJIT headers in build vcpkg: ${LUA_INCLUDE_DIR}")
                elseif(EXISTS "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include/luajit.h")
                    set(LUA_INCLUDE_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include")
                    message(STATUS "  Found LuaJIT headers in vcpkg: ${LUA_INCLUDE_DIR}")
                # Also check for lua.h as a fallback (in case luajit.h is not present)
                elseif(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/lua.h")
                    set(LUA_INCLUDE_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include")
                    message(STATUS "  Found Lua headers in build vcpkg: ${LUA_INCLUDE_DIR}")
                elseif(EXISTS "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include/lua.h")
                    set(LUA_INCLUDE_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include")
                    message(STATUS "  Found Lua headers in vcpkg: ${LUA_INCLUDE_DIR}")
                endif()
            endif()
            
            # Fallback: Try common Windows triplet if not found
            if(NOT DEFINED LUA_INCLUDE_DIR AND WIN32)
                if(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include/luajit.h")
                    set(LUA_INCLUDE_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include")
                    message(STATUS "  Found LuaJIT headers in build vcpkg (x64-windows): ${LUA_INCLUDE_DIR}")
                elseif(EXISTS "${VCPKG_ROOT}/installed/x64-windows/include/luajit.h")
                    set(LUA_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include")
                    message(STATUS "  Found LuaJIT headers in vcpkg (x64-windows): ${LUA_INCLUDE_DIR}")
                # Also check for lua.h as a fallback (in case luajit.h is not present)
                elseif(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include/lua.h")
                    set(LUA_INCLUDE_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include")
                    message(STATUS "  Found Lua headers in build vcpkg (x64-windows): ${LUA_INCLUDE_DIR}")
                elseif(EXISTS "${VCPKG_ROOT}/installed/x64-windows/include/lua.h")
                    set(LUA_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include")
                    message(STATUS "  Found Lua headers in vcpkg (x64-windows): ${LUA_INCLUDE_DIR}")
                endif()
            endif()
        endif()
    else()
        # Fall back to standard Lua
        find_package(lua CONFIG QUIET)
        if(lua_FOUND)
            set(Lua_FOUND TRUE)
            set(LUA_FOUND_VIA_CONFIG TRUE)
            message(STATUS "Lua found via vcpkg CONFIG mode")
            
            # Get Lua variables from the config
            # Some vcpkg versions set these, some don't
            if(NOT DEFINED LUA_INCLUDE_DIR AND DEFINED lua_INCLUDE_DIR)
                set(LUA_INCLUDE_DIR ${lua_INCLUDE_DIR})
            endif()
            if(NOT DEFINED LUA_LIBRARIES AND DEFINED lua_LIBRARIES)
                set(LUA_LIBRARIES ${lua_LIBRARIES})
            endif()
            
            # Try to extract include directory from target properties if not already set
            if(NOT DEFINED LUA_INCLUDE_DIR)
                if(TARGET lua)
                    get_target_property(LUA_INCLUDE_DIRS lua INTERFACE_INCLUDE_DIRECTORIES)
                    if(LUA_INCLUDE_DIRS)
                        set(LUA_INCLUDE_DIR ${LUA_INCLUDE_DIRS})
                    endif()
                elseif(TARGET lua::lua)
                    get_target_property(LUA_INCLUDE_DIRS lua::lua INTERFACE_INCLUDE_DIRECTORIES)
                    if(LUA_INCLUDE_DIRS)
                        set(LUA_INCLUDE_DIR ${LUA_INCLUDE_DIRS})
                    endif()
                endif()
            endif()
            
            # Final fallback: try to find lua.h in vcpkg installed directory
            if(NOT DEFINED LUA_INCLUDE_DIR)
                # Try with VCPKG_TARGET_TRIPLET if defined
                if(DEFINED VCPKG_TARGET_TRIPLET)
                    if(EXISTS "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include/lua.h")
                        set(LUA_INCLUDE_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include")
                        message(STATUS "  Found Lua headers in vcpkg: ${LUA_INCLUDE_DIR}")
                    elseif(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/lua.h")
                        set(LUA_INCLUDE_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include")
                        message(STATUS "  Found Lua headers in build vcpkg: ${LUA_INCLUDE_DIR}")
                    endif()
                endif()
                
                # Fallback: Try common Windows triplet if not found
                if(NOT DEFINED LUA_INCLUDE_DIR AND WIN32)
                    if(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include/lua.h")
                        set(LUA_INCLUDE_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include")
                        message(STATUS "  Found Lua headers in build vcpkg (x64-windows): ${LUA_INCLUDE_DIR}")
                    elseif(EXISTS "${VCPKG_ROOT}/installed/x64-windows/include/lua.h")
                        set(LUA_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include")
                        message(STATUS "  Found Lua headers in vcpkg (x64-windows): ${LUA_INCLUDE_DIR}")
                    endif()
                endif()
            endif()
        endif()
    endif()
endif()

if(NOT Lua_FOUND)
    # Try standard FindLua module (works with system installations)
    find_package(Lua QUIET)
    if(Lua_FOUND)
        message(STATUS "Lua found via FindLua module")
    endif()
endif()

if(NOT Lua_FOUND)
    message(WARNING "Lua not found. Asset management Lua integration will be disabled.")
    message(WARNING "  vcpkg install luajit:x64-windows")
    message(WARNING "  or vcpkg install lua:x64-windows")
endif()

# Sol2 for modern C++ Lua bindings
if(Lua_FOUND)
    find_package(sol2 QUIET CONFIG)
    if(NOT sol2_FOUND)
        message(WARNING "Sol2 not found. Lua scripting will use basic interface only.")
        message(WARNING "  vcpkg install sol2:x64-windows")
    else()
        message(STATUS "Sol2 found successfully")
        set(SOL2_AVAILABLE TRUE)
    endif()
endif()

# nlohmann-json for JSON parsing
find_package(nlohmann_json QUIET CONFIG)
if(NOT nlohmann_json_FOUND)
    message(WARNING "nlohmann-json not found. Asset pack manifest parsing will be limited.")
    message(WARNING "  vcpkg install nlohmann-json:x64-windows")
else()
    message(STATUS "nlohmann-json found successfully")
endif()

# Native File Dialog Extended for file dialogs
find_package(nfd QUIET CONFIG)
if(NOT nfd_FOUND)
    message(WARNING "Native File Dialog Extended (nfd) not found. File dialogs will not be available.")
    message(WARNING "  vcpkg install nativefiledialog-extended:x64-windows")
else()
    message(STATUS "Native File Dialog Extended found successfully")
endif()

# tinyobjloader for OBJ model loading
find_package(tinyobjloader QUIET CONFIG)
if(NOT tinyobjloader_FOUND)
    message(WARNING "tinyobjloader not found. OBJ model loading will be disabled.")
    message(WARNING "  vcpkg install tinyobjloader:x64-windows")
else()
    message(STATUS "tinyobjloader found successfully")
    set(TINYOBJLOADER_AVAILABLE TRUE)
endif()

message(STATUS "All find_package calls completed")

# ==============================================================================
# Dependency Check Summary
# ==============================================================================

# Check if critical dependencies are available
set(GLFW_AVAILABLE FALSE)
if(glfw3_FOUND OR GLFW_FOUND)
    set(GLFW_AVAILABLE TRUE)
endif()

set(DEPENDENCIES_OK TRUE)
set(MISSING_DEPS "")

if(NOT GLFW_AVAILABLE)
    set(DEPENDENCIES_OK FALSE)
    list(APPEND MISSING_DEPS "GLFW")
endif()

if(NOT glm_FOUND)
    set(DEPENDENCIES_OK FALSE)
    list(APPEND MISSING_DEPS "GLM")
endif()

# OpenGL dependencies (optional for Windows compatibility)
if(ENABLE_OPENGL)
    if(NOT OpenGL_FOUND)
        set(DEPENDENCIES_OK FALSE)
        list(APPEND MISSING_DEPS "OpenGL")
    endif()
    
    # GLEW is required on Windows for OpenGL extension loading
    if(NOT GLEW_FOUND)
        set(DEPENDENCIES_OK FALSE)
        list(APPEND MISSING_DEPS "GLEW")
    endif()
endif()

# If not using vcpkg and dependencies are missing, provide clear guidance
if(NOT USING_VCPKG AND NOT DEPENDENCIES_OK)
    message("")
    message("========================================================================")
    message(STATUS "DEPENDENCY CHECK FAILED")
    message("========================================================================")
    message(STATUS "The following required dependencies are missing:")
    foreach(dep ${MISSING_DEPS})
        message(STATUS "  - ${dep}")
    endforeach()
    message("")
    message(STATUS "To resolve this issue:")
    message("")
    message(STATUS "Use vcpkg for automatic dependency management (RECOMMENDED)")
    message(STATUS "  From the project root directory, run:")
    message(STATUS "     git clone https://github.com/Microsoft/vcpkg.git")
    message(STATUS "     cd vcpkg")
    message(STATUS "     bootstrap-vcpkg.bat")
    message(STATUS "     cd ..")
    message(STATUS "     generate_vs2022.bat")
    message("")
    message(STATUS "Alternative: Install dependencies manually")
    message(STATUS "  Download and install each library, then set CMAKE_PREFIX_PATH")
    message(STATUS "  See BUILD.md for detailed instructions")
    message("")
    message(WARNING "CMake configuration will continue, but compilation will fail without these dependencies!")
    message("========================================================================")
    message("")
elseif(NOT USING_VCPKG AND DEPENDENCIES_OK)
    message("")
    message("========================================================================")
    message(STATUS "All required dependencies found!")
    message("========================================================================")
    message("")
endif()

message(STATUS "Dependency checking completed")

# Enable GLM experimental features
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

# ==============================================================================
# Build Targets (Atlas-style modular architecture)
# ==============================================================================
# Fresh ships as multiple executables, all linked against the same engine core:
#   FreshEditor  - Full editor + runtime (superset)
#   FreshClient  - Player runtime
#   FreshServer  - Headless authoritative server
#   FreshRuntime - Runtime launcher with CLI options

message(STATUS "Configuring modular build targets...")

# Windows-specific definitions (applied globally before subdirectories)
if(WIN32)
    add_definitions(-DFRESH_WIN32_UI)
    message(STATUS "Win32 native UI enabled")
endif()

# --- Engine core library ---
add_subdirectory(engine)

# Configure engine library with dependencies and compiler settings
# Include directories (engine headers are co-located with sources)
# Also keep include/ for backward compatibility during transition
target_include_directories(FreshEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add Lua include directories to engine
if(Lua_FOUND AND DEFINED LUA_INCLUDE_DIR)
    target_include_directories(FreshEngine PUBLIC ${LUA_INCLUDE_DIR})
    message(STATUS "Lua include directories added: ${LUA_INCLUDE_DIR}")
endif()

# Link GLFW (windowing)
if(glfw3_FOUND)
    target_link_libraries(FreshEngine PUBLIC glfw)
elseif(GLFW_FOUND)
    target_include_directories(FreshEngine PUBLIC ${GLFW_INCLUDE_DIRS})
    target_link_libraries(FreshEngine PUBLIC ${GLFW_LIBRARIES})
endif()

# Link GLM (mathematics)
if(glm_FOUND)
    target_link_libraries(FreshEngine PUBLIC glm::glm)
endif()

# Link OpenAL (Audio)
if(OpenAL_FOUND)
    target_link_libraries(FreshEngine PUBLIC OpenAL::OpenAL)
    add_definitions(-DFRESH_OPENAL_AVAILABLE)
    message(STATUS "OpenAL enabled for audio support")
endif()

# Link libvorbis (OGG audio support)
if(Vorbis_FOUND)
    target_link_libraries(FreshEngine PUBLIC Vorbis::vorbisfile Vorbis::vorbis Ogg::ogg)
    add_definitions(-DFRESH_VORBIS_AVAILABLE)
    message(STATUS "libvorbis enabled for OGG audio support")
endif()

# Link Lua (Scripting)
if(Lua_FOUND)
    add_definitions(-DFRESH_LUA_AVAILABLE)
    
    if(NOT DEFINED LUA_INCLUDE_DIR)
        message(WARNING "LUA_INCLUDE_DIR not set - this may cause compilation errors with Sol2")
    endif()
    
    if(LUA_FOUND_VIA_CONFIG)
        if(TARGET luajit)
            target_link_libraries(FreshEngine PUBLIC luajit)
            message(STATUS "LuaJIT enabled via CONFIG mode - linked target 'luajit'")
        elseif(TARGET luajit::luajit)
            target_link_libraries(FreshEngine PUBLIC luajit::luajit)
            message(STATUS "LuaJIT enabled via CONFIG mode - linked target 'luajit::luajit'")
        elseif(TARGET lua)
            target_link_libraries(FreshEngine PUBLIC lua)
            message(STATUS "Lua enabled via CONFIG mode - linked target 'lua'")
        elseif(TARGET lua::lua)
            target_link_libraries(FreshEngine PUBLIC lua::lua)
            message(STATUS "Lua enabled via CONFIG mode - linked target 'lua::lua'")
        elseif(TARGET lua::library)
            target_link_libraries(FreshEngine PUBLIC lua::library)
            message(STATUS "Lua enabled via CONFIG mode - linked target 'lua::library'")
        else()
            if(DEFINED LUA_LIBRARIES)
                target_link_libraries(FreshEngine PUBLIC ${LUA_LIBRARIES})
                message(STATUS "Lua enabled via CONFIG mode - using variables")
            else()
                message(WARNING "Lua found via CONFIG but no targets or variables available")
            endif()
        endif()
    else()
        if(DEFINED LUA_LIBRARIES)
            target_link_libraries(FreshEngine PUBLIC ${LUA_LIBRARIES})
        endif()
        message(STATUS "Lua enabled via FindLua module")
    endif()
    
    # Add Sol2 for modern C++ Lua binding
    if(SOL2_AVAILABLE)
        target_link_libraries(FreshEngine PUBLIC sol2::sol2)
        add_definitions(-DFRESH_SOL2_AVAILABLE)
        message(STATUS "Sol2 enabled - using production Lua integration")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/sol2/include")
        target_include_directories(FreshEngine PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/external/sol2/include)
        add_definitions(-DFRESH_SOL2_AVAILABLE)
        message(STATUS "Sol2 found (local) - using production Lua integration")
    else()
        message(STATUS "Sol2 not found - Lua scripting features will be limited")
    endif()
    
    message(STATUS "Lua enabled for scripting and modding")
endif()

# Link nlohmann-json (JSON parsing for asset manifests)
if(nlohmann_json_FOUND)
    target_link_libraries(FreshEngine PUBLIC nlohmann_json::nlohmann_json)
    add_definitions(-DFRESH_JSON_AVAILABLE)
    message(STATUS "nlohmann-json enabled for asset manifest parsing")
endif()

# Link Native File Dialog Extended (file dialogs)
if(nfd_FOUND)
    target_link_libraries(FreshEngine PUBLIC nfd::nfd)
    add_definitions(-DFRESH_NFD_AVAILABLE)
    message(STATUS "Native File Dialog Extended enabled for file operations")
endif()

# Link tinyobjloader (OBJ model loading)
if(TINYOBJLOADER_AVAILABLE)
    target_link_libraries(FreshEngine PUBLIC tinyobjloader::tinyobjloader)
    add_definitions(-DFRESH_TINYOBJLOADER_AVAILABLE)
    message(STATUS "tinyobjloader enabled for OBJ model loading")
endif()

# Link DirectX (Windows only)
if(WIN32 AND ENABLE_DIRECTX)
    target_link_libraries(FreshEngine PUBLIC
        d3d11.lib 
        d3d12.lib 
        dxgi.lib 
        d3dcompiler.lib
    )
    message(STATUS "DirectX libraries linked")
endif()

# Link Win32 UI libraries (Windows only)
if(WIN32)
    target_link_libraries(FreshEngine PUBLIC
        comctl32.lib
        user32.lib
        gdi32.lib
        gdiplus.lib
        shell32.lib
        ole32.lib
        dwmapi.lib
    )
    message(STATUS "Win32 UI libraries linked")
endif()

# Link OpenGL and GLEW (cross-platform)
if(ENABLE_OPENGL AND OpenGL_FOUND)
    target_link_libraries(FreshEngine PUBLIC OpenGL::GL)
    message(STATUS "OpenGL library linked")
    
    if(NOT APPLE AND GLEW_FOUND)
        target_link_libraries(FreshEngine PUBLIC GLEW::GLEW)
        message(STATUS "GLEW library linked")
    endif()
endif()

# Compiler settings for engine library
if(NOT MSVC)
    message(WARNING "Non-MSVC compiler detected. Fresh Voxel Engine is optimized for MSVC/Visual Studio 2022.")
    message(WARNING "Other compilers may work but are not officially supported.")
endif()

if(MSVC)
    target_compile_options(FreshEngine PRIVATE /W4 /WX)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(FreshEngine PRIVATE 
            /O2 /Oi /Ot /GL /Gy /GS- /fp:fast /arch:AVX2
        )
    endif()
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(FreshEngine PRIVATE /ZI /Od /RTC1)
    endif()
endif()

# --- Executable targets (all link against FreshEngine) ---
add_subdirectory(editor)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(runtime)

# Compiler and linker settings are applied in each subdirectory's CMakeLists.txt
# via inheriting from FreshEngine's PUBLIC properties and per-target options

# Install targets
install(TARGETS FreshEngine DESTINATION lib)
install(DIRECTORY shaders/ DESTINATION bin/shaders)

# ==============================================================================
# Testing Support
# ==============================================================================

option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    message(STATUS "Building tests enabled")
    
    enable_testing()
    
    find_package(GTest QUIET)
    
    if(NOT GTest_FOUND)
        message(STATUS "Google Test not found, fetching from GitHub...")
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG release-1.12.1
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
    
    # Test source files
    set(TEST_SOURCES
        tests/test_main.cpp
        tests/voxel/ChunkTests.cpp
        tests/voxel/VoxelWorldTests.cpp
        tests/gameplay/TimeManagerTests.cpp
        tests/gameplay/WeatherManagerTests.cpp
        tests/gameplay/SeasonManagerTests.cpp
        tests/integration/WorldCreationIntegrationTest.cpp
        tests/generation/NoiseTests.cpp
        tests/generation/TerrainGeneratorTests.cpp
        tests/input/InputManagerTests.cpp
        tests/core/EngineInitializationTests.cpp
        tests/core/EngineSystemTests.cpp
        tests/core/EngineConfigTests.cpp
        tests/core/ReflectionTests.cpp
        tests/core/ResourceManagerTests.cpp
        tests/scripting/EventSystemTests.cpp
        tests/rpg/InventoryTests.cpp
        tests/rpg/CraftingSystemTests.cpp
        tests/character/VoxelCharacterTests.cpp
        tests/character/BodyPartLibraryTests.cpp
        tests/character/AnimationClipTests.cpp
        tests/editor/SelectionManagerTests.cpp
        tests/editor/SelectionRendererTests.cpp
        tests/ecs/HealthComponentTests.cpp
        tests/ecs/MovementComponentTests.cpp
        tests/ecs/CombatSystemTests.cpp
        tests/voxelship/VoxelBlockStretchTest.cpp
        tests/renderer/test_model_loader.cpp
        tests/ai/BehaviorTreeTests.cpp
        tests/ai/LLMClientTests.cpp
        tests/sim/TickSchedulerTests.cpp
        tests/sim/StateHasherTests.cpp
        tests/sim/ReplayRecorderTests.cpp
        tests/command/CommandHistoryTests.cpp
        tests/mod/ModLoaderTests.cpp
        tests/plugin/PluginSystemTests.cpp
        tests/schema/SchemaValidatorTests.cpp
        tests/module/ModuleLoaderTests.cpp
        tests/devtools/TimeCommandsTests.cpp
        tests/ui/InventoryPanelTests.cpp
        tests/viewport/ViewportContextTests.cpp
        tests/viewport/InputContextTests.cpp
        tests/viewport/ViewportRenderTargetTests.cpp
    )
    
    # Engine source files needed for tests (subset from engine/)
    # Note: Tests compile specific subsystem sources rather than linking the full
    # FreshEngine library because some engine modules require Windows-specific APIs
    # not available on all platforms (e.g., Win32 UI, DirectX).
    set(TEST_ENGINE_SOURCES
        engine/voxel/Chunk.cpp
        engine/voxel/VoxelWorld.cpp
        engine/voxel/MeshGenerator.cpp
        engine/generation/NoiseGenerator.cpp
        engine/generation/TerrainGenerator.cpp
        engine/core/MemoryManager.cpp
        engine/core/Logger.cpp
        engine/core/Reflection.cpp
        engine/core/ResourceManager.cpp
        engine/renderer/stb_image_impl.cpp
        engine/scripting/EventSystem.cpp
        engine/input/InputManager.cpp
        engine/gameplay/TimeManager.cpp
        engine/gameplay/Camera.cpp
        engine/gameplay/WeatherManager.cpp
        engine/gameplay/SeasonManager.cpp
        engine/rpg/Inventory.cpp
        engine/rpg/CraftingSystem.cpp
        engine/assets/ModularAssetSystem.cpp
        engine/character/VoxelCharacter.cpp
        engine/character/BodyPartLibrary.cpp
        engine/character/AnimationClip.cpp
        engine/character/AnimationController.cpp
        engine/editor/SelectionManager.cpp
        engine/editor/SelectionRenderer.cpp
        engine/editor/TransformGizmo.cpp
        engine/editor/TerraformingSystem.cpp
        engine/editor/FileDialogManager.cpp
        engine/devtools/DebugRenderer.cpp
        engine/devtools/DebugConsole.cpp
        engine/devtools/TimeCommands.cpp
        engine/ecs/EntityManager.cpp
        engine/voxelship/VoxelBlock.cpp
        engine/renderer/ModelLoader.cpp
        engine/ai/BehaviorTree.cpp
        engine/ai/LLMClient.cpp
        engine/ai/ProjectContext.cpp
        engine/sim/TickScheduler.cpp
        engine/sim/StateHasher.cpp
        engine/sim/ReplayRecorder.cpp
        engine/command/CommandHistory.cpp
        engine/mod/ModLoader.cpp
        engine/plugin/PluginSystem.cpp
        engine/schema/SchemaValidator.cpp
        engine/module/ModuleLoader.cpp
        engine/ui/InventoryPanel.cpp
        engine/audio/AudioEngine.cpp
        engine/viewport/ViewportContext.cpp
        engine/viewport/ViewportRenderTarget.cpp
        engine/viewport/InputContext.cpp
    )

    # Create test executable
    add_executable(FreshTests ${TEST_SOURCES} ${TEST_ENGINE_SOURCES})
    
    # Include directories for tests
    target_include_directories(FreshTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/engine
    )
    
    # Link test executable with Google Test and engine libraries
    # Note: Using GTest::gtest without gtest_main since we provide our own main
    target_link_libraries(FreshTests PRIVATE
        GTest::gtest
    )
    
    # Link GLFW if needed for tests
    if(glfw3_FOUND)
        target_link_libraries(FreshTests PRIVATE glfw)
    elseif(GLFW_FOUND)
        target_include_directories(FreshTests PRIVATE ${GLFW_INCLUDE_DIRS})
        target_link_libraries(FreshTests PRIVATE ${GLFW_LIBRARIES})
    else()
        if(UNIX AND NOT APPLE)
            target_link_libraries(FreshTests PRIVATE glfw)
        endif()
    endif()
    
    # Link GLM if found
    if(glm_FOUND)
        target_link_libraries(FreshTests PRIVATE glm::glm)
    endif()
    
    # Link Native File Dialog Extended if found (required for FileDialogManager)
    if(nfd_FOUND)
        target_link_libraries(FreshTests PRIVATE nfd::nfd)
        message(STATUS "Native File Dialog Extended linked to test executable")
    endif()
    
    # Compiler settings for tests
    target_compile_features(FreshTests PRIVATE cxx_std_20)
    
    if(MSVC)
        target_compile_options(FreshTests PRIVATE /W4 /WX)
    else()
        message(WARNING "Non-MSVC compiler for tests - may not be fully supported")
    endif()
    
    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(FreshTests
        DISCOVERY_MODE PRE_TEST
    )
    
    message(STATUS "Tests configured successfully")
    message(STATUS "Run tests with: ctest or FreshTests")
endif()

# ==============================================================================
# Example Programs
# ==============================================================================

message(STATUS "Examples are integrated into FreshEditor executable")
message(STATUS "Run FreshEditor and select from the main menu")
message(STATUS "See examples/README.md for demo documentation")

message(STATUS "")
message(STATUS "========================================================================")
message(STATUS "CMake configuration completed successfully!")
message(STATUS "========================================================================")
message(STATUS "")
