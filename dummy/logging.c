#include <ntifs.h>
#include <ntstrsafe.h>
#include "main.h"

#define READDRIVER_STATUS_LOG_PATH L"\\DosDevices\\C:\\dummy_logs.txt"
#define READDRIVER_TEXT_LOG_PATH L"\\DosDevices\\C:\\dummy_conversion_logs.txt"

static PCSTR DescribeStatus(NTSTATUS status)
{
    switch (status) {
    case STATUS_SUCCESS:
        return "STATUS_SUCCESS";
    case STATUS_INVALID_PARAMETER:
        return "STATUS_INVALID_PARAMETER";
    case STATUS_ACCESS_DENIED:
        return "STATUS_ACCESS_DENIED";
    case STATUS_ACCESS_VIOLATION:
        return "STATUS_ACCESS_VIOLATION";
    case STATUS_NOT_FOUND:
        return "STATUS_NOT_FOUND";
    case STATUS_BUFFER_TOO_SMALL:
        return "STATUS_BUFFER_TOO_SMALL";
    case STATUS_INSUFFICIENT_RESOURCES:
        return "STATUS_INSUFFICIENT_RESOURCES";
    case STATUS_NO_MEMORY:
        return "STATUS_NO_MEMORY";
    case STATUS_INVALID_DEVICE_REQUEST:
        return "STATUS_INVALID_DEVICE_REQUEST";
    case STATUS_OBJECT_NAME_NOT_FOUND:
        return "STATUS_OBJECT_NAME_NOT_FOUND";
    case STATUS_OBJECT_NAME_COLLISION:
        return "STATUS_OBJECT_NAME_COLLISION";
    case STATUS_NOT_SUPPORTED:
        return "STATUS_NOT_SUPPORTED";
    case STATUS_TIMEOUT:
        return "STATUS_TIMEOUT";
    default:
        return "STATUS_UNKNOWN";
    }
}

NTSTATUS FormatTimestamp(
    _In_ const LARGE_INTEGER* systemTime,
    _Out_writes_(bufferLength) PWCHAR buffer,
    _In_ SIZE_T bufferLength
)
{
    TIME_FIELDS timeFields;
    LARGE_INTEGER capturedTime;
    LARGE_INTEGER localTime;
    NTSTATUS status;

    if (systemTime == NULL || buffer == NULL || bufferLength == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    capturedTime = *systemTime;
    ExSystemTimeToLocalTime(&capturedTime, &localTime);
    RtlTimeToTimeFields(&localTime, &timeFields);

    status = RtlStringCchPrintfW(
        buffer,
        bufferLength,
        L"%04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu",
        (USHORT)timeFields.Year,
        (USHORT)timeFields.Month,
        (USHORT)timeFields.Day,
        (USHORT)timeFields.Hour,
        (USHORT)timeFields.Minute,
        (USHORT)timeFields.Second,
        (USHORT)timeFields.Milliseconds);

    return status;
}

NTSTATUS AppendLogLine(
    _In_ PCWSTR path,
    _In_ PCWSTR operation,
    _In_opt_ PCWSTR detail,
    _In_ NTSTATUS status
)
{
    UNICODE_STRING targetPath;
    OBJECT_ATTRIBUTES attributes;
    IO_STATUS_BLOCK ioStatus;
    HANDLE fileHandle = NULL;
    LARGE_INTEGER systemTime;
    WCHAR timestamp[64];
    CHAR logLine[512];
    NTSTATUS openStatus;

    if (path == NULL || operation == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlInitUnicodeString(&targetPath, path);
    InitializeObjectAttributes(&attributes, &targetPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    openStatus = ZwCreateFile(
        &fileHandle,
        FILE_APPEND_DATA | SYNCHRONIZE,
        &attributes,
        &ioStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0);

    if (!NT_SUCCESS(openStatus)) {
        return openStatus;
    }

    KeQuerySystemTime(&systemTime);
    if (!NT_SUCCESS(FormatTimestamp(&systemTime, timestamp, RTL_NUMBER_OF(timestamp)))) {
        RtlStringCchCopyW(timestamp, RTL_NUMBER_OF(timestamp), L"unknown-time");
    }

    if (detail != NULL && detail[0] != L'\0') {
        RtlStringCchPrintfA(
            logLine,
            RTL_NUMBER_OF(logLine),
            "[%ws] %ws status=0x%08X (%s) detail=%ws\r\n",
            timestamp,
            operation,
            status,
            DescribeStatus(status),
            detail);
    } else {
        RtlStringCchPrintfA(
            logLine,
            RTL_NUMBER_OF(logLine),
            "[%ws] %ws status=0x%08X (%s)\r\n",
            timestamp,
            operation,
            status,
            DescribeStatus(status));
    }

    openStatus = ZwWriteFile(
        fileHandle,
        NULL,
        NULL,
        NULL,
        &ioStatus,
        logLine,
        (ULONG)strlen(logLine),
        NULL,
        NULL);

    ZwClose(fileHandle);
    return openStatus;
}

VOID LogStatus(
    _In_ PCWSTR operation,
    _In_ NTSTATUS status,
    _In_opt_ PCWSTR detail,
    _In_ BOOLEAN silentMode
)
{
    if (!silentMode) {
        (VOID)AppendLogLine(READDRIVER_STATUS_LOG_PATH, operation, detail, status);
    }
}

VOID LogText(
    _In_ PCWSTR operation,
    _In_opt_ PCWSTR text,
    _In_ BOOLEAN silentMode
)
{
    if (!silentMode) {
        (VOID)AppendLogLine(READDRIVER_TEXT_LOG_PATH, operation, text, STATUS_SUCCESS);
    }
}
