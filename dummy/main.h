#pragma once

#include "..\common\protocol.h"

NTKERNELAPI
NTSTATUS
IoCreateDriver(
	_In_ PUNICODE_STRING DriverName, OPTIONAL
    _In_ PDRIVER_INITIALIZE InitializationFunction
	);

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH DevioctlDispatch;
_Dispatch_type_(IRP_MJ_CREATE) DRIVER_DISPATCH CreateDispatch;
_Dispatch_type_(IRP_MJ_CLOSE) DRIVER_DISPATCH CloseDispatch;
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH UnsupportedDispatch;

DRIVER_INITIALIZE DriverInitialize;
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;
#pragma alloc_text(INIT, DriverEntry)

NTSTATUS AppendLogLine(
    _In_ PCWSTR path,
    _In_ PCWSTR operation,
    _In_opt_ PCWSTR detail,
    _In_ NTSTATUS status
);

VOID LogStatus(
    _In_ PCWSTR operation,
    _In_ NTSTATUS status,
    _In_opt_ PCWSTR detail
);

VOID LogText(
    _In_ PCWSTR operation,
    _In_opt_ PCWSTR text
);

NTSTATUS FormatTimestamp(
    _In_ const LARGE_INTEGER* systemTime,
    _Out_writes_(bufferLength) PWCHAR buffer,
    _In_ SIZE_T bufferLength
);

NTSTATUS CopyProcessMemory(
    _In_ HANDLE sourceProcessId,
    _In_ PVOID sourceAddress,
    _In_ HANDLE targetProcessId,
    _In_ PVOID targetAddress,
    _In_ SIZE_T size
);

NTSTATUS ProtectProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ SIZE_T size
);

NTSTATUS PatchProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address
);

NTSTATUS WriteIntegerProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ ULONGLONG value,
    _In_ SIZE_T size
);

NTSTATUS HandleDeviceControl(
    _In_ PIO_STACK_LOCATION stack,
    _Inout_ PIRP irp,
    _Out_ PULONG_PTR bytesReturned
);
