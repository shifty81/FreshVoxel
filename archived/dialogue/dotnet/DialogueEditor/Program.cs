using System;
using System.Windows.Forms;

namespace FreshEngine.DialogueEditor;

/// <summary>
/// Entry point for the Dialogue Editor application
/// </summary>
public static class Program
{
    [STAThread]
    public static void Main(string[] args)
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.SetHighDpiMode(HighDpiMode.SystemAware);

        Console.WriteLine("==============================================");
        Console.WriteLine("Fresh Engine - Dialogue Editor");
        Console.WriteLine("==============================================");
        Console.WriteLine("Visual flowchart-based dialogue editor");
        Console.WriteLine("with Lua scripting support");
        Console.WriteLine();
        Console.WriteLine("Features:");
        Console.WriteLine("  - Node-based visual dialogue editing");
        Console.WriteLine("  - Drag and drop node creation");
        Console.WriteLine("  - Lua script integration for logic");
        Console.WriteLine("  - JSON serialization for dialogue data");
        Console.WriteLine("  - Unreal Engine-style dark theme");
        Console.WriteLine();
        Console.WriteLine("Controls:");
        Console.WriteLine("  - Left Click: Select/drag node");
        Console.WriteLine("  - Ctrl + Left Click: Create link between nodes");
        Console.WriteLine("  - Right Click: Context menu");
        Console.WriteLine("  - Double Click: Edit node");
        Console.WriteLine("  - Mouse Wheel: Zoom in/out");
        Console.WriteLine();
        Console.WriteLine("Opening Dialogue Editor...");
        Console.WriteLine("==============================================");
        Console.WriteLine();

        try
        {
            using var form = new DialogueEditorForm();
            Application.Run(form);
        }
        catch (Exception ex)
        {
            MessageBox.Show(
                $"Fatal error: {ex.Message}\n\n" +
                $"Stack trace:\n{ex.StackTrace}",
                "Application Error",
                MessageBoxButtons.OK,
                MessageBoxIcon.Error);
            
            Console.WriteLine($"ERROR: {ex.Message}");
            Console.WriteLine(ex.StackTrace);
        }
    }
}
