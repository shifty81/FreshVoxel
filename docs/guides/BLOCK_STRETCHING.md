# Block Stretching Feature Documentation

## Overview
The block stretching feature allows interactive manipulation of voxel blocks in builder mode, with automatic structural integrity constraints. This feature is also integrated into procedural generation for creating varied asteroid and station designs.

## Key Features

### 1. Interactive Block Stretching
- **Corner Stretching**: Grab and drag any of the 8 corners of a block to stretch it
- **Structural Integrity**: Automatic validation ensures blocks don't exceed material limits
- **Real-time Preview**: See changes as you drag
- **Snap to Grid**: Optional grid snapping for precise placement

### 2. Structural Integrity System
- **Maximum Stretch Ratio**: Each material has a `maxStretchRatio` (default: 5.0x)
- **Per-Dimension Tracking**: Stretch is calculated independently for X, Y, Z axes
- **Validation**: Blocks automatically prevent stretching beyond safe limits
- **Visual Feedback**: Blocks indicate when approaching structural limits

### 3. Procedural Generation Integration
- **Asteroid Shapes**: 5 different shapes with varied stretch patterns
  - Sphere: Uniform, no stretching
  - Cube: Slight variations (0.2x - 1.0x)
  - Triangular: Flattened in one dimension
  - Elongated: Stretched 2x-5x along one axis
  - Irregular: Random stretching in all dimensions
- **Station Variety**: Different build designs use stretching for unique aesthetics
- **Deterministic**: Same seed produces same stretched shapes

## API Reference

### VoxelBlock Class

#### Stretch Methods
```cpp
// Check if a stretch is valid
bool canStretch(const glm::vec3& newSize) const;

// Apply a new size to the block
bool stretch(const glm::vec3& newSize);

// Stretch by dragging a corner (0-7)
bool stretchCorner(int cornerIndex, const glm::vec3& delta);

// Get corner position (0-7)
glm::vec3 getCorner(int cornerIndex) const;

// Get current stretch ratio
float getStretchRatio() const;

// Check if block is structurally sound
bool isStructurallySound() const;
```

#### Corner Indexing
Corners are indexed using binary encoding:
- Bit 0: X axis (0=min, 1=max)
- Bit 1: Y axis (0=min, 1=max)
- Bit 2: Z axis (0=min, 1=max)

Examples:
- Corner 0 (000): min X, min Y, min Z
- Corner 7 (111): max X, max Y, max Z
- Corner 3 (011): min X, max Y, max Z

### BlockBuilder Class

#### Builder Mode
```cpp
// Enable/disable builder mode
void enable();
void disable();
bool isEnabled() const;

// Select a block for editing
void selectBlock(VoxelBlock* block);
void deselectBlock();

// Interactive stretching
bool startStretch(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);
bool updateStretch(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);
bool endStretch();
void cancelStretch();
```

#### Configuration
```cpp
// Set maximum stretch ratio (overrides material limit)
void setMaxStretchRatio(float ratio);

// Enable/disable snap to grid
void setSnapToGrid(bool snap);
void setGridSize(float size);
```

#### Procedural Generation
```cpp
// Apply random stretch to a block
void applyRandomStretch(VoxelBlock& block, 
                       float minRatio = 0.5f, 
                       float maxRatio = 3.0f, 
                       int seed = 0);
```

## Usage Examples

### Example 1: Interactive Stretching
```cpp
#include "editor/BlockBuilder.h"
#include "voxelship/VoxelBlock.h"

// Create builder
BlockBuilder builder;
builder.enable();

// Create a block
VoxelMaterial material;
material.maxStretchRatio = 5.0f;
VoxelBlock block(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), material);

// Select and stretch
builder.selectBlock(&block);

// User clicks and drags
glm::vec3 rayOrigin = camera.getPosition();
glm::vec3 rayDir = camera.getRayDirection(mouseX, mouseY);

builder.startStretch(rayOrigin, rayDir);

// As user moves mouse
builder.updateStretch(rayOrigin, rayDir);

// When user releases mouse
builder.endStretch();
```

### Example 2: Procedural Asteroid with Stretching
```cpp
#include "galaxy/GalaxyGenerator.h"

GalaxyGenerator generator;
auto sector = generator.generateSector(0, 0);

// Asteroids are automatically generated with varied shapes and stretching
for (const auto& asteroid : sector->getAsteroids()) {
    // asteroid.shape: Sphere, Cube, Triangular, Elongated, or Irregular
    // asteroid.stretchFactor: Applied stretching (within limits)
    
    // Use these values for rendering
    renderAsteroid(asteroid.position, 
                  asteroid.size * asteroid.stretchFactor,
                  asteroid.shape);
}
```

### Example 3: Manual Stretch with Validation
```cpp
VoxelMaterial strongMaterial;
strongMaterial.maxStretchRatio = 10.0f;  // Can stretch more

VoxelBlock block(glm::vec3(0, 0, 0), glm::vec3(2, 2, 2), strongMaterial);

// Try to stretch
glm::vec3 newSize(20, 2, 2);  // 10x stretch in X

if (block.canStretch(newSize)) {
    block.stretch(newSize);
    // Success!
} else {
    // Exceeds structural limits
}

// Check current state
float ratio = block.getStretchRatio();  // Returns 10.0
bool sound = block.isStructurallySound();  // Returns true
```

## Structural Integrity Guidelines

### Default Limits
- **Default maxStretchRatio**: 5.0x
- **Recommended range**: 1.0x - 10.0x
- **Extreme stretching**: Use with caution beyond 10.0x

### Material Types (Suggested Values)
- **Armor Plating**: 2.0x (rigid, minimal stretching)
- **Hull Material**: 5.0x (default, balanced)
- **Flexible Components**: 8.0x (wings, fins)
- **Energy Fields**: 15.0x (shields, special effects)

### Best Practices
1. Always check `canStretch()` before applying
2. Use `isStructurallySound()` after modifications
3. Provide visual feedback when approaching limits
4. Store original size for undo functionality
5. Apply stretching gradually in animations

## Integration with Existing Systems

### Galaxy Generation
Asteroid generation now uses shape and stretch factors:
```cpp
// In GalaxyGenerator::generateAsteroids()
asteroid.shape = getRandomAsteroidShape(rng);
asteroid.stretchFactor = generateStretchFactor(asteroid.shape, rng);
// stretchFactor respects MAX_STRETCH_RATIO of 5.0
```

### Station Generation
Stations use size multipliers for 4-5x scaling:
```cpp
// In GalaxyGenerator::generateStations()
station.sizeMultiplier = sizeMult(rng);  // 4.0 - 5.0
// Use with docking arms and design variants
```

### Builder Mode
BlockBuilder integrates with existing editor systems:
```cpp
// In EditorManager or WorldEditor
BlockBuilder blockBuilder;
blockBuilder.enable();

// Handle mouse input for stretching
if (mouseButtonPressed) {
    blockBuilder.startStretch(rayOrigin, rayDirection);
}
```

## Testing
Comprehensive tests are provided in:
- `tests/voxelship/VoxelBlockStretchTest.cpp` - Block stretching unit tests
- `tests/galaxy/GalaxyGeneratorEnhancedTest.cpp` - Generation integration tests

Run tests:
```bash
cd build
ctest --output-on-failure
```

## Performance Considerations
- **Stretch Calculations**: O(1) per stretch operation
- **Corner Lookup**: O(1) using binary indexing
- **Validation**: O(1) comparison against material limits
- **Memory**: +12 bytes per block (originalSize vector)

## Future Enhancements
- [ ] Edge and face stretching modes
- [ ] Non-uniform grid snapping
- [ ] Stretch animation system
- [ ] Advanced structural stress visualization
- [ ] Material deformation based on forces
- [ ] Multi-block stretching (stretch connected blocks together)

## See Also
- [GalaxySector.h](../engine/galaxy/GalaxySector.h) - Asteroid and station data structures
- [GalaxyGenerator.h](../engine/galaxy/GalaxyGenerator.h) - Procedural generation
- [VoxelBlock.h](../engine/voxelship/VoxelBlock.h) - Block implementation
- [BlockBuilder.h](../engine/editor/BlockBuilder.h) - Builder mode tools
