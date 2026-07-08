using System.Globalization;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

internal enum ReadDriverOperation : ulong
{
    Copy = 0,
    Read = 1,
    Scan = 2,
    Protect = 3,
    Patch = 4,
    WriteInteger = 5,
    BatchRead = 6,
    ReadWithProtect = 7,
    ReadHex = 8
}

internal static class ReadDriverFlags
{
    public const ulong Silent = 0x0000000000000001UL;
}

[StructLayout(LayoutKind.Sequential)]
internal struct ReadDriverRequest
{
    // This struct mirrors common/protocol.h and must remain layout-compatible with the native request.
    public ulong SourceProcessId;
    public ulong SourceAddress;
    public ulong TargetProcessId;
    public ulong TargetAddress;
    public ulong Size;
    public ulong Operation;
    public ulong Flags;
    public ulong Reserved2;
    public ulong Reserved3;
    public ulong Reserved4;
    public ulong Reserved5;

    internal byte[] GetResponseBytes(int count)
    {
        if (count <= 0)
        {
            return Array.Empty<byte>();
        }

        int responseSize = Marshal.SizeOf<ReadDriverRequest>();
        int copyLength = Math.Min(count, responseSize);
        Span<byte> bytes = stackalloc byte[responseSize];
        ReadDriverRequest value = this;

        MemoryMarshal.Write(bytes, in value);
        return bytes[..copyLength].ToArray();
    }

    internal ulong GetResponseAddress()
    {
        return SourceProcessId;
    }

    internal string GetResponseString(int count)
    {
        if (count <= 0)
            return string.Empty;
        var bytes = GetResponseBytes(count);
        int end = bytes.Length;
        while (end > 0 && bytes[end - 1] == 0)
            end--;
        return System.Text.Encoding.ASCII.GetString(bytes, 0, end);
    }
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

    internal static bool SendDriverRequest(ReadDriverRequest request, out ReadDriverRequest response, out uint bytesReturned, out string? error)
    {
        bytesReturned = 0;
        error = null;
        response = default;

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
            out ReadDriverRequest outBuffer,
            (uint)Marshal.SizeOf<ReadDriverRequest>(),
            out uint lpBytesReturned,
            IntPtr.Zero);

        if (!ok)
        {
            error = $"DeviceIoControl failed with error {Marshal.GetLastWin32Error()}";
            return false;
        }

        response = outBuffer;
        bytesReturned = lpBytesReturned;
        return true;
    }

    internal static byte[] ReadProcess(ulong pid, ulong address, ulong size)
    {
        var request = new ReadDriverRequest
        {
            SourceProcessId = pid,
            SourceAddress = address,
            Size = size,
            Operation = (ulong)ReadDriverOperation.Read,
            Flags = ReadDriverFlags.Silent
        };

        if (SendDriverRequest(request, out var resp, out var bytesReturned, out _))
        {
            return resp.GetResponseBytes((int)bytesReturned);
        }
        return Array.Empty<byte>();
    }

    internal static ulong ScanProcess(ulong pid, ulong startAddress, ulong size, byte[] pattern)
    {
        var request = new ReadDriverRequest
        {
            SourceProcessId = pid,
            SourceAddress = startAddress,
            TargetProcessId = size,
            Size = (ulong)pattern.Length,
            Operation = (ulong)ReadDriverOperation.Scan,
            Flags = ReadDriverFlags.Silent
        };

        if (pattern.Length > 0)
        {
            request.TargetAddress = pattern[0];
            for (int i = 1; i < pattern.Length && i < 8; i++)
            {
                request.TargetAddress |= ((ulong)pattern[i]) << (i * 8);
            }
        }

        if (SendDriverRequest(request, out var resp, out var bytesReturned, out _))
        {
            if (bytesReturned >= sizeof(ulong))
            {
                return resp.GetResponseAddress();
            }
        }
        return 0;
    }

    internal static string ReadHex(ulong pid, ulong address, ulong size)
    {
        var request = new ReadDriverRequest
        {
            SourceProcessId = pid,
            SourceAddress = address,
            Size = size,
            Operation = (ulong)ReadDriverOperation.ReadHex,
            Flags = ReadDriverFlags.Silent
        };

        if (SendDriverRequest(request, out var resp, out var bytesReturned, out _))
        {
            return resp.GetResponseString((int)bytesReturned);
        }
        return string.Empty;
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
