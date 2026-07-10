using System.Linq;
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
        /* Try freeze/unfreeze sub-commands first. */
        if (args.Length > 0)
        {
            if (string.Equals(args[0], "freeze", StringComparison.OrdinalIgnoreCase))
                return RunFreeze(args);
            if (string.Equals(args[0], "unfreeze", StringComparison.OrdinalIgnoreCase))
                return RunUnfreeze(args);
        }

        if (!TryParseRequest(args, out ReadDriverRequest request))
        {
            PrintUsage();
            return 1;
        }

        PrintRequestSummary(request);

        if (!DriverApi.SendDriverRequest(request, out ReadDriverRequest response, out uint bytesReturned, out string? error))
        {
            Console.Error.WriteLine(error ?? "Device request failed.");
            return 1;
        }

        Console.WriteLine($"Request completed, bytes returned: {bytesReturned}");

        if (bytesReturned > 0)
        {
            PrintResponseData(request, response, (int)bytesReturned);
        }

        return 0;
    }

    private static int RunFreeze(string[] args)
    {
        /* freeze <pid> <value> <size> <addr1> [addr2] [addr3] [addr4] */
        if (args.Length < 5)
        {
            Console.Error.WriteLine("Usage: client.exe freeze <pid> <value> <size> <addr1> [addr2] [addr3] [addr4]");
            return 1;
        }

        if (!DriverApi.TryParseUlong(args[1], out ulong pid) ||
            !DriverApi.TryParseUlong(args[2], out ulong value) ||
            !DriverApi.TryParseUlong(args[3], out ulong size))
        {
            Console.Error.WriteLine("Failed to parse arguments.");
            return 1;
        }

        ulong[] addresses = new ulong[args.Length - 4];
        for (int i = 0; i < addresses.Length; i++)
        {
            if (!DriverApi.TryParseUlong(args[4 + i], out addresses[i]))
            {
                Console.Error.WriteLine($"Failed to parse address {i + 1}.");
                return 1;
            }
        }

        if (!DriverApi.FreezeWrite(pid, value, addresses, size, out string? error))
        {
            Console.Error.WriteLine(error ?? "Freeze request failed.");
            return 1;
        }

        Console.WriteLine($"Freeze armed: value 0x{value:X} at {addresses.Length} address(es), size {size} byte(s), PID {pid}");
        return 0;
    }

    private static int RunUnfreeze(string[] args)
    {
        if (args.Length >= 2 && string.Equals(args[1], "all", StringComparison.OrdinalIgnoreCase))
        {
            if (!DriverApi.UnfreezeAll(out string? error))
            {
                Console.Error.WriteLine(error ?? "Unfreeze request failed.");
                return 1;
            }
            Console.WriteLine("All freeze slots cleared.");
            return 0;
        }

        if (args.Length >= 2 && DriverApi.TryParseUlong(args[1], out ulong address))
        {
            if (!DriverApi.Unfreeze(address, out string? error))
            {
                Console.Error.WriteLine(error ?? "Unfreeze request failed.");
                return 1;
            }
            Console.WriteLine($"Freeze slot cleared for address 0x{address:X}");
            return 0;
        }

        Console.Error.WriteLine("Usage: client.exe unfreeze <address> | unfreeze all");
        return 1;
    }

    private static bool TryParseRequest(string[] args, out ReadDriverRequest request)
    {
        request = default;

        if (args.Length == 1 && string.Equals(args[0], "unload", StringComparison.OrdinalIgnoreCase))
        {
            request.Operation = (ulong)ReadDriverOperation.Unload;
            return true;
        }

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
        Console.WriteLine("  client.exe unload");
        Console.WriteLine("  client.exe freeze <pid> <value> <size> <addr1> [addr2] [addr3] [addr4]");
        Console.WriteLine("  client.exe unfreeze <address> | unfreeze all");
        Console.WriteLine("  client.exe <srcPid> <srcAddress> <targetPid> <targetAddress> <size> <operation>");
        Console.WriteLine("  client.exe <processPid> <integerValue> <targetAddress> <size> 5");
        Console.WriteLine();
        Console.WriteLine("Examples:");
        Console.WriteLine("  Copy:     client.exe 5412 0x6520A9F5 6512 0x0203B3F94940 15 0");
        Console.WriteLine("  Protect:  client.exe 5412 0x0203B3F94940 40000 0 0 3");
        Console.WriteLine("  Patch:    client.exe 5412 0x0203B3F94940 0 0 0 4");
        Console.WriteLine("  WriteInt: client.exe 6512 500 0x0203B3F94940 2 5");
        Console.WriteLine("  Freeze:   client.exe freeze 6512 99 4 0x0203B3F94940 0x0203B3F94A00");
        Console.WriteLine("  Unfreeze: client.exe unfreeze 0x0203B3F94940");
        Console.WriteLine("  Unfreeze all: client.exe unfreeze all");
    }

    private static void PrintRequestSummary(ReadDriverRequest request)
    {
        Console.WriteLine($"Source PID:    {request.SourceProcessId}");
        Console.WriteLine($"Source Addr:   0x{request.SourceAddress:X}");
        Console.WriteLine($"Target PID:    {request.TargetProcessId}");
        Console.WriteLine($"Target Addr:   0x{request.TargetAddress:X}");
        Console.WriteLine($"Size:          {request.Size}");
        Console.WriteLine($"Operation:     {request.Operation}");
        Console.WriteLine($"Flags:         0x{request.Flags:X}");
    }

    private static void PrintResponseData(ReadDriverRequest request, ReadDriverRequest response, int bytesReturned)
    {
        if (bytesReturned <= 0)
            return;

        switch ((ReadDriverOperation)request.Operation)
        {
            case ReadDriverOperation.Read:
            case ReadDriverOperation.ReadWithProtect:
                var bytes = response.GetResponseBytes(bytesReturned);
                Console.WriteLine($"Response (hex): {string.Join(" ", bytes.Select(b => b.ToString("X2")))}");
                break;
            case ReadDriverOperation.ReadHex:
                Console.WriteLine($"Response: {response.GetResponseString(bytesReturned)}");
                break;
            case ReadDriverOperation.Scan:
                Console.WriteLine($"Matched address: 0x{response.GetResponseAddress():X}");
                break;
            case ReadDriverOperation.BatchRead:
                var batchBytes = response.GetResponseBytes(bytesReturned);
                int chunkSize = (int)request.Size;
                int chunks = bytesReturned / chunkSize;
                Console.WriteLine($"Response (batch, {chunks} chunks):");
                for (int i = 0; i < chunks && i * chunkSize < batchBytes.Length; i++)
                {
                    var chunk = batchBytes.Skip(i * chunkSize).Take(chunkSize).ToArray();
                    Console.WriteLine($"  {string.Join(" ", chunk.Select(b => b.ToString("X2")))}");
                }
                break;
            default:
                var defaultBytes = response.GetResponseBytes(bytesReturned);
                Console.WriteLine($"Response (hex): {string.Join(" ", defaultBytes.Select(b => b.ToString("X2")))}");
                break;
        }
    }
}
