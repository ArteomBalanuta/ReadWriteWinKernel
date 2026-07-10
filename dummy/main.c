#include <ntifs.h>
#include <ntddk.h>
#include "main.h"

NTSTATUS DriverEntry(
    _In_ struct _DRIVER_OBJECT* DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS status;
    UNICODE_STRING driverName;

    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    GenerateRandomDeviceNames();

    RtlInitUnicodeString(&driverName, g_driverNameBuffer);
    status = IoCreateDriver(&driverName, DriverInitialize);
    LogStatus(L"IoCreateDriver", status, NULL, FALSE);

    return status;
}
