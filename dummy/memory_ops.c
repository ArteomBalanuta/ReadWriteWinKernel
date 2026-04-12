#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>
#include "main.h"

#ifndef PROCESS_VM_OPERATION
#define PROCESS_VM_OPERATION ((ACCESS_MASK)0x0008)
#endif

NTSYSAPI NTSTATUS NTAPI ZwProtectVirtualMemory(
    _In_ HANDLE ProcessHandle,
    _Inout_ PVOID* BaseAddress,
    _Inout_ PSIZE_T RegionSize,
    _In_ ULONG NewProtect,
    _Out_ PULONG OldProtect
);

NTKERNELAPI NTSTATUS NTAPI MmCopyVirtualMemory(
    _In_ PEPROCESS FromProcess,
    _In_ PVOID FromAddress,
    _In_ PEPROCESS ToProcess,
    _Out_ PVOID ToAddress,
    _In_ SIZE_T BufferSize,
    _In_ KPROCESSOR_MODE PreviousMode,
    _Out_ PSIZE_T NumberOfBytesCopied
);

static NTSTATUS LookupProcessPair(
    HANDLE sourceProcessId,
    HANDLE targetProcessId,
    PEPROCESS* sourceProcess,
    PEPROCESS* targetProcess
)
{
    NTSTATUS status;

    *sourceProcess = NULL;
    *targetProcess = NULL;

    status = PsLookupProcessByProcessId(sourceProcessId, sourceProcess);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = PsLookupProcessByProcessId(targetProcessId, targetProcess);
    if (!NT_SUCCESS(status)) {
        ObDereferenceObject(*sourceProcess);
        *sourceProcess = NULL;
        return status;
    }

    return STATUS_SUCCESS;
}

NTSTATUS CopyProcessMemory(
    _In_ HANDLE sourceProcessId,
    _In_ PVOID sourceAddress,
    _In_ HANDLE targetProcessId,
    _In_ PVOID targetAddress,
    _In_ SIZE_T size
)
{
    PEPROCESS sourceProcess;
    PEPROCESS targetProcess;
    SIZE_T bytesCopied = 0;
    NTSTATUS status;

    status = LookupProcessPair(sourceProcessId, targetProcessId, &sourceProcess, &targetProcess);
    if (!NT_SUCCESS(status)) {
        LogStatus(L"PsLookupProcessByProcessId", status, L"failed to resolve process pair");
        return status;
    }

    __try {
        status = MmCopyVirtualMemory(
            sourceProcess,
            sourceAddress,
            targetProcess,
            targetAddress,
            size,
            KernelMode,
            &bytesCopied);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    ObDereferenceObject(sourceProcess);
    ObDereferenceObject(targetProcess);

    LogStatus(L"MmCopyVirtualMemory", status, NULL);
    return status;
}

NTSTATUS ProtectProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ SIZE_T size
)
{
    OBJECT_ATTRIBUTES attributes;
    CLIENT_ID clientId;
    HANDLE processHandle = NULL;
    PVOID baseAddress = address;
    SIZE_T regionSize = size;
    ULONG oldProtection = 0;
    NTSTATUS status;

    InitializeObjectAttributes(&attributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
    clientId.UniqueProcess = processId;
    clientId.UniqueThread = NULL;

    status = ZwOpenProcess(&processHandle, PROCESS_VM_OPERATION, &attributes, &clientId);
    if (!NT_SUCCESS(status)) {
        LogStatus(L"ZwOpenProcess", status, NULL);
        return status;
    }

    __try {
        status = ZwProtectVirtualMemory(
            processHandle,
            &baseAddress,
            &regionSize,
            PAGE_EXECUTE_READWRITE,
            &oldProtection);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    ZwClose(processHandle);
    LogStatus(L"ZwProtectVirtualMemory", status, NULL);
    return status;
}

NTSTATUS PatchProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address
)
{
    UCHAR patchBytes[] = { 0xB8, 0x00, 0x05, 0x00, 0x00, 0x90, 0x90, 0x90 };

    return CopyProcessMemory(
        PsGetCurrentProcessId(),
        patchBytes,
        processId,
        address,
        sizeof(patchBytes));
}

static BOOLEAN IsSupportedIntegerSize(_In_ SIZE_T size)
{
    return size == sizeof(UCHAR) ||
        size == sizeof(USHORT) ||
        size == sizeof(ULONG) ||
        size == sizeof(ULONGLONG);
}

NTSTATUS WriteIntegerProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ ULONGLONG value,
    _In_ SIZE_T size
)
{
    ULONGLONG valueBuffer;

    if (address == NULL || !IsSupportedIntegerSize(size)) {
        return STATUS_INVALID_PARAMETER;
    }

    valueBuffer = value;
    return CopyProcessMemory(
        PsGetCurrentProcessId(),
        &valueBuffer,
        processId,
        address,
        size);
}
