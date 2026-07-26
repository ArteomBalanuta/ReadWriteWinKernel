using System.Reflection;

namespace client.Tests;

public class UnitTest1
{
    [Fact]
    public void GetResponseBytes_returns_the_requested_struct_bytes()
    {
        Assembly assembly = LoadClientAssembly();
        Type requestType = assembly.GetType("ReadDriverRequest", throwOnError: true)!;
        object request = Activator.CreateInstance(requestType)!;

        requestType.GetField("SourceProcessId")!.SetValue(request, 0x1122334455667788UL);
        requestType.GetField("SourceAddress")!.SetValue(request, 0x8877665544332211UL);

        MethodInfo getResponseBytes = requestType.GetMethod(
            "GetResponseBytes",
            BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public)!;

        byte[] actual = (byte[])getResponseBytes.Invoke(request, new object[] { 16 })!;
        byte[] expected =
        [
            0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
        ];

        Assert.Equal(expected, actual);
    }

    [Fact]
    public void GetResponseString_trims_trailing_null_bytes()
    {
        Assembly assembly = LoadClientAssembly();
        Type requestType = assembly.GetType("ReadDriverRequest", throwOnError: true)!;
        object request = Activator.CreateInstance(requestType)!;

        requestType.GetField("SourceProcessId")!.SetValue(request, 0x0000000000004B4FUL);

        MethodInfo getResponseString = requestType.GetMethod(
            "GetResponseString",
            BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public)!;

        string actual = (string)getResponseString.Invoke(request, new object[] { 8 })!;

        Assert.Equal("OK", actual);
    }

    [Fact]
    public void Freeze_request_builder_packs_four_addresses()
    {
        ulong[] addresses =
        [
            0x0203B3F94940UL,
            0x0203B3F94A00UL,
            0x0203B3F94B00UL,
            0x0203B3F94C00UL
        ];

        object?[] arguments = [6512UL, 99UL, addresses, 4UL, null, null];
        bool success = (bool)GetFreezeRequestBuilder().Invoke(null, arguments)!;

        Assert.True(success);
        Assert.Null(arguments[5]);

        object request = arguments[4]!;
        Type requestType = request.GetType();
        Assert.Equal(6512UL, requestType.GetField("SourceProcessId")!.GetValue(request));
        Assert.Equal(99UL, requestType.GetField("SourceAddress")!.GetValue(request));
        Assert.Equal(addresses[0], requestType.GetField("TargetAddress")!.GetValue(request));
        Assert.Equal(addresses[1], requestType.GetField("TargetProcessId")!.GetValue(request));
        Assert.Equal(addresses[2], requestType.GetField("Reserved2")!.GetValue(request));
        Assert.Equal(addresses[3], requestType.GetField("Reserved3")!.GetValue(request));
        Assert.Equal(4UL, requestType.GetField("Size")!.GetValue(request));
        Assert.Equal(10UL, requestType.GetField("Operation")!.GetValue(request));
    }

    [Fact]
    public void Freeze_request_builder_rejects_invalid_arguments()
    {
        AssertFreezeRequestRejected(0, 99, [0x1000], 4);
        AssertFreezeRequestRejected(6512, 99, [0x1000], 3);
        AssertFreezeRequestRejected(6512, 5000, [0x1000], 1);
        AssertFreezeRequestRejected(6512, 99, [], 4);
        AssertFreezeRequestRejected(6512, 99, [0x1000, 0x1000], 4);
        AssertFreezeRequestRejected(6512, 99, [0], 4);
        AssertFreezeRequestRejected(6512, 99, [1, 2, 3, 4, 5], 4);
    }

    [Fact]
    public void Freeze_request_builder_accepts_a_value_that_fits_the_selected_width()
    {
        object?[] arguments = [6512UL, 5000UL, new ulong[] { 0x1000UL }, 2UL, null, null];

        bool success = (bool)GetFreezeRequestBuilder().Invoke(null, arguments)!;

        Assert.True(success);
        Assert.Null(arguments[5]);
    }

    [Fact]
    public void Unfreeze_all_keeps_the_GUI_address_list_available_for_rearming()
    {
        string source = File.ReadAllText(FindRepositoryFile("client", "ClientForm.cs"));
        int methodStart = source.IndexOf("private void UnfreezeAllAddresses()", StringComparison.Ordinal);
        int methodEnd = source.IndexOf("private bool TryBuildRequest", methodStart, StringComparison.Ordinal);

        Assert.True(methodStart >= 0 && methodEnd > methodStart, "Could not locate UnfreezeAllAddresses.");

        string handler = source[methodStart..methodEnd];
        Assert.DoesNotContain("_freezeAddresses.Items.Clear()", handler, StringComparison.Ordinal);
        Assert.Contains("remain listed", handler, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void Request_process_picker_uses_a_separate_target_only_for_copy()
    {
        Assembly assembly = LoadClientAssembly();
        Type formType = assembly.GetType("ClientForm", throwOnError: true)!;
        Type operationType = assembly.GetType("ReadDriverOperation", throwOnError: true)!;
        MethodInfo method = formType.GetMethod(
            "RequestOperationUsesSeparateTargetProcess",
            BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public)!;

        object copy = Enum.ToObject(operationType, 0UL);
        object protect = Enum.ToObject(operationType, 3UL);
        object patch = Enum.ToObject(operationType, 4UL);
        object writeInteger = Enum.ToObject(operationType, 5UL);

        Assert.True((bool)method.Invoke(null, [copy])!);
        Assert.False((bool)method.Invoke(null, [protect])!);
        Assert.False((bool)method.Invoke(null, [patch])!);
        Assert.False((bool)method.Invoke(null, [writeInteger])!);
    }

    [Fact]
    public void Process_picker_formats_name_and_PID()
    {
        Type formType = LoadClientAssembly().GetType("ClientForm", throwOnError: true)!;
        MethodInfo method = formType.GetMethod(
            "FormatProcessDisplayName",
            BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public)!;

        string displayName = (string)method.Invoke(null, ["notepad", 1234UL])!;

        Assert.Equal("notepad (1234)", displayName);
    }

    private static void AssertFreezeRequestRejected(
        ulong processId,
        ulong value,
        ulong[] addresses,
        ulong size)
    {
        object?[] arguments = [processId, value, addresses, size, null, null];
        bool success = (bool)GetFreezeRequestBuilder().Invoke(null, arguments)!;

        Assert.False(success);
        Assert.False(string.IsNullOrWhiteSpace(arguments[5] as string));
    }

    private static MethodInfo GetFreezeRequestBuilder()
    {
        Type driverApi = LoadClientAssembly().GetType("DriverApi", throwOnError: true)!;
        MethodInfo? method = driverApi.GetMethod(
            "TryCreateFreezeRequest",
            BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public);

        Assert.NotNull(method);
        return method;
    }

    private static Assembly LoadClientAssembly()
    {
        string assemblyPath = Path.Combine(AppContext.BaseDirectory, "client.dll");
        return Assembly.LoadFrom(assemblyPath);
    }

    private static string FindRepositoryFile(params string[] pathParts)
    {
        DirectoryInfo? directory = new(AppContext.BaseDirectory);
        while (directory is not null)
        {
            string candidate = Path.Combine([directory.FullName, .. pathParts]);
            if (File.Exists(candidate))
            {
                return candidate;
            }

            directory = directory.Parent;
        }

        throw new FileNotFoundException($"Could not find repository file: {Path.Combine(pathParts)}");
    }
}
