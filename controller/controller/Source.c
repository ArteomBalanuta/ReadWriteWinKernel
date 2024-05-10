
#include <windows.h>
#include <stdio.h>
#include <basetsd.h>
#include <stdlib.h>

typedef struct _INOUT_PARAM {
    ULONGLONG srcPid;
    ULONGLONG srcAddress;
    ULONGLONG targetPid;
    ULONGLONG targetAddress;
    ULONGLONG size;
    ULONGLONG isRead;
    ULONGLONG Param7;
    ULONGLONG Param8;
    ULONGLONG Param9;
    ULONGLONG Param10;
    ULONGLONG Param11;
} INOUT_PARAM, * PINOUT_PARAM;

#define REQUEST    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0701, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

VOID sendRequest(ULONGLONG srcPid, ULONGLONG srcAddress, ULONGLONG targetPid, ULONGLONG targetAddress, ULONGLONG size, ULONGLONG isRead)
{
    HANDLE          handle;
    INOUT_PARAM     tmp;
    DWORD           bytesIO;

    handle = CreateFile(TEXT("\\\\.\\PWN22"), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (handle != INVALID_HANDLE_VALUE) {

        tmp.srcPid = srcPid;
        tmp.srcAddress = srcAddress;
        tmp.targetPid = targetPid;
        tmp.targetAddress = targetAddress;
        tmp.size = size;
        tmp.isRead = isRead;

        printf("Sending request...");
        DeviceIoControl(handle, REQUEST,
            &tmp, sizeof(tmp), &tmp,
            sizeof(tmp), &bytesIO, NULL);
        
        printf("Request send!");

        CloseHandle(handle);
    }
    else {
        printf("handle failed");
    }
}

int main(int argc, char * argv[]) {
    if (argc != 7) {
        printf("Write example: \n");
        printf("Use: srcPid srcAddress targetPid targetAddress size isRead \n");
        printf("Example: controller.exe 5412 0x6520A9F5 6512 0x0203B3F94940 15 0\n");
        printf("    \n");
        printf("Read example: \n");
        printf("Going to copy 512mb \n");
        printf("Use: srcPid srcAddress(ignored) targetPid bufferAddress size isRead\n");
        printf("Example: controller.exe 5412 0x0000 6512 0x0203B3F94940 1 1\n");
        printf("Mem prot change: pid srcAddress size isRead\n");
        printf("Example: controller.exe 5412 0x0203B3F94940 40000 3\n");
        return 0;
    }

    ULONGLONG srcPid = (ULONGLONG) strtoll(argv[1], NULL, 0);
    ULONGLONG srcAddress = (ULONGLONG) strtoll("0x7FF6ED3F0390", NULL, 0);

    ULONGLONG targetPid = (ULONGLONG) strtoll(argv[3], NULL, 0);
    ULONGLONG targetAddress = (ULONGLONG) strtoll(argv[4], NULL, 0);

    ULONGLONG size = (ULONGLONG) strtoll(argv[5], NULL, 0);
    ULONGLONG isRead = (ULONGLONG)strtoll(argv[6], NULL, 0);

    printf("srcPid: %I64u \n", srcPid);
    printf("srcAddress: %I64u \n", srcAddress);
    printf("targetPid: %I64u \n", targetPid);
    printf("targetAddress: %I64u \n", targetAddress);
    printf("size: %I64u \n", size);
    printf("isRead: %I64u \n", isRead);

    sendRequest(srcPid, srcAddress, targetPid, targetAddress, size, isRead);
    
    return 0;
}