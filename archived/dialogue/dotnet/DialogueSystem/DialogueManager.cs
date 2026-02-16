using System;
using System.Collections.Generic;

namespace FreshEngine.DialogueSystem;

/// <summary>
/// Event args for dialogue events
/// </summary>
public class DialogueEventArgs : EventArgs
{
    public DialogueNode? CurrentNode { get; set; }
    public string? DialogueText { get; set; }
    public string? Speaker { get; set; }
    public List<string>? Choices { get; set; }
}

/// <summary>
/// Manages dialogue execution at runtime
/// </summary>
public class DialogueManager
{
    private DialogueGraph? _currentGraph;
    private DialogueNode? _currentNode;
    private readonly Stack<DialogueNode> _nodeHistory = new();
    private readonly Dictionary<string, object> _dialogueState = new();

    /// <summary>
    /// Event fired when dialogue text is displayed
    /// </summary>
    public event EventHandler<DialogueEventArgs>? OnDialogueDisplay;

    /// <summary>
    /// Event fired when player choices are presented
    /// </summary>
    public event EventHandler<DialogueEventArgs>? OnChoicePresented;

    /// <summary>
    /// Event fired when dialogue ends
    /// </summary>
    public event EventHandler? OnDialogueEnded;

    /// <summary>
    /// Event fired when a Lua script node is encountered
    /// </summary>
    public event EventHandler<string>? OnLuaScriptExecute;

    /// <summary>
    /// Gets the current dialogue graph
    /// </summary>
    public DialogueGraph? CurrentGraph => _currentGraph;

    /// <summary>
    /// Gets the current node
    /// </summary>
    public DialogueNode? CurrentNode => _currentNode;

    /// <summary>
    /// Gets whether a dialogue is currently active
    /// </summary>
    public bool IsDialogueActive => _currentGraph != null && _currentNode != null;

    /// <summary>
    /// Starts a dialogue from a graph
    /// </summary>
    public bool StartDialogue(DialogueGraph graph)
    {
        if (graph == null)
            throw new ArgumentNullException(nameof(graph));

        var startNode = graph.GetStartNode();
        if (startNode == null)
        {
            Console.WriteLine("ERROR: No start node found in dialogue graph");
            return false;
        }

        _currentGraph = graph;
        _currentNode = startNode;
        _nodeHistory.Clear();
        _dialogueState.Clear();

        ProcessCurrentNode();
        return true;
    }

    /// <summary>
    /// Ends the current dialogue
    /// </summary>
    public void EndDialogue()
    {
        _currentGraph = null;
        _currentNode = null;
        _nodeHistory.Clear();
        OnDialogueEnded?.Invoke(this, EventArgs.Empty);
    }

    /// <summary>
    /// Advances to the next node in the dialogue
    /// </summary>
    public bool Continue()
    {
        if (!IsDialogueActive || _currentGraph == null || _currentNode == null)
            return false;

        // For nodes with single outgoing link, follow it automatically
        var outgoingLinks = _currentGraph.GetOutgoingLinks(_currentNode.Id);
        
        if (outgoingLinks.Count == 0)
        {
            EndDialogue();
            return false;
        }

        if (outgoingLinks.Count == 1)
        {
            return FollowLink(outgoingLinks[0]);
        }

        // Multiple links require choice
        return false;
    }

    /// <summary>
    /// Makes a choice at a choice node
    /// </summary>
    public bool MakeChoice(int choiceIndex)
    {
        if (!IsDialogueActive || _currentGraph == null || _currentNode == null)
            return false;

        if (_currentNode.NodeType != DialogueNodeType.Choice)
            return false;

        var outgoingLinks = _currentGraph.GetOutgoingLinks(_currentNode.Id);
        var choiceLink = outgoingLinks.Find(l => l.ChoiceIndex == choiceIndex);

        if (choiceLink == null)
        {
            Console.WriteLine($"ERROR: No link found for choice index {choiceIndex}");
            return false;
        }

        return FollowLink(choiceLink);
    }

    /// <summary>
    /// Follows a link to the next node
    /// </summary>
    private bool FollowLink(DialogueLink link)
    {
        if (_currentGraph == null || _currentNode == null)
            return false;

        var nextNode = _currentGraph.GetNode(link.TargetNodeId);
        if (nextNode == null)
        {
            Console.WriteLine($"ERROR: Target node {link.TargetNodeId} not found");
            return false;
        }

        _nodeHistory.Push(_currentNode);
        _currentNode = nextNode;
        ProcessCurrentNode();
        return true;
    }

    /// <summary>
    /// Processes the current node based on its type
    /// </summary>
    private void ProcessCurrentNode()
    {
        if (_currentNode == null || _currentGraph == null)
            return;

        switch (_currentNode.NodeType)
        {
            case DialogueNodeType.Start:
                // Automatically continue from start node
                Continue();
                break;

            case DialogueNodeType.Dialogue:
                // Display dialogue text
                OnDialogueDisplay?.Invoke(this, new DialogueEventArgs
                {
                    CurrentNode = _currentNode,
                    DialogueText = _currentNode.DialogueText,
                    Speaker = _currentNode.Speaker
                });
                break;

            case DialogueNodeType.Choice:
                // Present choices to player
                OnChoicePresented?.Invoke(this, new DialogueEventArgs
                {
                    CurrentNode = _currentNode,
                    Choices = new List<string>(_currentNode.ChoiceOptions)
                });
                break;

            case DialogueNodeType.LuaScript:
                // Execute Lua script
                string script = !string.IsNullOrEmpty(_currentNode.LuaScriptFile)
                    ? System.IO.File.ReadAllText(_currentNode.LuaScriptFile)
                    : _currentNode.LuaScript;

                OnLuaScriptExecute?.Invoke(this, script);
                
                // Automatically continue after script execution
                Continue();
                break;

            case DialogueNodeType.Condition:
                // Evaluate condition (would need Lua integration)
                // For now, just follow the first link
                var links = _currentGraph.GetOutgoingLinks(_currentNode.Id);
                if (links.Count > 0)
                {
                    FollowLink(links[0]);
                }
                break;

            case DialogueNodeType.End:
                // End the dialogue
                EndDialogue();
                break;
        }
    }

    /// <summary>
    /// Goes back to the previous node
    /// </summary>
    public bool GoBack()
    {
        if (_nodeHistory.Count == 0)
            return false;

        _currentNode = _nodeHistory.Pop();
        ProcessCurrentNode();
        return true;
    }

    /// <summary>
    /// Sets a variable in the dialogue state
    /// </summary>
    public void SetState(string key, object value)
    {
        _dialogueState[key] = value;
    }

    /// <summary>
    /// Gets a variable from the dialogue state
    /// </summary>
    public object? GetState(string key)
    {
        return _dialogueState.TryGetValue(key, out var value) ? value : null;
    }

    /// <summary>
    /// Checks if a state variable exists
    /// </summary>
    public bool HasState(string key)
    {
        return _dialogueState.ContainsKey(key);
    }
}
