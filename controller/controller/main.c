#include <stdio.h>
#include "controller.h"

int main(int argc, char* argv[])
{
    READDRIVER_REQUEST request;
    DWORD bytesReturned = 0;

    if (!ParseRequestArguments(argc, argv, &request)) {
        return 1;
    }

    PrintRequestSummary(&request);

    if (!SendDriverRequest(&request, &bytesReturned)) {
        return 1;
    }

    printf("Request completed, bytes returned: %lu\n", bytesReturned);
    return 0;
}
