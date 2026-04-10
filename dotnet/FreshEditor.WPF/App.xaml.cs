using System.Windows;

namespace FreshEditor.WPF;

/// <summary>WPF application entry point for the Fresh Voxel Editor.</summary>
public partial class App : Application
{
    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);
        var win = new MainWindow();
        win.Show();
    }
}
