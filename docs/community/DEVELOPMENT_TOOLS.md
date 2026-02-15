# Development Tools and Standards

This document describes the development tools, code formatting standards, and automation workflows available in the Fresh Voxel Engine project.

## Table of Contents

- [Code Formatting](#code-formatting)
- [API Documentation](#api-documentation)
- [Release Process](#release-process)
- [Editor Configuration](#editor-configuration)

---

## Code Formatting

### Overview

The project uses **clang-format** for consistent C++ code formatting across all contributors. The formatting rules are defined in `.clang-format` at the repository root.

### Key Formatting Standards

- **Style Base**: LLVM with customizations
- **C++ Standard**: C++20
- **Indentation**: 4 spaces (no tabs)
- **Line Length**: 100 characters maximum
- **Braces**: K&R style (opening brace on same line, except for functions)

### Naming Conventions

| Type | Convention | Example |
|------|------------|---------|
| Classes/Structs | PascalCase | `VoxelWorld` |
| Functions/Methods | camelCase | `generateChunk()` |
| Variables | camelCase | `chunkSize` |
| Constants | UPPER_SNAKE_CASE | `MAX_CHUNK_SIZE` |
| Private Members | m_ prefix | `m_vertexBuffer` |

### Using clang-format

#### Install clang-format

**Ubuntu/Debian:**
```bash
sudo apt-get install clang-format
```

**macOS:**
```bash
brew install clang-format
```

**Windows:**
Download from [LLVM Releases](https://releases.llvm.org/download.html) or use Visual Studio's built-in clang-format.

#### Format Files

**Format a single file:**
```bash
clang-format -i engine/core/Engine.cpp
```

**Format all C++ files:**
```bash
find engine/ -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

**Check formatting without modifying:**
```bash
clang-format --dry-run --Werror engine/core/Engine.cpp
```

#### IDE Integration

**Visual Studio Code:**
1. Install the "C/C++" extension
2. Open Settings (Ctrl+,)
3. Search for "format on save"
4. Enable "Editor: Format On Save"

**Visual Studio 2022:**
1. Tools → Options → Text Editor → C/C++ → Formatting
2. Check "Automatically format on paste"
3. Check "Automatically format completed statement on ;"

**CLion:**
1. File → Settings → Editor → Code Style → C/C++
2. Set scheme to "Project"
3. Check "Enable ClangFormat"

---

## API Documentation

### Overview

The project uses **Doxygen** to generate comprehensive API documentation from code comments. The configuration is in `Doxyfile` at the repository root.

### Documentation Style

We use Javadoc-style comments with Doxygen tags:

```cpp
/**
 * @brief Generates terrain for a chunk using procedural generation
 * 
 * Uses multi-octave Perlin noise to create realistic terrain with biomes,
 * caves, and height variations.
 * 
 * @param chunkX The X coordinate of the chunk in world space
 * @param chunkZ The Z coordinate of the chunk in world space
 * @param seed The world seed for reproducible generation
 * @return std::unique_ptr<Chunk> The generated chunk
 * @throws std::runtime_error If chunk generation fails
 * 
 * @note This function is thread-safe
 * @see TerrainGenerator, NoiseGenerator
 */
std::unique_ptr<Chunk> generateChunk(int chunkX, int chunkZ, uint32_t seed);
```

### Common Doxygen Tags

| Tag | Purpose | Example |
|-----|---------|---------|
| `@brief` | Short description | `@brief Creates a new voxel world` |
| `@param` | Parameter description | `@param chunkSize Size of each chunk` |
| `@return` | Return value description | `@return Pointer to created chunk` |
| `@throws` | Exception description | `@throws std::bad_alloc On memory error` |
| `@note` | Additional notes | `@note Thread-safe` |
| `@warning` | Warning message | `@warning Must be called from main thread` |
| `@see` | Related items | `@see ChunkManager` |
| `@deprecated` | Marks deprecated code | `@deprecated Use generateChunk() instead` |

### Generating Documentation

#### Install Doxygen

**Ubuntu/Debian:**
```bash
sudo apt-get install doxygen graphviz
```

**macOS:**
```bash
brew install doxygen graphviz
```

**Windows:**
Download from [Doxygen Downloads](https://www.doxygen.nl/download.html)

#### Generate HTML Documentation

```bash
# From repository root
doxygen Doxyfile
```

This creates documentation in `docs/api/html/`. Open `docs/api/html/index.html` in a browser.

#### Continuous Documentation

Documentation is automatically generated and included in GitHub releases when you tag a version (see [Release Process](#release-process)).

---

## Release Process

### Overview

The project uses GitHub Actions to automate the release process. When you push a version tag, it automatically:

1. Creates a GitHub release
2. Builds binaries for Linux, Windows, and macOS
3. Generates API documentation
4. Uploads all artifacts to the release

### Creating a Release

#### 1. Update Version Number

Update the version in `CMakeLists.txt`:

```cmake
project(FreshVoxelEngine VERSION 0.2.0 LANGUAGES CXX)
```

Update the version in `vcpkg.json`:

```json
{
  "version": "0.2.0",
  ...
}
```

Update the version in `Doxyfile`:

```
PROJECT_NUMBER = 0.2.0
```

#### 2. Update CHANGELOG.md

Add a new section for the release:

```markdown
## [0.2.0] - 2025-11-10

### Added
- New feature X
- New feature Y

### Changed
- Modified feature Z

### Fixed
- Bug fix A
- Bug fix B
```

#### 3. Commit and Push Changes

```bash
git add CMakeLists.txt vcpkg.json Doxyfile CHANGELOG.md
git commit -m "Bump version to 0.2.0"
git push origin main
```

#### 4. Create and Push Tag

```bash
# Create annotated tag
git tag -a v0.2.0 -m "Release version 0.2.0"

# Push tag to trigger release workflow
git push origin v0.2.0
```

#### 5. Monitor Release Build

1. Go to GitHub → Actions → Release workflow
2. Monitor the build progress
3. Once complete, verify the release at GitHub → Releases

### Release Artifacts

Each release includes:

- `fresh-linux-x64-vX.Y.Z.tar.gz` - Linux binary
- `fresh-windows-x64-vX.Y.Z.zip` - Windows binary
- `fresh-macos-arm64-vX.Y.Z.tar.gz` - macOS binary
- `fresh-docs-vX.Y.Z.tar.gz` - API documentation

### Pre-releases

For alpha, beta, or release candidate versions, use tags like:

- `v0.2.0-alpha.1`
- `v0.2.0-beta.1`
- `v0.2.0-rc.1`

These will automatically be marked as "pre-release" on GitHub.

---

## Editor Configuration

### Overview

The project includes an `.editorconfig` file that ensures consistent coding styles across different editors and IDEs.

### Supported Settings

The EditorConfig defines standards for:

- **Character Encoding**: UTF-8
- **Line Endings**: LF (Unix-style) for most files, CRLF for Windows scripts
- **Indentation**: Spaces (4 for C++, 2 for YAML/JSON)
- **Trailing Whitespace**: Automatically trimmed
- **Final Newline**: Automatically inserted

### Editor Support

**Visual Studio Code:**
- Install the "EditorConfig for VS Code" extension
- Settings are applied automatically

**Visual Studio 2022:**
- EditorConfig is supported natively
- No additional setup needed

**CLion:**
- EditorConfig is supported natively
- Enable in Settings → Editor → Code Style

**Sublime Text:**
- Install the "EditorConfig" package via Package Control

**Atom:**
- Install the "editorconfig" package

### File-Specific Settings

| File Type | Indent | Line Length |
|-----------|--------|-------------|
| C++/C (.cpp, .h) | 4 spaces | 100 chars |
| C# (.cs) | 4 spaces | 120 chars |
| YAML (.yml) | 2 spaces | - |
| JSON (.json) | 2 spaces | - |
| Markdown (.md) | N/A | No limit |
| Shaders (.vert, .frag) | 4 spaces | - |

---

## Development Workflow Best Practices

### Before Committing

1. **Format your code:**
   ```bash
   clang-format -i engine/your_file.cpp
   ```

2. **Build and test:**
   ```bash
   cmake --build build
   ./build/FreshVoxelEngineTests
   ```

3. **Check documentation:**
   - Ensure public APIs have Doxygen comments
   - Verify `@param` and `@return` tags are accurate

### Pull Request Checklist

- [ ] Code follows formatting standards (clang-format)
- [ ] All files have proper EditorConfig formatting
- [ ] Public APIs are documented with Doxygen comments
- [ ] Tests pass locally
- [ ] CHANGELOG.md is updated (if applicable)
- [ ] No compiler warnings in your changes

---

## Troubleshooting

### clang-format Issues

**Problem:** clang-format not found
```bash
# Install clang-format (see installation section above)
which clang-format  # Verify installation
```

**Problem:** Different clang-format versions produce different results
- The project targets clang-format 14+
- Install a compatible version or use the version in CI

### Doxygen Issues

**Problem:** Doxygen generation fails
```bash
# Check Doxygen version
doxygen --version  # Should be 1.9.0 or higher

# Verbose output for debugging
doxygen Doxyfile 2>&1 | tee doxygen.log
```

**Problem:** Missing class diagrams
- Install Graphviz: `sudo apt-get install graphviz`
- Set `HAVE_DOT = YES` in Doxyfile

### EditorConfig Issues

**Problem:** Settings not applied
- Ensure your editor has EditorConfig support enabled
- Check that `.editorconfig` is in the repository root
- Restart your editor after installing EditorConfig plugin

---

## Additional Resources

- [clang-format Documentation](https://clang.llvm.org/docs/ClangFormat.html)
- [Doxygen Manual](https://www.doxygen.nl/manual/)
- [EditorConfig Specification](https://editorconfig.org/)
- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Semantic Versioning](https://semver.org/)

---

**Last Updated**: 2025-11-08
**Version**: 1.0
