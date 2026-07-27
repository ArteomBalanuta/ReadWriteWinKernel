#pragma once

#define READDRIVER_DEVICE_TYPE FILE_DEVICE_UNKNOWN
#define READDRIVER_IOCTL_CODE 0x0701
#define READDRIVER_IOCTL_REQUEST CTL_CODE(READDRIVER_DEVICE_TYPE, READDRIVER_IOCTL_CODE, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

/* Internal kernel object names — randomized at load time as \Driver\gammaA#### / \Device\gammaA####.
   These macros are build-time defaults only; the driver generates the actual names in DriverEntry. */
#define READDRIVER_DRIVER_NAME L"\\Driver\\gammaA3891"
#define READDRIVER_DEVICE_NAME L"\\Device\\gammaA3891"
#define READDRIVER_DOS_DEVICE_NAME L"\\DosDevices\\Ki000"
#define READDRIVER_GLOBAL_DOS_DEVICE_NAME L"\\DosDevices\\Global\\Ki000"
#define READDRIVER_USER_DEVICE_NAME L"\\\\.\\Global\\Ki000"

typedef enum _READDRIVER_OPERATION {
    ReadDriverOperationCopy = 0,
    ReadDriverOperationRead = 1,
    ReadDriverOperationScan = 2,
    ReadDriverOperationProtect = 3,
    ReadDriverOperationPatch = 4,
    ReadDriverOperationWriteInteger = 5,
    ReadDriverOperationBatchRead = 6,
    ReadDriverOperationReadWithProtect = 7,
    ReadDriverOperationReadHex = 8,
    ReadDriverOperationUnload = 9,
    ReadDriverOperationFreezeWrite = 10,
    ReadDriverOperationUnfreeze = 11
} READDRIVER_OPERATION;

/* Max freeze slots the driver can maintain simultaneously. */
#define READDRIVER_MAX_FREEZE_SLOTS 16

/* Silent mode: suppress driver logging (bit 0 of Reserved1). */
#define READDRIVER_FLAG_SILENT 0x0000000000000001UL

typedef struct _READDRIVER_REQUEST {
    ULONGLONG SourceProcessId; /* Source PID for copy, target PID for single-process operations, or unfreeze scope. */
    ULONGLONG SourceAddress;   /* Source address for copy, integer payload for op 5, or address to unfreeze. */
    ULONGLONG TargetProcessId; /* Target PID for copy, region size for op 3, or second freeze address. */
    ULONGLONG TargetAddress;   /* Target address for copy/write, or first freeze address. */
    ULONGLONG Size;            /* Transfer size, or integer width for op 5. */
    ULONGLONG Operation;       /* One of READDRIVER_OPERATION. */
    ULONGLONG Flags;           /* Bit 0 = silent mode (no logging). Reserved bits must be 0. */
    ULONGLONG Reserved2;       /* Third freeze address when operation is FreezeWrite. */
    ULONGLONG Reserved3;       /* Fourth freeze address when operation is FreezeWrite. */
    ULONGLONG Reserved4;
    ULONGLONG Reserved5;
} READDRIVER_REQUEST, *PREADDRIVER_REQUEST;
