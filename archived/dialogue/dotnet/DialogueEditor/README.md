# Fresh Engine - Dialogue Editor System

## Overview

The Fresh Engine Dialogue Editor is a visual, flowchart-based dialogue editing system with Lua scripting integration. It provides a beginner-friendly, Unreal Engine-style interface for creating interactive dialogue trees for your games.

## Architecture

```
┌─────────────────────────────────┐
│  Dialogue Editor (WinForms)     │
│  - Visual node editing          │
│  - Unreal-style dark theme      │
│  - Drag & drop interface        │
└─────────────────────────────────┘
            ↓ (Saves to)
┌─────────────────────────────────┐
│  JSON Dialogue Files            │
│  - Serialized graph data        │
│  - Node and link definitions    │
└─────────────────────────────────┘
            ↓ (Loaded by)
┌─────────────────────────────────┐
│  Dialogue Manager (Runtime)     │
│  - Executes dialogue flow       │
│  - Lua script integration       │
│  - State management             │
└─────────────────────────────────┘
```

## Features

### Visual Editor
- **Node-based editing**: Create dialogue flows with visual nodes and connections
- **Unreal Engine-style UI**: Familiar dark theme interface for Unreal users
- **Drag and drop**: Easily position and connect nodes
- **Real-time preview**: See your dialogue structure as you build it
- **Zoom and pan**: Navigate large dialogue trees easily

### Node Types
1. **Start Node** (Green): Entry point of the dialogue
2. **Dialogue Node** (Gray-blue): Standard NPC or narrator dialogue
3. **Choice Node** (Blue): Player choice/branching point
4. **Lua Script Node** (Purple): Execute Lua code for game logic
5. **Condition Node** (Orange): Branch based on game state
6. **End Node** (Red): Dialogue endpoint

### Lua Integration
- **Embedded scripts**: Write Lua code directly in nodes
- **External scripts**: Reference Lua files for complex logic
- **Game state access**: Query and modify game state from dialogue
- **Conditional branching**: Use Lua expressions for dynamic dialogue flow
- **Event triggers**: Call game functions from dialogue (quests, cutscenes, etc.)

## Getting Started

### Prerequisites
- .NET 9 SDK
- Windows 10/11 (x64)
- Visual Studio 2022 (optional)

### Building

```batch
cd dotnet/DialogueEditor
dotnet build -c Release
```

### Running

```batch
cd dotnet/DialogueEditor/bin/Release/net9.0-windows
DialogueEditor.exe
```

## Using the Editor

### Creating a New Dialogue

1. **Start the editor**: Run `DialogueEditor.exe`
2. **Add nodes**: Use toolbar buttons or right-click menu
   - Click "+ Dialogue" to add dialogue nodes
   - Click "+ Choice" to add player choices
   - Click "+ Lua Script" for scripting logic
3. **Connect nodes**: Hold Ctrl + Left Click on a node, then click on another node
4. **Edit properties**: Double-click a node or use the Details panel
5. **Save**: File → Save or Ctrl+S

### Editor Controls

| Action | Control |
|--------|---------|
| Select node | Left Click |
| Move node | Left Click + Drag |
| Create link | Ctrl + Left Click (source) → Click (target) |
| Edit node | Double Click or Details panel |
| Delete node | Right Click → Delete or Edit → Delete Node |
| Context menu | Right Click |
| Zoom in/out | Mouse Wheel |
| Pan view | Middle Mouse + Drag (future feature) |

### Node Properties

Each node has properties editable in the Details panel:

**Dialogue Node:**
- Title: Display name
- Speaker: Character name
- DialogueText: The dialogue content
- Position: X, Y coordinates
- Size: Width, Height

**Choice Node:**
- Title: Display name
- ChoiceOptions: List of choice texts
- Position, Size

**Lua Script Node:**
- Title: Display name
- LuaScript: Inline Lua code
- LuaScriptFile: Path to external .lua file
- Position, Size

**Condition Node:**
- Title: Display name
- Condition: Lua expression to evaluate
- Position, Size

### Creating Links

Links connect nodes to define dialogue flow:

1. Hold **Ctrl** and **Left Click** on the source node
2. **Left Click** on the target node
3. The link will be created with a Bezier curve (Unreal style)

For choice nodes, links automatically map to choice indices.

## Runtime Integration

### Loading Dialogue

```csharp
using FreshEngine.DialogueSystem;

// Load dialogue from JSON
var graph = DialogueGraph.LoadFromFile("dialogue/quest_dialogue.json");

// Create dialogue manager
var manager = new DialogueManager();

// Start dialogue
manager.StartDialogue(graph);
```

### Handling Events

```csharp
// Subscribe to dialogue events
manager.OnDialogueDisplay += (sender, args) =>
{
    Console.WriteLine($"{args.Speaker}: {args.DialogueText}");
    // Display in your UI
};

manager.OnChoicePresented += (sender, args) =>
{
    Console.WriteLine("Choices:");
    for (int i = 0; i < args.Choices.Count; i++)
    {
        Console.WriteLine($"{i + 1}. {args.Choices[i]}");
    }
    // Show choices in your UI
};

manager.OnLuaScriptExecute += (sender, script) =>
{
    // Execute Lua script
    luaManager.ExecuteScript(script);
};

manager.OnDialogueEnded += (sender, args) =>
{
    Console.WriteLine("Dialogue ended");
    // Clean up, return to gameplay
};
```

### Processing Dialogue

```csharp
// Continue to next node
manager.Continue();

// Make a choice (for choice nodes)
manager.MakeChoice(choiceIndex);

// Go back
manager.GoBack();
```

## Lua Scripting

### Basic Lua Script Example

```lua
-- Set quest flags
print("Quest started!")
questManager.startQuest("ancient_artifact")
player.setFlag("met_elder", true)

-- Give rewards
player.addGold(100)
player.addItem("quest_marker")
```

### Condition Example

```lua
-- Check if player has completed prerequisite
return player.hasCompletedQuest("tutorial_quest") 
   and player.level >= 5
```

### Available Lua Functions

The dialogue system provides these functions to Lua:

- `log(message)` - Log message to console
- `print(message)` - Alias for log
- Custom functions can be registered via `LuaScriptManager`

### Registering Custom Functions

```csharp
var luaManager = new LuaScriptManager();

// Register C# function callable from Lua
luaManager.RegisterFunction("giveGold", gameManager, "GivePlayerGold");

// Set global variables
luaManager.SetGlobal("playerLevel", player.Level);
luaManager.SetGlobal("playerName", player.Name);

// Execute dialogue scripts
manager.OnLuaScriptExecute += (sender, script) =>
{
    luaManager.ExecuteScript(script);
};
```

## File Format

Dialogues are saved as JSON with this structure:

```json
{
  "Id": "unique-id",
  "Name": "Dialogue Name",
  "Description": "Description",
  "Nodes": [
    {
      "Id": "node-id",
      "Title": "Node Title",
      "NodeType": "Dialogue|Choice|LuaScript|Condition|Start|End",
      "DialogueText": "Text content",
      "Speaker": "Character name",
      "LuaScript": "Lua code",
      "Condition": "Lua condition",
      "ChoiceOptions": ["Option 1", "Option 2"],
      "PositionX": 100.0,
      "PositionY": 200.0,
      "Width": 200.0,
      "Height": 100.0
    }
  ],
  "Links": [
    {
      "Id": "link-id",
      "SourceNodeId": "source-node-id",
      "TargetNodeId": "target-node-id",
      "Label": "Optional label",
      "ChoiceIndex": 0
    }
  ],
  "Version": "1.0"
}
```

## Examples

See the `/examples/dialogue` directory for:
- `sample_quest.json` - Complete quest dialogue example
- `dialogue_logic.lua` - Example Lua scripting functions

## Visual Theme

The editor uses an Unreal Engine-inspired dark theme:

### Colors
- **Background**: Dark gray (#2A2A2A)
- **Canvas**: Darker gray (#202020)
- **Panels**: Medium gray (#2A2A2A)
- **Text**: Light gray (#D2D2D2)
- **Selection**: Gold (#FFC850)
- **Grid**: Subtle gray lines with major/minor divisions

### Node Colors
- **Start**: Green (#3C8C3C)
- **End**: Red (#B43C3C)
- **Dialogue**: Gray-blue (#465A6E)
- **Choice**: Blue (#508CC8)
- **Lua Script**: Purple (#8C50B4)
- **Condition**: Orange (#C88C3C)

## Best Practices

1. **Keep it simple**: Don't create overly complex dialogue trees
2. **Use Lua wisely**: Keep game logic in Lua, dialogue text in nodes
3. **Organize nodes**: Space them well for readability
4. **Save often**: Use File → Save regularly
5. **Test early**: Load dialogues in your game and test frequently
6. **Version control**: JSON files work great with Git
7. **Comment Lua code**: Use `--` for comments in Lua scripts
8. **Validate**: Check for orphaned nodes and dead ends

## Troubleshooting

### Editor won't start
- Ensure .NET 9 runtime is installed
- Check that you're running on Windows

### Lua scripts don't work
- Verify NLua package is installed
- Check Lua syntax with a Lua validator
- Look for error messages in the console

### Can't create links
- Hold Ctrl while clicking the first node
- Ensure both nodes exist
- Check that you're not trying to link a node to itself

### Dialogue doesn't load in game
- Verify JSON file path is correct
- Check JSON syntax with a validator
- Ensure DialogueGraph.LoadFromFile() succeeds

## Contributing

When adding features to the dialogue system:

1. Update data models in `DialogueNode.cs`, `DialogueLink.cs`, `DialogueGraph.cs`
2. Add editor UI in `DialogueEditorForm.cs`
3. Implement runtime logic in `DialogueManager.cs`
4. Update Lua integration in `LuaScriptManager.cs`
5. Add examples and update this documentation

## License

Same as the main Fresh Voxel Engine project (MIT License).

## Credits

- UI Design: Inspired by Unreal Engine's Blueprint editor
- Lua Integration: NLua library
- Architecture: Following industry-standard dialogue system patterns
