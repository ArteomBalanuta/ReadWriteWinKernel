#include <ntifs.h>
#include <ntddk.h>
#include "main.h"

static NTSTATUS CompleteIrp(PIRP irp, NTSTATUS status, ULONG_PTR information)
{
    irp->IoStatus.Status = status;
    irp->IoStatus.Information = information;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return status;
}

static NTSTATUS ValidateDeviceControlRequest(
    _In_ PIO_STACK_LOCATION stack,
    _In_ PIRP irp,
    _Outptr_ PREADDRIVER_REQUEST* request
)
{
    if (stack == NULL || irp == NULL || request == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (stack->Parameters.DeviceIoControl.IoControlCode != READDRIVER_IOCTL_REQUEST) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(READDRIVER_REQUEST)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    *request = (PREADDRIVER_REQUEST)irp->AssociatedIrp.SystemBuffer;
    if (*request == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

static NTSTATUS DispatchReadDriverOperation(_In_ const READDRIVER_REQUEST* request)
{
    switch ((READDRIVER_OPERATION)request->Operation) {
    case ReadDriverOperationCopy:
        return CopyProcessMemory(
            (HANDLE)request->SourceProcessId,
            (PVOID)(ULONG_PTR)request->SourceAddress,
            (HANDLE)request->TargetProcessId,
            (PVOID)(ULONG_PTR)request->TargetAddress,
            (SIZE_T)request->Size);
    case ReadDriverOperationProtect:
        return ProtectProcessMemory(
            (HANDLE)request->SourceProcessId,
            (PVOID)(ULONG_PTR)request->SourceAddress,
            (SIZE_T)request->TargetProcessId);
    case ReadDriverOperationPatch:
        return PatchProcessMemory(
            (HANDLE)request->SourceProcessId,
            (PVOID)(ULONG_PTR)request->SourceAddress);
    case ReadDriverOperationWriteInteger:
        return WriteIntegerProcessMemory(
            (HANDLE)request->SourceProcessId,
            (PVOID)(ULONG_PTR)request->TargetAddress,
            request->SourceAddress,
            (SIZE_T)request->Size);
    default:
        return STATUS_NOT_SUPPORTED;
    }
}

NTSTATUS HandleDeviceControl(
    _In_ PIO_STACK_LOCATION stack,
    _Inout_ PIRP irp,
    _Out_ PULONG_PTR bytesReturned
)
{
    PREADDRIVER_REQUEST request;
    NTSTATUS status;

    if (bytesReturned == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    *bytesReturned = 0;

    status = ValidateDeviceControlRequest(stack, irp, &request);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = DispatchReadDriverOperation(request);
    if (NT_SUCCESS(status)) {
        *bytesReturned = sizeof(*request);
    }

    return status;
}

NTSTATUS DevioctlDispatch(
    _In_ struct _DEVICE_OBJECT* DeviceObject,
    _Inout_ struct _IRP* Irp
)
{
    PIO_STACK_LOCATION stack;
    ULONG_PTR bytesReturned = 0;
    NTSTATUS status;

    UNREFERENCED_PARAMETER(DeviceObject);

    stack = IoGetCurrentIrpStackLocation(Irp);
    status = HandleDeviceControl(stack, Irp, &bytesReturned);
    return CompleteIrp(Irp, status, bytesReturned);
}

NTSTATUS UnsupportedDispatch(
    _In_ struct _DEVICE_OBJECT* DeviceObject,
    _Inout_ struct _IRP* Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    return CompleteIrp(Irp, STATUS_NOT_SUPPORTED, 0);
}

NTSTATUS CreateDispatch(
    _In_ struct _DEVICE_OBJECT* DeviceObject,
    _Inout_ struct _IRP* Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    return CompleteIrp(Irp, STATUS_SUCCESS, 0);
}

NTSTATUS CloseDispatch(
    _In_ struct _DEVICE_OBJECT* DeviceObject,
    _Inout_ struct _IRP* Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    return CompleteIrp(Irp, STATUS_SUCCESS, 0);
}
