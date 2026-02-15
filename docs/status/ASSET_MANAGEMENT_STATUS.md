# Asset Management System - Status & Roadmap

This document outlines the current state of the asset management system and what remains to be implemented for complete drop-in asset management functionality.

## ✅ Currently Implemented

### Asset Pack System
- ✅ **Voxel Material/Texture Packs**: Complete system for custom block textures
  - Auto-discovery of asset packs in `asset_packs/` directory
  - JSON manifest parsing (`voxel_materials.json`)
  - Texture loading and atlas generation
  - Priority-based pack ordering
  - Template generation via `VoxelMaterialPackManager::createPackTemplate()`
  - Complete example pack with all 14 voxel types

- ✅ **3D Model Asset Packs**: Modular asset system for objects/decorations
  - Auto-discovery via `ModularAssetSystem`
  - JSON manifest parsing (`manifest.json`)
  - Support for .obj, .fbx, .gltf, .glb, .dae formats
  - 7 placement rules (biome-based, distance-based, etc.)
  - Template generation via `ModularAssetSystem::createAssetPackTemplate()`

### Asset Manager Core
- ✅ **AssetManager class**: Core asset management infrastructure
  - Singleton pattern for global access
  - Asset type detection by file extension
  - Lua configuration integration
  - Asset validation
  - Import callbacks

### Content Browser UI
- ✅ **ContentBrowserPanel**: Basic asset browsing
  - Directory navigation
  - Asset grid view with icons
  - Search/filter functionality
  - Asset details panel
  - Delete and rename operations
  - Basic import via `importAsset()` method

### Editor Integration
- ✅ **Menu Integration**: Import Assets menu item
  - Connected to ContentBrowserPanel
  - Callback system in place
  - Triggers `showImportDialog()` when clicked

## 🚧 Partially Implemented (Needs Work)

### File Dialog Integration
- ⚠️ **Native File Picker**: Currently just logs a message
  - `ContentBrowserPanel::showImportDialog()` exists but doesn't open a real file dialog
  - **Needed**: Integration with native file dialogs (Windows, Linux, macOS)
  - **Options**: 
    - Use ImGui's file dialog extensions
    - Integrate NFD (Native File Dialog) library
    - Platform-specific implementations

### Drag-and-Drop Support
- ⚠️ **GLFW Drag-and-Drop**: Window level support exists but not wired
  - GLFW supports `glfwSetDropCallback()`
  - **Needed**: Connect drop callback to AssetManager::ProcessNewAsset()
  - **Needed**: Visual feedback during drag operation
  - **Needed**: File type validation before accepting drop

### Asset Import Pipeline
- ⚠️ **Import Processing**: Basic copy operation only
  - Current: `ContentBrowserPanel::importAsset()` just copies files
  - **Needed**: Connect to `AssetManager::ProcessNewAsset()` for proper processing
  - **Needed**: Asset type detection and categorization
  - **Needed**: Automatic folder organization (textures/, models/, sounds/, etc.)
  - **Needed**: Import settings dialog (compression, quality, etc.)

## ❌ Not Yet Implemented

### Essential Features for Drop-In Asset Management

#### 1. Real-Time Asset Hot-Reload
- ❌ **File System Watching**
  - Watch asset directories for file changes
  - Automatically reload modified assets
  - Update rendered content without restart
  - **Implementation**: Use C# FileSystemWatcher (mentioned in AssetManager comments) or native solution

#### 2. Asset Preview System
- ❌ **Texture Preview**: Show texture thumbnails in Content Browser
- ❌ **3D Model Preview**: Render 3D models in preview window
- ❌ **Audio Preview**: Play sound samples
- ❌ **Material Preview**: Show material properties visually

#### 3. Asset Import Wizard
- ❌ **Multi-Step Import Dialog**
  - Step 1: Select files (file picker or drag-drop)
  - Step 2: Configure import settings per asset type
  - Step 3: Choose destination folder
  - Step 4: Preview and confirm
  - Step 5: Import with progress bar

#### 4. Asset Type-Specific Import Settings

**For Textures:**
- ❌ Compression options (DXT, BC7, etc.)
- ❌ Mipmap generation
- ❌ Max resolution limits
- ❌ Format conversion (PNG, DDS, etc.)

**For 3D Models:**
- ❌ Scale adjustments
- ❌ Axis conversion (Y-up to Z-up, etc.)
- ❌ LOD generation
- ❌ Material extraction
- ❌ Animation import settings

**For Audio:**
- ❌ Format conversion (WAV, OGG, etc.)
- ❌ Compression settings
- ❌ Volume normalization
- ❌ Looping settings

#### 5. Asset Dependencies & References
- ❌ **Dependency Tracking**: Know which assets are used where
- ❌ **Reference Counting**: Track usage of assets in scenes
- ❌ **Broken Reference Detection**: Find and fix missing assets
- ❌ **Bulk Operations**: Update multiple references at once

#### 6. Asset Metadata & Tagging
- ❌ **Custom Metadata**: User-defined properties per asset
- ❌ **Tag System**: Categorize assets with tags
- ❌ **Search by Metadata**: Find assets by properties
- ❌ **Batch Tagging**: Apply tags to multiple assets

#### 7. Asset Pack Management UI
- ❌ **Pack Manager Panel**: View all installed asset packs
- ❌ **Enable/Disable Packs**: Toggle packs without deleting
- ❌ **Pack Priority Editor**: Change load order visually
- ❌ **Pack Creation Wizard**: GUI for creating new packs
- ❌ **Pack Export**: Package packs for distribution

#### 8. Advanced Import Features
- ❌ **Batch Import**: Import multiple files at once
- ❌ **Folder Import**: Import entire folder structures
- ❌ **Import Presets**: Save and reuse import settings
- ❌ **Auto-Naming**: Smart naming conventions
- ❌ **Duplicate Detection**: Warn about duplicate assets

#### 9. Asset Validation & QA
- ❌ **Format Validation**: Check file format correctness
- ❌ **Size Limits**: Enforce maximum file sizes
- ❌ **Naming Convention Checks**: Validate asset names
- ❌ **Texture Power-of-Two Check**: Warn about non-POT textures
- ❌ **Asset Health Report**: Overall asset quality metrics

#### 10. Version Control Integration
- ❌ **Git Integration**: Show git status of assets
- ❌ **Conflict Resolution**: Handle asset conflicts
- ❌ **Change History**: View asset modification history

## 🎯 Priority Implementation Roadmap

### Phase 1: Essential Drop-In Functionality (High Priority)
1. **Native File Dialog Integration**
   - Add NFD or platform-specific dialogs
   - Replace `showImportDialog()` placeholder
   - Support multi-file selection

2. **Drag-and-Drop Implementation**
   - Wire GLFW drop callback
   - Add visual drop zone feedback
   - Connect to AssetManager pipeline

3. **Import Pipeline Integration**
   - Connect ContentBrowserPanel to AssetManager::ProcessNewAsset()
   - Implement proper asset type detection
   - Add automatic folder organization

4. **Asset Preview - Textures**
   - Generate thumbnails for images
   - Show preview in Content Browser
   - Cache previews for performance

### Phase 2: Enhanced User Experience (Medium Priority)
5. **Import Progress Feedback**
   - Show import progress bar
   - Display current file being processed
   - Allow cancellation

6. **Basic Import Settings**
   - Texture compression toggle
   - Model scale adjustment
   - Destination folder selection

7. **Asset Pack UI Panel**
   - List all installed packs
   - Enable/disable packs
   - View pack information

8. **File System Watching**
   - Implement hot-reload for modified assets
   - Show notification when assets change
   - Auto-refresh Content Browser

### Phase 3: Advanced Features (Lower Priority)
9. **3D Model Preview**
   - Render models in preview window
   - Interactive camera controls
   - Show wireframe option

10. **Asset Metadata System**
    - Tag support
    - Custom properties
    - Search by metadata

11. **Dependency Tracking**
    - Track asset usage
    - Find references
    - Update references tool

12. **Advanced Import Wizard**
    - Multi-step import process
    - Import presets
    - Batch operations

## 🔧 Technical Implementation Notes

### File Dialog Options
**Option 1: Native File Dialog (NFD)** ⭐ Recommended
- Cross-platform (Windows, Linux, macOS)
- Lightweight (single header + impl)
- No external dependencies beyond system APIs
- Installation: `vcpkg install nativefiledialog`

**Option 2: ImGui File Dialog Extensions**
- Pure ImGui implementation
- No additional dependencies
- More styling control
- May look less native

**Option 3: Platform-Specific**
- Best native look and feel
- More code to maintain
- Need separate implementations per platform

### Drag-and-Drop Implementation
```cpp
// In Window.cpp initialization:
glfwSetDropCallback(m_window, [](GLFWwindow* window, int count, const char** paths) {
    auto* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine && engine->getAssetManager()) {
        for (int i = 0; i < count; i++) {
            engine->getAssetManager()->ProcessNewAsset(paths[i], "auto");
        }
    }
});
```

### Hot-Reload File Watching
**Option 1: C++ File System Watcher**
- Use `std::filesystem` with polling
- Cross-platform but less efficient

**Option 2: Platform-Specific Watchers**
- Windows: ReadDirectoryChangesW
- Linux: inotify
- macOS: FSEvents

**Option 3: C# FileSystemWatcher** ⭐ Mentioned in AssetManager
- Use .NET FileSystemWatcher through dotnet interop
- Most efficient
- Requires .NET runtime

### Asset Preview Rendering
- Create off-screen render target
- Render preview to texture
- Display texture in ImGui window
- Cache rendered previews
- Regenerate when asset changes

## 📊 Completion Status

| Feature Category | Status | Completion |
|-----------------|--------|------------|
| Asset Pack System | ✅ Complete | 100% |
| Asset Manager Core | ✅ Complete | 100% |
| Content Browser UI | 🟨 Functional | 60% |
| File Dialog | ❌ Not Started | 0% |
| Drag-and-Drop | ❌ Not Started | 0% |
| Asset Preview | ❌ Not Started | 0% |
| Import Pipeline | 🟨 Basic | 30% |
| Hot-Reload | ❌ Not Started | 0% |
| Import Settings | ❌ Not Started | 0% |
| Dependency Tracking | ❌ Not Started | 0% |
| Metadata/Tags | ❌ Not Started | 0% |
| Pack Management UI | ❌ Not Started | 0% |
| Batch Operations | ❌ Not Started | 0% |

**Overall Completion: ~35%**

## 🎯 Next Steps for Contributors

To work on drop-in asset management, start with Phase 1 items:

1. **Add Native File Dialog** (Highest Impact)
   - Install NFD via vcpkg
   - Replace showImportDialog() implementation
   - Test on Windows/Linux/macOS

2. **Implement Drag-and-Drop** (High Impact)
   - Add GLFW drop callback
   - Connect to AssetManager
   - Add visual feedback

3. **Wire Import Pipeline** (High Impact)
   - Update ContentBrowserPanel::importAsset()
   - Call AssetManager::ProcessNewAsset()
   - Add proper error handling

4. **Add Texture Previews** (Medium Impact)
   - Generate thumbnails on import
   - Display in Content Browser grid
   - Cache for performance

## 📚 Related Documentation

- [ASSET_PACK_GUIDE.md](ASSET_PACK_GUIDE.md) - Creating asset packs
- [asset_packs/README.md](asset_packs/README.md) - Asset pack directory info
- [docs/EDITOR_INTEGRATION.md](docs/EDITOR_INTEGRATION.md) - Editor features
- [engine/assets/AssetManager.h](engine/assets/AssetManager.h) - AssetManager API

## 🤝 Contributing

Want to implement any of these features? Check out:
1. This document for what needs to be done
2. Existing code in `engine/ui/ContentBrowserPanel.cpp` for reference
3. `engine/assets/AssetManager.h` for the core API
4. Create a PR with your implementation!

---

*Last Updated: 2025-11-14*
*Status: Living Document - Update as features are implemented*
