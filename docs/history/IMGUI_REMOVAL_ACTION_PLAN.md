# ImGui Removal Action Plan

**Date:** 2025-11-21  
**Goal:** Complete removal of ImGui dependency from Fresh Voxel Engine  
**Policy:** Windows Native Win32 UI Only (see WINDOWS_NATIVE_ONLY_POLICY.md)

---

## 📋 Executive Summary

This action plan provides step-by-step instructions to remove ALL ImGui code and dependencies from the Fresh Voxel Engine codebase, replacing it with native Windows Win32 UI.

**Estimated Time:** 2-4 hours  
**Risk Level:** LOW (ImGui is already mostly unused, native Win32 is primary)  
**Testing Required:** Build, startup, and basic editor functionality

---

## ✅ Phase 1: Dependency Removal (30 minutes)

### 1.1 Remove from vcpkg.json

**File:** `vcpkg.json`

**REMOVE this line:**
```json
"imgui"
```

**Result:**
```json
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "name": "fresh-voxel-engine",
  "version": "0.1.0",
  "description": "A modern voxel-based game engine built with C++20 for Windows with DirectX 11/12 support and native Windows UI",
  "homepage": "https://github.com/shifty81/fresh",
  "license": "MIT",
  "dependencies": [
    "glfw3",
    "glew",
    "glm",
    "openal-soft",
    "libvorbis",
    "luajit",
    "gtest",
    "nlohmann-json",
    "tinyobjloader",
    "nativefiledialog-extended"
  ],
  "builtin-baseline": "b27f6bfad367d64f8f0f3296351c5d246e182bb4"
}
```

### 1.2 Remove from CMakeLists.txt

**File:** `CMakeLists.txt`

**REMOVE these sections:**

Around lines 184-193:
```cmake
# ImGui for UI (optional, will migrate to native Win32 UI fully)
find_package(imgui QUIET CONFIG)
if(imgui_FOUND)
    message(STATUS "ImGui found - UI panels will be available")
    set(IMGUI_AVAILABLE TRUE)
else()
    message(WARNING "ImGui not found. Some UI panels will not be available.")
    message(WARNING "  vcpkg install imgui:x64-windows")
    set(IMGUI_AVAILABLE FALSE)
endif()
```

Around lines 662-667:
```cmake
# Link ImGui (UI library)
if(IMGUI_AVAILABLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
    target_compile_definitions(${PROJECT_NAME} PRIVATE FRESH_IMGUI_AVAILABLE)
    message(STATUS "ImGui enabled for UI panels")
endif()
```

---

## ✅ Phase 2: Remove ImGui Source Files (15 minutes)

### 2.1 Delete ImGui Wrapper Files

```bash
# Delete ImGui context wrapper
rm engine/ui/ImGuiContext.h
rm engine/ui/ImGuiContext.cpp
```

### 2.2 Archive ImGui-based Panel Files (Optional)

If these files contain ImGui implementations, move to archived/ directory:

```bash
mkdir -p archived/imgui-panels
mv engine/ui/MainMenuPanel.h archived/imgui-panels/ 2>/dev/null || true
mv engine/ui/MainMenuPanel.cpp archived/imgui-panels/ 2>/dev/null || true
```

**Note:** Check these files first - they may already use Win32 native implementations.

---

## ✅ Phase 3: Update Source Code (1-2 hours)

### 3.1 Remove ImGui Include from EditorManager

**File:** `engine/editor/EditorManager.cpp`

**Line 40:** REMOVE:
```cpp
#include "ui/ImGuiContext.h"
```

### 3.2 Remove FRESH_IMGUI_AVAILABLE Conditionals

**File:** `engine/editor/EditorManager.cpp`

**Find and remove ALL `#ifdef FRESH_IMGUI_AVAILABLE` blocks.**

**BEFORE:**
```cpp
#ifdef FRESH_IMGUI_AVAILABLE
    // Initialize ImGui-based UI panels (non-Windows platforms)
    // ...panel initialization code...
    return true;
#else
    // On Windows with native UI, updating world is simpler
    m_world = world;
    m_worldEditor = worldEditor;
    LOG_INFO_C("EditorManager updated with new world (Windows native UI)", "EditorManager");
    return true;
#endif // FRESH_IMGUI_AVAILABLE
```

**AFTER:**
```cpp
// Windows native UI - update world reference
m_world = world;
m_worldEditor = worldEditor;
LOG_INFO_C("EditorManager updated with new world (Windows native UI)", "EditorManager");
return true;
```

**Locations to update (estimated from grep results):**
- Line ~707-828: `updateWorld()` method
- Line ~1172: Unknown context
- Line ~1325: Unknown context

**Action:** Search for all occurrences:
```bash
grep -n "FRESH_IMGUI_AVAILABLE" engine/editor/EditorManager.cpp
```

Then remove each `#ifdef FRESH_IMGUI_AVAILABLE` / `#else` / `#endif` block, keeping only the Win32 native code path.

### 3.3 Update Panel Header Files

**Files to update:**
- `engine/ui/ConsolePanel.h`
- `engine/ui/SceneHierarchyPanel.h`
- `engine/ui/InspectorPanel.h`
- `engine/ui/ContentBrowserPanel.h`

**BEFORE:**
```cpp
#pragma once

// This header is deprecated - Windows native UI is used instead
// For Windows: Use ui/native/Win32ConsolePanel.h
// This stub exists for backward compatibility during migration

#ifdef _WIN32
    #include "ui/native/Win32ConsolePanel.h"
    
    namespace fresh
    {
        // Type alias for backward compatibility
        using ConsolePanel = Win32ConsolePanel;
    }
#endif
```

**AFTER:**
```cpp
#pragma once

#include "ui/native/Win32ConsolePanel.h"

namespace fresh
{
    // Windows native UI implementation
    using ConsolePanel = Win32ConsolePanel;
}
```

Apply to all four files above (remove `#ifdef _WIN32` since we're Windows-only).

### 3.4 Remove ImGui Fallback in EditorSettingsDialog

**File:** `engine/editor/EditorSettingsDialog.cpp`

**REMOVE these TODO comments and stub implementations:**

Lines with:
```cpp
// TODO: Implement with Windows native UI (ImGui has been removed from this project)
LOG_WARNING_C("EditorSettingsDialog::render() - ImGui UI not available, use Win32SettingsDialog instead", "EditorSettingsDialog");
// Stub - ImGui UI not available
```

**Replace with:**
```cpp
// Windows native UI - EditorSettingsDialog is implemented using Win32SettingsDialog
// See engine/ui/native/Win32SettingsDialog.h for the native implementation
LOG_INFO_C("EditorSettingsDialog uses Win32SettingsDialog (native Windows UI)", "EditorSettingsDialog");
```

---

## ✅ Phase 4: Update Documentation (30 minutes)

### 4.1 Update README.md

**File:** `README.md`

**Find and replace:** "ImGui" → "Native Windows Win32 UI"

Specifically update these sections:
- Line ~23: Editor description
- Line ~46-64: Editor & Tools section

**BEFORE:**
```markdown
- **Windows Native UI** - All editor panels use native Windows controls
```

**AFTER:**
```markdown
- **Windows Native Win32 UI** - All editor panels use native Windows controls (no ImGui)
```

### 4.2 Update GUI_WORK_REMAINING_SUMMARY.md

**File:** `GUI_WORK_REMAINING_SUMMARY.md`

Update line 10 to clarify:

**BEFORE:**
```markdown
The Fresh Voxel Engine's GUI implementation is **nearly complete** for core functionality.
```

**AFTER:**
```markdown
The Fresh Voxel Engine's GUI implementation uses **Windows Native Win32 UI** and is **nearly complete** for core functionality. ImGui is NOT used - see WINDOWS_NATIVE_ONLY_POLICY.md.
```

### 4.3 Update IMGUI_INTEGRATION_TODO.md

**Rename file:**
```bash
mv IMGUI_INTEGRATION_TODO.md IMGUI_INTEGRATION_TODO.md.DEPRECATED
```

**Add deprecation notice at top:**
```markdown
# ⚠️ DEPRECATED - DO NOT IMPLEMENT

**This document is DEPRECATED as of 2025-11-21.**

Fresh Voxel Engine uses **Windows Native Win32 UI ONLY**.
ImGui is not used and will not be integrated.

See: WINDOWS_NATIVE_ONLY_POLICY.md

---

# [Original content below - for historical reference only]
```

### 4.4 Update Other Documentation

**Search and update all docs:**
```bash
grep -r "ImGui" docs/ --include="*.md"
```

Replace all mentions of "ImGui-based panels" with "Native Win32 panels" or "Windows native UI".

**Key files to check:**
- `docs/editor/GUI_GUIDE.md`
- `docs/editor/EDITOR_FEATURES_STATUS.md`
- `docs/editor/EDITOR_INTEGRATION.md`
- `docs/history/GUI_REMAINING_TASKS.md`
- `docs/history/NATIVE_GUI_REMAINING_WORK.md`

---

## ✅ Phase 5: Clean Build (30 minutes)

### 5.1 Remove Old Build Artifacts

```powershell
# Remove build directory
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue

# Remove vcpkg installed packages (will reinstall without imgui)
Remove-Item -Recurse -Force vcpkg/installed -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force vcpkg/packages -ErrorAction SilentlyContinue
```

### 5.2 Regenerate Build Files

```powershell
.\generate_vs2022.bat
```

**Expected output:**
```
-- Configuring Fresh Voxel Engine for Windows Gaming Platform
-- GLFW found
-- GLM found
-- OpenGL found
-- GLEW found for OpenGL extension loading
-- OpenAL found
-- nlohmann-json enabled for asset manifest parsing
-- Native File Dialog Extended enabled for file operations
-- DirectX libraries linked
-- Win32 UI libraries linked
...
[NO ImGui messages]
```

### 5.3 Build Project

```powershell
cd build
cmake --build . --config Release
```

**Check for:**
- ✅ No ImGui-related errors
- ✅ No missing symbol errors
- ✅ Successful build completion

### 5.4 Verify No ImGui References

```bash
# Search for any remaining ImGui references in code
grep -r "ImGui" engine/ --include="*.h" --include="*.cpp" | grep -v "Removed ImGui"

# Should return minimal results (only comments about removal)
```

---

## ✅ Phase 6: Testing (30 minutes)

### 6.1 Startup Test

```powershell
cd build/Release
.\FreshVoxelEngine.exe
```

**Verify:**
- ✅ Application starts without errors
- ✅ Main window appears
- ✅ Native Win32 menu bar visible
- ✅ Native Win32 toolbar visible
- ✅ No ImGui-related error messages in console

### 6.2 Editor Functionality Test

**Test these features:**
- ✅ Create new world (File > New World)
- ✅ World generates and displays
- ✅ Menu items clickable and functional
- ✅ Toolbar buttons respond
- ✅ Scene hierarchy panel (if visible)
- ✅ Console panel (if visible)

### 6.3 Build Test Suite

```powershell
# Run tests to ensure nothing broke
cd build
ctest -C Release --output-on-failure
```

**Expected:**
- ✅ All 373 tests pass (no regressions)

---

## ✅ Phase 7: Final Verification (15 minutes)

### 7.1 Dependency Check

```powershell
# Verify imgui is not in dependencies
vcpkg list | Select-String "imgui"
```

**Expected output:** (empty - no imgui)

### 7.2 Size Check

Compare binary size before/after ImGui removal:

**Expected:** Slightly smaller executable (~500KB-1MB reduction)

### 7.3 Documentation Review

**Verify these files are updated:**
- [ ] vcpkg.json - imgui removed
- [ ] CMakeLists.txt - imgui sections removed
- [ ] README.md - mentions Native Win32, not ImGui
- [ ] WINDOWS_NATIVE_ONLY_POLICY.md - created
- [ ] IMGUI_INTEGRATION_TODO.md - deprecated
- [ ] GUI_WORK_REMAINING_SUMMARY.md - clarified
- [ ] Source files - FRESH_IMGUI_AVAILABLE conditionals removed

---

## ✅ Phase 8: Commit and PR (15 minutes)

### 8.1 Review Changes

```bash
git status
git diff
```

**Expected changes:**
- Modified: vcpkg.json
- Modified: CMakeLists.txt
- Deleted: engine/ui/ImGuiContext.h
- Deleted: engine/ui/ImGuiContext.cpp
- Modified: engine/editor/EditorManager.cpp
- Modified: 4+ engine/ui/*.h files
- Modified: README.md
- Modified: multiple docs/*.md files
- Added: WINDOWS_NATIVE_ONLY_POLICY.md
- Renamed: IMGUI_INTEGRATION_TODO.md.DEPRECATED

### 8.2 Commit Changes

```bash
git add .
git commit -m "Remove ImGui dependency - enforce Windows Native Win32 UI only

- Remove imgui from vcpkg.json dependencies
- Remove ImGui find/link from CMakeLists.txt
- Delete ImGuiContext wrapper files
- Remove FRESH_IMGUI_AVAILABLE conditionals from EditorManager
- Update panel headers to always use Win32 native implementations
- Add WINDOWS_NATIVE_ONLY_POLICY.md official policy document
- Deprecate IMGUI_INTEGRATION_TODO.md
- Update README and documentation to reflect Native Win32 UI
- Clean up references to 'ImGui-based panels'

This establishes Windows Native Win32 UI as the ONLY supported UI system.
ImGui is permanently removed and will not be supported going forward.

See WINDOWS_NATIVE_ONLY_POLICY.md for full policy details."
```

### 8.3 Create Pull Request

**PR Title:** Remove ImGui - Enforce Windows Native Win32 UI Only Policy

**PR Description:**
```markdown
## Summary
Removes ALL ImGui code and dependencies from the codebase, establishing Windows Native Win32 UI as the only supported UI system.

## Changes
- Removed imgui from vcpkg.json
- Removed ImGui sections from CMakeLists.txt
- Deleted ImGuiContext wrapper files
- Removed all FRESH_IMGUI_AVAILABLE conditional compilation
- Updated panel headers to remove platform conditionals
- Created WINDOWS_NATIVE_ONLY_POLICY.md
- Deprecated IMGUI_INTEGRATION_TODO.md
- Updated all documentation

## Testing
- ✅ Clean build successful
- ✅ Application starts and runs
- ✅ Native Win32 UI functional
- ✅ All 373 tests pass
- ✅ No ImGui references remain in code

## Policy
This implements the official Windows Native UI Only policy. See WINDOWS_NATIVE_ONLY_POLICY.md.

## Breaking Changes
None - ImGui was already optional and not the primary UI system.

## Closes
N/A (establishes new policy)
```

---

## 📊 Success Criteria

After completing this action plan:

- ✅ imgui NOT in vcpkg.json
- ✅ No imgui references in CMakeLists.txt
- ✅ No FRESH_IMGUI_AVAILABLE in code
- ✅ ImGuiContext files deleted
- ✅ All `#ifdef _WIN32` removed from UI headers
- ✅ Documentation updated
- ✅ WINDOWS_NATIVE_ONLY_POLICY.md created
- ✅ Clean build successful
- ✅ Application runs without errors
- ✅ All tests pass
- ✅ Binary size reduced
- ✅ No ImGui in vcpkg list

---

## 🚨 Rollback Plan

If issues arise after removal:

1. **Revert the commit:**
   ```bash
   git revert HEAD
   ```

2. **Reinstall ImGui temporarily:**
   ```bash
   vcpkg install imgui:x64-windows
   ```

3. **Investigate the issue**

4. **Fix the issue in native Win32 code** (don't re-add ImGui)

5. **Try removal again**

**Note:** ImGui removal should be LOW RISK since it's already mostly unused.

---

## 📝 Notes

### Why This is Safe

1. **Native Win32 is already primary** - Most code uses Win32
2. **ImGui was optional** - Guarded by FRESH_IMGUI_AVAILABLE
3. **No core features depend on ImGui** - All implemented in Win32
4. **Tests don't use ImGui** - 373 tests all use native code

### What Might Break

- ❌ **Nothing critical** - ImGui was only for optional fallback panels
- ⚠️ **Possible:** Some commented-out code that referenced ImGui
- ⚠️ **Possible:** Documentation that assumed ImGui was available

### Post-Removal Benefits

- ✅ Smaller binary size
- ✅ Faster build times
- ✅ One less dependency to manage
- ✅ Clearer codebase (no dual UI systems)
- ✅ Better Windows integration
- ✅ Aligned with project goals (Windows-exclusive)

---

## ✅ Completion Checklist

Before marking this task as DONE:

- [ ] Phase 1: Dependency removal complete
- [ ] Phase 2: Source files deleted
- [ ] Phase 3: Code updated (all FRESH_IMGUI_AVAILABLE removed)
- [ ] Phase 4: Documentation updated
- [ ] Phase 5: Clean build successful
- [ ] Phase 6: Testing passed
- [ ] Phase 7: Final verification done
- [ ] Phase 8: Committed and PR created
- [ ] Code review passed
- [ ] PR merged

---

**Estimated Total Time:** 2-4 hours  
**Risk Level:** LOW  
**Priority:** HIGH (establishes architectural direction)

---

*For questions, see WINDOWS_NATIVE_ONLY_POLICY.md or open a GitHub Discussion.*
