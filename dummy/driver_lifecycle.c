#include <ntifs.h>
#include <ntddk.h>
#include "main.h"

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

    RtlInitUnicodeString(&deviceName, READDRIVER_DEVICE_NAME);
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

    RtlInitUnicodeString(&symbolicLink, READDRIVER_DOS_DEVICE_NAME);
    status = IoCreateSymbolicLink(&symbolicLink, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(*deviceObject);
        *deviceObject = NULL;
        LogStatus(L"IoCreateSymbolicLink", status, NULL, FALSE);
        return status;
    }

    // Hide: delete the local symbolic link so \DosDevices scans miss us.
    IoDeleteSymbolicLink(&symbolicLink);

    // Keep the user-visible path in the global MS-DOS namespace for clients.
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

NTSTATUS RemoveUserVisibleDeviceLink(_In_ BOOLEAN silentMode)
{
    UNICODE_STRING symbolicLink;
    NTSTATUS status;

    RtlInitUnicodeString(&symbolicLink, READDRIVER_GLOBAL_DOS_DEVICE_NAME);
    status = IoDeleteSymbolicLink(&symbolicLink);
    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
        status = STATUS_SUCCESS;
    }

    LogStatus(L"IoDeleteSymbolicLink (Global)", status, NULL, silentMode);
    return status;
}

VOID DriverUnload(_In_ struct _DRIVER_OBJECT* DriverObject)
{
    UNICODE_STRING symbolicLink;

    RtlInitUnicodeString(&symbolicLink, READDRIVER_GLOBAL_DOS_DEVICE_NAME);
    IoDeleteSymbolicLink(&symbolicLink);

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

    status = CreateDeviceAndLink(DriverObject, &deviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    RegisterDispatchHandlers(DriverObject);

    deviceObject->Flags |= DO_BUFFERED_IO;
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;
}
