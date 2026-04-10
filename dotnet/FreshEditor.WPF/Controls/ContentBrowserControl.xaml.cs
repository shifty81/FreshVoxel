using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using FreshEngine.Managed;

namespace FreshEditor.WPF.Controls;

// ---------------------------------------------------------------------------
// FileItem — displayed in the right-hand file grid
// ---------------------------------------------------------------------------

public sealed class FileItem
{
    public string Name { get; init; } = string.Empty;
    public string FullPath { get; init; } = string.Empty;
    public string Icon { get; init; } = "📄";
    public bool   IsVox  => Path.GetExtension(FullPath).Equals(".vox", StringComparison.OrdinalIgnoreCase);
}

// ---------------------------------------------------------------------------
// ContentBrowserControl
// ---------------------------------------------------------------------------

public partial class ContentBrowserControl : UserControl
{
    public static readonly DependencyProperty EngineProperty =
        DependencyProperty.Register(nameof(Engine), typeof(Engine),
            typeof(ContentBrowserControl), new PropertyMetadata(null));

    public Engine? Engine
    {
        get => (Engine?)GetValue(EngineProperty);
        set => SetValue(EngineProperty, value);
    }

    private readonly ObservableCollection<FileItem> _files = new();
    private string _currentDir = string.Empty;

    public ContentBrowserControl()
    {
        InitializeComponent();
        FileList.ItemsSource = _files;
    }

    public void SetRootDirectory(string rootDir)
    {
        if (!Directory.Exists(rootDir)) return;
        _currentDir = rootDir;
        CurrentPathLabel.Text = rootDir;
        PopulateFolderTree(rootDir);
        PopulateFileList(rootDir);
    }

    private void PopulateFolderTree(string root)
    {
        FolderTree.Items.Clear();
        var item = BuildTreeItem(root);
        FolderTree.Items.Add(item);
        item.IsExpanded = true;
    }

    private static TreeViewItem BuildTreeItem(string dir)
    {
        var item = new TreeViewItem
        {
            Header = Path.GetFileName(dir) is { Length: > 0 } name ? name : dir,
            Tag    = dir,
            Foreground = System.Windows.Media.Brushes.LightGray
        };
        try
        {
            foreach (var sub in Directory.GetDirectories(dir))
                item.Items.Add(BuildTreeItem(sub));
        }
        catch (UnauthorizedAccessException) { /* skip inaccessible dirs */ }
        return item;
    }

    private void PopulateFileList(string dir)
    {
        _files.Clear();
        if (!Directory.Exists(dir)) return;

        try
        {
            foreach (string file in Directory.GetFiles(dir))
            {
                string ext = Path.GetExtension(file).ToLowerInvariant();
                string icon = ext switch
                {
                    ".vox"  => "🧊",
                    ".png" or ".jpg" or ".bmp" => "🖼",
                    ".lua"  => "📜",
                    ".json" => "🗂",
                    _       => "📄"
                };
                _files.Add(new FileItem
                {
                    Name     = Path.GetFileName(file),
                    FullPath = file,
                    Icon     = icon
                });
            }
        }
        catch (UnauthorizedAccessException) { }
    }

    private void FolderTree_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        if (e.NewValue is TreeViewItem { Tag: string dir })
        {
            _currentDir = dir;
            CurrentPathLabel.Text = dir;
            PopulateFileList(dir);
        }
    }

    private void FileList_DoubleClick(object sender, System.Windows.Input.MouseButtonEventArgs e)
    {
        if (FileList.SelectedItem is not FileItem fi) return;
        if (!fi.IsVox) return;
        ImportVoxFile(fi.FullPath);
    }

    private void FileList_Drop(object sender, DragEventArgs e)
    {
        if (!e.Data.GetDataPresent(DataFormats.FileDrop)) return;
        var paths = (string[]?)e.Data.GetData(DataFormats.FileDrop);
        if (paths is null) return;
        foreach (string path in paths)
        {
            if (Path.GetExtension(path).Equals(".vox", StringComparison.OrdinalIgnoreCase))
                ImportVoxFile(path);
        }
    }

    private void ImportVoxFile(string path)
    {
        if (Engine is null)
        {
            MessageBox.Show("No engine attached.", "Content Browser", MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }

        // Import at world origin (0,0,0) — future: pick via ray-cast
        bool ok = Engine.LoadVoxFile(path, 0, 0, 0);
        string msg = ok
            ? $"Imported: {Path.GetFileName(path)}"
            : $"Failed to import: {Path.GetFileName(path)}";
        MessageBox.Show(msg, "Content Browser", MessageBoxButton.OK,
            ok ? MessageBoxImage.Information : MessageBoxImage.Error);
    }

    private void RefreshButton_Click(object sender, RoutedEventArgs e)
    {
        if (_currentDir.Length > 0)
            PopulateFileList(_currentDir);
    }
}
