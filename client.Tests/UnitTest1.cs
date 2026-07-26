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
        AssertFreezeRequestRejected(6512, 99, [], 4);
        AssertFreezeRequestRejected(6512, 99, [0x1000, 0x1000], 4);
        AssertFreezeRequestRejected(6512, 99, [0], 4);
        AssertFreezeRequestRejected(6512, 99, [1, 2, 3, 4, 5], 4);
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
}
