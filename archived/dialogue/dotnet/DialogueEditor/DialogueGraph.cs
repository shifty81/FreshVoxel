using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace FreshEngine.DialogueSystem;

/// <summary>
/// Represents a complete dialogue graph with nodes and links
/// </summary>
public class DialogueGraph
{
    /// <summary>
    /// Unique identifier for the dialogue graph
    /// </summary>
    public string Id { get; set; } = Guid.NewGuid().ToString();

    /// <summary>
    /// Name of the dialogue
    /// </summary>
    public string Name { get; set; } = "New Dialogue";

    /// <summary>
    /// Description of the dialogue
    /// </summary>
    public string Description { get; set; } = string.Empty;

    /// <summary>
    /// All nodes in the graph
    /// </summary>
    public List<DialogueNode> Nodes { get; set; } = new();

    /// <summary>
    /// All links between nodes
    /// </summary>
    public List<DialogueLink> Links { get; set; } = new();

    /// <summary>
    /// Metadata for the dialogue graph
    /// </summary>
    public Dictionary<string, string> Metadata { get; set; } = new();

    /// <summary>
    /// Version of the dialogue format
    /// </summary>
    public string Version { get; set; } = "1.0";

    /// <summary>
    /// Creates a new empty dialogue graph
    /// </summary>
    public DialogueGraph()
    {
    }

    /// <summary>
    /// Adds a node to the graph
    /// </summary>
    public void AddNode(DialogueNode node)
    {
        if (node == null)
            throw new ArgumentNullException(nameof(node));

        Nodes.Add(node);
    }

    /// <summary>
    /// Removes a node from the graph and all links connected to it
    /// </summary>
    public bool RemoveNode(string nodeId)
    {
        var node = Nodes.FirstOrDefault(n => n.Id == nodeId);
        if (node == null)
            return false;

        // Remove all links connected to this node
        Links.RemoveAll(l => l.SourceNodeId == nodeId || l.TargetNodeId == nodeId);
        
        return Nodes.Remove(node);
    }

    /// <summary>
    /// Gets a node by its ID
    /// </summary>
    public DialogueNode? GetNode(string nodeId)
    {
        return Nodes.FirstOrDefault(n => n.Id == nodeId);
    }

    /// <summary>
    /// Adds a link between two nodes
    /// </summary>
    public void AddLink(DialogueLink link)
    {
        if (link == null)
            throw new ArgumentNullException(nameof(link));

        // Validate that both nodes exist
        if (!Nodes.Any(n => n.Id == link.SourceNodeId))
            throw new ArgumentException($"Source node {link.SourceNodeId} not found");
        
        if (!Nodes.Any(n => n.Id == link.TargetNodeId))
            throw new ArgumentException($"Target node {link.TargetNodeId} not found");

        Links.Add(link);
    }

    /// <summary>
    /// Removes a link from the graph
    /// </summary>
    public bool RemoveLink(string linkId)
    {
        var link = Links.FirstOrDefault(l => l.Id == linkId);
        return link != null && Links.Remove(link);
    }

    /// <summary>
    /// Gets all links originating from a node
    /// </summary>
    public List<DialogueLink> GetOutgoingLinks(string nodeId)
    {
        return Links.Where(l => l.SourceNodeId == nodeId).ToList();
    }

    /// <summary>
    /// Gets all links pointing to a node
    /// </summary>
    public List<DialogueLink> GetIncomingLinks(string nodeId)
    {
        return Links.Where(l => l.TargetNodeId == nodeId).ToList();
    }

    /// <summary>
    /// Gets the start node of the dialogue (first Start type node)
    /// </summary>
    public DialogueNode? GetStartNode()
    {
        return Nodes.FirstOrDefault(n => n.NodeType == DialogueNodeType.Start);
    }

    /// <summary>
    /// Validates the dialogue graph structure
    /// </summary>
    public List<string> Validate()
    {
        var errors = new List<string>();

        // Check for start node
        var startNodes = Nodes.Where(n => n.NodeType == DialogueNodeType.Start).ToList();
        if (startNodes.Count == 0)
            errors.Add("Dialogue graph must have at least one Start node");
        else if (startNodes.Count > 1)
            errors.Add("Dialogue graph should have only one Start node");

        // Check for orphaned nodes (except Start)
        foreach (var node in Nodes.Where(n => n.NodeType != DialogueNodeType.Start))
        {
            if (!Links.Any(l => l.TargetNodeId == node.Id))
                errors.Add($"Node '{node.Title}' ({node.Id}) has no incoming connections");
        }

        // Check for dead ends (except End)
        foreach (var node in Nodes.Where(n => n.NodeType != DialogueNodeType.End))
        {
            if (!Links.Any(l => l.SourceNodeId == node.Id))
                errors.Add($"Node '{node.Title}' ({node.Id}) has no outgoing connections");
        }

        // Validate links reference existing nodes
        foreach (var link in Links)
        {
            if (!Nodes.Any(n => n.Id == link.SourceNodeId))
                errors.Add($"Link {link.Id} references non-existent source node {link.SourceNodeId}");
            
            if (!Nodes.Any(n => n.Id == link.TargetNodeId))
                errors.Add($"Link {link.Id} references non-existent target node {link.TargetNodeId}");
        }

        return errors;
    }

    /// <summary>
    /// Serializes the dialogue graph to JSON
    /// </summary>
    public string ToJson()
    {
        var options = new JsonSerializerOptions
        {
            WriteIndented = true,
            DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull
        };
        return JsonSerializer.Serialize(this, options);
    }

    /// <summary>
    /// Deserializes a dialogue graph from JSON
    /// </summary>
    public static DialogueGraph? FromJson(string json)
    {
        var options = new JsonSerializerOptions
        {
            PropertyNameCaseInsensitive = true
        };
        return JsonSerializer.Deserialize<DialogueGraph>(json, options);
    }

    /// <summary>
    /// Saves the dialogue graph to a file
    /// </summary>
    public void SaveToFile(string filePath)
    {
        var json = ToJson();
        System.IO.File.WriteAllText(filePath, json);
    }

    /// <summary>
    /// Loads a dialogue graph from a file
    /// </summary>
    public static DialogueGraph? LoadFromFile(string filePath)
    {
        var json = System.IO.File.ReadAllText(filePath);
        return FromJson(json);
    }
}
