# 2D Engine Systems Documentation

## Overview

The Fresh Voxel Engine now includes comprehensive 2D game development capabilities alongside its 3D voxel features. This allows developers to create 2D platformers, top-down games, and other 2D game genres using the same engine infrastructure.

## Architecture

The 2D engine follows the Entity Component System (ECS) architecture used throughout Fresh. It consists of:

- **Components**: Data containers attached to entities
- **Systems**: Logic processors that operate on entities with specific component combinations

## Components

### Transform2DComponent

Manages 2D spatial transformation for entities.

**Properties:**
- `position`: World position (glm::vec2)
- `rotation`: Rotation in radians (float)
- `scale`: Scale factor (glm::vec2)
- `layer`: Rendering layer for sorting (int)
- `depth`: Depth within layer for sub-sorting (float)

**Methods:**
- `getTransformMatrix()`: Get 3x3 transformation matrix
- `getForward()`: Get forward direction vector
- `getRight()`: Get right direction vector
- `transformPoint()`: Transform local point to world space
- `inverseTransformPoint()`: Transform world point to local space

**Usage:**
```cpp
Transform2DComponent transform;
transform.position = glm::vec2(100.0f, 200.0f);
transform.setRotationDegrees(45.0f);
transform.scale = glm::vec2(2.0f, 2.0f);
```

### Sprite2DComponent

Handles 2D sprite rendering.

**Properties:**
- `texturePath`: Path to texture file
- `textureID`: Texture handle (managed by renderer)
- `size`: Size in world units
- `pivot`: Pivot point (0,0 = bottom-left, 0.5,0.5 = center)
- `color`: Color tint (RGBA)
- `alpha`: Transparency
- `isSprite`: True if using sprite sheet
- `spriteSheetSize`: Grid dimensions (columns, rows)
- `spriteIndex`: Current sprite in sheet
- `flipX/flipY`: Flip flags
- `visible`: Visibility flag

**Methods:**
- `getSpriteUVs()`: Get UV coordinates for current sprite
- `setSprite(col, row)`: Set current sprite
- `setSpriteSheet(cols, rows)`: Configure sprite sheet

**Usage:**
```cpp
Sprite2DComponent sprite("textures/player.png");
sprite.size = glm::vec2(1.0f, 1.0f);
sprite.pivot = glm::vec2(0.5f, 0.5f);
sprite.setSpriteSheet(8, 4); // 8 columns, 4 rows
sprite.setSprite(0, 0); // First sprite
```

### RigidBody2DComponent

Provides 2D physics simulation.

**Body Types:**
- `Static`: Immovable (infinite mass)
- `Kinematic`: Moves but not affected by forces
- `Dynamic`: Full physics simulation

**Properties:**
- `bodyType`: Type of physics body
- `velocity`: Linear velocity
- `angularVelocity`: Rotation speed
- `mass`: Body mass
- `drag`: Linear damping
- `angularDrag`: Angular damping
- `restitution`: Bounciness (0-1)
- `friction`: Surface friction (0-1)
- `useGravity`: Apply gravity?
- `freezeRotation`: Lock rotation?
- `gravityScale`: Gravity multiplier per axis

**Methods:**
- `addForce(force)`: Apply force
- `addImpulse(impulse)`: Apply instant velocity change
- `addTorque(torque)`: Apply rotational force
- `setMass(mass)`: Update mass
- `clearForces()`: Reset forces
- `getKineticEnergy()`: Calculate kinetic energy

**Usage:**
```cpp
RigidBody2DComponent rb;
rb.bodyType = RigidBody2DComponent::BodyType::Dynamic;
rb.mass = 1.0f;
rb.useGravity = true;
rb.addForce(glm::vec2(100.0f, 0.0f));
```

### Collider2DComponent

Defines collision shapes.

**Shape Types:**
- `Box`: Axis-aligned bounding box
- `Circle`: Circle shape
- `Polygon`: Convex polygon
- `Edge`: Line segment

**Properties:**
- `shapeType`: Type of collision shape
- `boxSize`: Box half-extents
- `circleRadius`: Circle radius
- `vertices`: Polygon points
- `edgeStart/edgeEnd`: Edge endpoints
- `offset`: Position offset from entity
- `isTrigger`: Trigger vs solid collision
- `enabled`: Active flag
- `layer`: Collision layer
- `mask`: Collision mask for filtering

**Static Methods:**
- `createBox(size)`: Create box collider
- `createCircle(radius)`: Create circle collider
- `createPolygon(vertices)`: Create polygon collider
- `createEdge(start, end)`: Create edge collider

**Methods:**
- `getAABB(position, rotation)`: Get bounding box
- `containsPoint(point, position, rotation)`: Point-in-shape test

**Usage:**
```cpp
auto collider = Collider2DComponent::createBox(glm::vec2(1.0f, 1.0f));
collider.isTrigger = false;
collider.layer = 1;
collider.mask = 0xFFFFFFFF;
```

### Animation2DComponent

Manages sprite animations.

**Structures:**
- `AnimationClip`: Named animation with frames, frame rate, and loop flag

**Properties:**
- `clips`: Map of animation clips
- `currentClip`: Currently playing animation
- `currentFrame`: Current frame index
- `playing`: Is playing?
- `paused`: Is paused?
- `playbackSpeed`: Speed multiplier

**Methods:**
- `addClip(name, clip)`: Add animation clip
- `addClipFromRange(name, start, count, cols, fps, loop)`: Create clip from range
- `play(clipName)`: Play animation
- `stop()`: Stop animation
- `pause()/resume()`: Pause/resume
- `getCurrentFrameCoords()`: Get current frame
- `hasFinished()`: Check if finished
- `reset()`: Reset to first frame

**Usage:**
```cpp
Animation2DComponent anim;
anim.addClipFromRange("walk", 0, 0, 8, 8, 10.0f, true);
anim.addClipFromRange("jump", 0, 1, 4, 8, 8.0f, false);
anim.play("walk");
```

### Camera2DComponent

Manages 2D camera view.

**Projection Modes:**
- `Orthographic`: No perspective (standard 2D)
- `Perspective`: Perspective view (for 2.5D effects)

**Properties:**
- `projectionMode`: Projection type
- `orthographicSize`: Camera view height
- `aspectRatio`: Width/height ratio
- `isPrimary`: Main camera flag
- `followTarget`: Entity to follow
- `followOffset`: Follow offset
- `followSmoothing`: Follow smoothing factor
- `useBounds`: Constrain to bounds?
- `boundsMin/boundsMax`: World bounds
- `zoom`: Zoom level
- `minZoom/maxZoom`: Zoom limits

**Methods:**
- `getViewMatrix(position, rotation)`: Get view matrix
- `getProjectionMatrix()`: Get projection matrix
- `screenToWorld(screenPos, cameraPos, screenSize)`: Screen to world conversion
- `worldToScreen(worldPos, cameraPos, screenSize)`: World to screen conversion

**Usage:**
```cpp
Camera2DComponent camera;
camera.orthographicSize = 10.0f;
camera.followTarget = playerEntity;
camera.followSmoothing = 0.1f;
camera.useBounds = true;
camera.boundsMin = glm::vec2(0.0f, 0.0f);
camera.boundsMax = glm::vec2(100.0f, 100.0f);
```

## Systems

### Physics2DSystem

Simulates 2D physics with fixed timestep integration.

**Features:**
- Fixed timestep for stability
- Gravity application
- Force integration
- Velocity integration
- Position/rotation updates
- Drag application

**Configuration:**
```cpp
Physics2DSystem physics(&entityManager);
physics.setGravity(glm::vec2(0.0f, -9.81f));
physics.setFixedTimeStep(1.0f / 60.0f);
```

### Collision2DSystem

Detects and resolves 2D collisions.

**Features:**
- Broad phase (AABB overlap)
- Narrow phase (shape-specific detection)
- Collision resolution with impulses
- Trigger detection
- Collision callbacks
- Layer-based filtering

**Supported Collisions:**
- Box-Box
- Circle-Circle
- Box-Circle

**Usage:**
```cpp
Collision2DSystem collision(&entityManager);
collision.onCollision([](const Collision2D& col) {
    // Handle collision
});
collision.onTrigger([](const Collision2D& col) {
    // Handle trigger
});
```

### Animation2DSystem

Updates sprite animations.

**Features:**
- Frame progression based on time
- Loop handling
- Animation completion detection
- Sprite component synchronization

**Usage:**
```cpp
Animation2DSystem animation(&entityManager);
animation.update(deltaTime);
```

### Camera2DSystem

Manages camera behavior.

**Features:**
- Target following with smoothing
- Boundary constraints
- Zoom clamping
- Aspect ratio management

**Usage:**
```cpp
Camera2DSystem cameraSystem(&entityManager);
cameraSystem.setScreenAspectRatio(16.0f / 9.0f);
Entity primaryCamera = cameraSystem.getPrimaryCamera();
```

## Example: Creating a 2D Player

```cpp
// Create player entity
Entity player = entityManager.createEntity();

// Add transform
auto* transform = player.addComponent<Transform2DComponent>();
transform->position = glm::vec2(0.0f, 0.0f);

// Add sprite
auto* sprite = player.addComponent<Sprite2DComponent>("textures/player.png");
sprite->size = glm::vec2(1.0f, 2.0f);
sprite->setSpriteSheet(8, 4);

// Add physics
auto* rigidbody = player.addComponent<RigidBody2DComponent>();
rigidbody->mass = 1.0f;
rigidbody->useGravity = true;

// Add collider
auto* collider = player.addComponent<Collider2DComponent>();
*collider = Collider2DComponent::createBox(glm::vec2(0.5f, 1.0f));

// Add animation
auto* animation = player.addComponent<Animation2DComponent>();
animation->addClipFromRange("idle", 0, 0, 1, 8, 1.0f, true);
animation->addClipFromRange("walk", 1, 0, 8, 8, 10.0f, true);
animation->addClipFromRange("jump", 2, 0, 4, 8, 8.0f, false);
animation->play("idle");

// Create camera
Entity cameraEntity = entityManager.createEntity();
auto* cameraTransform = cameraEntity.addComponent<Transform2DComponent>();
auto* camera = cameraEntity.addComponent<Camera2DComponent>();
camera->followTarget = player;
camera->followSmoothing = 0.1f;
```

## Best Practices

1. **Layer Management**: Use layers for rendering order and collision filtering
2. **Fixed Timestep**: Physics2DSystem uses fixed timestep for deterministic behavior
3. **Collision Masks**: Use layer masks to optimize collision detection
4. **Sprite Sheets**: Prefer sprite sheets over individual textures for better performance
5. **Camera Bounds**: Use camera bounds to prevent showing areas outside the game world
6. **Triggers**: Use trigger colliders for zones, pickups, and non-physical interactions

## Integration with 3D Engine

The 2D systems are designed to coexist with the 3D voxel engine:

- 2D entities can exist in the same world as 3D entities
- Use different layers to separate 2D and 3D rendering
- Camera systems are separate (Camera2D vs standard Camera)
- Physics systems are independent (Physics2D vs Physics3D)

## Performance Considerations

- **Collision Detection**: O(n²) broad phase can be expensive with many entities. Consider spatial partitioning for large worlds.
- **Animation Updates**: Only active animations are processed
- **Sprite Batching**: Renderer should batch sprites with same texture
- **Fixed Timestep**: Physics uses fixed timestep, may run multiple times per frame

## Future Enhancements

Potential areas for expansion:

- Spatial partitioning for collision detection (quadtree, grid)
- More collision shapes (capsule, tilemap)
- Joint system for connected bodies
- Particle system integration
- 2D lighting system
- Tile map rendering
- Path finding integration
