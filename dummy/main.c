#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>
#include "main.h"


PVOID ExAllocatePoolWithTag(
    __drv_strictTypeMatch(__drv_typeExpr)POOL_TYPE PoolType,
    SIZE_T                                         NumberOfBytes,
    ULONG                                          Tag
);


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

VOID logToFile(const WCHAR* operation, NTSTATUS status, const WCHAR* addressRange);

NTSTATUS GetProcessValidAddressSpace(HANDLE ProcessId, PVOID* StartAddress, PVOID* EndAddress)
{
    PEPROCESS Process = NULL;
    NTSTATUS Status = PsLookupProcessByProcessId(ProcessId, &Process);
    if (!NT_SUCCESS(Status))
    {
        // Failed to lookup process
        return Status;
    }

    // Get the process address space
    KAPC_STATE ApcState;
    KeStackAttachProcess(Process, &ApcState);

    // Use MmGetVirtualForPhysical to get the start and end valid addresses
    PHYSICAL_ADDRESS physicalStart = MmGetPhysicalAddress((PVOID)0);
    PHYSICAL_ADDRESS physicalEnd = MmGetPhysicalAddress((PVOID)~(0ULL));

    *StartAddress = MmGetVirtualForPhysical(physicalStart);
    *EndAddress = MmGetVirtualForPhysical(physicalEnd);

    // Restore APC state
    KeUnstackDetachProcess(&ApcState);

    // Dereference the process
    ObDereferenceObject(Process);

    return STATUS_SUCCESS;
}

NTSTATUS VirtualProtectMem(HANDLE pid, PVOID address, ULONG pageSize);

NTSTATUS VirtualProtectMem(HANDLE ProcessId, PVOID address, ULONG pageSize)
{
    NTSTATUS status = STATUS_SUCCESS;
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
        PVOID StartAddress, EndAddress;
        GetProcessValidAddressSpace(ProcessId, &StartAddress, &EndAddress);
        if (address < StartAddress || address > EndAddress) {
            WCHAR addressRange[85];
            swprintf(addressRange, L"%p - %p, requested: %p", StartAddress, EndAddress, address);
            logToFile(L"Invalid Address ", STATUS_INVALID_PARAMETER, addressRange);
            ZwClose(hProcess);
            return STATUS_INVALID_PARAMETER;
        }

        ULONG curProtection = NULL; // variable to store the previous page protection
        SIZE_T size = pageSize;

        // Attempting to change the page protection
        status = ZwProtectVirtualMemory(hProcess, address, &size, PAGE_EXECUTE_READWRITE, &curProtection);

        ZwClose(hProcess);

        if (!NT_SUCCESS(status)) {
            logToFile(L"ZwProtectVirtualMemory", status, NULL);
        }
        else {
            logToFile(L"ZwProtectVirtualMemory", status, NULL);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        // Log exception using logToFile
        logToFile(L"Exception in VirtualProtectMem", GetExceptionCode(), NULL);
        status = STATUS_UNSUCCESSFUL; // Set a suitable status to indicate exception handling
    }

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
        RtlInitUnicodeString(&wsFinalPath, L"\\DosDevices\\D:\\dummy_logs.txt");

        OBJECT_ATTRIBUTES oa;
        InitializeObjectAttributes(&oa, &wsFinalPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

        IO_STATUS_BLOCK io;
        HANDLE hFile;

        auto res = ZwCreateFile(&hFile, GENERIC_WRITE, &oa, &io, NULL,
            FILE_ATTRIBUTE_NORMAL,
            0,
            FILE_OPEN_IF,
            FILE_SYNCHRONOUS_IO_NONALERT,
            NULL, 0);

        if (NT_SUCCESS(res)) {
            // Move file pointer to the end of the file
            LARGE_INTEGER li;
            li.QuadPart = 0;
            res = ZwSetInformationFile(hFile, &io, &li, sizeof(LARGE_INTEGER), FileEndOfFileInformation);

            if (NT_SUCCESS(res)) {
                // Construct log content with operation and status information
                CHAR logContent[256]; // Adjust size according to your needs

                // Get current system time
                LARGE_INTEGER systemTime;
                KeQuerySystemTime(&systemTime);

                // Format system time as a string
                WCHAR timeString[70];
                UNICODE_STRING usTimeString;
                RtlTimeToTimeFields(&systemTime, (PTIME_FIELDS)&timeString);
                RtlInitUnicodeString(&usTimeString, timeString);

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
                    sprintf_s(logContent, sizeof(logContent), "[%S] - Operation: %S Status: 0x%08X (%s) AddressRange: %S\n", timeString, operation, status, statusDescription, addressRange);
                }
                else {
                    sprintf_s(logContent, sizeof(logContent), "[%S] - Operation: %S Status: 0x%08X (%s)\n", timeString, operation, status, statusDescription);
                }

                // Now you can write the log content to the file
                ULONG bytesWritten;
                res = ZwWriteFile(hFile, NULL, NULL, NULL, &io, logContent, (ULONG)strlen(logContent), NULL, NULL);
                if (!NT_SUCCESS(res)) {
                    // Handle write failure
                }
            }
            else {
                // Handle file pointer positioning failure
            }

            ZwClose(hFile);
        }
        else {
            // Handle file creation failure
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER) {

    }
}




VOID copy(HANDLE processSrcId, HANDLE processTrgtId, PVOID getFromAddress, PVOID putToAddress, SIZE_T size) {
    PEPROCESS processSource, processTarget;
    
    NTSTATUS srcStatus = PsLookupProcessByProcessId(processSrcId, &processSource);
    NTSTATUS trgtStatus = PsLookupProcessByProcessId(processTrgtId, &processTarget);
    
    if (STATUS_SUCCESS == srcStatus && STATUS_SUCCESS == trgtStatus) {
        KCopyProcessMemory(processSource, processTarget, getFromAddress, putToAddress, size * sizeof(unsigned char));
    }
}
/*
VOID read(PEPROCESS processSource, PEPROCESS processTarget, PVOID getFromAddress, PVOID putToAddress, SIZE_T size) {
    KCopyProcessMemory(processSource, processTarget, getFromAddress, putToAddress, size);
}*/


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

            //copy((HANDLE)rp->Param1, (HANDLE)rp->Param3, (PVOID)rp->Param2, (PVOID)rp->Param4, rp->Param5);
            /* 0 - copy specific part 
               1 - dump all the memory
               2 - change page access lvl
            */
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

                VirtualProtectMem((HANDLE)rp->Param1, (PVOID)rp->Param2, rp->Param3);
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

    RtlInitUnicodeString(&DevName, L"\\Device\\PWN17");
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

    RtlInitUnicodeString(&SymLink, L"\\DosDevices\\PWN17");
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

    RtlInitUnicodeString(&drvName, L"\\Driver\\PWN17");
    status = IoCreateDriver(&drvName, &DriverInitialize);

    DbgPrintEx(DPFLTR_DEFAULT_ID,
        DPFLTR_INFO_LEVEL,
        "%s IoCreateDriver(%wZ) = %lx\n",
        __FUNCTION__,
        drvName,
        status);

    return status;
}
