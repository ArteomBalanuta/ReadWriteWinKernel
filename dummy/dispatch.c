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

    if (responseBuffer != NULL) {
        RtlZeroMemory(responseBuffer, sizeof(READDRIVER_REQUEST));
    }

    switch ((READDRIVER_OPERATION)request->Operation) {
    case ReadDriverOperationCopy:
        return CopyProcessMemory(
            (HANDLE)request->SourceProcessId,
            (PVOID)(ULONG_PTR)request->SourceAddress,
            (HANDLE)request->TargetProcessId,
            (PVOID)(ULONG_PTR)request->TargetAddress,
            (SIZE_T)request->Size,
            silentMode);
    case ReadDriverOperationRead: {
        /* Read bytes from target process, return in response buffer.
           SourceProcessId = PID, SourceAddress = address, Size = byte count. */
        if (responseBuffer != NULL && request->Size > 0 && request->Size <= sizeof(READDRIVER_REQUEST)) {
            NTSTATUS status = ReadProcessMemory(
                (HANDLE)request->SourceProcessId,
                (PVOID)(ULONG_PTR)request->SourceAddress,
                (SIZE_T)request->Size,
                responseBuffer,
                silentMode);
            if (NT_SUCCESS(status)) {
                *bytesReturned = request->Size;
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
        ULONG patternSize = (ULONG)request->Size;

        if (patternSize == 0 || patternSize > sizeof(pattern)) {
            return STATUS_INVALID_PARAMETER;
        }

        RtlCopyMemory(pattern, &request->TargetAddress, patternSize);

        NTSTATUS status = ScanProcessMemory(
            (HANDLE)request->SourceProcessId,
            (PVOID)(ULONG_PTR)request->SourceAddress,
            (SIZE_T)request->TargetProcessId,
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
            (HANDLE)request->SourceProcessId,
            (PVOID)(ULONG_PTR)request->SourceAddress,
            (SIZE_T)request->TargetProcessId,
            silentMode);
    case ReadDriverOperationPatch:
        return PatchProcessMemory(
            (HANDLE)request->SourceProcessId,
            (PVOID)(ULONG_PTR)request->SourceAddress,
            silentMode);
    case ReadDriverOperationWriteInteger:
        /* Operation 5 treats SourceAddress as the integer value and TargetAddress as the destination. */
        return WriteIntegerProcessMemory(
            (HANDLE)request->SourceProcessId,
            (PVOID)(ULONG_PTR)request->TargetAddress,
            request->SourceAddress,
            (SIZE_T)request->Size,
            silentMode);
    case ReadDriverOperationBatchRead: {
        /* Batch read multiple addresses in one syscall.
           SourceProcessId = PID, SourceAddress = address 1,
           TargetProcessId = address 2, TargetAddress = address 3,
           Size = read size per address. Reads up to 5 addresses
           (3 from main fields + 2 from reserved if non-zero). */
        ULONGLONG addresses[5];
        ULONG count = 1;

        addresses[0] = request->SourceAddress;
        if (request->TargetProcessId != 0) {
            addresses[count++] = request->TargetProcessId;
        }
        if (request->TargetAddress != 0) {
            addresses[count++] = request->TargetAddress;
        }
        if (request->Reserved2 != 0) {
            addresses[count++] = request->Reserved2;
        }
        if (request->Reserved3 != 0) {
            addresses[count++] = request->Reserved3;
        }

        SIZE_T readSize = request->Size;
        if (readSize == 0 || readSize > sizeof(ULONGLONG)) {
            return STATUS_INVALID_PARAMETER;
        }

        if (responseBuffer != NULL) {
            NTSTATUS status = BatchReadProcessMemory(
                (HANDLE)request->SourceProcessId,
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
        if (responseBuffer != NULL && request->Size > 0 && request->Size <= sizeof(READDRIVER_REQUEST)) {
            ULONG originalProtection = 0;
            NTSTATUS status = ReadWithProtect(
                (HANDLE)request->SourceProcessId,
                (PVOID)(ULONG_PTR)request->SourceAddress,
                (SIZE_T)request->Size,
                responseBuffer,
                &originalProtection,
                silentMode);
            if (NT_SUCCESS(status)) {
                *bytesReturned = request->Size;
            }
            return status;
        }
        return STATUS_BUFFER_TOO_SMALL;
    }
    case ReadDriverOperationReadHex: {
        /* Read bytes and return as hex string.
           SourceProcessId = PID, SourceAddress = address, Size = byte count. */
        if (responseBuffer != NULL && request->Size > 0 && request->Size <= 256) {
            SIZE_T hexBufferSize = request->Size * 3 + 1;
            if (hexBufferSize > sizeof(READDRIVER_REQUEST)) {
                return STATUS_BUFFER_TOO_SMALL;
            }
            NTSTATUS status = ReadProcessMemoryAsHex(
                (HANDLE)request->SourceProcessId,
                (PVOID)(ULONG_PTR)request->SourceAddress,
                (SIZE_T)request->Size,
                (PCHAR)responseBuffer,
                hexBufferSize,
                silentMode);
            if (NT_SUCCESS(status)) {
                *bytesReturned = (ULONG)(request->Size * 3 + 1);
            }
            return status;
        }
        return STATUS_BUFFER_TOO_SMALL;
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
