using System.Drawing;
using System.Windows.Forms;

namespace FreshEngine.DialogueEditor;

/// <summary>
/// Custom renderer for Unreal Engine-style dark theme menus and toolbars
/// </summary>
public class UnrealStyleRenderer : ToolStripProfessionalRenderer
{
    public UnrealStyleRenderer() : base(new UnrealColorTable())
    {
    }

    protected override void OnRenderMenuItemBackground(ToolStripItemRenderEventArgs e)
    {
        if (!e.Item.Selected)
        {
            base.OnRenderMenuItemBackground(e);
        }
        else
        {
            // Unreal-style hover/selection color
            var rect = new Rectangle(Point.Empty, e.Item.Size);
            using var brush = new SolidBrush(Color.FromArgb(70, 70, 70));
            e.Graphics.FillRectangle(brush, rect);
        }
    }

    protected override void OnRenderButtonBackground(ToolStripItemRenderEventArgs e)
    {
        var button = e.Item as ToolStripButton;
        if (button != null)
        {
            var rect = new Rectangle(Point.Empty, e.Item.Size);
            
            if (button.Pressed)
            {
                // Pressed state - darker
                using var brush = new SolidBrush(Color.FromArgb(25, 25, 25));
                e.Graphics.FillRectangle(brush, rect);
            }
            else if (button.Selected)
            {
                // Hover state - lighter
                using var brush = new SolidBrush(Color.FromArgb(70, 70, 70));
                e.Graphics.FillRectangle(brush, rect);
            }
            else
            {
                // Normal state
                using var brush = new SolidBrush(Color.FromArgb(42, 42, 42));
                e.Graphics.FillRectangle(brush, rect);
            }
        }
        else
        {
            base.OnRenderButtonBackground(e);
        }
    }

    protected override void OnRenderItemText(ToolStripItemTextRenderEventArgs e)
    {
        e.TextColor = e.Item.Enabled ? Color.FromArgb(210, 210, 210) : Color.FromArgb(100, 100, 100);
        base.OnRenderItemText(e);
    }

    protected override void OnRenderToolStripBorder(ToolStripRenderEventArgs e)
    {
        // No border for clean Unreal look
    }

    protected override void OnRenderSeparator(ToolStripSeparatorRenderEventArgs e)
    {
        var rect = new Rectangle(0, e.Item.Height / 2, e.Item.Width, 1);
        using var pen = new Pen(Color.FromArgb(60, 60, 60));
        e.Graphics.DrawLine(pen, rect.Left, rect.Top, rect.Right, rect.Top);
    }
}

/// <summary>
/// Color table for Unreal Engine-style dark theme
/// </summary>
public class UnrealColorTable : ProfessionalColorTable
{
    public override Color MenuBorder => Color.FromArgb(60, 60, 60);
    
    public override Color MenuItemBorder => Color.FromArgb(80, 80, 80);
    
    public override Color MenuItemSelected => Color.FromArgb(70, 70, 70);
    
    public override Color MenuItemSelectedGradientBegin => Color.FromArgb(70, 70, 70);
    
    public override Color MenuItemSelectedGradientEnd => Color.FromArgb(70, 70, 70);
    
    public override Color MenuItemPressedGradientBegin => Color.FromArgb(50, 50, 50);
    
    public override Color MenuItemPressedGradientEnd => Color.FromArgb(50, 50, 50);
    
    public override Color MenuStripGradientBegin => Color.FromArgb(42, 42, 42);
    
    public override Color MenuStripGradientEnd => Color.FromArgb(42, 42, 42);
    
    public override Color ToolStripBorder => Color.FromArgb(42, 42, 42);
    
    public override Color ToolStripDropDownBackground => Color.FromArgb(42, 42, 42);
    
    public override Color ToolStripGradientBegin => Color.FromArgb(42, 42, 42);
    
    public override Color ToolStripGradientEnd => Color.FromArgb(42, 42, 42);
    
    public override Color ToolStripGradientMiddle => Color.FromArgb(42, 42, 42);
    
    public override Color ToolStripPanelGradientBegin => Color.FromArgb(42, 42, 42);
    
    public override Color ToolStripPanelGradientEnd => Color.FromArgb(42, 42, 42);
    
    public override Color ButtonSelectedBorder => Color.FromArgb(100, 100, 100);
    
    public override Color ButtonSelectedGradientBegin => Color.FromArgb(70, 70, 70);
    
    public override Color ButtonSelectedGradientEnd => Color.FromArgb(70, 70, 70);
    
    public override Color ButtonSelectedGradientMiddle => Color.FromArgb(70, 70, 70);
    
    public override Color ButtonPressedBorder => Color.FromArgb(120, 120, 120);
    
    public override Color ButtonPressedGradientBegin => Color.FromArgb(50, 50, 50);
    
    public override Color ButtonPressedGradientEnd => Color.FromArgb(50, 50, 50);
    
    public override Color ButtonPressedGradientMiddle => Color.FromArgb(50, 50, 50);
    
    public override Color SeparatorDark => Color.FromArgb(60, 60, 60);
    
    public override Color SeparatorLight => Color.FromArgb(70, 70, 70);
    
    public override Color ImageMarginGradientBegin => Color.FromArgb(42, 42, 42);
    
    public override Color ImageMarginGradientEnd => Color.FromArgb(42, 42, 42);
    
    public override Color ImageMarginGradientMiddle => Color.FromArgb(42, 42, 42);
}
