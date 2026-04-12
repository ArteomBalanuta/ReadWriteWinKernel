#include <windows.h>
#include "..\common\protocol.h"

VOID test(
	VOID
	)
{
	HANDLE          h;
	READDRIVER_REQUEST tmp;
	DWORD           bytesIO;

	h = CreateFileW(READDRIVER_USER_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (h != INVALID_HANDLE_VALUE) {

		ZeroMemory(&tmp, sizeof(tmp));
		tmp.SourceProcessId = 0xAAAAAAAA;
		tmp.SourceAddress = 0xBBBBBBBB;
		tmp.TargetProcessId = 0xCCCCCCCC;
		tmp.TargetAddress = 0xDDDDDDDD;
		tmp.Operation = ReadDriverOperationCopy;

		DeviceIoControl(h, READDRIVER_IOCTL_REQUEST,
			&tmp, sizeof(tmp), &tmp,
			sizeof(tmp), &bytesIO, NULL);

		CloseHandle(h);
	}

}
