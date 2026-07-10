#pragma once

#include "..\common\protocol.h"

NTKERNELAPI
NTSTATUS
IoCreateDriver(
	_In_ PUNICODE_STRING DriverName, OPTIONAL
    _In_ PDRIVER_INITIALIZE InitializationFunction
	);

/* Runtime device name buffers — populated once per load with a random suffix. */
extern WCHAR g_driverNameBuffer[64];
extern WCHAR g_deviceNameBuffer[64];

VOID GenerateRandomDeviceNames(VOID);

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH DevioctlDispatch;
_Dispatch_type_(IRP_MJ_CREATE) DRIVER_DISPATCH CreateDispatch;
_Dispatch_type_(IRP_MJ_CLOSE) DRIVER_DISPATCH CloseDispatch;
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH UnsupportedDispatch;

DRIVER_INITIALIZE DriverInitialize;
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;
#pragma alloc_text(INIT, DriverEntry)

NTSTATUS RemoveUserVisibleDeviceLink(
    _In_ BOOLEAN silentMode
);

BOOLEAN IsDriverDecommissioned(VOID);

NTSTATUS AppendLogLine(
    _In_ PCWSTR path,
    _In_ PCWSTR operation,
    _In_opt_ PCWSTR detail,
    _In_ NTSTATUS status
);

VOID LogStatus(
    _In_ PCWSTR operation,
    _In_ NTSTATUS status,
    _In_opt_ PCWSTR detail,
    _In_ BOOLEAN silentMode
);

VOID LogText(
    _In_ PCWSTR operation,
    _In_opt_ PCWSTR text,
    _In_ BOOLEAN silentMode
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
    _In_ SIZE_T size,
    _In_ BOOLEAN silentMode
);

NTSTATUS ProtectProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ SIZE_T size,
    _In_ BOOLEAN silentMode
);

NTSTATUS PatchProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ BOOLEAN silentMode
);

NTSTATUS WriteIntegerProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ ULONGLONG value,
    _In_ SIZE_T size,
    _In_ BOOLEAN silentMode
);

NTSTATUS ReadProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ SIZE_T size,
    _Out_writes_(size) PVOID buffer,
    _In_ BOOLEAN silentMode
);

NTSTATUS ScanProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID startAddress,
    _In_ SIZE_T size,
    _In_reads_(patternSize) const UCHAR* pattern,
    _In_ ULONG patternSize,
    _Out_ PULONGLONG matchedAddress,
    _In_ BOOLEAN silentMode
);

NTSTATUS BatchReadProcessMemory(
    _In_ HANDLE processId,
    _In_reads_(count) const ULONGLONG* addresses,
    _In_ ULONG count,
    _In_ SIZE_T readSize,
    _Out_writes_(count * readSize) PVOID buffer,
    _In_ BOOLEAN silentMode
);

NTSTATUS ReadWithProtect(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ SIZE_T size,
    _Out_writes_(size) PVOID buffer,
    _Out_opt_ PULONG originalProtection,
    _In_ BOOLEAN silentMode
);

NTSTATUS ReadProcessMemoryAsHex(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ SIZE_T size,
    _Out_writes_(hexBufferSize) PCHAR hexBuffer,
    _In_ SIZE_T hexBufferSize,
    _In_ BOOLEAN silentMode
);

NTSTATUS HandleDeviceControl(
    _In_ PIO_STACK_LOCATION stack,
    _Inout_ PIRP irp,
    _Out_ PULONG_PTR bytesReturned
);

NTSTATUS InitializeFreezeTimer(VOID);
VOID StopFreezeTimer(VOID);

NTSTATUS AddFreezeSlot(
    _In_ HANDLE processId,
    _In_ ULONG_PTR address,
    _In_ ULONGLONG value,
    _In_ SIZE_T size
);

NTSTATUS ClearFreezeSlot(
    _In_ ULONG_PTR address
);
