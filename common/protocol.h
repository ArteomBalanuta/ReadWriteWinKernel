#pragma once

#define READDRIVER_DEVICE_TYPE FILE_DEVICE_UNKNOWN
#define READDRIVER_IOCTL_CODE 0x0701
#define READDRIVER_IOCTL_REQUEST CTL_CODE(READDRIVER_DEVICE_TYPE, READDRIVER_IOCTL_CODE, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define READDRIVER_DRIVER_NAME L"\\Driver\\Ki000"
#define READDRIVER_DEVICE_NAME L"\\Device\\Ki000"
#define READDRIVER_DOS_DEVICE_NAME L"\\DosDevices\\Ki000"
#define READDRIVER_USER_DEVICE_NAME L"\\\\.\\Ki000"

typedef enum _READDRIVER_OPERATION {
    ReadDriverOperationCopy = 0,
    ReadDriverOperationRead = 1,
    ReadDriverOperationScan = 2,
    ReadDriverOperationProtect = 3,
    ReadDriverOperationPatch = 4,
    ReadDriverOperationWriteInteger = 5,
    ReadDriverOperationBatchRead = 6,
    ReadDriverOperationReadWithProtect = 7,
    ReadDriverOperationReadHex = 8
} READDRIVER_OPERATION;

/* Silent mode: suppress driver logging (bit 0 of Reserved1). */
#define READDRIVER_FLAG_SILENT 0x0000000000000001UL

typedef struct _READDRIVER_REQUEST {
    ULONGLONG SourceProcessId; /* Source PID for copy, or the target PID for single-process operations. */
    ULONGLONG SourceAddress;   /* Source address for copy, patch address for op 4, or integer payload for op 5. */
    ULONGLONG TargetProcessId; /* Target PID for copy, or region size for op 3. */
    ULONGLONG TargetAddress;   /* Target address for copy, or write destination for op 5. */
    ULONGLONG Size;            /* Transfer size, or integer width for op 5. */
    ULONGLONG Operation;       /* One of READDRIVER_OPERATION. */
    ULONGLONG Flags;           /* Bit 0 = silent mode (no logging). Reserved bits must be 0. */
    ULONGLONG Reserved2;
    ULONGLONG Reserved3;
    ULONGLONG Reserved4;
    ULONGLONG Reserved5;
} READDRIVER_REQUEST, *PREADDRIVER_REQUEST;
