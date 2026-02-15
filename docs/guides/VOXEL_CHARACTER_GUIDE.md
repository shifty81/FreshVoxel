# Voxel Character System Guide

## Overview

The Fresh Voxel Engine includes a comprehensive character system for creating animated voxel-based characters. Characters are built from voxels bound to a hierarchical skeleton, allowing for realistic animation and customization.

## Components

### 1. Bone System

The `Bone` class represents a single bone in a skeletal hierarchy.

**Key Features:**
- Hierarchical parent-child relationships
- Local and world-space transformations
- Quaternion-based rotations for smooth animation
- Automatic transform propagation

**Example:**
```cpp
// Create a simple bone chain
auto rootBone = std::make_unique<Bone>("root");
auto childBone = std::make_unique<Bone>("child", rootBone.get());

// Set transforms
childBone->setLocalPosition(glm::vec3(0.0f, 5.0f, 0.0f));
childBone->setLocalRotation(glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0)));

// Get world-space position
glm::vec3 worldPos = childBone->getWorldPosition();
```

### 2. Humanoid Skeleton

The `HumanoidSkeleton` class provides a standard 18-bone humanoid skeleton.

**Bone Structure:**
- **Root** (pelvis/hips)
  - **Spine** → **Chest** → **Head**
  - **Left/Right Shoulder** → **Upper Arm** → **Lower Arm** → **Hand**
  - **Left/Right Upper Leg** → **Lower Leg** → **Foot**

**Example:**
```cpp
HumanoidSkeleton skeleton;
skeleton.initialize(16.0f); // 16 voxels tall

// Access bones by name
Bone* leftArm = skeleton.getBone("left_upper_arm");

// Or by enum
Bone* rightArm = skeleton.getBone(HumanoidSkeleton::BoneName::RightUpperArm);

// Modify bone transforms for animation
leftArm->setLocalRotation(glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 0, 1)));
```

### 3. Voxel Character

The `VoxelCharacter` class combines a skeleton with voxel geometry.

**Features:**
- Procedural character generation
- Customizable body proportions
- Color palette system
- Bone-weighted voxel binding
- Modular body part system

**Example:**
```cpp
VoxelCharacter character;

// Generate with custom parameters
CharacterGenerationParams params;
params.baseHeight = 20;           // 20 voxels tall
params.headScale = 1.2f;          // Larger head
params.limbScale = 0.9f;          // Slimmer limbs
params.skinColor = glm::vec3(0.9f, 0.7f, 0.6f);
params.primaryColor = glm::vec3(0.2f, 0.2f, 0.8f); // Blue clothes
params.seed = 12345;              // For variation

character.generateFromParams(params);

// Position in world
character.setPosition(glm::vec3(0, 0, 0));

// Animate bones
int leftArmIndex = character.getBoneIndex("LeftArm");
character.setBoneRotation(leftArmIndex, glm::vec3(0, 0, glm::radians(45.0f)));

// Get voxels for rendering
auto voxels = character.getTransformedVoxels();
```

## Customization

### Body Proportions

Adjust character appearance through proportions:

```cpp
CharacterGenerationParams params;
params.baseHeight = 16;      // Height in voxels (12-20 recommended)
params.headScale = 1.0f;     // Head size multiplier (0.5-1.5)
params.torsoScale = 1.2f;    // Torso thickness (0.8-1.2)
params.limbScale = 0.9f;     // Arm/leg thickness (0.8-1.2)
params.bulky = true;         // Muscular build
```

### Color Palette

Customize character colors:

```cpp
params.skinColor = glm::vec3(0.8f, 0.6f, 0.5f);        // Skin tone
params.primaryColor = glm::vec3(0.2f, 0.4f, 0.8f);     // Clothing/armor
params.secondaryColor = glm::vec3(0.3f, 0.3f, 0.3f);   // Accent color
```

### Modular Assembly

Build characters from pre-made body parts:

```cpp
std::vector<BodyPartTemplate> parts;

// Load or create body part templates
BodyPartTemplate head;
head.type = BodyPartType::Head;
// ... fill with voxels ...
parts.push_back(head);

// Assemble character
character.assembleFromParts(parts);
```

## Animation

### Basic Animation

Animate by rotating bones:

```cpp
// Walking animation (simplified)
float walkCycle = sin(time * 2.0f);

int leftLeg = character.getBoneIndex("LeftLeg");
int rightLeg = character.getBoneIndex("RightLeg");

character.setBoneRotation(leftLeg, glm::vec3(walkCycle * 0.5f, 0, 0));
character.setBoneRotation(rightLeg, glm::vec3(-walkCycle * 0.5f, 0, 0));

// Update character
character.update(deltaTime);
```

### T-Pose Reset

Reset skeleton to default T-pose:

```cpp
skeleton.resetToPose();
```

## Integration with World

### Rendering Characters

```cpp
VoxelCharacter character;
character.setPosition(glm::vec3(worldX, worldY, worldZ));

// Get voxels in world space
auto voxels = character.getTransformedVoxels();

for (const auto& voxel : voxels) {
    // Render voxel at voxel.position with voxel.color
    renderVoxel(voxel.position, voxel.color);
}
```

### Multiple Characters

```cpp
std::vector<VoxelCharacter> characters;

// Create diverse characters
for (int i = 0; i < 10; ++i) {
    CharacterGenerationParams params;
    params.seed = i * 1000;  // Different seed for variation
    params.baseHeight = 14 + (i % 5);  // Vary height
    
    VoxelCharacter character;
    character.generateFromParams(params);
    character.setPosition(glm::vec3(i * 5, 0, 0));
    
    characters.push_back(std::move(character));
}
```

## Performance Considerations

1. **Voxel Count**: Characters typically have 200-500 voxels. Optimize rendering for multiple characters.

2. **Bone Updates**: Only update bone transforms when animation changes.

3. **Level of Detail**: Consider reducing voxel density for distant characters.

4. **Instancing**: Use instanced rendering for characters with identical geometry.

## Future Features

The character system is designed to support future enhancements:

- **Animation System**: Keyframe animation, blending, state machines
- **IK (Inverse Kinematics)**: Foot placement, hand IK, look-at constraints
- **Weapon/Tool Handling**: Dynamic attachment system
- **Physics**: Ragdoll, cloth simulation, hit reactions
- **Body Part Templates**: Library of pre-made parts for mixing and matching

## See Also

- `engine/character/Bone.h` - Bone class API
- `engine/character/HumanoidSkeleton.h` - Humanoid skeleton API
- `engine/character/VoxelCharacter.h` - Character class API
- `tests/character/` - Unit tests and examples
