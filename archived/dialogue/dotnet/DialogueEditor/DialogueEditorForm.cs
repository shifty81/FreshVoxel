using System;
using System.Drawing;
using System.Windows.Forms;
using FreshEngine.DialogueSystem;

namespace FreshEngine.DialogueEditor;

/// <summary>
/// Main form for the dialogue editor
/// </summary>
public class DialogueEditorForm : Form
{
    private DialogueGraph _graph;
    private DialogueNode? _selectedNode;
    private DialogueLink? _selectedLink;
    private Point _dragOffset;
    private bool _isDraggingNode;
    private bool _isCreatingLink;
    private DialogueNode? _linkSourceNode;
    private Point _linkEndPoint;

    // UI Components
    private Panel _canvasPanel;
    private Panel _propertiesPanel;
    private MenuStrip _menuStrip;
    private ToolStrip _toolStrip;
    private PropertyGrid _propertyGrid;
    private StatusStrip _statusStrip;
    private ToolStripStatusLabel _statusLabel;

    // Transform state
    private float _zoomLevel = 1.0f;
    private Point _panOffset = Point.Empty;

    public DialogueEditorForm()
    {
        _graph = new DialogueGraph();
        InitializeComponents();
        SetupEventHandlers();
    }

    private void InitializeComponents()
    {
        // Form settings - Unreal Engine style
        Text = "Fresh Engine - Dialogue Editor";
        Size = new Size(1400, 900);
        StartPosition = FormStartPosition.CenterScreen;
        BackColor = Color.FromArgb(42, 42, 42); // Unreal dark background
        ForeColor = Color.FromArgb(210, 210, 210); // Light text

        // Menu Strip - Unreal Engine dark theme
        _menuStrip = new MenuStrip();
        _menuStrip.BackColor = Color.FromArgb(42, 42, 42);
        _menuStrip.ForeColor = Color.FromArgb(210, 210, 210);
        _menuStrip.Renderer = new UnrealStyleRenderer();
        
        var fileMenu = new ToolStripMenuItem("&File");
        fileMenu.DropDownItems.Add("&New", null, (s, e) => NewDialogue());
        fileMenu.DropDownItems.Add("&Open...", null, (s, e) => OpenDialogue());
        fileMenu.DropDownItems.Add("&Save", null, (s, e) => SaveDialogue());
        fileMenu.DropDownItems.Add("Save &As...", null, (s, e) => SaveDialogueAs());
        fileMenu.DropDownItems.Add(new ToolStripSeparator());
        fileMenu.DropDownItems.Add("E&xit", null, (s, e) => Close());
        _menuStrip.Items.Add(fileMenu);

        var editMenu = new ToolStripMenuItem("&Edit");
        editMenu.DropDownItems.Add("&Delete Node", null, (s, e) => DeleteSelectedNode());
        editMenu.DropDownItems.Add("Delete &Link", null, (s, e) => DeleteSelectedLink());
        _menuStrip.Items.Add(editMenu);

        var viewMenu = new ToolStripMenuItem("&View");
        viewMenu.DropDownItems.Add("&Zoom In", null, (s, e) => ZoomIn());
        viewMenu.DropDownItems.Add("Zoom &Out", null, (s, e) => ZoomOut());
        viewMenu.DropDownItems.Add("&Reset View", null, (s, e) => ResetView());
        _menuStrip.Items.Add(viewMenu);

        var helpMenu = new ToolStripMenuItem("&Help");
        helpMenu.DropDownItems.Add("&About", null, (s, e) => ShowAbout());
        _menuStrip.Items.Add(helpMenu);

        Controls.Add(_menuStrip);
        MainMenuStrip = _menuStrip;

        // Tool Strip - Unreal Engine dark theme
        _toolStrip = new ToolStrip();
        _toolStrip.BackColor = Color.FromArgb(42, 42, 42);
        _toolStrip.ForeColor = Color.FromArgb(210, 210, 210);
        _toolStrip.Renderer = new UnrealStyleRenderer();
        _toolStrip.GripStyle = ToolStripGripStyle.Hidden;
        _toolStrip.Padding = new Padding(5, 2, 5, 2);
        
        _toolStrip.Items.Add(new ToolStripButton("New", null, (s, e) => NewDialogue()) { DisplayStyle = ToolStripItemDisplayStyle.Text });
        _toolStrip.Items.Add(new ToolStripButton("Open", null, (s, e) => OpenDialogue()) { DisplayStyle = ToolStripItemDisplayStyle.Text });
        _toolStrip.Items.Add(new ToolStripButton("Save", null, (s, e) => SaveDialogue()) { DisplayStyle = ToolStripItemDisplayStyle.Text });
        _toolStrip.Items.Add(new ToolStripSeparator());
        
        // Node type buttons - Unreal style with colored indicators
        var addDialogueBtn = new ToolStripButton("+ Dialogue", null, (s, e) => AddNode(DialogueNodeType.Dialogue));
        addDialogueBtn.DisplayStyle = ToolStripItemDisplayStyle.Text;
        addDialogueBtn.ForeColor = Color.FromArgb(150, 200, 220);
        _toolStrip.Items.Add(addDialogueBtn);

        var addChoiceBtn = new ToolStripButton("+ Choice", null, (s, e) => AddNode(DialogueNodeType.Choice));
        addChoiceBtn.DisplayStyle = ToolStripItemDisplayStyle.Text;
        addChoiceBtn.ForeColor = Color.FromArgb(100, 180, 255);
        _toolStrip.Items.Add(addChoiceBtn);

        var addLuaBtn = new ToolStripButton("+ Lua Script", null, (s, e) => AddNode(DialogueNodeType.LuaScript));
        addLuaBtn.DisplayStyle = ToolStripItemDisplayStyle.Text;
        addLuaBtn.ForeColor = Color.FromArgb(200, 120, 255);
        _toolStrip.Items.Add(addLuaBtn);

        var addConditionBtn = new ToolStripButton("+ Condition", null, (s, e) => AddNode(DialogueNodeType.Condition));
        addConditionBtn.DisplayStyle = ToolStripItemDisplayStyle.Text;
        addConditionBtn.ForeColor = Color.FromArgb(255, 180, 100);
        _toolStrip.Items.Add(addConditionBtn);

        var addEndBtn = new ToolStripButton("+ End", null, (s, e) => AddNode(DialogueNodeType.End));
        addEndBtn.DisplayStyle = ToolStripItemDisplayStyle.Text;
        addEndBtn.ForeColor = Color.FromArgb(255, 120, 120);
        _toolStrip.Items.Add(addEndBtn);

        Controls.Add(_toolStrip);

        // Main split container
        var splitContainer = new SplitContainer
        {
            Dock = DockStyle.Fill,
            Orientation = Orientation.Vertical,
            SplitterDistance = 1000,
            FixedPanel = FixedPanel.Panel2
        };

        // Canvas Panel - Unreal Engine viewport style (darker background)
        _canvasPanel = new Panel
        {
            Dock = DockStyle.Fill,
            BackColor = Color.FromArgb(32, 32, 32), // Unreal viewport background
            BorderStyle = BorderStyle.None
        };
        _canvasPanel.Paint += Canvas_Paint;
        _canvasPanel.MouseDown += Canvas_MouseDown;
        _canvasPanel.MouseMove += Canvas_MouseMove;
        _canvasPanel.MouseUp += Canvas_MouseUp;
        _canvasPanel.MouseWheel += Canvas_MouseWheel;
        splitContainer.Panel1.Controls.Add(_canvasPanel);

        // Properties Panel - Unreal Details panel style
        _propertiesPanel = new Panel
        {
            Dock = DockStyle.Fill,
            BackColor = Color.FromArgb(42, 42, 42)
        };

        var propertiesLabel = new Label
        {
            Text = "Details", // Unreal uses "Details" instead of "Properties"
            Dock = DockStyle.Top,
            Height = 32,
            BackColor = Color.FromArgb(28, 28, 28),
            ForeColor = Color.FromArgb(210, 210, 210),
            TextAlign = ContentAlignment.MiddleLeft,
            Padding = new Padding(10, 0, 0, 0),
            Font = new Font("Segoe UI", 9, FontStyle.Bold)
        };
        _propertiesPanel.Controls.Add(propertiesLabel);

        _propertyGrid = new PropertyGrid
        {
            Dock = DockStyle.Fill,
            BackColor = Color.FromArgb(42, 42, 42),
            ViewBackColor = Color.FromArgb(42, 42, 42),
            ViewForeColor = Color.FromArgb(210, 210, 210),
            CategoryForeColor = Color.FromArgb(210, 210, 210),
            HelpBackColor = Color.FromArgb(28, 28, 28),
            HelpForeColor = Color.FromArgb(210, 210, 210),
            LineColor = Color.FromArgb(60, 60, 60),
            CategorySplitterColor = Color.FromArgb(60, 60, 60),
            DisabledItemForeColor = Color.FromArgb(128, 128, 128)
        };
        _propertyGrid.PropertyValueChanged += PropertyGrid_PropertyValueChanged;
        _propertiesPanel.Controls.Add(_propertyGrid);
        propertiesLabel.BringToFront();

        splitContainer.Panel2.Controls.Add(_propertiesPanel);
        
        // Style the splitter - Unreal style
        splitContainer.BackColor = Color.FromArgb(28, 28, 28);
        splitContainer.Panel1.BackColor = Color.FromArgb(32, 32, 32);
        splitContainer.Panel2.BackColor = Color.FromArgb(42, 42, 42);
        
        Controls.Add(splitContainer);

        // Status Strip - Unreal style
        _statusStrip = new StatusStrip();
        _statusStrip.BackColor = Color.FromArgb(28, 28, 28);
        _statusStrip.ForeColor = Color.FromArgb(180, 180, 180);
        _statusStrip.Renderer = new UnrealStyleRenderer();
        _statusLabel = new ToolStripStatusLabel("Ready");
        _statusLabel.ForeColor = Color.FromArgb(180, 180, 180);
        _statusStrip.Items.Add(_statusLabel);
        Controls.Add(_statusStrip);

        // Add initial start node
        var startNode = new DialogueNode(DialogueNodeType.Start)
        {
            PositionX = 50,
            PositionY = 50
        };
        _graph.AddNode(startNode);
    }

    private void SetupEventHandlers()
    {
        _canvasPanel.DoubleClick += (s, e) => 
        {
            var mousePos = _canvasPanel.PointToClient(MousePosition);
            var worldPos = ScreenToWorld(mousePos);
            var node = GetNodeAtPosition(worldPos);
            if (node != null)
            {
                EditNodeText(node);
            }
        };
    }

    // Canvas rendering
    private void Canvas_Paint(object? sender, PaintEventArgs e)
    {
        var g = e.Graphics;
        g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;

        // Apply transform
        g.TranslateTransform(_panOffset.X, _panOffset.Y);
        g.ScaleTransform(_zoomLevel, _zoomLevel);

        // Draw grid
        DrawGrid(g);

        // Draw links
        foreach (var link in _graph.Links)
        {
            DrawLink(g, link);
        }

        // Draw nodes
        foreach (var node in _graph.Nodes)
        {
            DrawNode(g, node);
        }

        // Draw temporary link when creating
        if (_isCreatingLink && _linkSourceNode != null)
        {
            var startPoint = GetNodeCenter(_linkSourceNode);
            using var pen = new Pen(Color.Yellow, 2);
            g.DrawLine(pen, startPoint, ScreenToWorld(_linkEndPoint));
        }
    }

    private void DrawGrid(Graphics g)
    {
        const int gridSize = 50;
        const int majorGridSize = 200; // Unreal has major/minor grid lines
        var clientRect = _canvasPanel.ClientRectangle;
        
        // Minor grid lines - subtle like Unreal
        using (var minorPen = new Pen(Color.FromArgb(40, 40, 40)))
        {
            for (int x = 0; x < clientRect.Width / _zoomLevel; x += gridSize)
            {
                g.DrawLine(minorPen, x, 0, x, clientRect.Height / _zoomLevel);
            }
            
            for (int y = 0; y < clientRect.Height / _zoomLevel; y += gridSize)
            {
                g.DrawLine(minorPen, 0, y, clientRect.Width / _zoomLevel, y);
            }
        }
        
        // Major grid lines - slightly brighter like Unreal
        using (var majorPen = new Pen(Color.FromArgb(55, 55, 55)))
        {
            for (int x = 0; x < clientRect.Width / _zoomLevel; x += majorGridSize)
            {
                g.DrawLine(majorPen, x, 0, x, clientRect.Height / _zoomLevel);
            }
            
            for (int y = 0; y < clientRect.Height / _zoomLevel; y += majorGridSize)
            {
                g.DrawLine(majorPen, 0, y, clientRect.Width / _zoomLevel, y);
            }
        }
    }

    private void DrawNode(Graphics g, DialogueNode node)
    {
        var rect = new RectangleF(node.PositionX, node.PositionY, node.Width, node.Height);
        
        // Node color based on type - Unreal Blueprint style colors
        Color nodeColor = node.NodeType switch
        {
            DialogueNodeType.Start => Color.FromArgb(60, 140, 60), // Green
            DialogueNodeType.End => Color.FromArgb(180, 60, 60), // Red
            DialogueNodeType.Choice => Color.FromArgb(80, 140, 200), // Blue
            DialogueNodeType.LuaScript => Color.FromArgb(140, 80, 180), // Purple
            DialogueNodeType.Condition => Color.FromArgb(200, 140, 60), // Orange
            _ => Color.FromArgb(70, 90, 110) // Gray-blue
        };

        // Unreal-style node rendering with header bar
        var headerHeight = 28f;
        var headerRect = new RectangleF(rect.X, rect.Y, rect.Width, headerHeight);
        var bodyRect = new RectangleF(rect.X, rect.Y + headerHeight, rect.Width, rect.Height - headerHeight);

        // Draw shadow (Unreal style - offset and blurred)
        var shadowRect = rect;
        shadowRect.Offset(4, 4);
        using (var shadowBrush = new SolidBrush(Color.FromArgb(80, 0, 0, 0)))
        {
            g.FillRectangle(shadowBrush, shadowRect);
        }

        // Draw header - colored based on node type
        using (var headerBrush = new SolidBrush(nodeColor))
        {
            g.FillRectangle(headerBrush, headerRect);
        }

        // Draw body - dark background like Unreal nodes
        using (var bodyBrush = new SolidBrush(Color.FromArgb(48, 48, 48)))
        {
            g.FillRectangle(bodyBrush, bodyRect);
        }

        // Draw selection border - bright like Unreal's selection
        if (node == _selectedNode)
        {
            using var pen = new Pen(Color.FromArgb(255, 200, 80), 2);
            g.DrawRectangle(pen, rect.X, rect.Y, rect.Width, rect.Height);
        }
        else
        {
            // Draw subtle border
            using var pen = new Pen(Color.FromArgb(80, 80, 80), 1);
            g.DrawRectangle(pen, rect.X, rect.Y, rect.Width, rect.Height);
        }

        // Draw title in header
        using (var font = new Font("Segoe UI", 9, FontStyle.Bold))
        using (var brush = new SolidBrush(Color.FromArgb(240, 240, 240)))
        {
            var format = new StringFormat { Alignment = StringAlignment.Center, LineAlignment = StringAlignment.Center };
            g.DrawString(node.Title, font, brush, headerRect, format);
        }

        // Draw content preview in body
        string preview = node.NodeType switch
        {
            DialogueNodeType.Dialogue => node.DialogueText,
            DialogueNodeType.Choice => $"{node.ChoiceOptions.Count} choices",
            DialogueNodeType.LuaScript => "Lua Script",
            DialogueNodeType.Condition => node.Condition,
            _ => ""
        };

        if (!string.IsNullOrEmpty(preview))
        {
            using var font = new Font("Segoe UI", 8);
            using var brush = new SolidBrush(Color.FromArgb(200, 200, 200));
            var format = new StringFormat { Alignment = StringAlignment.Center, LineAlignment = StringAlignment.Center };
            var contentRect = new RectangleF(rect.X + 8, rect.Y + headerHeight + 4, rect.Width - 16, rect.Height - headerHeight - 8);
            g.DrawString(TruncateText(preview, 50), font, brush, contentRect, format);
        }

        // Draw connection points (pins) - Unreal style
        DrawConnectionPins(g, node);
    }

    private void DrawConnectionPins(Graphics g, DialogueNode node)
    {
        const float pinSize = 12f;
        const float pinOffset = 14f;
        
        // Input pin (left side) - Unreal uses circles for pins
        if (node.NodeType != DialogueNodeType.Start)
        {
            var inputPinX = node.PositionX - pinSize / 2;
            var inputPinY = node.PositionY + node.Height / 2 - pinSize / 2;
            using (var brush = new SolidBrush(Color.FromArgb(80, 180, 80)))
            using (var pen = new Pen(Color.FromArgb(40, 40, 40), 2))
            {
                g.FillEllipse(brush, inputPinX, inputPinY, pinSize, pinSize);
                g.DrawEllipse(pen, inputPinX, inputPinY, pinSize, pinSize);
            }
        }

        // Output pin (right side)
        if (node.NodeType != DialogueNodeType.End)
        {
            var outputPinX = node.PositionX + node.Width - pinSize / 2;
            var outputPinY = node.PositionY + node.Height / 2 - pinSize / 2;
            using (var brush = new SolidBrush(Color.FromArgb(80, 180, 80)))
            using (var pen = new Pen(Color.FromArgb(40, 40, 40), 2))
            {
                g.FillEllipse(brush, outputPinX, outputPinY, pinSize, pinSize);
                g.DrawEllipse(pen, outputPinX, outputPinY, pinSize, pinSize);
            }
        }
    }

    private void DrawLink(Graphics g, DialogueLink link)
    {
        var sourceNode = _graph.GetNode(link.SourceNodeId);
        var targetNode = _graph.GetNode(link.TargetNodeId);

        if (sourceNode == null || targetNode == null)
            return;

        // Calculate connection points from pins (Unreal style)
        var start = new PointF(sourceNode.PositionX + sourceNode.Width, sourceNode.PositionY + sourceNode.Height / 2);
        var end = new PointF(targetNode.PositionX, targetNode.PositionY + targetNode.Height / 2);

        // Unreal-style Bezier curve connection
        var controlPointOffset = Math.Abs(end.X - start.X) * 0.5f;
        var cp1 = new PointF(start.X + controlPointOffset, start.Y);
        var cp2 = new PointF(end.X - controlPointOffset, end.Y);

        // Draw connection line with Bezier curve (Unreal style)
        using (var pen = new Pen(link == _selectedLink ? Color.FromArgb(255, 200, 80) : Color.FromArgb(150, 150, 150), 2.5f))
        {
            pen.EndCap = System.Drawing.Drawing2D.LineCap.Round;
            g.DrawBezier(pen, start, cp1, cp2, end);
            
            // Draw arrow at the end
            DrawArrowHead(g, cp2, end, pen.Color);
        }

        // Draw label if present (Unreal style with background box)
        if (!string.IsNullOrEmpty(link.Label))
        {
            var midPoint = CalculateBezierPoint(start, cp1, cp2, end, 0.5f);
            using var font = new Font("Segoe UI", 8, FontStyle.Bold);
            using var textBrush = new SolidBrush(Color.FromArgb(240, 240, 240));
            using var bgBrush = new SolidBrush(Color.FromArgb(220, 30, 30, 30));
            using var borderPen = new Pen(Color.FromArgb(100, 100, 100), 1);
            
            var size = g.MeasureString(link.Label, font);
            var labelRect = new RectangleF(
                midPoint.X - size.Width / 2 - 4, 
                midPoint.Y - size.Height / 2 - 2, 
                size.Width + 8, 
                size.Height + 4);
            
            g.FillRectangle(bgBrush, labelRect);
            g.DrawRectangle(borderPen, labelRect.X, labelRect.Y, labelRect.Width, labelRect.Height);
            g.DrawString(link.Label, font, textBrush, midPoint.X, midPoint.Y, 
                new StringFormat { Alignment = StringAlignment.Center, LineAlignment = StringAlignment.Center });
        }
    }

    private void DrawArrowHead(Graphics g, PointF p1, PointF p2, Color color)
    {
        const float arrowSize = 10f;
        var angle = Math.Atan2(p2.Y - p1.Y, p2.X - p1.X);
        
        var arrowP1 = new PointF(
            p2.X - arrowSize * (float)Math.Cos(angle - Math.PI / 6),
            p2.Y - arrowSize * (float)Math.Sin(angle - Math.PI / 6));
        
        var arrowP2 = new PointF(
            p2.X - arrowSize * (float)Math.Cos(angle + Math.PI / 6),
            p2.Y - arrowSize * (float)Math.Sin(angle + Math.PI / 6));

        using var brush = new SolidBrush(color);
        g.FillPolygon(brush, new[] { p2, arrowP1, arrowP2 });
    }

    private PointF CalculateBezierPoint(PointF p0, PointF p1, PointF p2, PointF p3, float t)
    {
        var u = 1 - t;
        var tt = t * t;
        var uu = u * u;
        var uuu = uu * u;
        var ttt = tt * t;

        var x = uuu * p0.X + 3 * uu * t * p1.X + 3 * u * tt * p2.X + ttt * p3.X;
        var y = uuu * p0.Y + 3 * uu * t * p1.Y + 3 * u * tt * p2.Y + ttt * p3.Y;

        return new PointF(x, y);
    }

    private Point GetNodeCenter(DialogueNode node)
    {
        return new Point(
            (int)(node.PositionX + node.Width / 2),
            (int)(node.PositionY + node.Height / 2)
        );
    }

    private DialogueNode? GetNodeAtPosition(Point position)
    {
        for (int i = _graph.Nodes.Count - 1; i >= 0; i--)
        {
            var node = _graph.Nodes[i];
            var rect = new RectangleF(node.PositionX, node.PositionY, node.Width, node.Height);
            if (rect.Contains(position))
                return node;
        }
        return null;
    }

    private Point ScreenToWorld(Point screenPoint)
    {
        return new Point(
            (int)((screenPoint.X - _panOffset.X) / _zoomLevel),
            (int)((screenPoint.Y - _panOffset.Y) / _zoomLevel)
        );
    }

    private string TruncateText(string text, int maxLength)
    {
        if (text.Length <= maxLength)
            return text;
        return text.Substring(0, maxLength) + "...";
    }

    // Mouse event handlers
    private void Canvas_MouseDown(object? sender, MouseEventArgs e)
    {
        var worldPos = ScreenToWorld(e.Location);
        var clickedNode = GetNodeAtPosition(worldPos);

        if (e.Button == MouseButtons.Left)
        {
            if (ModifierKeys.HasFlag(Keys.Control))
            {
                // Start creating link
                if (clickedNode != null)
                {
                    _isCreatingLink = true;
                    _linkSourceNode = clickedNode;
                    _linkEndPoint = e.Location;
                }
            }
            else if (clickedNode != null)
            {
                // Start dragging node
                _selectedNode = clickedNode;
                _isDraggingNode = true;
                _dragOffset = new Point((int)(worldPos.X - clickedNode.PositionX), (int)(worldPos.Y - clickedNode.PositionY));
                _propertyGrid.SelectedObject = clickedNode;
            }
            else
            {
                // Deselect
                _selectedNode = null;
                _propertyGrid.SelectedObject = null;
            }
        }
        else if (e.Button == MouseButtons.Right)
        {
            ShowContextMenu(worldPos, clickedNode);
        }

        _canvasPanel.Invalidate();
    }

    private void Canvas_MouseMove(object? sender, MouseEventArgs e)
    {
        var worldPos = ScreenToWorld(e.Location);

        if (_isDraggingNode && _selectedNode != null)
        {
            _selectedNode.PositionX = worldPos.X - _dragOffset.X;
            _selectedNode.PositionY = worldPos.Y - _dragOffset.Y;
            _canvasPanel.Invalidate();
        }
        else if (_isCreatingLink)
        {
            _linkEndPoint = e.Location;
            _canvasPanel.Invalidate();
        }

        // Update status
        _statusLabel.Text = $"Position: ({worldPos.X}, {worldPos.Y}) | Zoom: {_zoomLevel:P0}";
    }

    private void Canvas_MouseUp(object? sender, MouseEventArgs e)
    {
        if (_isCreatingLink && _linkSourceNode != null)
        {
            var worldPos = ScreenToWorld(e.Location);
            var targetNode = GetNodeAtPosition(worldPos);

            if (targetNode != null && targetNode != _linkSourceNode)
            {
                // Create link
                var link = new DialogueLink(_linkSourceNode.Id, targetNode.Id);
                _graph.AddLink(link);
                _canvasPanel.Invalidate();
            }

            _isCreatingLink = false;
            _linkSourceNode = null;
        }

        _isDraggingNode = false;
    }

    private void Canvas_MouseWheel(object? sender, MouseEventArgs e)
    {
        if (e.Delta > 0)
            ZoomIn();
        else
            ZoomOut();
    }

    // Menu handlers
    private void NewDialogue()
    {
        if (ConfirmUnsavedChanges())
        {
            _graph = new DialogueGraph();
            var startNode = new DialogueNode(DialogueNodeType.Start) { PositionX = 50, PositionY = 50 };
            _graph.AddNode(startNode);
            _selectedNode = null;
            _propertyGrid.SelectedObject = null;
            _canvasPanel.Invalidate();
            _statusLabel.Text = "New dialogue created";
        }
    }

    private void OpenDialogue()
    {
        if (!ConfirmUnsavedChanges())
            return;

        using var dialog = new OpenFileDialog
        {
            Filter = "Dialogue Files (*.json)|*.json|All Files (*.*)|*.*",
            Title = "Open Dialogue"
        };

        if (dialog.ShowDialog() == DialogResult.OK)
        {
            try
            {
                _graph = DialogueGraph.LoadFromFile(dialog.FileName) ?? new DialogueGraph();
                _selectedNode = null;
                _propertyGrid.SelectedObject = null;
                _canvasPanel.Invalidate();
                _statusLabel.Text = $"Opened: {dialog.FileName}";
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error opening file: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }

    private void SaveDialogue()
    {
        using var dialog = new SaveFileDialog
        {
            Filter = "Dialogue Files (*.json)|*.json|All Files (*.*)|*.*",
            Title = "Save Dialogue",
            DefaultExt = "json"
        };

        if (dialog.ShowDialog() == DialogResult.OK)
        {
            try
            {
                _graph.SaveToFile(dialog.FileName);
                _statusLabel.Text = $"Saved: {dialog.FileName}";
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error saving file: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }

    private void SaveDialogueAs()
    {
        SaveDialogue();
    }

    private void AddNode(DialogueNodeType type)
    {
        var node = new DialogueNode(type)
        {
            PositionX = 100 + _graph.Nodes.Count * 50,
            PositionY = 100 + _graph.Nodes.Count * 30
        };
        _graph.AddNode(node);
        _selectedNode = node;
        _propertyGrid.SelectedObject = node;
        _canvasPanel.Invalidate();
        _statusLabel.Text = $"Added {type} node";
    }

    private void DeleteSelectedNode()
    {
        if (_selectedNode != null)
        {
            if (MessageBox.Show($"Delete node '{_selectedNode.Title}'?", "Confirm Delete", 
                MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
            {
                _graph.RemoveNode(_selectedNode.Id);
                _selectedNode = null;
                _propertyGrid.SelectedObject = null;
                _canvasPanel.Invalidate();
                _statusLabel.Text = "Node deleted";
            }
        }
    }

    private void DeleteSelectedLink()
    {
        if (_selectedLink != null)
        {
            _graph.RemoveLink(_selectedLink.Id);
            _selectedLink = null;
            _canvasPanel.Invalidate();
            _statusLabel.Text = "Link deleted";
        }
    }

    private void ZoomIn()
    {
        _zoomLevel = Math.Min(_zoomLevel * 1.2f, 3.0f);
        _canvasPanel.Invalidate();
    }

    private void ZoomOut()
    {
        _zoomLevel = Math.Max(_zoomLevel / 1.2f, 0.25f);
        _canvasPanel.Invalidate();
    }

    private void ResetView()
    {
        _zoomLevel = 1.0f;
        _panOffset = Point.Empty;
        _canvasPanel.Invalidate();
    }

    private void ShowAbout()
    {
        MessageBox.Show(
            "Fresh Engine - Dialogue Editor\n" +
            "Version 0.1.0\n\n" +
            "Visual flowchart-based dialogue editor with Lua scripting support.\n\n" +
            "Controls:\n" +
            "- Left Click: Select node\n" +
            "- Ctrl + Left Click: Create link\n" +
            "- Right Click: Context menu\n" +
            "- Double Click: Edit node text\n" +
            "- Mouse Wheel: Zoom in/out",
            "About Dialogue Editor",
            MessageBoxButtons.OK,
            MessageBoxIcon.Information
        );
    }

    private void ShowContextMenu(Point position, DialogueNode? node)
    {
        var menu = new ContextMenuStrip();

        if (node != null)
        {
            menu.Items.Add("Edit Node", null, (s, e) => EditNodeText(node));
            menu.Items.Add("Delete Node", null, (s, e) => { _selectedNode = node; DeleteSelectedNode(); });
        }
        else
        {
            menu.Items.Add("Add Dialogue Node", null, (s, e) => AddNodeAtPosition(DialogueNodeType.Dialogue, position));
            menu.Items.Add("Add Choice Node", null, (s, e) => AddNodeAtPosition(DialogueNodeType.Choice, position));
            menu.Items.Add("Add Lua Script Node", null, (s, e) => AddNodeAtPosition(DialogueNodeType.LuaScript, position));
            menu.Items.Add("Add Condition Node", null, (s, e) => AddNodeAtPosition(DialogueNodeType.Condition, position));
            menu.Items.Add("Add End Node", null, (s, e) => AddNodeAtPosition(DialogueNodeType.End, position));
        }

        menu.Show(_canvasPanel, _canvasPanel.PointToClient(Cursor.Position));
    }

    private void AddNodeAtPosition(DialogueNodeType type, Point position)
    {
        var node = new DialogueNode(type)
        {
            PositionX = position.X,
            PositionY = position.Y
        };
        _graph.AddNode(node);
        _canvasPanel.Invalidate();
    }

    private void EditNodeText(DialogueNode node)
    {
        _selectedNode = node;
        _propertyGrid.SelectedObject = node;
    }

    private void PropertyGrid_PropertyValueChanged(object? s, PropertyValueChangedEventArgs e)
    {
        _canvasPanel.Invalidate();
    }

    private bool ConfirmUnsavedChanges()
    {
        // In a real application, track if there are unsaved changes
        return true;
    }
}
