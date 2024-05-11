#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>
#include "main.h"


NTSTATUS RtlTimeToTimeString(const LARGE_INTEGER* SystemTime, WCHAR* Buffer, ULONG* ReturnLength);

NTSYSAPI NTSTATUS NTAPI ZwSuspendThread(IN HANDLE ThreadHandle, OUT PULONG
    PreviousSuspendCount OPTIONAL);

NTSYSAPI NTSTATUS NTAPI ZwProtectVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID* BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG NewProtect,
    OUT PULONG OldProtect
);

NTSTATUS GetProcessValidAddressSpace(HANDLE ProcessId, PVOID* StartAddress, PVOID* EndAddress);

VOID logValues(const WCHAR* operation, const WCHAR* value);



/*
Here are some Windows kernel functions commonly used to obtain valid memory ranges for a given process:

    ZwQueryVirtualMemory / NtQueryVirtualMemory:
        These functions can be used to query information about the virtual memory layout of a specified process. By iterating through the memory regions returned by these functions, you can obtain valid memory ranges.
    MmGetVirtualForPhysical:
        While this function converts a physical address to a virtual address in the current process context, it can indirectly provide information about valid memory ranges when used in conjunction with other methods.
    PsGetProcessSectionBaseAddress:
        This function returns the base address of the executable image mapped into the virtual address space of a specified process. It can provide information about valid memory ranges within the executable image.
    MmCopyVirtualMemory:
        This function allows for the copying of memory from one process to another. While its primary purpose is memory manipulation, it indirectly provides information about valid memory ranges in both the source and destination processes.
    ZwMapViewOfSection / NtMapViewOfSection:
        These functions are used to map a view of a section of a file into the virtual address space of a specified process. By mapping specific sections, you can obtain valid memory ranges within the process.
    ZwQuerySystemInformation / NtQuerySystemInformation:
        These functions can be used to query system information, including information about the virtual memory layout of processes. The SystemProcessInformation class provides details about each process, including memory ranges.
    ZwReadVirtualMemory / NtReadVirtualMemory:
        While primarily used for reading memory from a specified process, these functions can indirectly provide information about valid memory ranges within the target process.

These are some of the commonly used functions for obtaining information about valid memory ranges within a process in the Windows kernel. 
However, it's important to note that accessing process memory from kernel mode requires appropriate permissions and should be done carefully to avoid system instability or security vulnerabilities.

*/
NTSTATUS GetProcessValidAddressSpace(HANDLE processHandle, PVOID* StartAddress, PVOID* EndAddress)
{
    // Define variables for ZwQueryVirtualMemory
    MEMORY_BASIC_INFORMATION MemoryInfo;
    SIZE_T ReturnLength;

    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    // Initialize the start and end addresses to NULL
    *StartAddress = NULL;
    *EndAddress = NULL;

    // Define a variable to store the current address
    PVOID CurrentAddress = NULL;

    // Loop through the process's address space regions
    while (NT_SUCCESS(Status = ZwQueryVirtualMemory(processHandle, CurrentAddress, MemoryBasicInformation, &MemoryInfo, sizeof(MemoryInfo), &ReturnLength)))
    {
        // Check if the region is valid and committed
        if (MemoryInfo.State == MEM_COMMIT && MemoryInfo.Type != MEM_MAPPED && MemoryInfo.Type != MEM_PRIVATE)
        {
            // If StartAddress is not set, set it to the base address of the region
            if (*StartAddress == NULL)
            {
                *StartAddress = MemoryInfo.BaseAddress;
            }

            // Set EndAddress to the end address of the current region
            *EndAddress = (PVOID)((ULONG_PTR)MemoryInfo.BaseAddress + MemoryInfo.RegionSize);
        }

        // Move to the next region
        CurrentAddress = (PVOID)((ULONG_PTR)MemoryInfo.BaseAddress + MemoryInfo.RegionSize);

        // Check if we've reached the end of the process's address space
        if (CurrentAddress >= MM_HIGHEST_USER_ADDRESS)
        {
            break;
        }
    }

    // Dereference the process

    // Log the addresses
    WCHAR str[256];
    swprintf(str, L"Start Address: %p, End Address: %p", *StartAddress, *EndAddress);
    logValues(L"GetProcessValidAddressSpace", str);

    return Status;
}



NTSTATUS VirtualProtectMem(HANDLE pid, PVOID address, ULONG pageSize);

NTSTATUS VirtualProtectMem(HANDLE ProcessId, PVOID address, ULONG pageSize)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    HANDLE hProcess = NULL;

    __try {
        // Initialize the OBJECT_ATTRIBUTES structure
        OBJECT_ATTRIBUTES objAttr;
        InitializeObjectAttributes(&objAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

        // Initialize the CLIENT_ID structure
        CLIENT_ID clientId;
        clientId.UniqueProcess = ProcessId;
        clientId.UniqueThread = NULL;

        // Open a handle to the target process
        status = ZwOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &objAttr, &clientId);
        if (!NT_SUCCESS(status)) {
            logToFile(L"ZwOpenProcess", status, NULL);
            return status;
        }

        // Validate address within process address space
        PVOID StartAddress = 0, EndAddress = 0;
        status = GetProcessValidAddressSpace(hProcess, &StartAddress, &EndAddress);
        if (!NT_SUCCESS(status)) {
            // Handle error from GetProcessValidAddressSpace
            logToFile(L"GetProcessValidAddressSpace ", status, NULL);
            ZwClose(hProcess);
            return status;
        }

        if (address < StartAddress || address > EndAddress) {
            WCHAR addressRange[150];
            swprintf(addressRange, L"\n %p - %p, requested: %p", StartAddress, EndAddress, address);
            logToFile(L"Invalid Address ", STATUS_INVALID_PARAMETER, addressRange);
            ZwClose(hProcess);
            return STATUS_INVALID_PARAMETER;
        }

        ULONG curProtection = NULL; // variable to store the previous page protection
        SIZE_T size = pageSize;

        WCHAR str[256];
        swprintf(str, L"Args - address: %p, size: %d", address, pageSize);
        logValues(L"Calling ZwProtectVirtualMemory", str);

        // Attempting to change the page protection
        status = ZwProtectVirtualMemory(hProcess, address, &size, PAGE_EXECUTE_READWRITE, &curProtection);

        if (!NT_SUCCESS(status)) {
            logToFile(L"ZwProtectVirtualMemory", status, NULL);
        }
        else {
            logToFile(L"ZwProtectVirtualMemory", status, NULL);
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // Log exception using logToFile
        logToFile(L"Exception in VirtualProtectMem", GetExceptionCode(), NULL);
        status = STATUS_UNSUCCESSFUL; // Set a suitable status to indicate exception handling
    }
    ZwClose(hProcess);
    return status;
}



NTKERNELAPI NTSTATUS NTAPI MmCopyVirtualMemory(
    IN PEPROCESS FromProcess,
    IN PVOID FromAddress,
    IN PEPROCESS ToProcess,
    OUT PVOID ToAddress,
    IN SIZE_T BufferSize,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PSIZE_T NumberOfBytesCopied
);

NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(
    _In_ HANDLE ProcessId,
    _Outptr_ PEPROCESS* Process
);



NTSTATUS KCopyProcessMemory(PEPROCESS srcProcess, PEPROCESS trgtProcess, PVOID sourceAddress, PVOID targetAddress, SIZE_T size) {
    PEPROCESS sourceProcess = srcProcess;
    PEPROCESS targetProcess = trgtProcess;
    SIZE_T Result;
    NTSTATUS status;
    try {

    status = MmCopyVirtualMemory(sourceProcess, sourceAddress, targetProcess, targetAddress, size, KernelMode, &Result);
    logToFile(L"MmCopyVirtualMemory", status, NULL);

    } except(EXCEPTION_EXECUTE_HANDLER) {
        // Save exception using writeFailFile
        logToFile(L"MmCopyVirtualMemory Exception", GetExceptionCode(), NULL);
        status = STATUS_UNSUCCESSFUL; // Set a suitable status to indicate exception handling
    }
    return status;
}
VOID logToFile(const WCHAR* operation, NTSTATUS status, const WCHAR* addressRange)
{
    try {
        UNICODE_STRING wsFinalPath;
        RtlInitUnicodeString(&wsFinalPath, L"\\DosDevices\\C:\\dummy_logs.txt");

        OBJECT_ATTRIBUTES oa;
        InitializeObjectAttributes(&oa, &wsFinalPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

        IO_STATUS_BLOCK io;
        HANDLE hFile;

        auto res = ZwCreateFile(&hFile, FILE_APPEND_DATA, &oa, &io, NULL,
            FILE_ATTRIBUTE_NORMAL,
            0,
            FILE_OPEN, // Open the file only if it exists, don't create it
            FILE_SYNCHRONOUS_IO_NONALERT,
            NULL, 0);

        if (NT_SUCCESS(res)) {
            // Construct log content with operation and status information
            CHAR logContent[256]; // Adjust size according to your needs

            LARGE_INTEGER systemTime;
            KeQuerySystemTime(&systemTime);

            // Format system time as a human-readable string
            WCHAR timeString[500];
            ULONG returnLength = 0;
            RtlTimeToTimeString(&systemTime, timeString, &returnLength);
            timeString[returnLength / sizeof(WCHAR)] = L'\0'; // Ensure null termination

            // Match the status code with its description
            const char* statusDescription;
            switch (status) {
            case STATUS_SUCCESS:
                statusDescription = "STATUS_SUCCESS";
                break;
            case STATUS_INVALID_PARAMETER:
                statusDescription = "STATUS_INVALID_PARAMETER";
                break;
            case STATUS_ACCESS_DENIED:
                statusDescription = "STATUS_ACCESS_DENIED";
                break;
            case STATUS_NOT_FOUND:
                statusDescription = "STATUS_NOT_FOUND";
                break;
            case STATUS_INSUFFICIENT_RESOURCES:
                statusDescription = "STATUS_INSUFFICIENT_RESOURCES";
                break;
            case STATUS_NO_MEMORY:
                statusDescription = "STATUS_NO_MEMORY";
                break;
            case STATUS_OBJECT_NAME_NOT_FOUND:
                statusDescription = "STATUS_OBJECT_NAME_NOT_FOUND";
                break;
            case STATUS_OBJECT_NAME_COLLISION:
                statusDescription = "STATUS_OBJECT_NAME_COLLISION";
                break;
            case STATUS_TIMEOUT:
                statusDescription = "STATUS_TIMEOUT";
                break;
            default:
                statusDescription = "Unknown status code";
                break;
            }

            // Construct the log content with datetime, operation, status, and address range
            if (addressRange != NULL) {
                sprintf_s(logContent, sizeof(logContent), "[%S] - Operation: %S Status: 0x%08X (%s) AddressRange: %S \r\n", timeString, operation, status, statusDescription, addressRange);
            }
            else {
                sprintf_s(logContent, sizeof(logContent), "[%S] - Operation: %S Status: 0x%08X (%s) \r\n", timeString, operation, status, statusDescription);
            }

            // Now you can write the log content to the file
            res = ZwWriteFile(hFile, NULL, NULL, NULL, &io, logContent, (ULONG)strlen(logContent), NULL, NULL);
            if (!NT_SUCCESS(res)) {
                // Handle write failure
            }

            ZwClose(hFile);
        }
        else {
            // Handle file open failure
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER) {

    }
}



NTSTATUS RtlTimeToTimeString(const LARGE_INTEGER* SystemTime, WCHAR* Buffer, ULONG* ReturnLength)
{
    if (SystemTime == NULL || Buffer == NULL || ReturnLength == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

    TIME_FIELDS TimeFields;
    LARGE_INTEGER LocalTime;

    // Convert system time to local time
    ExSystemTimeToLocalTime(SystemTime, &LocalTime);

    // Extract time fields
    RtlTimeToTimeFields(&LocalTime, &TimeFields);

    // Format the time string
    *ReturnLength = swprintf(Buffer, L"%04u-%02u-%02u %02u:%02u:%02u:%03u",
        TimeFields.Year, TimeFields.Month, TimeFields.Day,
        TimeFields.Hour, TimeFields.Minute, TimeFields.Second,
        TimeFields.Milliseconds);

    return STATUS_SUCCESS;
}


VOID logValues(const WCHAR* operation, const WCHAR* value)
{
    UNICODE_STRING wsFinalPath;
    RtlInitUnicodeString(&wsFinalPath, L"\\DosDevices\\C:\\dummy_conversion_logs.txt");

    OBJECT_ATTRIBUTES oa;
    InitializeObjectAttributes(&oa, &wsFinalPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    IO_STATUS_BLOCK io;
    HANDLE hFile;

    auto res = ZwCreateFile(&hFile, FILE_APPEND_DATA, &oa, &io, NULL,
        FILE_ATTRIBUTE_NORMAL,
        0,
        FILE_OPEN, // Open the file only if it exists, don't create it
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);

    if (!NT_SUCCESS(res)) {
        return;
    }

    CHAR logContent[1024]; // Adjust size according to your needs
    sprintf_s(logContent, sizeof(logContent), "Operation: %S Value: %S \r\n", operation, value);

    // Now you can write the log content to the file
    res = ZwWriteFile(hFile, NULL, NULL, NULL, &io, logContent, (ULONG)strlen(logContent), NULL, NULL);
    if (!NT_SUCCESS(res)) {
        // Handle write failure
    }
    ZwClose(hFile);
}


VOID copy(HANDLE processSrcId, HANDLE processTrgtId, PVOID getFromAddress, PVOID putToAddress, SIZE_T size) {
    PEPROCESS processSource, processTarget;
    
    NTSTATUS srcStatus = PsLookupProcessByProcessId(processSrcId, &processSource);
    NTSTATUS trgtStatus = PsLookupProcessByProcessId(processTrgtId, &processTarget);
    
    if (STATUS_SUCCESS == srcStatus && STATUS_SUCCESS == trgtStatus) 
    {
        KCopyProcessMemory(processSource, processTarget, getFromAddress, putToAddress, size * sizeof(unsigned char));
    }
}
/*
VOID read(PEPROCESS processSource, PEPROCESS processTarget, PVOID getFromAddress, PVOID putToAddress, SIZE_T size) {
    KCopyProcessMemory(processSource, processTarget, getFromAddress, putToAddress, size);
}*/


VOID hardWrite(HANDLE ProcessId, PVOID address) {
    UCHAR NewValue[] = { 0xB8, 0x00, 0x05, 0x00, 0x00, 0x90, 0x90, 0x90 };

    NTSTATUS status = OverwriteMemory(ProcessId, address, NewValue, sizeof(NewValue));
    if (!NT_SUCCESS(status)) 
    {
        logToFile(L"OverwriteMemory Exception", status, NULL);
        status = STATUS_UNSUCCESSFUL; // Set a suitable status to indicate exception handling
    }
}



/* alternative 1 */

NTSTATUS OverwriteMemory(HANDLE ProcessId, PVOID VirtualAddress, PVOID NewValue, SIZE_T Size)
{
    NTSTATUS status = STATUS_SUCCESS;
    PEPROCESS TargetProcess = NULL;
    PVOID MappedAddress = NULL;
    PHYSICAL_ADDRESS PhysicalAddress;

    // Open a handle to the target process
    status = PsLookupProcessByProcessId(ProcessId, &TargetProcess);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    __try
    {
        // Attach to the target process
        KeAttachProcess(TargetProcess);

        // Step 1: Get the physical address corresponding to the virtual address
        PhysicalAddress = MmGetPhysicalAddress(VirtualAddress);

        // Step 2: Map the physical address into kernel space
        MappedAddress = MmMapIoSpace(PhysicalAddress, Size, MmNonCached);
        if (MappedAddress == NULL)
        {
            status = STATUS_UNSUCCESSFUL;
            __leave;
        }

        // Step 3: Perform DMA write operation
        // (Assuming DMA-capable hardware is properly configured)

        // Copy the new value to the mapped kernel virtual address
        RtlCopyMemory(MappedAddress, NewValue, Size);
    }
    __finally
    {
        // Step 4: Cleanup
        // Detach from the target process
        KeDetachProcess();

        if (MappedAddress)
        {
            MmUnmapIoSpace(MappedAddress, Size);
        }

        // Dereference the target process
        if (TargetProcess)
        {
            ObDereferenceObject(TargetProcess);
        }
    }

    return status;
}

NTSTATUS DevioctlDispatch(
    _In_ struct _DEVICE_OBJECT* DeviceObject,
    _Inout_ struct _IRP* Irp
)
{
    ULONG					bytesIO = 0;
    PIO_STACK_LOCATION		stack;
    NTSTATUS				status = STATUS_SUCCESS;
    BOOLEAN					condition = FALSE;
    PINOUTPARAM             rp;

    UNREFERENCED_PARAMETER(DeviceObject);

    stack = IoGetCurrentIrpStackLocation(Irp);

    do {
        if (stack == NULL) {
            status = STATUS_INTERNAL_ERROR;
            break;
        }

        rp = (PINOUTPARAM)Irp->AssociatedIrp.SystemBuffer;
        
        if (rp == NULL) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        switch (stack->Parameters.DeviceIoControl.IoControlCode) {
        case DUMMYDRV_REQUEST1:

            if (stack->Parameters.DeviceIoControl.InputBufferLength != sizeof(INOUT_PARAM)) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }
            if (rp->Param6 == 0) {
                copy((HANDLE)rp->Param1, (HANDLE)rp->Param3, (PVOID)rp->Param2, (PVOID)rp->Param4, rp->Param5);

                DbgPrintEx(DPFLTR_DEFAULT_ID,
                    DPFLTR_INFO_LEVEL,
                    "%s WRITE REQUEST: from pid- %d from - %p, to pid - %d to - %p, size - %d, isReady - %d",
                    __FUNCTION__,
                    rp->Param1,
                    rp->Param2,
                    rp->Param3,
                    rp->Param4,
                    rp->Param5,
                    rp->Param6);
            }
            if (rp->Param6 == 3) {
                DbgPrintEx(DPFLTR_DEFAULT_ID,
                    DPFLTR_INFO_LEVEL,
                    "%s PAGEPROT REQUEST: pid - %d, address - %p, size - %d, isReady - %d",
                    __FUNCTION__,
                    rp->Param1,
                    rp->Param2,
                    rp->Param3,
                    rp->Param6);

                VirtualProtectMem((HANDLE)rp->Param1, (PVOID)rp->Param2, (ULONGLONG) rp->Param3);
            }
            if (rp->Param6 == 4) {
                DbgPrintEx(DPFLTR_DEFAULT_ID,
                    DPFLTR_INFO_LEVEL,
                    "%s PAGEPROT REQUEST: pid - %d, address - %p, size - %d, isReady - %d",
                    __FUNCTION__,
                    rp->Param1,
                    rp->Param2,
                    rp->Param3,
                    rp->Param6);

                hardWrite((HANDLE)rp->Param1, (PVOID)rp->Param2);
            }
            /*
            if (rp->Param6 == 1) {
                DbgPrintEx(DPFLTR_DEFAULT_ID,
                    DPFLTR_INFO_LEVEL,
                    "%s READ REQUEST: from pid- %d from - %p, to pid - %d to - %p, size - %d, isReady - %d",
                    __FUNCTION__,
                    rp->Param1,
                    rp->Param2,
                    rp->Param3,
                    rp->Param4,
                    rp->Param5,
                    rp->Param6);

                HANDLE processSrcId = (HANDLE)rp->Param1;
                HANDLE processTrgtId = (HANDLE)rp->Param3;

                PEPROCESS processSource, processTarget;
                NTSTATUS srcStatus = PsLookupProcessByProcessId(processSrcId, &processSource);
                NTSTATUS trgtStatus = PsLookupProcessByProcessId(processTrgtId, &processTarget);

                if (STATUS_SUCCESS == srcStatus && STATUS_SUCCESS == trgtStatus) {
                    //~10mb = 18709120ULL
                    //using size as number of megabytes to read 
                    ULONGLONG step = 256ULL;
                    ULONGLONG numberOfBytesToCopy = (rp->Param5 * 1024ULL) * 1024ULL;
                    for (ULONGLONG i = 0ULL, address = 33554432ULL; i < numberOfBytesToCopy; i += step, address += step) {
                        read(processSource, processTarget, (PVOID) address, (PVOID) (rp->Param4 + i) , step);
                    }
                }

            }
            */ 

            status = STATUS_SUCCESS;
            bytesIO = sizeof(INOUT_PARAM);
            break;
        default:
            DbgPrintEx(DPFLTR_DEFAULT_ID,
                DPFLTR_INFO_LEVEL,
                "%s hit with invalid IoControlCode",
                __FUNCTION__);

            status = STATUS_INVALID_PARAMETER;
            break;
        };

    } while (condition);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = bytesIO;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS UnsupportedDispatch(
    _In_ struct _DEVICE_OBJECT* DeviceObject,
    _Inout_ struct _IRP* Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_NOT_SUPPORTED;
}

NTSTATUS CreateDispatch(
    _In_ struct _DEVICE_OBJECT* DeviceObject,
    _Inout_ struct _IRP* Irp
)
{
    NTSTATUS status = Irp->IoStatus.Status;
    UNREFERENCED_PARAMETER(DeviceObject);

    DbgPrintEx(DPFLTR_DEFAULT_ID,
        DPFLTR_INFO_LEVEL,
        "%s Create",
        __FUNCTION__);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS CloseDispatch(
    _In_ struct _DEVICE_OBJECT* DeviceObject,
    _Inout_ struct _IRP* Irp
)
{
    NTSTATUS status = Irp->IoStatus.Status;
    UNREFERENCED_PARAMETER(DeviceObject);

    DbgPrintEx(DPFLTR_DEFAULT_ID,
        DPFLTR_INFO_LEVEL,
        "%s Close",
        __FUNCTION__);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS DriverInitialize(
    _In_  struct _DRIVER_OBJECT* DriverObject,
    _In_  PUNICODE_STRING RegistryPath
)
{
    NTSTATUS        status;
    UNICODE_STRING  SymLink, DevName;
    PDEVICE_OBJECT  devobj;
    ULONG           t;

    UNREFERENCED_PARAMETER(RegistryPath);

    DbgPrintEx(DPFLTR_DEFAULT_ID,
        DPFLTR_INFO_LEVEL,
        "%s\n",
        __FUNCTION__);

    RtlInitUnicodeString(&DevName, L"\\Device\\PWN29");
    status = IoCreateDevice(DriverObject, 0, &DevName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, TRUE, &devobj);

    DbgPrintEx(DPFLTR_DEFAULT_ID,
        DPFLTR_INFO_LEVEL,
        "%s IoCreateDevice(%wZ) = %lx\n",
        __FUNCTION__,
        DevName,
        status);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    RtlInitUnicodeString(&SymLink, L"\\DosDevices\\PWN29");
    status = IoCreateSymbolicLink(&SymLink, &DevName);

    DbgPrintEx(DPFLTR_DEFAULT_ID,
        DPFLTR_INFO_LEVEL,
        "%s IoCreateSymbolicLink(%wZ) = %lx\n",
        __FUNCTION__,
        SymLink,
        status);

    devobj->Flags |= DO_BUFFERED_IO;

    for (t = 0; t <= IRP_MJ_MAXIMUM_FUNCTION; t++)
        DriverObject->MajorFunction[t] = &UnsupportedDispatch;

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &DevioctlDispatch;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = &CreateDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = &CloseDispatch;
    DriverObject->DriverUnload = NULL;

    devobj->Flags &= ~DO_DEVICE_INITIALIZING;
    return status;
}

NTSTATUS DriverEntry(
    _In_  struct _DRIVER_OBJECT* DriverObject,
    _In_  PUNICODE_STRING RegistryPath
)
{
    NTSTATUS        status;
    UNICODE_STRING  drvName;

    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    DbgPrintEx(DPFLTR_DEFAULT_ID,
        DPFLTR_INFO_LEVEL,
        "%s\n",
        __FUNCTION__);

    RtlInitUnicodeString(&drvName, L"\\Driver\\PWN29");
    status = IoCreateDriver(&drvName, &DriverInitialize);

    DbgPrintEx(DPFLTR_DEFAULT_ID,
        DPFLTR_INFO_LEVEL,
        "%s IoCreateDriver(%wZ) = %lx\n",
        __FUNCTION__,
        drvName,
        status);

    return status;
}
