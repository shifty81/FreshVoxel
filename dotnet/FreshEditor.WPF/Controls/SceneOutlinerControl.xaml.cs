using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;
using FreshEditor.WPF.ViewModels;

namespace FreshEditor.WPF.Controls;

public partial class SceneOutlinerControl : UserControl
{
    public static readonly DependencyProperty ViewModelProperty =
        DependencyProperty.Register(nameof(ViewModel), typeof(EditorViewModel),
            typeof(SceneOutlinerControl), new PropertyMetadata(null, OnVMChanged));

    public EditorViewModel? ViewModel
    {
        get => (EditorViewModel?)GetValue(ViewModelProperty);
        set => SetValue(ViewModelProperty, value);
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
}
