#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>
#include "main.h"


PVOID ExAllocatePoolWithTag(
    __drv_strictTypeMatch(__drv_typeExpr)POOL_TYPE PoolType,
    SIZE_T                                         NumberOfBytes,
    ULONG                                          Tag
);

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

NTSTATUS KeReadProcessMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size) {
    PEPROCESS SourceProcess = Process;
    PEPROCESS TargetProcess = PsGetCurrentProcess();
    SIZE_T Result;
    if (NT_SUCCESS(MmCopyVirtualMemory(SourceProcess, SourceAddress, TargetProcess, TargetAddress, Size, KernelMode, &Result)))
        return STATUS_SUCCESS;
    else
        return STATUS_ACCESS_DENIED;
}

NTSTATUS KeWriteProcessMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size) {
    PEPROCESS SourceProcess = PsGetCurrentProcess();
    PEPROCESS TargetProcess = Process;
    SIZE_T Result;

    if (NT_SUCCESS(MmCopyVirtualMemory(SourceProcess, SourceAddress, TargetProcess, TargetAddress, Size, KernelMode, &Result)))
        return STATUS_SUCCESS;
    else
        return STATUS_ACCESS_DENIED;
}

VOID read(HANDLE processId, PVOID targetAddress, PVOID bufferAddress, SIZE_T size) {
    PEPROCESS process;
    PVOID buffer = ExAllocatePoolWithTag(NonPagedPool, 20, '0');

    PsLookupProcessByProcessId(processId, &process);
    KeReadProcessMemory(process, targetAddress, buffer, size * sizeof(char));

    DbgPrintEx(DPFLTR_DEFAULT_ID,
        DPFLTR_INFO_LEVEL,
        "Read: %s",
        buffer,
        __FUNCTION__);
}

VOID readWriteProcess() {
    
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

            DbgPrintEx(DPFLTR_DEFAULT_ID,
                DPFLTR_INFO_LEVEL,
                "%s requst: pid - %d, address - %p, bufferAdr - %p, size - %d",
                __FUNCTION__,
                rp->Param1,
                rp->Param2,
                rp->Param3,
                rp->Param4);

            read((HANDLE)rp->Param1, (PVOID)rp->Param2, (PVOID)rp->Param3, rp->Param4);

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

    RtlInitUnicodeString(&DevName, L"\\Device\\TDLD2");
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

    RtlInitUnicodeString(&SymLink, L"\\DosDevices\\TDLD2");
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

    RtlInitUnicodeString(&drvName, L"\\Driver\\TDLD2");
    status = IoCreateDriver(&drvName, &DriverInitialize);

    DbgPrintEx(DPFLTR_DEFAULT_ID,
        DPFLTR_INFO_LEVEL,
        "%s IoCreateDriver(%wZ) = %lx\n",
        __FUNCTION__,
        drvName,
        status);

    return status;
}
