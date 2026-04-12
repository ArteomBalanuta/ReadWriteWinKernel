#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "controller.h"

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
}

BOOL ParseRequestArguments(int argc, char* argv[], READDRIVER_REQUEST* request)
{
    ULONGLONG operationValue = 0;

    if (request == NULL) {
        return FALSE;
    }

    if (argc != 7 && argc != 6) {
        PrintUsage();
        return FALSE;
    }

    ZeroMemory(request, sizeof(*request));

    if (argc == 6) {
        if (!ParseUnsignedLongLong(argv[1], &request->SourceProcessId) ||
            !ParseUnsignedLongLong(argv[2], &request->SourceAddress) ||
            !ParseUnsignedLongLong(argv[3], &request->TargetAddress) ||
            !ParseUnsignedLongLong(argv[4], &request->Size) ||
            !ParseUnsignedLongLong(argv[5], &operationValue)) {
            fprintf(stderr, "Failed to parse one or more numeric arguments.\n");
            PrintUsage();
            return FALSE;
        }

        if (operationValue != ReadDriverOperationWriteInteger) {
            fprintf(stderr, "The 5-argument form is reserved for operation 5 (WriteInteger).\n");
            PrintUsage();
            return FALSE;
        }

        request->Operation = operationValue;
        request->TargetProcessId = 0;
        return TRUE;
    }

    if (!ParseUnsignedLongLong(argv[1], &request->SourceProcessId) ||
        !ParseUnsignedLongLong(argv[2], &request->SourceAddress) ||
        !ParseUnsignedLongLong(argv[3], &request->TargetProcessId) ||
        !ParseUnsignedLongLong(argv[4], &request->TargetAddress) ||
        !ParseUnsignedLongLong(argv[5], &request->Size) ||
        !ParseUnsignedLongLong(argv[6], &request->Operation)) {
        fprintf(stderr, "Failed to parse one or more numeric arguments.\n");
        PrintUsage();
        return FALSE;
    }

    return TRUE;
}
