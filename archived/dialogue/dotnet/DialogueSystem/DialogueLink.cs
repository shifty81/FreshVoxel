using System;

namespace FreshEngine.DialogueSystem;

/// <summary>
/// Represents a connection/link between two dialogue nodes
/// </summary>
public class DialogueLink
{
    /// <summary>
    /// Unique identifier for the link
    /// </summary>
    public string Id { get; set; } = Guid.NewGuid().ToString();

    /// <summary>
    /// ID of the source node
    /// </summary>
    public string SourceNodeId { get; set; } = string.Empty;

    /// <summary>
    /// ID of the target node
    /// </summary>
    public string TargetNodeId { get; set; } = string.Empty;

    /// <summary>
    /// Label for the link (e.g., choice text, condition result)
    /// </summary>
    public string Label { get; set; } = string.Empty;

    /// <summary>
    /// Choice index for links from choice nodes (0-based)
    /// </summary>
    public int ChoiceIndex { get; set; } = -1;

    /// <summary>
    /// Condition result for links from condition nodes (true/false)
    /// </summary>
    public bool? ConditionResult { get; set; }

    /// <summary>
    /// Creates a new dialogue link
    /// </summary>
    public DialogueLink()
    {
    }

    /// <summary>
    /// Creates a new dialogue link between two nodes
    /// </summary>
    public DialogueLink(string sourceNodeId, string targetNodeId)
    {
        SourceNodeId = sourceNodeId;
        TargetNodeId = targetNodeId;
    }

    /// <summary>
    /// Creates a new dialogue link for a choice option
    /// </summary>
    public DialogueLink(string sourceNodeId, string targetNodeId, int choiceIndex, string label)
    {
        SourceNodeId = sourceNodeId;
        TargetNodeId = targetNodeId;
        ChoiceIndex = choiceIndex;
        Label = label;
    }
}
