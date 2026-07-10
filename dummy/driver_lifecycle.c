#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>
#include "main.h"

static volatile LONG g_driverDecommissioned;

/* Runtime device name buffers — shared with main.c for IoCreateDriver. */
WCHAR g_driverNameBuffer[64];
WCHAR g_deviceNameBuffer[64];

VOID GenerateRandomDeviceNames(VOID)
{
    ULONG randomValue;
    LARGE_INTEGER tickCount;

    KeQuerySystemTime(&tickCount);
    randomValue = (ULONG)(tickCount.QuadPart & 0x00000000FFFFFFFF);
    randomValue = (randomValue % 9000) + 1000; /* 1000..9999 */

    RtlStringCchPrintfW(g_driverNameBuffer, RTL_NUMBER_OF(g_driverNameBuffer),
        L"\\Driver\\gammaA%04u", randomValue);
    RtlStringCchPrintfW(g_deviceNameBuffer, RTL_NUMBER_OF(g_deviceNameBuffer),
        L"\\Device\\gammaA%04u", randomValue);
}

BOOLEAN IsDriverDecommissioned(VOID)
{
    return InterlockedCompareExchange(&g_driverDecommissioned, 0, 0) != 0;
}

static VOID RegisterDispatchHandlers(_Inout_ PDRIVER_OBJECT driverObject)
{
    ULONG index;

    for (index = 0; index <= IRP_MJ_MAXIMUM_FUNCTION; ++index) {
        driverObject->MajorFunction[index] = UnsupportedDispatch;
    }

    driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DevioctlDispatch;
    driverObject->MajorFunction[IRP_MJ_CREATE] = CreateDispatch;
    driverObject->MajorFunction[IRP_MJ_CLOSE] = CloseDispatch;
    driverObject->DriverUnload = DriverUnload;
}

static NTSTATUS CreateDeviceAndLink(
    _In_ PDRIVER_OBJECT driverObject,
    _Outptr_ PDEVICE_OBJECT* deviceObject
)
{
    UNICODE_STRING symbolicLink;
    UNICODE_STRING deviceName;
    NTSTATUS status;

    *deviceObject = NULL;

    RtlInitUnicodeString(&deviceName, g_deviceNameBuffer);
    status = IoCreateDevice(
        driverObject,
        0,
        &deviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        deviceObject);
    if (!NT_SUCCESS(status)) {
        LogStatus(L"IoCreateDevice", status, NULL, FALSE);
        return status;
    }

    RtlInitUnicodeString(&symbolicLink, READDRIVER_GLOBAL_DOS_DEVICE_NAME);
    status = IoCreateSymbolicLink(&symbolicLink, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(*deviceObject);
        *deviceObject = NULL;
        LogStatus(L"IoCreateSymbolicLink (Global)", status, NULL, FALSE);
        return status;
    }

    return STATUS_SUCCESS;
}

static NTSTATUS DeleteLinkByName(
    _In_ PCWSTR linkName,
    _In_ PCWSTR operationName,
    _In_ BOOLEAN silentMode
)
{
    UNICODE_STRING symbolicLink;
    NTSTATUS status;

    RtlInitUnicodeString(&symbolicLink, linkName);
    status = IoDeleteSymbolicLink(&symbolicLink);
    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
        status = STATUS_SUCCESS;
    }

    LogStatus(operationName, status, NULL, silentMode);
    return status;
}

NTSTATUS RemoveUserVisibleDeviceLink(_In_ BOOLEAN silentMode)
{
    NTSTATUS globalStatus;

    InterlockedExchange(&g_driverDecommissioned, 1);
    StopFreezeTimer();

    globalStatus = DeleteLinkByName(
        READDRIVER_GLOBAL_DOS_DEVICE_NAME,
        L"IoDeleteSymbolicLink (Global)",
        silentMode);

    (VOID)DeleteLinkByName(
        READDRIVER_DOS_DEVICE_NAME,
        L"IoDeleteSymbolicLink (Local)",
        silentMode);

    return globalStatus;
}

VOID DriverUnload(_In_ struct _DRIVER_OBJECT* DriverObject)
{
    InterlockedExchange(&g_driverDecommissioned, 1);
    StopFreezeTimer();

    (VOID)DeleteLinkByName(READDRIVER_GLOBAL_DOS_DEVICE_NAME, L"IoDeleteSymbolicLink (Global)", FALSE);
    (VOID)DeleteLinkByName(READDRIVER_DOS_DEVICE_NAME, L"IoDeleteSymbolicLink (Local)", FALSE);

    if (DriverObject->DeviceObject != NULL) {
        IoDeleteDevice(DriverObject->DeviceObject);
    }
}

NTSTATUS DriverInitialize(
    _In_ struct _DRIVER_OBJECT* DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;

    UNREFERENCED_PARAMETER(RegistryPath);

    InterlockedExchange(&g_driverDecommissioned, 0);

    status = CreateDeviceAndLink(DriverObject, &deviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    RegisterDispatchHandlers(DriverObject);

    deviceObject->Flags |= DO_BUFFERED_IO;
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    status = InitializeFreezeTimer();
    if (!NT_SUCCESS(status)) {
        (VOID)DeleteLinkByName(READDRIVER_GLOBAL_DOS_DEVICE_NAME, L"IoDeleteSymbolicLink (Global)", FALSE);
        (VOID)DeleteLinkByName(READDRIVER_DOS_DEVICE_NAME, L"IoDeleteSymbolicLink (Local)", FALSE);
        IoDeleteDevice(deviceObject);
        return status;
    }

    return STATUS_SUCCESS;
}
