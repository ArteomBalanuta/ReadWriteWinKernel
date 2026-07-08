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

    private static Assembly LoadClientAssembly()
    {
        string assemblyPath = Path.Combine(AppContext.BaseDirectory, "client.dll");
        return Assembly.LoadFrom(assemblyPath);
    }
}
