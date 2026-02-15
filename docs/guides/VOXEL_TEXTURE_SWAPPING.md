# Voxel Texture Swapping Guide

## Overview

The Fresh Voxel Engine supports custom voxel block textures and materials through **Voxel Material Packs**, similar to Minecraft resource packs. This guide explains the three key approaches for customizing voxel appearance as outlined in the problem statement.

## Three Approaches to Custom Voxel Assets

### 1. Swapping Voxel Textures/Materials (Minecraft Style) ✅ IMPLEMENTED

This is the primary approach implemented for customizing individual voxel blocks.

**How it works:**
- Create or obtain custom textures using pixel art tools (Aseprite, GIMP, Photoshop, etc.)
- Define block types in `voxel_materials.json` manifest
- Map textures to block IDs (VoxelType enum)
- System generates texture atlases for optimization
- Apply materials to voxel chunks during rendering

**Use cases:**
- Custom block textures (stone, dirt, grass, wood, etc.)
- Themed texture sets (medieval, sci-fi, fantasy)
- High-resolution texture packs
- Stylized looks (painterly, cel-shaded, realistic)

**Implementation:**
- `VoxelMaterialPack` class handles pack loading
- `VoxelMaterialPackManager` manages multiple packs with priority
- Integration with `VoxelTextureLoader` for seamless texture access
- Support for face-specific textures (grass blocks with different top/side/bottom)

### 2. Using Custom 3D Models for Specific Assets ✅ ALREADY EXISTS

For non-terrain elements like trees, rocks, buildings, and decorations.

**How it works:**
- Create 3D assets using modeling software (Blender, VoxEdit, MagicaVoxel)
- Import models into asset packs (`.obj`, `.fbx`, `.gltf`, `.glb`, `.dae`)
- Program placement logic via biome rules and placement patterns
- Models are placed as instances during world generation

**Use cases:**
- Trees and vegetation
- Rocks and boulders
- Buildings and structures
- Decorative objects
- Characters and NPCs

**Implementation:**
- `ModularAssetSystem` handles 3D model asset packs
- `AssetPack` class loads and manages model assets
- Biome-based and rule-based placement (Random, Clustered, Grid, NearWater, etc.)
- See `ASSET_PACK_GUIDE.md` for details

### 3. Defining Material/Biome Collections (Smooth Terrain Style) 🔄 PARTIAL

For more realistic terrain with material blending based on terrain properties.

**How it works:**
- Design complex materials with multiple texture maps (albedo, normal, metallic-roughness, AO)
- Define material rules based on terrain properties (height, slope, noise values)
- Blend materials smoothly based on terrain data

**Status:**
- Material properties supported in voxel material packs
- PBR material support (metallic, roughness, normal maps, AO, emissive)
- Biome-based material selection
- Future: Smooth blending based on Marching Cubes or similar

**Implementation:**
- Material properties in `VoxelMaterialDefinition`
- PBR support through `Material` class
- Ready for integration with advanced terrain systems

---

## Creating a Voxel Material Pack

### Step 1: Generate Template

Use the engine's built-in template generator:

```cpp
VoxelMaterialPackManager::createPackTemplate("path/to/MyVoxelPack");
```

This creates:
```
MyVoxelPack/
├── voxel_materials.json   # Material definitions
├── README.md              # Complete documentation
└── textures/              # Texture directory
```

### Step 2: Create Textures

1. **Choose your style**: Realistic, stylized, pixelated, painterly, etc.
2. **Select resolution**: 16x16 (retro), 32x32 (balanced), 64x64 (high detail)
3. **Create tileable textures**: Ensure edges match for seamless tiling
4. **Use appropriate tools**:
   - **Aseprite**: Excellent for pixel art
   - **GIMP**: Free, supports layers and transparency
   - **Photoshop**: Professional tools
   - **Krita**: Free, great for digital painting
   - **Paint.NET**: Simple, Windows-friendly

5. **Save as PNG**: Recommended for transparency support

### Step 3: Define Materials

Edit `voxel_materials.json`:

```json
{
  "name": "MyCustomTextures",
  "version": "1.0.0",
  "author": "Your Name",
  "description": "My custom voxel textures",
  "priority": 0,
  "materials": [
    {
      "voxelType": "Stone",
      "name": "Custom Stone",
      "textures": {
        "all": "textures/stone.png"
      },
      "materialProperties": {
        "roughness": 0.8,
        "metallic": 0.0
      }
    },
    {
      "voxelType": "Grass",
      "name": "Custom Grass",
      "textures": {
        "top": "textures/grass_top.png",
        "bottom": "textures/dirt.png",
        "sides": "textures/grass_side.png"
      }
    }
  ]
}
```

### Step 4: Install and Test

1. Copy your pack folder to the materials directory
2. Launch the engine
3. Check console for pack loading messages
4. Create/load a world to see your textures in action

---

## Supported Voxel Types

You can customize textures for these voxel types:

| Voxel Type | Description | Face-Specific? |
|------------|-------------|----------------|
| `Stone` | Stone blocks | No |
| `Dirt` | Dirt blocks | No |
| `Grass` | Grass blocks | Yes (top/bottom/sides) |
| `Sand` | Sand blocks | No |
| `Water` | Water blocks | No (transparent) |
| `Wood` | Wood/log blocks | Yes (optional) |
| `Leaves` | Leaf blocks | No (transparent) |
| `Bedrock` | Bedrock blocks | No |
| `Snow` | Snow blocks | No |
| `Ice` | Ice blocks | No (transparent) |
| `Cobblestone` | Cobblestone blocks | No |
| `Planks` | Wooden plank blocks | No |
| `Glass` | Glass blocks | No (transparent) |

---

## Advanced Features

### Face-Specific Textures

Some blocks look better with different textures on different faces:

```json
{
  "voxelType": "Grass",
  "textures": {
    "top": "textures/grass_top.png",
    "bottom": "textures/dirt.png",
    "north": "textures/grass_side.png",
    "south": "textures/grass_side.png",
    "east": "textures/grass_side.png",
    "west": "textures/grass_side.png"
  }
}
```

Or use the shorthand:
```json
{
  "voxelType": "Grass",
  "textures": {
    "top": "textures/grass_top.png",
    "bottom": "textures/dirt.png",
    "sides": "textures/grass_side.png"
  }
}
```

### PBR Materials

For realistic lighting and material properties:

```json
{
  "voxelType": "Stone",
  "textures": {
    "all": "textures/stone_albedo.png"
  },
  "materialProperties": {
    "normalMap": "textures/stone_normal.png",
    "metallicRoughnessMap": "textures/stone_mr.png",
    "aoMap": "textures/stone_ao.png",
    "roughness": 0.8,
    "metallic": 0.0,
    "ao": 1.0
  }
}
```

### Transparent/Emissive Blocks

For glass, water, or glowing blocks:

```json
{
  "voxelType": "Water",
  "textures": {
    "all": "textures/water.png"
  },
  "materialProperties": {
    "alpha": 0.6,
    "alphaBlend": true
  }
}
```

```json
{
  "voxelType": "Glass",
  "textures": {
    "all": "textures/glass.png"
  },
  "materialProperties": {
    "alpha": 0.3,
    "alphaBlend": true,
    "roughness": 0.1,
    "metallic": 0.5
  }
}
```

### Multiple Packs with Priority

Load multiple packs with different priorities:

**Base pack** (`priority: -10`):
```json
{
  "name": "BasePack",
  "priority": -10,
  "materials": [...]
}
```

**Override pack** (`priority: 10`):
```json
{
  "name": "HighResOverride",
  "priority": 10,
  "materials": [
    {
      "voxelType": "Stone",
      ...
    }
  ]
}
```

Higher priority packs override lower priority ones for matching voxel types.

---

## Texture Guidelines

### Resolution

- **16x16**: Retro/pixel art style, low memory, best performance
- **32x32**: Balanced detail and performance (recommended)
- **64x64**: High detail, more memory usage
- **128x128+**: Very high detail, significant memory impact

### Format

- **PNG**: Recommended (supports transparency, lossless)
- **JPG**: For opaque textures only, smaller file size
- **TGA**: Alternative with transparency support

### Design Tips

1. **Make textures tileable**: Test by placing 2x2 in an image editor
2. **Use consistent style**: All blocks should match visually
3. **Consider lighting**: Textures should work in different light conditions
4. **Test in-game**: What looks good in an editor may differ in 3D
5. **Provide variants**: Create multiple color variations
6. **Include normals**: For realistic lighting (PBR)

### Performance

- Lower resolutions = better performance
- Use texture atlases (engine handles this automatically)
- Minimize unique textures (reuse where possible)
- Compress textures appropriately for your target platform

---

## Integration with Game Engine

### In Your Code

```cpp
// Initialize material pack manager
auto& matPackMgr = VoxelMaterialPackManager::getInstance();
matPackMgr.initialize("path/to/materials");

// The manager automatically scans and loads all packs

// Get texture for a voxel type
auto stoneTexture = matPackMgr.getTexture(VoxelType::Stone, "all");

// Get material definition
auto* grassMat = matPackMgr.getMaterialDefinition(VoxelType::Grass);
if (grassMat) {
    // Access texture paths, material properties, etc.
}

// Print statistics
matPackMgr.printStats();
```

### In VoxelTextureLoader

The `VoxelTextureLoader` now checks material packs first:

```cpp
VoxelTextureLoader textureLoader;
textureLoader.loadAllTextures();

// This will use material pack textures if available,
// otherwise falls back to default textures
auto texture = textureLoader.getTexture(VoxelType::Stone);
```

---

## Examples

### Example 1: Simple Stone Texture Pack

**voxel_materials.json:**
```json
{
  "name": "SimpleStone",
  "version": "1.0.0",
  "author": "Alice",
  "description": "Custom stone texture",
  "materials": [
    {
      "voxelType": "Stone",
      "name": "Smooth Gray Stone",
      "textures": {
        "all": "textures/stone.png"
      },
      "materialProperties": {
        "roughness": 0.7
      }
    }
  ]
}
```

### Example 2: Complete Grass Block

**voxel_materials.json:**
```json
{
  "name": "DetailedGrass",
  "version": "1.0.0",
  "author": "Bob",
  "description": "High-detail grass block",
  "materials": [
    {
      "voxelType": "Grass",
      "name": "Lush Grass",
      "textures": {
        "top": "textures/grass_top.png",
        "bottom": "textures/dirt.png",
        "sides": "textures/grass_side.png"
      },
      "materialProperties": {
        "normalMap": "textures/grass_normal.png",
        "roughness": 0.9
      }
    }
  ]
}
```

### Example 3: Themed Pack (Sci-Fi)

**voxel_materials.json:**
```json
{
  "name": "SciFiPack",
  "version": "1.0.0",
  "author": "Carol",
  "description": "Futuristic sci-fi textures",
  "priority": 5,
  "materials": [
    {
      "voxelType": "Stone",
      "name": "Metal Plating",
      "textures": {
        "all": "textures/metal_plating.png"
      },
      "materialProperties": {
        "normalMap": "textures/metal_plating_normal.png",
        "metallicRoughnessMap": "textures/metal_plating_mr.png",
        "metallic": 0.9,
        "roughness": 0.3
      }
    },
    {
      "voxelType": "Glass",
      "name": "Energy Shield",
      "textures": {
        "all": "textures/energy_shield.png"
      },
      "materialProperties": {
        "alpha": 0.4,
        "alphaBlend": true,
        "emissive": [0.0, 0.5, 1.0],
        "emissiveStrength": 2.0
      }
    }
  ]
}
```

---

## Troubleshooting

### Pack Not Loading

**Problem:** Material pack doesn't appear in console output

**Solutions:**
- Verify `voxel_materials.json` or `materials.json` exists in pack root
- Check JSON syntax with a validator
- Ensure pack is in the correct materials directory
- Check file permissions

### Textures Not Appearing

**Problem:** Blocks still show default textures

**Solutions:**
- Verify texture file paths are correct and relative to pack root
- Check that texture files actually exist at specified paths
- Ensure voxel type names match exactly (case-sensitive)
- Try increasing pack priority
- Check console for error messages

### Performance Issues

**Problem:** Game runs slowly with material pack

**Solutions:**
- Reduce texture resolution (64x64 → 32x32 → 16x16)
- Ensure textures are power-of-2 dimensions
- Limit number of unique textures
- Check texture file sizes (compress if needed)
- Disable PBR maps if not needed

### Wrong Priority

**Problem:** Wrong pack's textures are showing

**Solutions:**
- Check priority values in all loaded packs
- Higher priority = higher precedence
- Use `printStats()` to see load order
- Unload unwanted packs or adjust priorities

---

## API Reference

### VoxelMaterialPackManager

```cpp
// Singleton access
VoxelMaterialPackManager& mgr = VoxelMaterialPackManager::getInstance();

// Initialize with directory
mgr.initialize("path/to/materials");

// Scan and load all packs
mgr.scanAndLoadPacks();

// Load specific pack
bool success = mgr.loadPack("path/to/pack");

// Unload pack by name
mgr.unloadPack("PackName");

// Get material definition (from highest priority pack)
const VoxelMaterialDefinition* matDef = 
    mgr.getMaterialDefinition(VoxelType::Stone);

// Get texture (from highest priority pack)
std::shared_ptr<Texture> texture = 
    mgr.getTexture(VoxelType::Grass, "top");

// Create template
VoxelMaterialPackManager::createPackTemplate("path/to/new/pack");

// Validate pack
bool valid = mgr.validatePack("path/to/pack");

// Print statistics
mgr.printStats();
```

### VoxelMaterialPack

```cpp
VoxelMaterialPack pack("path/to/pack");

// Load pack
bool success = pack.load();

// Get pack info
std::string name = pack.getName();
std::string version = pack.getVersion();
int priority = pack.getPriority();

// Check if pack overrides a type
bool overrides = pack.overridesVoxelType(VoxelType::Stone);

// Get material definition
const VoxelMaterialDefinition* matDef = 
    pack.getMaterialDefinition(VoxelType::Stone);

// Get texture
std::shared_ptr<Texture> texture = 
    pack.getTexture(VoxelType::Stone, "all");
```

---

## Future Enhancements

Planned features for voxel material system:

1. **Texture Atlas Generation**: Automatic packing of textures into atlases
2. **Animation Support**: Animated textures for water, lava, etc.
3. **Seasonal Variants**: Different textures based on time/biome
4. **Block Variants**: Multiple random textures per block type
5. **Connected Textures**: Context-aware texturing (CTM)
6. **3D Noise Textures**: Volumetric texture generation
7. **Material Blending**: Smooth transitions between materials
8. **UV Mapping Options**: Custom UV coordinates per block
9. **Shader Customization**: Per-material shader overrides
10. **Runtime Reloading**: Hot-reload without restart

---

## See Also

- [ASSET_PACK_GUIDE.md](../ASSET_PACK_GUIDE.md) - Complete asset pack documentation
- [Material.h](../engine/renderer/Material.h) - PBR material implementation
- [VoxelMaterialPack.h](../engine/assets/VoxelMaterialPack.h) - API reference
- [ModularAssetSystem.h](../engine/assets/ModularAssetSystem.h) - 3D asset packs

---

**Happy texture creating!** Share your custom packs with the community!
