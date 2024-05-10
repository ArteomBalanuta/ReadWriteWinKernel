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

VOID writeFailFile(BOOLEAN status);

NTSTATUS KCopyProcessMemory(PEPROCESS srcProcess, PEPROCESS trgtProcess, PVOID sourceAddress, PVOID targetAddress, SIZE_T size) {
    PEPROCESS sourceProcess = srcProcess;
    PEPROCESS targetProcess = trgtProcess;
    SIZE_T Result;
    if (NT_SUCCESS(MmCopyVirtualMemory(sourceProcess, sourceAddress, targetProcess, targetAddress, size, KernelMode, &Result))) {
        writeFailFile(TRUE);
        return STATUS_SUCCESS;
    } else {
        writeFailFile(FALSE);
        return STATUS_ACCESS_DENIED;
    }
}

VOID writeFailFile(BOOLEAN status) {
        UNICODE_STRING wsFinalPath;
        if (status == TRUE) {
            RtlInitUnicodeString(&wsFinalPath, L"\\DosDevices\\D:\\true.txt"); //L"\\SystemRoot\\drvhinf.txt");  // or L"\\SystemRoot\\example.txt" \\DosDevices\\C:\\WINDOWS\\example.txt
        } else {
            RtlInitUnicodeString(&wsFinalPath, L"\\DosDevices\\D:\\fail.txt"); //L"\\SystemRoot\\drvhinf.txt");  // or L"\\SystemRoot\\example.txt" \\DosDevices\\C:\\WINDOWS\\example.txt
        }

        OBJECT_ATTRIBUTES oa;
        InitializeObjectAttributes(&oa, &wsFinalPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

        IO_STATUS_BLOCK io;
        HANDLE hFile;

        auto res = ZwCreateFile(&hFile, GENERIC_WRITE, &oa, &io, NULL,
        FILE_ATTRIBUTE_NORMAL,
        0,
        FILE_OVERWRITE_IF,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);

        ZwClose(hFile);
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

    RtlInitUnicodeString(&DevName, L"\\Device\\PWN4");
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

    RtlInitUnicodeString(&SymLink, L"\\DosDevices\\PWN4");
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

    RtlInitUnicodeString(&drvName, L"\\Driver\\PWN4");
    status = IoCreateDriver(&drvName, &DriverInitialize);

    DbgPrintEx(DPFLTR_DEFAULT_ID,
        DPFLTR_INFO_LEVEL,
        "%s IoCreateDriver(%wZ) = %lx\n",
        __FUNCTION__,
        drvName,
        status);

    return status;
}
