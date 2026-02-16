using System;
using System.Collections.Generic;

namespace FreshEngine.DialogueSystem;

/// <summary>
/// Types of dialogue nodes
/// </summary>
public enum DialogueNodeType
{
    /// <summary>Standard dialogue text node</summary>
    Dialogue,
    /// <summary>Player choice/branching node</summary>
    Choice,
    /// <summary>Node that executes Lua logic</summary>
    LuaScript,
    /// <summary>Conditional branch based on game state</summary>
    Condition,
    /// <summary>Start point of dialogue</summary>
    Start,
    /// <summary>End point of dialogue</summary>
    End
}

/// <summary>
/// Represents a single node in the dialogue graph
/// </summary>
public class DialogueNode
{
    /// <summary>
    /// Unique identifier for the node
    /// </summary>
    public string Id { get; set; } = Guid.NewGuid().ToString();

    /// <summary>
    /// Display title of the node
    /// </summary>
    public string Title { get; set; } = "New Node";

    /// <summary>
    /// Type of the dialogue node
    /// </summary>
    public DialogueNodeType NodeType { get; set; } = DialogueNodeType.Dialogue;

    /// <summary>
    /// Dialogue text content (for Dialogue nodes)
    /// </summary>
    public string DialogueText { get; set; } = string.Empty;

    /// <summary>
    /// Speaker name (for Dialogue nodes)
    /// </summary>
    public string Speaker { get; set; } = string.Empty;

    /// <summary>
    /// Lua script code (for LuaScript nodes)
    /// </summary>
    public string LuaScript { get; set; } = string.Empty;

    /// <summary>
    /// Reference to external Lua file
    /// </summary>
    public string LuaScriptFile { get; set; } = string.Empty;

    /// <summary>
    /// Condition expression (for Condition nodes)
    /// </summary>
    public string Condition { get; set; } = string.Empty;

    /// <summary>
    /// Choice options (for Choice nodes)
    /// </summary>
    public List<string> ChoiceOptions { get; set; } = new();

    /// <summary>
    /// Position in the editor canvas
    /// </summary>
    public float PositionX { get; set; }

    /// <summary>
    /// Position in the editor canvas
    /// </summary>
    public float PositionY { get; set; }

    /// <summary>
    /// Width of the node in the editor
    /// </summary>
    public float Width { get; set; } = 200f;

    /// <summary>
    /// Height of the node in the editor
    /// </summary>
    public float Height { get; set; } = 100f;

    /// <summary>
    /// Custom metadata for the node
    /// </summary>
    public Dictionary<string, string> Metadata { get; set; } = new();

    /// <summary>
    /// Creates a new dialogue node
    /// </summary>
    public DialogueNode()
    {
    }

    /// <summary>
    /// Creates a new dialogue node with specified type
    /// </summary>
    public DialogueNode(DialogueNodeType type)
    {
        NodeType = type;
        Title = type switch
        {
            DialogueNodeType.Start => "Start",
            DialogueNodeType.End => "End",
            DialogueNodeType.Choice => "Player Choice",
            DialogueNodeType.LuaScript => "Lua Script",
            DialogueNodeType.Condition => "Condition",
            _ => "Dialogue"
        };
    }
}
