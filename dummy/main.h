#pragma once

NTKERNELAPI
NTSTATUS
IoCreateDriver(
	_In_ PUNICODE_STRING DriverName, OPTIONAL
    _In_ PDRIVER_INITIALIZE InitializationFunction
	);

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH DevioctlDispatch;
_Dispatch_type_(IRP_MJ_CREATE)
DRIVER_DISPATCH CreateDispatch;
_Dispatch_type_(IRP_MJ_CLOSE)
DRIVER_DISPATCH CloseDispatch;

_Dispatch_type_(IRP_MJ_CREATE)
_Dispatch_type_(IRP_MJ_CREATE_NAMED_PIPE)
_Dispatch_type_(IRP_MJ_CLOSE)
_Dispatch_type_(IRP_MJ_READ)
_Dispatch_type_(IRP_MJ_WRITE)
_Dispatch_type_(IRP_MJ_QUERY_INFORMATION)
_Dispatch_type_(IRP_MJ_SET_INFORMATION)
_Dispatch_type_(IRP_MJ_QUERY_EA)
_Dispatch_type_(IRP_MJ_SET_EA)
_Dispatch_type_(IRP_MJ_FLUSH_BUFFERS)
_Dispatch_type_(IRP_MJ_QUERY_VOLUME_INFORMATION)
_Dispatch_type_(IRP_MJ_SET_VOLUME_INFORMATION)
_Dispatch_type_(IRP_MJ_DIRECTORY_CONTROL)
_Dispatch_type_(IRP_MJ_FILE_SYSTEM_CONTROL)
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
_Dispatch_type_(IRP_MJ_INTERNAL_DEVICE_CONTROL)
_Dispatch_type_(IRP_MJ_SHUTDOWN)
_Dispatch_type_(IRP_MJ_LOCK_CONTROL)
_Dispatch_type_(IRP_MJ_CLEANUP)
_Dispatch_type_(IRP_MJ_CREATE_MAILSLOT)
_Dispatch_type_(IRP_MJ_QUERY_SECURITY)
_Dispatch_type_(IRP_MJ_SET_SECURITY)
_Dispatch_type_(IRP_MJ_POWER)
_Dispatch_type_(IRP_MJ_SYSTEM_CONTROL)
_Dispatch_type_(IRP_MJ_DEVICE_CHANGE)
_Dispatch_type_(IRP_MJ_QUERY_QUOTA)
_Dispatch_type_(IRP_MJ_SET_QUOTA)
_Dispatch_type_(IRP_MJ_PNP)
DRIVER_DISPATCH UnsupportedDispatch;

DRIVER_INITIALIZE DriverInitialize;
DRIVER_INITIALIZE DriverEntry;
#pragma alloc_text(INIT, DriverEntry)

#define DUMMYDRV_REQUEST1    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0701, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

typedef struct _INOUT_PARAM {
	ULONGLONG Param1;
	ULONGLONG Param2;
	ULONGLONG Param3;
	ULONGLONG Param4;
	ULONGLONG Param5;
	ULONGLONG Param6;
	ULONGLONG Param7;
	ULONGLONG Param8;
	ULONGLONG Param9;
	ULONGLONG Param10;
	ULONGLONG Param11;
} INOUT_PARAM, *PINOUTPARAM;

VOID logToFile(const WCHAR* operation, NTSTATUS status, const WCHAR* addressRange);

NTSTATUS OverwriteMemory(HANDLE ProcessId, PVOID VirtualAddress, PVOID NewValue, SIZE_T Size);