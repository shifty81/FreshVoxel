using System;
using System.Windows;
using System.Windows.Controls;
using FreshEditor.WPF.ViewModels;
using FreshEngine.Managed;

namespace FreshEditor.WPF.Controls;

/// <summary>
/// Shows component properties for the currently selected entity.
/// Properties are laid out as label + editable TextBox pairs.
/// </summary>
public partial class DetailsControl : UserControl
{
    public static readonly DependencyProperty ViewModelProperty =
        DependencyProperty.Register(nameof(ViewModel), typeof(EditorViewModel),
            typeof(DetailsControl), new PropertyMetadata(null, OnVMChanged));

    public EditorViewModel? ViewModel
    {
        get => (EditorViewModel?)GetValue(ViewModelProperty);
        set => SetValue(ViewModelProperty, value);
    }

    public static readonly DependencyProperty EngineProperty =
        DependencyProperty.Register(nameof(Engine), typeof(Engine),
            typeof(DetailsControl), new PropertyMetadata(null));

    public Engine? Engine
    {
        get => (Engine?)GetValue(EngineProperty);
        set => SetValue(EngineProperty, value);
    }

    public DetailsControl()
    {
        InitializeComponent();
    }

    private static void OnVMChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        var ctrl = (DetailsControl)d;
        if (e.OldValue is EditorViewModel oldVm)
            oldVm.PropertyChanged -= ctrl.OnVMPropertyChanged;
        if (e.NewValue is EditorViewModel newVm)
            newVm.PropertyChanged += ctrl.OnVMPropertyChanged;
    }

    private void OnVMPropertyChanged(object? sender, System.ComponentModel.PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(EditorViewModel.SelectedEntityId))
            RefreshProperties();
    }

    private void RefreshProperties()
    {
        PropertyStack.Children.Clear();
        if (ViewModel is null || ViewModel.SelectedEntityId < 0) return;

        int entityId = ViewModel.SelectedEntityId;

        // Find the entity node and show its components as editable sections
        EntityNode? node = null;
        foreach (var en in ViewModel.Entities)
        {
            if (en.Id == entityId) { node = en; break; }
        }
        if (node is null) return;

        AddSectionHeader($"Entity: {node.Name} [{entityId}]");

        foreach (string comp in node.Components)
        {
            AddSectionHeader(comp);
            // Placeholder property rows — real implementation queries
            // engine reflection to list actual component fields
            AddPropertyRow(entityId, comp, "posX", "0");
            AddPropertyRow(entityId, comp, "posY", "0");
            AddPropertyRow(entityId, comp, "posZ", "0");
        }
    }

    private void AddSectionHeader(string title)
    {
        var tb = new TextBlock
        {
            Text       = title,
            Foreground = System.Windows.Media.Brushes.LightGray,
            FontWeight = FontWeights.Bold,
            Margin     = new Thickness(0, 6, 0, 2)
        };
        PropertyStack.Children.Add(tb);
    }

    private void AddPropertyRow(int entityId, string component, string key, string defaultValue)
    {
        var panel = new DockPanel { Margin = new Thickness(0, 1, 0, 1) };

        var label = new TextBlock
        {
            Text              = key,
            Width             = 80,
            Foreground        = System.Windows.Media.Brushes.DarkGray,
            VerticalAlignment = VerticalAlignment.Center
        };
        DockPanel.SetDock(label, Dock.Left);

        var box = new TextBox
        {
            Text            = defaultValue,
            Background      = System.Windows.Media.Brushes.DarkSlateGray,
            Foreground      = System.Windows.Media.Brushes.White,
            BorderBrush     = System.Windows.Media.Brushes.SlateGray,
            Padding         = new Thickness(2)
        };
        box.LostFocus += (_, _) =>
            Engine?.SetComponentProperty(entityId, component, key, box.Text);

        panel.Children.Add(label);
        panel.Children.Add(box);
        PropertyStack.Children.Add(panel);
    }
}
