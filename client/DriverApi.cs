using System.Globalization;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

internal enum ReadDriverOperation : ulong
{
    Copy = 0,
    Read = 1,
    Protect = 3,
    Patch = 4,
    WriteInteger = 5
}

[StructLayout(LayoutKind.Sequential)]
internal struct ReadDriverRequest
{
    public ulong SourceProcessId;
    public ulong SourceAddress;
    public ulong TargetProcessId;
    public ulong TargetAddress;
    public ulong Size;
    public ulong Operation;
    public ulong Reserved7;
    public ulong Reserved8;
    public ulong Reserved9;
    public ulong Reserved10;
    public ulong Reserved11;
}

internal static class DriverApi
{
    private const string DeviceName = @"\\.\PWN29";

    private const uint FileDeviceUnknown = 0x00000022;
    private const uint ReadDriverIoctlCode = 0x0701;
    private const uint MethodBuffered = 0;
    private const uint FileSpecialAccess = 0;
    private const uint ReadDriverIoctlRequest =
        (FileDeviceUnknown << 16) | (FileSpecialAccess << 14) | (ReadDriverIoctlCode << 2) | MethodBuffered;

    private const uint GenericRead = 0x80000000;
    private const uint GenericWrite = 0x40000000;
    private const uint FileShareRead = 0x00000001;
    private const uint FileShareWrite = 0x00000002;
    private const uint OpenExisting = 3;

    internal static bool TryParseUlong(string input, out ulong value)
    {
        if (input.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
        {
            return ulong.TryParse(input.AsSpan(2), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out value);
        }

        return ulong.TryParse(input, NumberStyles.Integer, CultureInfo.InvariantCulture, out value);
    }

    internal static bool SendDriverRequest(ReadDriverRequest request, out uint bytesReturned, out string? error)
    {
        bytesReturned = 0;
        error = null;

        using SafeFileHandle handle = CreateFileW(
            DeviceName,
            GenericRead | GenericWrite,
            FileShareRead | FileShareWrite,
            IntPtr.Zero,
            OpenExisting,
            0,
            IntPtr.Zero);

        if (handle.IsInvalid)
        {
            error = $"CreateFileW failed with error {Marshal.GetLastWin32Error()}";
            return false;
        }

        bool ok = DeviceIoControl(
            handle,
            ReadDriverIoctlRequest,
            ref request,
            (uint)Marshal.SizeOf<ReadDriverRequest>(),
            out ReadDriverRequest _,
            (uint)Marshal.SizeOf<ReadDriverRequest>(),
            out bytesReturned,
            IntPtr.Zero);

        if (!ok)
        {
            error = $"DeviceIoControl failed with error {Marshal.GetLastWin32Error()}";
            return false;
        }

        return true;
    }

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern SafeFileHandle CreateFileW(
        string lpFileName,
        uint dwDesiredAccess,
        uint dwShareMode,
        IntPtr lpSecurityAttributes,
        uint dwCreationDisposition,
        uint dwFlagsAndAttributes,
        IntPtr hTemplateFile);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool DeviceIoControl(
        SafeFileHandle hDevice,
        uint dwIoControlCode,
        ref ReadDriverRequest lpInBuffer,
        uint nInBufferSize,
        out ReadDriverRequest lpOutBuffer,
        uint nOutBufferSize,
        out uint lpBytesReturned,
        IntPtr lpOverlapped);
}
