using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Text.Json;
using System.Windows.Input;
using FreshEngine.Managed;

namespace FreshEditor.WPF.ViewModels;

// ---------------------------------------------------------------------------
// EntityNode — one item in the Scene Outliner tree
// ---------------------------------------------------------------------------

public sealed class EntityNode : INotifyPropertyChanged
{
    private string _name = string.Empty;

    public int Id { get; init; }

    public string Name
    {
        get => _name;
        set { _name = value; OnPropertyChanged(); }
    }

    public ObservableCollection<string> Components { get; } = new();

    public event PropertyChangedEventHandler? PropertyChanged;
    private void OnPropertyChanged([CallerMemberName] string? name = null) =>
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}

// ---------------------------------------------------------------------------
// RelayCommand — lightweight ICommand adapter
// ---------------------------------------------------------------------------

public sealed class RelayCommand : ICommand
{
    private readonly Action<object?> _execute;
    private readonly Func<object?, bool>? _canExecute;

    public RelayCommand(Action<object?> execute, Func<object?, bool>? canExecute = null)
    {
        _execute    = execute;
        _canExecute = canExecute;
    }

    public event EventHandler? CanExecuteChanged
    {
        add    => CommandManager.RequerySuggested += value;
        remove => CommandManager.RequerySuggested -= value;
    }

    public bool CanExecute(object? p) => _canExecute?.Invoke(p) ?? true;
    public void Execute(object? p)    => _execute(p);
}

// ---------------------------------------------------------------------------
// EditorViewModel — central MVVM hub for the WPF editor
// ---------------------------------------------------------------------------

public sealed class EditorViewModel : INotifyPropertyChanged, IDisposable
{
    // ---- Engine handle ----
    private readonly Engine _engine;

    // ---- Play-mode state ----
    private bool _isEditorMode = true;
    private bool _isDirty;
    private int  _selectedEntityId = -1;

    // ---- Scene outliner ----
    public ObservableCollection<EntityNode> Entities { get; } = new();

    // ---- Log messages ----
    public ObservableCollection<LogEntry> LogEntries { get; } = new();

    // ---- Commands ----
    public ICommand PlayCommand       { get; }
    public ICommand StopCommand       { get; }
    public ICommand UndoCommand       { get; }
    public ICommand RedoCommand       { get; }
    public ICommand FrameCommand      { get; }
    public ICommand RefreshSceneCommand { get; }

    public EditorViewModel(Engine engine)
    {
        _engine = engine;

        PlayCommand    = new RelayCommand(_ => SetPlayMode(true),  _ => IsEditorMode);
        StopCommand    = new RelayCommand(_ => SetPlayMode(false), _ => !IsEditorMode);
        UndoCommand    = new RelayCommand(_ => _engine.Undo());
        RedoCommand    = new RelayCommand(_ => _engine.Redo());
        FrameCommand   = new RelayCommand(_ => _engine.FrameSelection());
        RefreshSceneCommand = new RelayCommand(_ => RefreshSceneEntities());

        // Subscribe to engine log messages
        _engine.SetLogCallback((level, msg) =>
            System.Windows.Application.Current.Dispatcher.Invoke(() =>
                LogEntries.Add(new LogEntry(level, msg))));
    }

    // ---- Play-mode ----

    public bool IsEditorMode
    {
        get => _isEditorMode;
        private set
        {
            if (_isEditorMode == value) return;
            _isEditorMode = value;
            OnPropertyChanged();
            OnPropertyChanged(nameof(IsPlayMode));
        }
    }

    public bool IsPlayMode => !_isEditorMode;

    private void SetPlayMode(bool play)
    {
        IsEditorMode = !play;
        _engine.IsEditorMode = !play;
    }

    // ---- Dirty flag ----

    public bool IsDirty
    {
        get => _isDirty;
        set { _isDirty = value; OnPropertyChanged(); }
    }

    // ---- Entity selection ----

    public int SelectedEntityId
    {
        get => _selectedEntityId;
        set { _selectedEntityId = value; OnPropertyChanged(); }
    }

    // ---- Scene hierarchy ----

    public void RefreshSceneEntities()
    {
        Entities.Clear();
        try
        {
            string json = _engine.GetSceneEntitiesJson();
            using var doc = JsonDocument.Parse(json);
            if (!doc.RootElement.TryGetProperty("entities", out var arr)) return;

            foreach (var el in arr.EnumerateArray())
            {
                var node = new EntityNode
                {
                    Id   = el.TryGetProperty("id",   out var idProp)   ? idProp.GetInt32()  : -1,
                    Name = el.TryGetProperty("name", out var nameProp) ? nameProp.GetString() ?? "Entity" : "Entity"
                };
                if (el.TryGetProperty("components", out var comps))
                {
                    foreach (var c in comps.EnumerateArray())
                        node.Components.Add(c.GetString() ?? "");
                }
                Entities.Add(node);
            }
        }
        catch (JsonException)
        {
            // Engine returned malformed JSON — ignore silently
        }
    }

    // ---- INotifyPropertyChanged ----

    public event PropertyChangedEventHandler? PropertyChanged;
    private void OnPropertyChanged([CallerMemberName] string? name = null) =>
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));

    // ---- IDisposable ----

    public void Dispose()
    {
        _engine.SetLogCallback(null);
    }
}

// ---------------------------------------------------------------------------
// LogEntry — single line in the Output Log
// ---------------------------------------------------------------------------

public sealed record LogEntry(string Level, string Message)
{
    public System.Windows.Media.Brush LevelBrush => Level switch
    {
        "WARN"  => System.Windows.Media.Brushes.Yellow,
        "ERROR" => System.Windows.Media.Brushes.OrangeRed,
        _       => System.Windows.Media.Brushes.White
    };
}
