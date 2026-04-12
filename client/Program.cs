using System.Windows.Forms;

internal static class Program
{
    [STAThread]
    private static int Main(string[] args)
    {
        if (args.Length > 0)
        {
            return RunCli(args);
        }

        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new ClientForm());
        return 0;
    }

    private static int RunCli(string[] args)
    {
        if (!TryParseRequest(args, out ReadDriverRequest request))
        {
            PrintUsage();
            return 1;
        }

        PrintRequestSummary(request);

        if (!DriverApi.SendDriverRequest(request, out uint bytesReturned, out string? error))
        {
            Console.Error.WriteLine(error ?? "Device request failed.");
            return 1;
        }

        Console.WriteLine($"Request completed, bytes returned: {bytesReturned}");
        return 0;
    }

    private static bool TryParseRequest(string[] args, out ReadDriverRequest request)
    {
        request = default;

        if (args.Length == 5)
        {
            bool parsed =
                DriverApi.TryParseUlong(args[0], out request.SourceProcessId) &&
                DriverApi.TryParseUlong(args[1], out request.SourceAddress) &&
                DriverApi.TryParseUlong(args[2], out request.TargetAddress) &&
                DriverApi.TryParseUlong(args[3], out request.Size) &&
                DriverApi.TryParseUlong(args[4], out request.Operation);

            if (!parsed || request.Operation != (ulong)ReadDriverOperation.WriteInteger)
            {
                return false;
            }

            request.TargetProcessId = 0;
            return true;
        }

        if (args.Length != 6)
        {
            return false;
        }

        return
            DriverApi.TryParseUlong(args[0], out request.SourceProcessId) &&
            DriverApi.TryParseUlong(args[1], out request.SourceAddress) &&
            DriverApi.TryParseUlong(args[2], out request.TargetProcessId) &&
            DriverApi.TryParseUlong(args[3], out request.TargetAddress) &&
            DriverApi.TryParseUlong(args[4], out request.Size) &&
            DriverApi.TryParseUlong(args[5], out request.Operation);
    }

    private static void PrintUsage()
    {
        Console.WriteLine("Usage:");
        Console.WriteLine("  client.exe <srcPid> <srcAddress> <targetPid> <targetAddress> <size> <operation>");
        Console.WriteLine("  client.exe <processPid> <integerValue> <targetAddress> <size> 5");
        Console.WriteLine();
        Console.WriteLine("Examples:");
        Console.WriteLine("  Copy:    client.exe 5412 0x6520A9F5 6512 0x0203B3F94940 15 0");
        Console.WriteLine("  Protect: client.exe 5412 0x0203B3F94940 40000 0 0 3");
        Console.WriteLine("  Patch:   client.exe 5412 0x0203B3F94940 0 0 0 4");
        Console.WriteLine("  WriteInt: client.exe 6512 500 0x0203B3F94940 2 5");
    }

    private static void PrintRequestSummary(ReadDriverRequest request)
    {
        Console.WriteLine($"Source PID:    {request.SourceProcessId}");
        Console.WriteLine($"Source Addr:   0x{request.SourceAddress:X}");
        Console.WriteLine($"Target PID:    {request.TargetProcessId}");
        Console.WriteLine($"Target Addr:   0x{request.TargetAddress:X}");
        Console.WriteLine($"Size:          {request.Size}");
        Console.WriteLine($"Operation:     {request.Operation}");
    }
}
