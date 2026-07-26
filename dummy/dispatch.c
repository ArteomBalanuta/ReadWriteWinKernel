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

static NTSTATUS DispatchReadDriverOperation(
    _In_ const READDRIVER_REQUEST* request,
    _Inout_updates_bytes_opt_(sizeof(READDRIVER_REQUEST)) PVOID responseBuffer,
    _Inout_ PULONG_PTR bytesReturned,
    _In_ BOOLEAN silentMode
)
{
    if (bytesReturned != NULL) {
        *bytesReturned = 0;
    }

    /* Capture request fields before zeroing the shared SystemBuffer. */
    READDRIVER_REQUEST local;
    RtlCopyMemory(&local, request, sizeof(READDRIVER_REQUEST));

    if (responseBuffer != NULL) {
        RtlZeroMemory(responseBuffer, sizeof(READDRIVER_REQUEST));
    }

    switch ((READDRIVER_OPERATION)local.Operation) {
    case ReadDriverOperationCopy:
        return CopyProcessMemory(
            (HANDLE)local.SourceProcessId,
            (PVOID)(ULONG_PTR)local.SourceAddress,
            (HANDLE)local.TargetProcessId,
            (PVOID)(ULONG_PTR)local.TargetAddress,
            (SIZE_T)local.Size,
            silentMode);
    case ReadDriverOperationRead: {
        /* Read bytes from target process, return in response buffer.
           SourceProcessId = PID, SourceAddress = address, Size = byte count. */
        if (responseBuffer != NULL && local.Size > 0 && local.Size <= sizeof(READDRIVER_REQUEST)) {
            NTSTATUS status = ReadProcessMemory(
                (HANDLE)local.SourceProcessId,
                (PVOID)(ULONG_PTR)local.SourceAddress,
                (SIZE_T)local.Size,
                responseBuffer,
                silentMode);
            if (NT_SUCCESS(status)) {
                *bytesReturned = local.Size;
            }
            return status;
        }
        return STATUS_BUFFER_TOO_SMALL;
    }
    case ReadDriverOperationScan: {
        /* Scan memory range for byte pattern.
           SourceProcessId = PID, SourceAddress = start address,
           TargetProcessId = scan size, TargetAddress = pattern bytes (low 8 bytes),
           Size = pattern length (1-8). */
        ULONGLONG matchedAddress = 0;
        UCHAR pattern[8];
        ULONG patternSize = (ULONG)local.Size;

        if (patternSize == 0 || patternSize > sizeof(pattern)) {
            return STATUS_INVALID_PARAMETER;
        }

        RtlCopyMemory(pattern, &local.TargetAddress, patternSize);

        NTSTATUS status = ScanProcessMemory(
            (HANDLE)local.SourceProcessId,
            (PVOID)(ULONG_PTR)local.SourceAddress,
            (SIZE_T)local.TargetProcessId,
            pattern,
            patternSize,
            &matchedAddress,
            silentMode);

        if (responseBuffer != NULL) {
            RtlCopyMemory(responseBuffer, &matchedAddress, sizeof(ULONGLONG));
            *bytesReturned = sizeof(ULONGLONG);
        }
        return status;
    }
    case ReadDriverOperationProtect:
        return ProtectProcessMemory(
            (HANDLE)local.SourceProcessId,
            (PVOID)(ULONG_PTR)local.SourceAddress,
            (SIZE_T)local.TargetProcessId,
            silentMode);
    case ReadDriverOperationPatch:
        return PatchProcessMemory(
            (HANDLE)local.SourceProcessId,
            (PVOID)(ULONG_PTR)local.SourceAddress,
            silentMode);
    case ReadDriverOperationWriteInteger:
        /* Operation 5 treats SourceAddress as the integer value and TargetAddress as the destination. */
        return WriteIntegerProcessMemory(
            (HANDLE)local.SourceProcessId,
            (PVOID)(ULONG_PTR)local.TargetAddress,
            local.SourceAddress,
            (SIZE_T)local.Size,
            silentMode);
    case ReadDriverOperationBatchRead: {
        /* Batch read multiple addresses in one syscall.
           SourceProcessId = PID, SourceAddress = address 1,
           TargetProcessId = address 2, TargetAddress = address 3,
           Size = read size per address. Reads up to 5 addresses
           (3 from main fields + 2 from reserved if non-zero). */
        ULONGLONG addresses[5];
        ULONG count = 1;

        addresses[0] = local.SourceAddress;
        if (local.TargetProcessId != 0) {
            addresses[count++] = local.TargetProcessId;
        }
        if (local.TargetAddress != 0) {
            addresses[count++] = local.TargetAddress;
        }
        if (local.Reserved2 != 0) {
            addresses[count++] = local.Reserved2;
        }
        if (local.Reserved3 != 0) {
            addresses[count++] = local.Reserved3;
        }

        SIZE_T readSize = local.Size;
        if (readSize == 0 || readSize > sizeof(ULONGLONG)) {
            return STATUS_INVALID_PARAMETER;
        }

        if (responseBuffer != NULL) {
            NTSTATUS status = BatchReadProcessMemory(
                (HANDLE)local.SourceProcessId,
                addresses,
                count,
                readSize,
                responseBuffer,
                silentMode);
            if (NT_SUCCESS(status)) {
                *bytesReturned = count * readSize;
            }
            return status;
        }
        return STATUS_BUFFER_TOO_SMALL;
    }
    case ReadDriverOperationReadWithProtect: {
        /* Protect → read → restore in one syscall.
           SourceProcessId = PID, SourceAddress = address, Size = byte count. */
        if (responseBuffer != NULL && local.Size > 0 && local.Size <= sizeof(READDRIVER_REQUEST)) {
            ULONG originalProtection = 0;
            NTSTATUS status = ReadWithProtect(
                (HANDLE)local.SourceProcessId,
                (PVOID)(ULONG_PTR)local.SourceAddress,
                (SIZE_T)local.Size,
                responseBuffer,
                &originalProtection,
                silentMode);
            if (NT_SUCCESS(status)) {
                *bytesReturned = local.Size;
            }
            return status;
        }
        return STATUS_BUFFER_TOO_SMALL;
    }
    case ReadDriverOperationReadHex: {
        /* Read bytes and return as hex string.
           SourceProcessId = PID, SourceAddress = address, Size = byte count. */
        if (responseBuffer != NULL && local.Size > 0 && local.Size <= 256) {
            SIZE_T hexBufferSize = local.Size * 3 + 1;
            if (hexBufferSize > sizeof(READDRIVER_REQUEST)) {
                return STATUS_BUFFER_TOO_SMALL;
            }
            NTSTATUS status = ReadProcessMemoryAsHex(
                (HANDLE)local.SourceProcessId,
                (PVOID)(ULONG_PTR)local.SourceAddress,
                (SIZE_T)local.Size,
                (PCHAR)responseBuffer,
                hexBufferSize,
                silentMode);
            if (NT_SUCCESS(status)) {
                *bytesReturned = (ULONG)(local.Size * 3 + 1);
            }
            return status;
        }
        return STATUS_BUFFER_TOO_SMALL;
    }
    case ReadDriverOperationUnload: {
        /* This build cannot self-unload; remove the user-visible link instead. */
        return RemoveUserVisibleDeviceLink(silentMode);
    }
    case ReadDriverOperationFreezeWrite: {
        /* Freeze value at address(es), rewritten ~10x/sec by timer DPC.
           SourceProcessId = PID, SourceAddress = value,
           TargetAddress = address 1, TargetProcessId = address 2 (optional),
           Reserved2/3 = address 3/4 (optional), Size = integer width. */
        ULONGLONG addresses[4];
        ULONG count = 1;

        addresses[0] = local.TargetAddress;
        if (local.TargetProcessId != 0) {
            addresses[count++] = local.TargetProcessId;
        }
        if (local.Reserved2 != 0) {
            addresses[count++] = local.Reserved2;
        }
        if (local.Reserved3 != 0) {
            addresses[count++] = local.Reserved3;
        }

        SIZE_T writeSize = local.Size;
        if (writeSize != 1 && writeSize != 2 &&
            writeSize != 4 && writeSize != 8) {
            return STATUS_INVALID_PARAMETER;
        }
        if (writeSize < sizeof(ULONGLONG) &&
            (local.SourceAddress >> (writeSize * 8)) != 0) {
            return STATUS_INTEGER_OVERFLOW;
        }

        for (ULONG i = 0; i < count; ++i) {
            NTSTATUS status = AddFreezeSlot(
                (HANDLE)local.SourceProcessId,
                addresses[i],
                local.SourceAddress,
                writeSize);
            if (!NT_SUCCESS(status)) {
                return status;
            }
        }
        return STATUS_SUCCESS;
    }
    case ReadDriverOperationUnfreeze: {
        /* Clear freeze slot(s). SourceAddress = address (0 = clear all). */
        return ClearFreezeSlot(local.SourceAddress);
    }
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
    BOOLEAN silentMode;
    NTSTATUS status;

    if (bytesReturned == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    *bytesReturned = 0;

    status = ValidateDeviceControlRequest(stack, irp, &request);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    silentMode = (request->Flags & READDRIVER_FLAG_SILENT) != 0;

    if (IsDriverDecommissioned() &&
        request->Operation != ReadDriverOperationUnload) {
        return STATUS_DELETE_PENDING;
    }

    status = DispatchReadDriverOperation(request, irp->AssociatedIrp.SystemBuffer, bytesReturned, silentMode);

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
