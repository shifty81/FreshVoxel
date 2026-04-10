using System;
using System.Windows;
using System.Windows.Input;
using FreshEditor.WPF.Controls;
using FreshEditor.WPF.ViewModels;
using FreshEngine.Managed;

namespace FreshEditor.WPF;

/// <summary>
/// Unreal Engine-style editor main window.
///
/// Layout (left → right, top → bottom):
///   Menu + Toolbar
///   [Place Panel] | [3-D Viewport (HwndHost → DX11)] | [Scene Outliner + Details]
///   [Content Browser]                                 | [Output Log]
///   Status Bar
/// </summary>
public partial class MainWindow : Window
{
    private Engine?          _engine;
    private EditorViewModel? _viewModel;

    public MainWindow()
    {
        InitializeComponent();

        Loaded  += OnLoaded;
        KeyDown += OnKeyDown;
    }

    // -------------------------------------------------------------------------
    // Startup / shutdown
    // -------------------------------------------------------------------------

    private void OnLoaded(object sender, RoutedEventArgs e)
    {
        try
        {
            _engine = new Engine();
            bool ok = _engine.Initialize(editorMode: true);
            if (!ok)
            {
                MessageBox.Show("Engine initialization failed.", "Fresh Voxel Editor",
                    MessageBoxButton.OK, MessageBoxImage.Error);
                Close();
                return;
            }

            _viewModel = new EditorViewModel(_engine);
            DataContext = _viewModel;

            // Wire panels to the engine / view-model
            SceneOutliner.ViewModel = _viewModel;
            OutputLog.ViewModel     = _viewModel;
            DetailsPanel.ViewModel  = _viewModel;
            DetailsPanel.Engine     = _engine;
            ContentBrowser.Engine   = _engine;

            // Attach engine to the viewport HwndHost — starts the render loop
            ViewportHost.AttachEngine(_engine);

            // Point the content browser at the project's assets folder
            string assetRoot = System.IO.Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory, "assets");
            if (System.IO.Directory.Exists(assetRoot))
                ContentBrowser.SetRootDirectory(assetRoot);

            // Initial scene refresh
            _viewModel.RefreshSceneEntities();

            StatusLabel.Text = "Engine ready";
            Title = "Fresh Voxel Editor — New Scene";
        }
        catch (Exception ex)
        {
            MessageBox.Show($"Startup error:\n{ex.Message}", "Fresh Voxel Editor",
                MessageBoxButton.OK, MessageBoxImage.Error);
        }
    }

    private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
    {
        ViewportHost.DetachEngine();
        _viewModel?.Dispose();

        _engine?.Shutdown();
        _engine?.Dispose();
        _engine = null;
    }

    // -------------------------------------------------------------------------
    // Keyboard shortcuts
    // -------------------------------------------------------------------------

    private void OnKeyDown(object sender, KeyEventArgs e)
    {
        if (_engine is null) return;
        switch (e.Key)
        {
            case Key.Z when Keyboard.Modifiers == ModifierKeys.Control:
                _engine.Undo(); e.Handled = true; break;
            case Key.Y when Keyboard.Modifiers == ModifierKeys.Control:
                _engine.Redo(); e.Handled = true; break;
            case Key.F:
                _engine.FrameSelection(); e.Handled = true; break;
            case Key.W:
                GizmoTranslate_Click(this, null!); e.Handled = true; break;
            case Key.E:
                GizmoRotate_Click(this, null!); e.Handled = true; break;
            case Key.R:
                GizmoScale_Click(this, null!); e.Handled = true; break;
        }
    }

    // -------------------------------------------------------------------------
    // Menu handlers
    // -------------------------------------------------------------------------

    private void NewScene_Click(object sender, RoutedEventArgs e)
    {
        StatusLabel.Text = "New Scene";
        Title = "Fresh Voxel Editor — New Scene";
        _viewModel?.RefreshSceneEntities();
    }

    private void OpenScene_Click(object sender, RoutedEventArgs e)
    {
        var dlg = new Microsoft.Win32.OpenFileDialog
        {
            Filter = "Fresh Scene (*.fvscene)|*.fvscene|All files (*.*)|*.*",
            Title  = "Open Scene"
        };
        if (dlg.ShowDialog() == true)
        {
            StatusLabel.Text = $"Opened: {System.IO.Path.GetFileName(dlg.FileName)}";
            Title = $"Fresh Voxel Editor — {System.IO.Path.GetFileNameWithoutExtension(dlg.FileName)}";
        }
    }

    private void SaveScene_Click(object sender, RoutedEventArgs e) =>
        StatusLabel.Text = "Scene saved";

    private void Exit_Click(object sender, RoutedEventArgs e) => Close();

    private void Undo_Click(object sender, RoutedEventArgs e) => _engine?.Undo();
    private void Redo_Click(object sender, RoutedEventArgs e) => _engine?.Redo();

    private void ImportVox_Click(object sender, RoutedEventArgs e)
    {
        if (_engine is null) return;
        var dlg = new Microsoft.Win32.OpenFileDialog
        {
            Filter = "MagicaVoxel (*.vox)|*.vox|All files (*.*)|*.*",
            Title  = "Import .vox File"
        };
        if (dlg.ShowDialog() != true) return;

        bool ok = _engine.LoadVoxFile(dlg.FileName, 0, 0, 0);
        StatusLabel.Text = ok
            ? $"Imported: {System.IO.Path.GetFileName(dlg.FileName)}"
            : $"Import failed: {System.IO.Path.GetFileName(dlg.FileName)}";
        _viewModel?.RefreshSceneEntities();
    }

    private void ExportVox_Click(object sender, RoutedEventArgs e)
    {
        // Future: allow user to define region, then call VoxExporter via engine API
        MessageBox.Show("Export to .vox — select a region in the viewport first.",
            "Export .vox", MessageBoxButton.OK, MessageBoxImage.Information);
    }

    private void BuildAll_Click(object sender, RoutedEventArgs e) =>
        StatusLabel.Text = "Build complete";

    private void About_Click(object sender, RoutedEventArgs e) =>
        MessageBox.Show("Fresh Voxel Engine v0.2.7 Alpha\nWPF Editor Layer\n\n" +
                        "Borderlands-style cell shading enabled.",
            "About", MessageBoxButton.OK, MessageBoxImage.Information);

    // -------------------------------------------------------------------------
    // Toolbar handlers
    // -------------------------------------------------------------------------

    private void Play_Click(object sender, RoutedEventArgs e)
    {
        if (_engine is null) return;
        _engine.IsEditorMode = false;
        PlayButton.IsEnabled = false;
        StopButton.IsEnabled = true;
        ViewportModeLabel.Text = "PLAYING";
        StatusLabel.Text = "Play mode";
    }

    private void Stop_Click(object sender, RoutedEventArgs e)
    {
        if (_engine is null) return;
        _engine.IsEditorMode = true;
        PlayButton.IsEnabled = true;
        StopButton.IsEnabled = false;
        ViewportModeLabel.Text = "EDITOR";
        StatusLabel.Text = "Editor mode";
    }

    private void GizmoTranslate_Click(object sender, RoutedEventArgs e) =>
        StatusLabel.Text = "Gizmo: Translate";

    private void GizmoRotate_Click(object sender, RoutedEventArgs e) =>
        StatusLabel.Text = "Gizmo: Rotate";

    private void GizmoScale_Click(object sender, RoutedEventArgs e) =>
        StatusLabel.Text = "Gizmo: Scale";

    private void CellShading_Changed(object sender, RoutedEventArgs e)
    {
        if (_engine is null) return;
        bool enabled = CellShadingToggle.IsChecked == true;
        _engine.CellShadingEnabled = enabled;
        StatusLabel.Text = enabled ? "Cell shading ON" : "Cell shading OFF";
    }
}
