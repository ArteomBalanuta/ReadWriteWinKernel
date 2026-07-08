#pragma once

#include <windows.h>
#include "..\..\common\protocol.h"

void PrintUsage(void);
void PrintRequestSummary(const READDRIVER_REQUEST* request);
void PrintResponseData(const READDRIVER_REQUEST* request, const BYTE* buffer, DWORD bytesReturned);
BOOL ParseRequestArguments(int argc, char* argv[], READDRIVER_REQUEST* request);
BOOL SendDriverRequest(
    const READDRIVER_REQUEST* request,
    LPVOID responseBuffer,
    DWORD responseBufferSize,
    DWORD* bytesReturned);
