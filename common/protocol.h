#pragma once

#define READDRIVER_DEVICE_TYPE FILE_DEVICE_UNKNOWN
#define READDRIVER_IOCTL_CODE 0x0701
#define READDRIVER_IOCTL_REQUEST CTL_CODE(READDRIVER_DEVICE_TYPE, READDRIVER_IOCTL_CODE, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define READDRIVER_DEVICE_NAME L"\\Device\\PWN29"
#define READDRIVER_DOS_DEVICE_NAME L"\\DosDevices\\PWN29"
#define READDRIVER_USER_DEVICE_NAME L"\\\\.\\PWN29"

typedef enum _READDRIVER_OPERATION {
    ReadDriverOperationCopy = 0,
    ReadDriverOperationRead = 1,
    ReadDriverOperationProtect = 3,
    ReadDriverOperationPatch = 4,
    ReadDriverOperationWriteInteger = 5
} READDRIVER_OPERATION;

typedef struct _READDRIVER_REQUEST {
    ULONGLONG SourceProcessId;
    ULONGLONG SourceAddress;
    ULONGLONG TargetProcessId;
    ULONGLONG TargetAddress;
    ULONGLONG Size;
    ULONGLONG Operation;
    ULONGLONG Reserved7;
    ULONGLONG Reserved8;
    ULONGLONG Reserved9;
    ULONGLONG Reserved10;
    ULONGLONG Reserved11;
} READDRIVER_REQUEST, *PREADDRIVER_REQUEST;
