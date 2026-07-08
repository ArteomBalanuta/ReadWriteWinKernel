#include <stdio.h>
#include "controller.h"

int main(int argc, char* argv[])
{
    READDRIVER_REQUEST request;
    BYTE responseBuffer[sizeof(READDRIVER_REQUEST)];
    DWORD bytesReturned = 0;

    if (!ParseRequestArguments(argc, argv, &request)) {
        return 1;
    }

    PrintRequestSummary(&request);

    if (!SendDriverRequest(&request, responseBuffer, sizeof(responseBuffer), &bytesReturned)) {
        return 1;
    }

    printf("Request completed, bytes returned: %lu\n", bytesReturned);

    if (bytesReturned > 0) {
        PrintResponseData(&request, responseBuffer, bytesReturned);
    }

    return 0;
}
