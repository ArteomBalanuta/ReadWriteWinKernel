#include <stdio.h>
#include "controller.h"

static HANDLE OpenDriverDevice(void)
{
    HANDLE handle;

    handle = CreateFileW(
        READDRIVER_USER_DEVICE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateFile failed with error %lu\n", GetLastError());
    }

    return handle;
}

BOOL SendDriverRequest(const READDRIVER_REQUEST* request, DWORD* bytesReturned)
{
    HANDLE handle;
    READDRIVER_REQUEST response;

    if (request == NULL || bytesReturned == NULL) {
        fprintf(stderr, "SendDriverRequest received invalid arguments.\n");
        return FALSE;
    }

    handle = OpenDriverDevice();
    if (handle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    ZeroMemory(&response, sizeof(response));
    if (!DeviceIoControl(
        handle,
        READDRIVER_IOCTL_REQUEST,
        (LPVOID)request,
        sizeof(*request),
        &response,
        sizeof(response),
        bytesReturned,
        NULL)) {
        fprintf(stderr, "DeviceIoControl failed with error %lu\n", GetLastError());
        CloseHandle(handle);
        return FALSE;
    }

    CloseHandle(handle);
    return TRUE;
}
