# Fresh Voxel Engine - C++ Development Guidelines

## Code Style and Standards

### General Principles
1. **Modern C++20**: Use modern C++ features (auto, smart pointers, lambdas, constexpr, concepts, ranges)
2. **RAII**: Resource Acquisition Is Initialization - manage resources with constructors/destructors
3. **Rule of Zero/Five**: Follow the rule of zero when possible, rule of five when needed
4. **DRY**: Don't Repeat Yourself - factor out common code
5. **SOLID**: Follow SOLID principles for object-oriented design
6. **Performance First**: Optimize hot paths, profile before optimizing cold paths

### Naming Conventions

#### Classes and Structs
```cpp
class PlayerController { };           // PascalCase for classes
struct TransformComponent { };        // PascalCase for structs
```

#### Functions and Methods
```cpp
void updatePhysics(float deltaTime);  // camelCase for functions
void setPosition(const glm::vec3& pos);
bool isGrounded() const;              // 'is/has/can' prefix for booleans
```

#### Variables
```cpp
float playerSpeed = 5.0f;             // camelCase for variables
const int MAX_PLAYERS = 12;           // UPPER_CASE for constants
glm::vec3 m_position;                 // m_ prefix for member variables (optional)
static int s_instanceCount;           // s_ prefix for static members (optional)
```

#### Namespaces
```cpp
namespace fresh {                     // lowercase for namespaces
namespace rendering {                 // nested namespaces okay
```

#### Files
```cpp
PlayerController.h                    // PascalCase matching class name
PlayerController.cpp
VoxelWorld.h
```

### Code Organization

#### Header Files (.h)
```cpp
#pragma once                          // Use pragma once (widely supported)

#include <system_headers>             // System includes first
#include <third_party/headers>        // Third party next
#include "engine/headers.h"           // Local headers last

namespace fresh {

// Forward declarations
class VoxelWorld;

/**
 * @brief Brief description
 * 
 * Detailed description of the class purpose,
 * usage patterns, and important notes.
 */
class MyClass {
public:
    // Public interface first
    MyClass();
    ~MyClass();
    
    // Public methods
    void update(float deltaTime);
    
    // Getters/setters grouped together
    float getValue() const { return value; }
    void setValue(float v) { value = v; }
    
private:
    // Private methods
    void helperFunction();
    
    // Member variables last
    float value;
    VoxelWorld* world;
};

} // namespace fresh
```

#### Source Files (.cpp)
```cpp
#include "MyClass.h"

#include "OtherDependencies.h"

namespace fresh {

MyClass::MyClass() 
    : value(0.0f), world(nullptr) {  // Initialize in declaration order
    // Constructor body
}

MyClass::~MyClass() {
    // Cleanup
}

void MyClass::update(float deltaTime) {
    // Implementation
}

void MyClass::helperFunction() {
    // Private helper
}

} // namespace fresh
```

### Memory Management

#### Smart Pointers
```cpp
// Prefer smart pointers over raw pointers
std::unique_ptr<Player> player;           // Unique ownership
std::shared_ptr<Texture> texture;         // Shared ownership
std::weak_ptr<GameObject> weakRef;        // Weak reference

// Use make_unique/make_shared
auto player = std::make_unique<Player>();
auto texture = std::make_shared<Texture>("path.png");

// Raw pointers only for:
// 1. Non-owning references
// 2. Optional parameters
// 3. Performance-critical code (with documentation)
GameObject* getParent();                  // Non-owning, never null
GameObject* findObject(const std::string& name); // Can be null
```

#### Resource Management
```cpp
class ResourceHandle {
    // Use RAII for all resources
    ResourceHandle() { acquire(); }
    ~ResourceHandle() { release(); }
    
    // Delete copy, implement move
    ResourceHandle(const ResourceHandle&) = delete;
    ResourceHandle& operator=(const ResourceHandle&) = delete;
    ResourceHandle(ResourceHandle&&) noexcept = default;
    ResourceHandle& operator=(ResourceHandle&&) noexcept = default;
};
```

### Error Handling

#### Use Exceptions Sparingly
```cpp
// For recoverable errors, return error codes or optional
std::optional<Player> loadPlayer(const std::string& file);

// For programming errors, use assertions
assert(world != nullptr && "World must be initialized");

// For critical errors, use exceptions
if (!vulkanDevice) {
    throw std::runtime_error("Failed to initialize Vulkan device");
}

// Use result types for operations that can fail
enum class LoadResult {
    Success,
    FileNotFound,
    InvalidFormat,
    OutOfMemory
};

LoadResult loadTexture(const std::string& path);
```

### Performance Best Practices

#### Pass by const reference
```cpp
void processVector(const std::vector<int>& data);  // Large objects
void setValue(float value);                         // Small POD types okay
void update(const glm::mat4& transform);            // Matrix by const ref
```

#### Use move semantics
```cpp
std::vector<Vertex> createMesh() {
    std::vector<Vertex> vertices;
    // ... fill vertices
    return vertices;  // RVO/Move, no copy
}

void setData(std::vector<Vertex>&& data) {  // Accept by rvalue reference
    vertices = std::move(data);
}
```

#### Reserve capacity
```cpp
std::vector<Chunk*> chunks;
chunks.reserve(1000);  // Avoid reallocations

std::unordered_map<int, Entity> entities;
entities.reserve(5000);
```

#### Cache-friendly code
```cpp
// Prefer arrays over linked structures
std::vector<Transform> transforms;  // Good - contiguous
std::list<Transform> transforms;    // Bad - scattered in memory

// Structure of Arrays (SoA) for performance
struct TransformSoA {
    std::vector<glm::vec3> positions;
    std::vector<glm::quat> rotations;
    std::vector<glm::vec3> scales;
};
```

### Const Correctness

```cpp
class Player {
public:
    // Mark methods const if they don't modify state
    glm::vec3 getPosition() const { return position; }
    bool isGrounded() const { return grounded; }
    
    // Non-const methods modify state
    void setPosition(const glm::vec3& pos) { position = pos; }
    
private:
    glm::vec3 position;
    bool grounded;
};

// Use const for parameters that shouldn't be modified
void render(const Mesh& mesh, const Material& material);

// Use const for pointers
const Texture* getTexture() const;  // Can't modify texture, can't modify this
```

### Threading and Concurrency

```cpp
// Use std::mutex for synchronization
class ThreadSafeQueue {
private:
    std::queue<Task> tasks;
    mutable std::mutex mutex;  // mutable for const methods
    
public:
    void push(Task task) {
        std::lock_guard<std::mutex> lock(mutex);
        tasks.push(std::move(task));
    }
    
    std::optional<Task> pop() {
        std::lock_guard<std::mutex> lock(mutex);
        if (tasks.empty()) return std::nullopt;
        Task task = std::move(tasks.front());
        tasks.pop();
        return task;
    }
};

// Use std::atomic for simple flags
std::atomic<bool> shouldRun{true};

// Prefer std::thread or std::async
std::thread worker(&MyClass::workerFunction, this);
auto future = std::async(std::launch::async, &MyClass::heavyComputation, this);
```

### Documentation

#### Use Doxygen-style comments
```cpp
/**
 * @brief Brief one-line description
 * 
 * Detailed description that can span multiple lines.
 * Explain complex behavior, edge cases, and usage.
 * 
 * @param deltaTime Time since last frame in seconds
 * @param force Force vector to apply
 * @return true if successful, false otherwise
 * 
 * @note Important note about usage
 * @warning Potential pitfall or danger
 * @see RelatedClass for more information
 */
bool applyForce(float deltaTime, const glm::vec3& force);
```

#### Comment complex logic
```cpp
// Good comments explain WHY, not WHAT
// Bad: Increment i
i++;

// Good: Skip header bytes when parsing file format
i += sizeof(FileHeader);

// Comment complex algorithms
// Bresenham's line algorithm for voxel raycast
// Efficiently steps through grid cells along ray direction
for (int i = 0; i < maxSteps; ++i) {
    // ... implementation
}
```

### Templates and Generic Programming

```cpp
// Use templates for generic algorithms
template<typename T>
T clamp(T value, T min, T max) {
    return std::max(min, std::min(max, value));
}

// Use C++20 concepts for better type constraints
template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template<Arithmetic T>
T square(T value) {
    return value * value;
}

// Use template specialization sparingly
template<typename T>
class Pool { /* generic implementation */ };

template<>
class Pool<Chunk> { /* specialized for chunks */ };
```

### Include Guards and Dependencies

```cpp
// Prefer #pragma once
#pragma once

// Minimize includes in headers - use forward declarations
class VoxelWorld;  // Forward declare instead of #include

// Only include what you use
#include <vector>  // Actually used
// Don't include unnecessary headers

// In .cpp files, include everything needed
#include "MyClass.h"
#include "VoxelWorld.h"
#include <algorithm>
#include <iostream>
```

### Platform Independence

```cpp
// Use standard library types
#include <cstdint>
int32_t myInt;    // Not 'int' - size varies
uint64_t mySize;  // Not 'unsigned long'
size_t count;     // For sizes and counts

// Use platform-independent file paths
#include <filesystem>
std::filesystem::path assetPath = "assets/textures/stone.png";
```

### Testing and Assertions

```cpp
// Use assertions for programmer errors
assert(index < size && "Index out of bounds");
assert(pointer != nullptr);

// Create custom assertion for debug builds
#ifdef FRESH_DEBUG
    #define FRESH_ASSERT(condition, message) \
        if (!(condition)) { \
            std::cerr << "Assertion failed: " << message << std::endl; \
            std::abort(); \
        }
#else
    #define FRESH_ASSERT(condition, message)
#endif

// Use static_assert for compile-time checks
static_assert(sizeof(Vertex) == 32, "Vertex size mismatch");
static_assert(std::is_pod<ChunkData>::value, "ChunkData must be POD");
```

### Common Patterns

#### Singleton (use sparingly)
```cpp
class ResourceManager {
public:
    static ResourceManager& getInstance() {
        static ResourceManager instance;
        return instance;
    }
    
    // Delete copy and move
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
private:
    ResourceManager() = default;
    ~ResourceManager() = default;
};
```

#### Factory Pattern
```cpp
class EntityFactory {
public:
    static std::unique_ptr<Entity> create(EntityType type) {
        switch (type) {
            case EntityType::Player: return std::make_unique<Player>();
            case EntityType::Enemy: return std::make_unique<Enemy>();
            default: return nullptr;
        }
    }
};
```

#### Observer Pattern
```cpp
class Observable {
private:
    std::vector<std::function<void()>> observers;
    
public:
    void addObserver(std::function<void()> observer) {
        observers.push_back(observer);
    }
    
    void notify() {
        for (auto& observer : observers) {
            observer();
        }
    }
};
```

### Build Configuration

```cmake
# CMakeLists.txt best practices

# Set minimum version
cmake_minimum_required(VERSION 3.20)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable warnings
if(MSVC)
    add_compile_options(/W4 /WX)  # Warning level 4, warnings as errors
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Debug/Release configurations
if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DFRESH_DEBUG)
endif()

# Organize source files
file(GLOB_RECURSE SOURCES "engine/*.cpp")
file(GLOB_RECURSE HEADERS "engine/*.h")
```

### Git and Version Control

```bash
# .gitignore essentials
build/
*.o
*.a
*.so
*.dll
*.exe
.vscode/
.vs/
CMakeCache.txt
CMakeFiles/

# Commit messages format
# <type>: <subject>
# 
# <body>
# 
# <footer>

# Examples:
feat: Add chunk streaming system
fix: Resolve memory leak in texture loading
docs: Update API documentation for ResourceManager
refactor: Simplify collision detection algorithm
perf: Optimize mesh generation with greedy meshing
test: Add unit tests for noise generator
```

### Code Review Checklist

- [ ] Code follows naming conventions
- [ ] Memory is managed with RAII/smart pointers
- [ ] Functions are small and focused (<50 lines ideal)
- [ ] No memory leaks (use valgrind/sanitizers)
- [ ] Thread-safe where needed
- [ ] Const correctness applied
- [ ] Error handling in place
- [ ] Documentation for public APIs
- [ ] No compiler warnings
- [ ] Performance considered for hot paths
- [ ] Unit tests added/updated
- [ ] Works on multiple platforms

### Development Workflow

1. **Branch Strategy**: Use feature branches
   ```bash
   git checkout -b feature/chunk-streaming
   git checkout -b fix/memory-leak
   ```

2. **Incremental Development**: Small, focused commits
   ```bash
   git commit -m "feat: Add ChunkStreamer class skeleton"
   git commit -m "feat: Implement chunk loading queue"
   git commit -m "feat: Add background generation thread"
   ```

3. **Testing**: Test early and often
   ```bash
   mkdir build && cd build
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   make -j$(nproc)
   ./FreshVoxelEngine
   ```

4. **Profiling**: Profile before optimizing
   ```batch
   # Use Visual Studio Profiler
   # Debug -> Performance Profiler
   # Select CPU Usage, Memory Usage, or GPU Usage
   
   # Or use command line
   vsperf /launch FreshVoxelEngine.exe
   ```

### Resources

- **C++ Core Guidelines**: https://isocpp.github.io/CppCoreGuidelines/
- **Effective Modern C++**: Scott Meyers
- **Google C++ Style Guide**: https://google.github.io/styleguide/cppguide.html
- **cppreference.com**: Comprehensive C++ reference

---

## Summary of Key Principles

1. **Modern C++**: Use C++20 features, smart pointers, RAII, concepts, ranges
2. **Clear Naming**: Descriptive names following conventions
3. **Memory Safety**: Smart pointers, RAII, no manual new/delete
4. **Const Correctness**: Mark everything const that should be
5. **Performance**: Cache-friendly, move semantics, reserve capacity
6. **Documentation**: Document interfaces and complex logic
7. **Thread Safety**: Protect shared data with mutexes
8. **Testability**: Write testable, modular code
9. **Maintainability**: Keep functions small, follow SOLID
10. **Consistency**: Follow these guidelines throughout

---

**Last Updated**: 2025-11-03
**Version**: 1.0.0
