using System.Collections.ObjectModel;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using FreshEditor.WPF.ViewModels;
using FreshEngine.Managed;

namespace FreshEditor.WPF.Controls;

public partial class SceneOutlinerControl : UserControl
{
    public static readonly DependencyProperty ViewModelProperty =
        DependencyProperty.Register(nameof(ViewModel), typeof(EditorViewModel),
            typeof(SceneOutlinerControl), new PropertyMetadata(null, OnVMChanged));

    public static readonly DependencyProperty EngineProperty =
        DependencyProperty.Register(nameof(Engine), typeof(Engine),
            typeof(SceneOutlinerControl), new PropertyMetadata(null));

    public EditorViewModel? ViewModel
    {
        get => (EditorViewModel?)GetValue(ViewModelProperty);
        set => SetValue(ViewModelProperty, value);
    }

    public Engine? Engine
    {
        get => (Engine?)GetValue(EngineProperty);
        set => SetValue(EngineProperty, value);
    }

    public SceneOutlinerControl()
    {
        InitializeComponent();
    }

    private static void OnVMChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        var ctrl = (SceneOutlinerControl)d;
        if (e.NewValue is EditorViewModel vm)
            ctrl.EntityTree.ItemsSource = vm.Entities;
    }

    private void SearchBox_TextChanged(object sender, TextChangedEventArgs e)
    {
        string filter = SearchBox.Text.Trim().ToLowerInvariant();
        if (ViewModel is null) return;

        // Simple linear filter — replace with CollectionView for large scenes
        var view = System.Windows.Data.CollectionViewSource.GetDefaultView(ViewModel.Entities);
        view.Filter = filter.Length == 0
            ? null
            : (obj => obj is EntityNode node &&
                       node.Name.Contains(filter, System.StringComparison.OrdinalIgnoreCase));
    }

    private void EntityTree_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        if (ViewModel is null) return;
        if (e.NewValue is EntityNode node)
            ViewModel.SelectedEntityId = node.Id;
    }

    // -------------------------------------------------------------------------
    // Context menu: Save as Prefab
    // -------------------------------------------------------------------------

    private void SaveAsPrefab_Click(object sender, RoutedEventArgs e)
    {
        if (Engine is null || ViewModel is null) return;

        uint entityId = ViewModel.SelectedEntityId;
        if (entityId == 0)
        {
            MessageBox.Show("No entity selected.", "Save Prefab",
                MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }

        var dlg = new Microsoft.Win32.SaveFileDialog
        {
            Title      = "Save Entity as Prefab",
            Filter     = "Prefab Files (*.prefab)|*.prefab|All Files (*.*)|*.*",
            DefaultExt = ".prefab",
            FileName   = $"entity_{entityId}"
        };

        if (dlg.ShowDialog() != true) return;

        bool ok = Engine.SavePrefab(entityId, dlg.FileName);
        string msg = ok
            ? $"Prefab saved: {Path.GetFileName(dlg.FileName)}"
            : $"Failed to save prefab for entity {entityId}";
        MessageBox.Show(msg, "Save Prefab", MessageBoxButton.OK,
            ok ? MessageBoxImage.Information : MessageBoxImage.Error);
    }
}
