using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;
using FreshEditor.WPF.ViewModels;

namespace FreshEditor.WPF.Controls;

public partial class OutputLogControl : UserControl
{
    public static readonly DependencyProperty ViewModelProperty =
        DependencyProperty.Register(nameof(ViewModel), typeof(EditorViewModel),
            typeof(OutputLogControl), new PropertyMetadata(null, OnVMChanged));

    public EditorViewModel? ViewModel
    {
        get => (EditorViewModel?)GetValue(ViewModelProperty);
        set => SetValue(ViewModelProperty, value);
    }

    public OutputLogControl()
    {
        InitializeComponent();
    }

    private static void OnVMChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        var ctrl = (OutputLogControl)d;
        if (e.OldValue is EditorViewModel oldVm)
            oldVm.LogEntries.CollectionChanged -= ctrl.OnLogEntriesChanged;
        if (e.NewValue is EditorViewModel newVm)
        {
            ctrl.LogList.ItemsSource = newVm.LogEntries;
            newVm.LogEntries.CollectionChanged += ctrl.OnLogEntriesChanged;
        }
    }

    private void OnLogEntriesChanged(object? sender, NotifyCollectionChangedEventArgs e)
    {
        // Auto-scroll to latest entry
        if (LogList.Items.Count > 0)
            LogList.ScrollIntoView(LogList.Items[^1]);
    }

    private void ClearButton_Click(object sender, RoutedEventArgs e)
    {
        ViewModel?.LogEntries.Clear();
    }
}
