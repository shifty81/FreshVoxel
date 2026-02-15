# Dialogue Editor Integration Guide

## Overview

The Dialogue Editor has been integrated into the Fresh Engine's main editor as a menu-accessible tool. This integration allows users to launch the dialogue editor directly from within the engine editor interface.

## How to Access

### From the Editor Menu

1. Launch the Fresh Engine editor
2. Go to **Tools → Dialogue Editor** (or press **Ctrl+D**)
3. The dialogue editor will launch in a separate window

## Integration Details

### Menu Structure

The dialogue editor is accessible through a new "Tools" menu in the main menu bar:

```
File  Edit  Window  Tools  Build  Settings  Help
                      │
                      ├─ Dialogue Editor (Ctrl+D)
                      ├─ ───────────────
                      └─ Import Assets... (Ctrl+I)
```

### Implementation

The integration consists of:

1. **EditorManager** (`EditorManager.h/cpp`)
   - Added `launchDialogueEditor()` method
   - Launches the C# WinForms DialogueEditor as a separate process
   - Uses Windows `CreateProcessA` API to spawn the editor

2. **EditorMenuBar** (`EditorMenuBar.h/cpp`)
   - Added new "Tools" menu
   - Added "Dialogue Editor" menu item with Ctrl+D shortcut
   - Added callback mechanism for dialogue editor launch

3. **DialogueEditor Application** (`dotnet/DialogueEditor/`)
   - Standalone WinForms application
   - Can be launched from within the editor
   - Can also be run independently

### File Paths

The integration looks for the dialogue editor executable in:
- Primary: `dotnet/DialogueEditor/bin/Release/net9.0-windows/DialogueEditor.exe`
- Fallback: `dotnet/DialogueEditor/bin/Debug/net9.0-windows/DialogueEditor.exe`

## Building the Dialogue Editor

Before using the integrated dialogue editor, you need to build it:

```bash
cd dotnet/DialogueEditor
dotnet build -c Release
```

Or for development:

```bash
cd dotnet/DialogueEditor
dotnet build -c Debug
```

## Usage Workflow

### Typical Workflow

1. **Open Fresh Engine Editor**
   - Launch the main Fresh Engine application
   - The editor loads with all panels

2. **Access Dialogue Editor**
   - Click **Tools → Dialogue Editor** or press **Ctrl+D**
   - The dialogue editor opens in a new window

3. **Create/Edit Dialogues**
   - Use the dialogue editor to create and edit dialogue graphs
   - Save dialogues as JSON files in your project

4. **Use Dialogues in Game**
   - Load dialogue graphs using `DialogueGraph.LoadFromFile()`
   - Execute dialogues with `DialogueManager`

### Integration with Game Logic

The dialogue system integrates with your game through:

```csharp
// In your game code
using FreshEngine.DialogueSystem;

// Load dialogue created in the editor
var dialogue = DialogueGraph.LoadFromFile("dialogues/my_quest.json");

// Create runtime manager
var manager = new DialogueManager();

// Wire up events
manager.OnDialogueDisplay += (s, e) => {
    // Display dialogue in your UI
    ShowDialogue(e.Speaker, e.DialogueText);
};

// Start dialogue
manager.StartDialogue(dialogue);
```

## Platform Support

**Windows Only**: The dialogue editor is currently only available on Windows because:
- It's built with WinForms (.NET 9 Windows)
- The integration uses Windows-specific APIs (`CreateProcessA`)

On non-Windows platforms, the "Dialogue Editor" menu item will show a warning message.

## Troubleshooting

### Editor Won't Launch

**Problem**: Clicking "Dialogue Editor" does nothing or shows an error.

**Solutions**:
1. Make sure the DialogueEditor is built:
   ```bash
   cd dotnet/DialogueEditor
   dotnet build -c Release
   ```

2. Check that the executable exists:
   - Look in `dotnet/DialogueEditor/bin/Release/net9.0-windows/DialogueEditor.exe`
   - Or in `dotnet/DialogueEditor/bin/Debug/net9.0-windows/DialogueEditor.exe`

3. Check the engine console/logs for error messages

### .NET 9 Not Found

**Problem**: "Failed to launch dialogue editor" with .NET error.

**Solution**: Install .NET 9 SDK from https://dotnet.microsoft.com/download/dotnet/9.0

### Editor Crashes on Launch

**Problem**: The dialogue editor window appears but crashes immediately.

**Solutions**:
1. Make sure NLua dependencies are installed (handled automatically by NuGet)
2. Check Windows version compatibility (requires Windows 10/11)
3. Check system DPI settings (editor supports high DPI)

## Standalone vs Integrated

You can use the dialogue editor in two ways:

### 1. Integrated Mode (Recommended)
- Launch from within the Fresh Engine editor
- Better workflow integration
- Access via Tools menu or Ctrl+D

### 2. Standalone Mode
- Run DialogueEditor.exe directly
- Useful for dedicated dialogue writing
- Can run on a different machine
- Navigate to: `dotnet/DialogueEditor/bin/Release/net9.0-windows/DialogueEditor.exe`

Both modes produce the same JSON dialogue files that can be used in the game.

## Future Enhancements

Potential improvements to the integration:

1. **In-Editor Dialogue Preview**
   - Show dialogue preview panel within main editor
   - Real-time dialogue testing

2. **Dialogue Asset Browser**
   - Browse dialogue files in Content Browser
   - Quick preview and editing

3. **Lua Debugging Integration**
   - Debug Lua scripts from within dialogue editor
   - Breakpoints and variable inspection

4. **Live Reload**
   - Hot-reload dialogue changes without restarting game
   - Useful for rapid iteration

5. **Multi-Platform Support**
   - Port to cross-platform UI framework
   - Support for Linux and macOS

## Related Documentation

- **Dialogue Editor User Guide**: `DIALOGUE_EDITOR_QUICKSTART.md`
- **Comprehensive Documentation**: `dotnet/DialogueEditor/README.md`
- **Implementation Details**: `DIALOGUE_EDITOR_IMPLEMENTATION_SUMMARY.md`
- **Visual Design Spec**: `DIALOGUE_EDITOR_VISUAL_MOCKUP.md`

## API Reference

### EditorManager

```cpp
// Launch the dialogue editor
void EditorManager::launchDialogueEditor()
```

Launches the dialogue editor as a separate process. Available on Windows only.

### EditorMenuBar

```cpp
// Set callback for dialogue editor launch
void EditorMenuBar::setDialogueEditorCallback(MenuCallback callback)
```

Sets the callback function to be called when the user selects "Tools → Dialogue Editor".

## Support

For issues or questions about the dialogue editor integration:
- Check the documentation in `dotnet/DialogueEditor/README.md`
- Review the implementation in `engine/editor/EditorManager.cpp`
- Check logs in the engine console for error messages
