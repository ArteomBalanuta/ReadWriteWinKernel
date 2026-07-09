#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "controller.h"

#define CONTROLLER_UNLOAD_ARGC 2
#define CONTROLLER_SHORTCUT_ARGC 6
#define CONTROLLER_FULL_ARGC 7
#define CONTROLLER_PARSE_ERROR "Failed to parse one or more numeric arguments.\n"

static int ParseUnsignedLongLong(const char* text, ULONGLONG* value)
{
    char* end = NULL;

    errno = 0;
    *value = _strtoui64(text, &end, 0);
    return errno == 0 && end != text && *end == '\0';
}

void PrintUsage(void)
{
    printf("Usage:\n");
    printf("  controller.exe unload\n");
    printf("  controller.exe <srcPid> <srcAddress> <targetPid> <targetAddress> <size> <operation>\n");
    printf("  controller.exe <processPid> <integerValue> <targetAddress> <size> 5\n");
    printf("\n");
    printf("Examples:\n");
    printf("  Copy:    controller.exe 5412 0x6520A9F5 6512 0x0203B3F94940 15 0\n");
    printf("  Protect: controller.exe 5412 0x0203B3F94940 40000 0 0 3\n");
    printf("  Patch:   controller.exe 5412 0x0203B3F94940 0 0 0 4\n");
    printf("  WriteInt: controller.exe 6512 500 0x0203B3F94940 2 5\n");
}

void PrintRequestSummary(const READDRIVER_REQUEST* request)
{
    printf("Source PID:    %llu\n", request->SourceProcessId);
    printf("Source Addr:   0x%llX\n", request->SourceAddress);
    printf("Target PID:    %llu\n", request->TargetProcessId);
    printf("Target Addr:   0x%llX\n", request->TargetAddress);
    printf("Size:          %llu\n", request->Size);
    printf("Operation:     %llu\n", request->Operation);
    printf("Flags:         0x%llX\n", request->Flags);
}

void PrintResponseData(const READDRIVER_REQUEST* request, const BYTE* buffer, DWORD bytesReturned)
{
    if (buffer == NULL || bytesReturned == 0) {
        return;
    }

    switch ((READDRIVER_OPERATION)request->Operation) {
    case ReadDriverOperationRead:
    case ReadDriverOperationReadWithProtect:
        /* Print raw bytes as hex. */
        printf("Response (hex): ");
        for (DWORD i = 0; i < bytesReturned; ++i) {
            printf("%02X ", buffer[i]);
        }
        printf("\n");
        break;
    case ReadDriverOperationReadHex:
        /* Response is already a hex string. */
        printf("Response: %s\n", (const char*)buffer);
        break;
    case ReadDriverOperationScan:
        /* Response is a matched address (ULONGLONG). */
        if (bytesReturned >= sizeof(ULONGLONG)) {
            ULONGLONG matchedAddr;
            RtlCopyMemory(&matchedAddr, buffer, sizeof(ULONGLONG));
            printf("Matched address: 0x%llX\n", matchedAddr);
        }
        break;
    case ReadDriverOperationBatchRead:
        /* Print each read chunk as hex. */
        printf("Response (batch, %lu chunks):\n", bytesReturned / (DWORD)request->Size);
        for (DWORD offset = 0; offset < bytesReturned; offset += (DWORD)request->Size) {
            printf("  ");
            for (DWORD i = 0; i < request->Size && (offset + i) < bytesReturned; ++i) {
                printf("%02X ", buffer[offset + i]);
            }
            printf("\n");
        }
        break;
    default:
        /* Generic hex dump for unknown operations. */
        printf("Response (hex): ");
        for (DWORD i = 0; i < bytesReturned; ++i) {
            printf("%02X ", buffer[i]);
        }
        printf("\n");
        break;
    }
}

BOOL ParseRequestArguments(int argc, char* argv[], READDRIVER_REQUEST* request)
{
    ULONGLONG operationValue = 0;

    if (request == NULL) {
        return FALSE;
    }

    if (argc == CONTROLLER_UNLOAD_ARGC && _stricmp(argv[1], "unload") == 0) {
        ZeroMemory(request, sizeof(*request));
        request->Operation = ReadDriverOperationUnload;
        return TRUE;
    }

    if (argc != CONTROLLER_FULL_ARGC && argc != CONTROLLER_SHORTCUT_ARGC) {
        PrintUsage();
        return FALSE;
    }

    ZeroMemory(request, sizeof(*request));

    if (argc == CONTROLLER_SHORTCUT_ARGC) {
        if (!ParseUnsignedLongLong(argv[1], &request->SourceProcessId) ||
            !ParseUnsignedLongLong(argv[2], &request->SourceAddress) ||
            !ParseUnsignedLongLong(argv[3], &request->TargetAddress) ||
            !ParseUnsignedLongLong(argv[4], &request->Size) ||
            !ParseUnsignedLongLong(argv[5], &operationValue)) {
            fputs(CONTROLLER_PARSE_ERROR, stderr);
            PrintUsage();
            return FALSE;
        }

        if (operationValue != ReadDriverOperationWriteInteger) {
            fprintf(stderr, "The 5-argument form is reserved for operation 5 (WriteInteger).\n");
            PrintUsage();
            return FALSE;
        }

        request->Operation = operationValue;
        return TRUE;
    }

    if (!ParseUnsignedLongLong(argv[1], &request->SourceProcessId) ||
        !ParseUnsignedLongLong(argv[2], &request->SourceAddress) ||
        !ParseUnsignedLongLong(argv[3], &request->TargetProcessId) ||
        !ParseUnsignedLongLong(argv[4], &request->TargetAddress) ||
        !ParseUnsignedLongLong(argv[5], &request->Size) ||
        !ParseUnsignedLongLong(argv[6], &request->Operation)) {
        fputs(CONTROLLER_PARSE_ERROR, stderr);
        PrintUsage();
        return FALSE;
    }

    return TRUE;
}
