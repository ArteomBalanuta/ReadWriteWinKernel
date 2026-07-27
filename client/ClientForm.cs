using System.Diagnostics;
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
    private readonly ComboBox _requestProcess = new() { DropDownStyle = ComboBoxStyle.DropDownList };
    private readonly ComboBox _requestTargetProcess = new() { DropDownStyle = ComboBoxStyle.DropDownList };
    private readonly Label _requestProcessLabel = new() { AutoSize = true, TextAlign = ContentAlignment.MiddleLeft };
    private readonly Label _requestTargetProcessLabel = new() { AutoSize = true, TextAlign = ContentAlignment.MiddleLeft };
    private readonly TextBox _freezePid = new();
    private readonly ComboBox _freezeProcess = new() { DropDownStyle = ComboBoxStyle.DropDownList };
    private readonly TextBox _freezeValue = new();
    private readonly ComboBox _freezeSize = new() { DropDownStyle = ComboBoxStyle.DropDownList };
    private readonly TextBox _freezeAddress = new();
    private readonly ListBox _freezeAddresses = new() { IntegralHeight = false };
    private readonly TextBox _freezeStatus = new() { Multiline = true, ReadOnly = true, ScrollBars = ScrollBars.Vertical };

    internal ClientForm()
    {
        Text = "readDriver client";
        StartPosition = FormStartPosition.CenterScreen;
        MinimumSize = new Size(820, 600);
        Size = new Size(920, 680);

        _operation.Items.Add(new OperationItem("Copy (0)", ReadDriverOperation.Copy));
        _operation.Items.Add(new OperationItem("Protect (3)", ReadDriverOperation.Protect));
        _operation.Items.Add(new OperationItem("Patch (4)", ReadDriverOperation.Patch));
        _operation.Items.Add(new OperationItem("WriteInteger (5)", ReadDriverOperation.WriteInteger));
        _operation.SelectedIndex = 0;
        _operation.SelectedIndexChanged += (_, _) => UpdateRequestProcessPickerState();

        _freezeSize.Items.AddRange(["1 byte", "2 bytes", "4 bytes", "8 bytes"]);
        _freezeSize.SelectedIndex = 2;

        var tabs = new TabControl
        {
            Dock = DockStyle.Fill,
            Padding = new Point(16, 6)
        };
        tabs.TabPages.Add(BuildRequestTab());
        tabs.TabPages.Add(BuildFreezeTab());
        Controls.Add(tabs);
    }

    private TabPage BuildRequestTab()
    {
        var sendButton = new Button { Text = "Send Request", AutoSize = true };
        sendButton.Click += (_, _) => SendRequest();

        var unloadButton = new Button { Text = "Remove Driver Link", AutoSize = true };
        unloadButton.Click += (_, _) => UnloadDriver();

        var actionPanel = new FlowLayoutPanel
        {
            AutoSize = true,
            Dock = DockStyle.Fill,
            FlowDirection = FlowDirection.LeftToRight,
            WrapContents = false
        };
        actionPanel.Controls.Add(sendButton);
        actionPanel.Controls.Add(unloadButton);

        _requestProcess.SelectedIndexChanged += (_, _) => ApplySelectedProcess(_requestProcess, _sourcePid);
        _requestTargetProcess.SelectedIndexChanged += (_, _) => ApplySelectedProcess(_requestTargetProcess, _targetPid);

        var requestProcessPicker = CreateProcessPicker(_requestProcess);
        var targetProcessPicker = CreateProcessPicker(_requestTargetProcess);

        var grid = new TableLayoutPanel
        {
            Dock = DockStyle.Fill,
            ColumnCount = 2,
            RowCount = 10,
            Padding = new Padding(12),
            AutoSize = true
        };
        grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 30));
        grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 70));

        AddRow(grid, 0, "Operation", _operation);
        AddRow(grid, 1, _requestProcessLabel, requestProcessPicker);
        AddRow(grid, 2, "Process PID", _sourcePid);
        AddRow(grid, 3, _requestTargetProcessLabel, targetProcessPicker);
        AddRow(grid, 4, "Target PID / Region Size", _targetPid);
        AddRow(grid, 5, "Source Address / Integer Value", _sourceAddress);
        AddRow(grid, 6, "Target Address", _targetAddress);
        AddRow(grid, 7, "Size", _size);
        AddRow(grid, 8, "Action", actionPanel);
        AddRow(grid, 9, "Status", _status);

        _status.Dock = DockStyle.Fill;
        _status.Height = 180;

        RefreshProcessPicker(_requestProcess);
        RefreshProcessPicker(_requestTargetProcess);
        UpdateRequestProcessPickerState();

        var page = new TabPage("Requests");
        page.Controls.Add(grid);
        return page;
    }

    private TabPage BuildFreezeTab()
    {
        var intro = new Label
        {
            AutoSize = true,
            Text = "Keep one integer value synchronized across up to four target addresses. " +
                   "The driver performs periodic writes every 100 ms."
        };

        var addAddressButton = new Button { Text = "Add Address", AutoSize = true };
        addAddressButton.Click += (_, _) => AddFreezeAddress();

        _freezeAddress.KeyDown += (_, eventArgs) =>
        {
            if (eventArgs.KeyCode == Keys.Enter)
            {
                AddFreezeAddress();
                eventArgs.SuppressKeyPress = true;
            }
        };

        var addressEditor = new TableLayoutPanel
        {
            AutoSize = true,
            ColumnCount = 2,
            Dock = DockStyle.Fill
        };
        addressEditor.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        addressEditor.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
        _freezeAddress.Dock = DockStyle.Fill;
        addressEditor.Controls.Add(_freezeAddress, 0, 0);
        addressEditor.Controls.Add(addAddressButton, 1, 0);

        var removeAddressButton = new Button { Text = "Remove Selected", AutoSize = true };
        removeAddressButton.Click += (_, _) => RemoveSelectedFreezeAddress();

        var armButton = new Button { Text = "Arm Freeze", AutoSize = true };
        armButton.Click += (_, _) => ArmFreeze();

        var unfreezeButton = new Button { Text = "Unfreeze Selected", AutoSize = true };
        unfreezeButton.Click += (_, _) => UnfreezeSelectedAddress();

        var unfreezeAllButton = new Button { Text = "Unfreeze All", AutoSize = true };
        unfreezeAllButton.Click += (_, _) => UnfreezeAllAddresses();

        var actionPanel = new FlowLayoutPanel
        {
            AutoSize = true,
            Dock = DockStyle.Fill,
            FlowDirection = FlowDirection.LeftToRight,
            WrapContents = true
        };
        actionPanel.Controls.Add(armButton);
        actionPanel.Controls.Add(unfreezeButton);
        actionPanel.Controls.Add(unfreezeAllButton);
        actionPanel.Controls.Add(removeAddressButton);

        _freezeProcess.SelectedIndexChanged += (_, _) => ApplySelectedProcess(_freezeProcess, _freezePid);
        var freezeProcessPicker = CreateProcessPicker(_freezeProcess);

        var grid = new TableLayoutPanel
        {
            Dock = DockStyle.Fill,
            ColumnCount = 2,
            RowCount = 9,
            Padding = new Padding(12)
        };
        grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 30));
        grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 70));
        grid.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        grid.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        grid.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        grid.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        grid.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        grid.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        grid.RowStyles.Add(new RowStyle(SizeType.Percent, 40));
        grid.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        grid.RowStyles.Add(new RowStyle(SizeType.Percent, 60));

        AddSpanningRow(grid, 0, intro);
        AddRow(grid, 1, "Process", freezeProcessPicker);
        AddRow(grid, 2, "Process ID", _freezePid);
        AddRow(grid, 3, "Integer Value", _freezeValue);
        AddRow(grid, 4, "Integer Size", _freezeSize);
        AddRow(grid, 5, "Target Address", addressEditor);
        AddRow(grid, 6, "Target Addresses", _freezeAddresses);
        AddRow(grid, 7, "Actions", actionPanel);
        AddRow(grid, 8, "Freeze Status", _freezeStatus);

        _freezeAddresses.Dock = DockStyle.Fill;
        _freezeStatus.Dock = DockStyle.Fill;
        RefreshProcessPicker(_freezeProcess);

        var page = new TabPage("Freeze");
        page.Controls.Add(grid);
        return page;
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

    private void UnloadDriver()
    {
        if (DriverApi.UnloadDriver(out string? error))
        {
            AppendStatus("Driver link removed successfully.");
        }
        else
        {
            AppendStatus(error ?? "Failed to remove driver link.");
        }
    }

    private void AddFreezeAddress()
    {
        string text = _freezeAddress.Text.Trim();
        if (!DriverApi.TryParseUlong(text, out ulong address) || address == 0)
        {
            AppendFreezeStatus("Invalid target address. Enter a non-zero decimal or hexadecimal value.");
            return;
        }

        foreach (AddressItem existing in _freezeAddresses.Items)
        {
            if (existing.Value == address)
            {
                AppendFreezeStatus($"Address 0x{address:X} is already in the target list.");
                return;
            }
        }

        if (_freezeAddresses.Items.Count >= 4)
        {
            AppendFreezeStatus("The current protocol supports at most four target addresses.");
            return;
        }

        _freezeAddresses.Items.Add(new AddressItem(address));
        _freezeAddress.Clear();
        _freezeAddress.Focus();
    }

    private void RemoveSelectedFreezeAddress()
    {
        if (_freezeAddresses.SelectedItem is not AddressItem selected)
        {
            AppendFreezeStatus("Select an address to remove.");
            return;
        }

        if (!TryGetFreezeProcessId(out ulong processId))
        {
            return;
        }

        if (!DriverApi.Unfreeze(processId, selected.Value, out string? error))
        {
            AppendFreezeStatus(error ?? $"Failed to unfreeze 0x{selected.Value:X}.");
            return;
        }

        _freezeAddresses.Items.Remove(selected);
        AppendFreezeStatus($"Freeze cleared for PID {processId}, address 0x{selected.Value:X}.");
    }

    private void ArmFreeze()
    {
        if (!DriverApi.TryParseUlong(_freezePid.Text.Trim(), out ulong processId))
        {
            AppendFreezeStatus("Invalid process ID.");
            return;
        }

        if (!DriverApi.TryParseUlong(_freezeValue.Text.Trim(), out ulong value))
        {
            AppendFreezeStatus("Invalid integer value.");
            return;
        }

        ulong size = 1UL << _freezeSize.SelectedIndex;
        var addresses = new ulong[_freezeAddresses.Items.Count];
        for (int i = 0; i < addresses.Length; i++)
        {
            addresses[i] = ((AddressItem)_freezeAddresses.Items[i]!).Value;
        }

        if (!DriverApi.TryCreateFreezeRequest(processId, value, addresses, size, out _, out string? validationError))
        {
            AppendFreezeStatus(validationError ?? "Invalid freeze request.");
            return;
        }

        if (!DriverApi.UnfreezeAll(processId, out string? clearError))
        {
            AppendFreezeStatus(clearError ?? $"Failed to clear previous freeze slots for PID {processId}.");
            return;
        }

        if (!DriverApi.FreezeWrite(processId, value, addresses, size, out string? error))
        {
            AppendFreezeStatus(error ?? "Freeze request failed.");
            return;
        }

        AppendFreezeStatus(
            $"Previous freeze slots cleared. Freeze armed for PID {processId}: value 0x{value:X}, {size} byte(s), " +
            $"{addresses.Length} address(es).");
    }

    private void UnfreezeSelectedAddress()
    {
        if (_freezeAddresses.SelectedItem is not AddressItem selected)
        {
            AppendFreezeStatus("Select an address to unfreeze.");
            return;
        }

        if (!TryGetFreezeProcessId(out ulong processId))
        {
            return;
        }

        if (!DriverApi.Unfreeze(processId, selected.Value, out string? error))
        {
            AppendFreezeStatus(error ?? $"Failed to unfreeze 0x{selected.Value:X}.");
            return;
        }

        _freezeAddresses.Items.Remove(selected);
        AppendFreezeStatus($"Freeze cleared for PID {processId}, address 0x{selected.Value:X}.");
    }

    private void UnfreezeAllAddresses()
    {
        if (!TryGetFreezeProcessId(out ulong processId))
        {
            return;
        }

        if (!DriverApi.UnfreezeAll(processId, out string? error))
        {
            AppendFreezeStatus(error ?? $"Failed to clear freeze slots for PID {processId}.");
            return;
        }

        AppendFreezeStatus(
            $"All freeze slots for PID {processId} were cleared. Target addresses remain listed for re-arming.");
    }

    private bool TryGetFreezeProcessId(out ulong processId)
    {
        if (!DriverApi.TryParseUlong(_freezePid.Text.Trim(), out processId) || processId == 0)
        {
            AppendFreezeStatus("Invalid process ID.");
            return false;
        }

        return true;
    }

    private bool TryBuildRequest(out ReadDriverRequest request, out string error)
    {
        request = default;

        if (_operation.SelectedItem is not OperationItem selected)
        {
            error = "Select an operation.";
            return false;
        }

        bool usesSeparateTargetProcess = RequestOperationUsesSeparateTargetProcess(selected.Operation);
        if (!DriverApi.TryParseUlong(_sourcePid.Text.Trim(), out request.SourceProcessId))
        {
            error = usesSeparateTargetProcess ? "Source PID is not a valid number." : "Process PID is not a valid number.";
            return false;
        }

        if (!DriverApi.TryParseUlong(_sourceAddress.Text.Trim(), out request.SourceAddress))
        {
            error = "Source Address / Integer Value is not valid.";
            return false;
        }

        if (usesSeparateTargetProcess &&
            !DriverApi.TryParseUlong(_targetPid.Text.Trim(), out request.TargetProcessId))
        {
            error = "Target PID is not a valid number.";
            return false;
        }

        if (!usesSeparateTargetProcess &&
            !string.IsNullOrWhiteSpace(_targetPid.Text) &&
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

    internal static bool RequestOperationUsesSeparateTargetProcess(ReadDriverOperation operation)
    {
        return operation == ReadDriverOperation.Copy;
    }

    internal static string FormatProcessDisplayName(string processName, ulong processId)
    {
        return $"{processName} ({processId})";
    }

    private FlowLayoutPanel CreateProcessPicker(ComboBox picker)
    {
        picker.Width = 280;

        var refreshButton = new Button { Text = "Refresh", AutoSize = true };
        refreshButton.Click += (_, _) => RefreshProcessPicker(picker);

        var panel = new FlowLayoutPanel
        {
            AutoSize = true,
            Dock = DockStyle.Fill,
            FlowDirection = FlowDirection.LeftToRight,
            WrapContents = false
        };
        panel.Controls.Add(picker);
        panel.Controls.Add(refreshButton);
        return panel;
    }

    private void RefreshProcessPicker(ComboBox picker)
    {
        ulong? selectedProcessId = (picker.SelectedItem as ProcessItem)?.ProcessId;
        var processes = new List<ProcessItem>();

        foreach (Process process in Process.GetProcesses())
        {
            try
            {
                processes.Add(new ProcessItem(process.ProcessName, (ulong)process.Id));
            }
            catch (InvalidOperationException)
            {
                // The process exited while its details were being read.
            }
            catch (System.ComponentModel.Win32Exception)
            {
                // Windows denied access to its details.
            }
            finally
            {
                process.Dispose();
            }
        }

        processes.Sort((left, right) =>
        {
            int nameComparison = string.Compare(left.Name, right.Name, StringComparison.OrdinalIgnoreCase);
            return nameComparison != 0 ? nameComparison : left.ProcessId.CompareTo(right.ProcessId);
        });

        picker.BeginUpdate();
        picker.Items.Clear();
        foreach (ProcessItem process in processes)
        {
            int index = picker.Items.Add(process);
            if (process.ProcessId == selectedProcessId)
            {
                picker.SelectedIndex = index;
            }
        }
        picker.EndUpdate();
    }

    private static void ApplySelectedProcess(ComboBox picker, TextBox targetPid)
    {
        if (picker.SelectedItem is ProcessItem process)
        {
            targetPid.Text = process.ProcessId.ToString();
        }
    }

    private void UpdateRequestProcessPickerState()
    {
        bool usesSeparateTargetProcess = _operation.SelectedItem is OperationItem selected &&
            RequestOperationUsesSeparateTargetProcess(selected.Operation);

        _requestProcessLabel.Text = usesSeparateTargetProcess ? "Source Process" : "Process";
        _requestTargetProcessLabel.Text = usesSeparateTargetProcess ? "Target Process" : "Target Process (Copy only)";
        _requestTargetProcess.Enabled = usesSeparateTargetProcess;
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

    private void AppendFreezeStatus(string message)
    {
        var line = $"{DateTime.Now:yyyy-MM-dd HH:mm:ss} | {message}";
        if (_freezeStatus.TextLength == 0)
        {
            _freezeStatus.Text = line;
            return;
        }

        _freezeStatus.AppendText(Environment.NewLine + line);
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

        AddRow(grid, row, label, control);
    }

    private static void AddRow(TableLayoutPanel grid, int row, Label label, Control control)
    {
        label.Anchor = AnchorStyles.Left;
        label.Margin = new Padding(0, 8, 8, 8);

        control.Dock = DockStyle.Fill;
        control.Margin = new Padding(0, 4, 0, 4);

        grid.Controls.Add(label, 0, row);
        grid.Controls.Add(control, 1, row);
    }

    private static void AddSpanningRow(TableLayoutPanel grid, int row, Control control)
    {
        control.Dock = DockStyle.Fill;
        control.Margin = new Padding(0, 4, 0, 12);
        grid.Controls.Add(control, 0, row);
        grid.SetColumnSpan(control, 2);
    }

    private sealed class ProcessItem
    {
        internal ProcessItem(string name, ulong processId)
        {
            Name = name;
            ProcessId = processId;
        }

        internal string Name { get; }
        internal ulong ProcessId { get; }

        public override string ToString() => FormatProcessDisplayName(Name, ProcessId);
    }

    private sealed class AddressItem
    {
        internal AddressItem(ulong value)
        {
            Value = value;
        }

        internal ulong Value { get; }

        public override string ToString() => $"0x{Value:X16}";
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
