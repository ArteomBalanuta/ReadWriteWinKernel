using System.Drawing;
using System.Windows.Forms;

internal sealed class ClientForm : Form
{
    private readonly ComboBox _operation = new() { DropDownStyle = ComboBoxStyle.DropDownList };
    private readonly TextBox _sourcePid = new();
    private readonly TextBox _sourceAddress = new();
    private readonly TextBox _targetPid = new();
    private readonly TextBox _targetAddress = new();
    private readonly TextBox _size = new();
    private readonly TextBox _status = new() { Multiline = true, ReadOnly = true, ScrollBars = ScrollBars.Vertical };

    internal ClientForm()
    {
        Text = "readDriver client";
        StartPosition = FormStartPosition.CenterScreen;
        MinimumSize = new Size(760, 520);

        _operation.Items.Add(new OperationItem("Copy (0)", ReadDriverOperation.Copy));
        _operation.Items.Add(new OperationItem("Protect (3)", ReadDriverOperation.Protect));
        _operation.Items.Add(new OperationItem("Patch (4)", ReadDriverOperation.Patch));
        _operation.Items.Add(new OperationItem("WriteInteger (5)", ReadDriverOperation.WriteInteger));
        _operation.SelectedIndex = 0;

        var sendButton = new Button { Text = "Send Request", AutoSize = true };
        sendButton.Click += (_, _) => SendRequest();

        var grid = new TableLayoutPanel
        {
            Dock = DockStyle.Fill,
            ColumnCount = 2,
            RowCount = 8,
            Padding = new Padding(12),
            AutoSize = true
        };
        grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 30));
        grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 70));

        AddRow(grid, 0, "Operation", _operation);
        AddRow(grid, 1, "Source PID", _sourcePid);
        AddRow(grid, 2, "Source Address / Integer Value", _sourceAddress);
        AddRow(grid, 3, "Target PID / Region Size", _targetPid);
        AddRow(grid, 4, "Target Address", _targetAddress);
        AddRow(grid, 5, "Size", _size);
        AddRow(grid, 6, "Action", sendButton);
        AddRow(grid, 7, "Status", _status);

        _status.Dock = DockStyle.Fill;
        _status.Height = 180;

        Controls.Add(grid);
    }

    private void SendRequest()
    {
        if (!TryBuildRequest(out ReadDriverRequest request, out string validationError))
        {
            AppendStatus($"Invalid input: {validationError}");
            return;
        }

        if (!DriverApi.SendDriverRequest(request, out _, out uint bytesReturned, out string? error))
        {
            AppendStatus(error ?? "Device request failed.");
            return;
        }

        AppendStatus($"Success. Bytes returned: {bytesReturned}");
    }

    private bool TryBuildRequest(out ReadDriverRequest request, out string error)
    {
        request = default;

        if (_operation.SelectedItem is not OperationItem selected)
        {
            error = "Select an operation.";
            return false;
        }

        if (!DriverApi.TryParseUlong(_sourcePid.Text.Trim(), out request.SourceProcessId))
        {
            error = "Source PID is not a valid number.";
            return false;
        }

        if (!DriverApi.TryParseUlong(_sourceAddress.Text.Trim(), out request.SourceAddress))
        {
            error = "Source Address / Integer Value is not valid.";
            return false;
        }

        if (!string.IsNullOrWhiteSpace(_targetPid.Text) &&
            !DriverApi.TryParseUlong(_targetPid.Text.Trim(), out request.TargetProcessId))
        {
            error = "Target PID / Region Size is not valid.";
            return false;
        }

        if (!string.IsNullOrWhiteSpace(_targetAddress.Text) &&
            !DriverApi.TryParseUlong(_targetAddress.Text.Trim(), out request.TargetAddress))
        {
            error = "Target Address is not valid.";
            return false;
        }

        if (!string.IsNullOrWhiteSpace(_size.Text) &&
            !DriverApi.TryParseUlong(_size.Text.Trim(), out request.Size))
        {
            error = "Size is not valid.";
            return false;
        }

        request.Operation = (ulong)selected.Operation;
        if (selected.Operation == ReadDriverOperation.WriteInteger)
        {
            request.TargetProcessId = 0;
        }

        error = string.Empty;
        return true;
    }

    private void AppendStatus(string message)
    {
        var line = $"{DateTime.Now:yyyy-MM-dd HH:mm:ss} | {message}";
        if (_status.TextLength == 0)
        {
            _status.Text = line;
            return;
        }

        _status.AppendText(Environment.NewLine + line);
    }

    private static void AddRow(TableLayoutPanel grid, int row, string labelText, Control control)
    {
        var label = new Label
        {
            Text = labelText,
            AutoSize = true,
            Anchor = AnchorStyles.Left,
            Margin = new Padding(0, 8, 8, 8)
        };

        control.Dock = DockStyle.Fill;
        control.Margin = new Padding(0, 4, 0, 4);

        grid.Controls.Add(label, 0, row);
        grid.Controls.Add(control, 1, row);
    }

    private sealed class OperationItem
    {
        internal OperationItem(string label, ReadDriverOperation operation)
        {
            Label = label;
            Operation = operation;
        }

        internal string Label { get; }
        internal ReadDriverOperation Operation { get; }

        public override string ToString() => Label;
    }
}
