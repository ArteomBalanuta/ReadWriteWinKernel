#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>
#include "main.h"

#ifndef PROCESS_VM_OPERATION
#define PROCESS_VM_OPERATION ((ACCESS_MASK)0x0008)
#endif

#ifndef READDRIVER_VERBOSE_LOG
#define READDRIVER_VERBOSE_LOG 0
#endif

#define READDRIVER_SCAN_BUFFER_SIZE 4096

/* Freeze timer fires ~10 times/second (100 ms interval). */
#define READDRIVER_FREEZE_INTERVAL_MS 100

typedef struct _READDRIVER_FREEZE_SLOT {
    BOOLEAN active;
    HANDLE processId;
    ULONG_PTR address;
    ULONGLONG value;
    SIZE_T size;
} READDRIVER_FREEZE_SLOT;

static READDRIVER_FREEZE_SLOT g_freezeSlots[READDRIVER_MAX_FREEZE_SLOTS];
static KSPIN_LOCK g_freezeLock;
static KTIMER g_freezeTimer;
static KDPC g_freezeDpc;
static BOOLEAN g_freezeInitialized;

static VOID FreezeTimerDpc(
    _In_ PRKDPC Dpc,
    _In_opt_ PVOID DeferredContext,
    _In_opt_ PVOID SystemArgument1,
    _In_opt_ PVOID SystemArgument2
);

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

static VOID LogMemoryOpStatus(
    _In_ PCWSTR operation,
    _In_ NTSTATUS status,
    _In_opt_ PCWSTR detail,
    _In_ BOOLEAN silentMode
)
{
    if (!NT_SUCCESS(status) || READDRIVER_VERBOSE_LOG) {
        LogStatus(operation, status, detail, silentMode);
    }
}

NTSTATUS CopyProcessMemory(
    _In_ HANDLE sourceProcessId,
    _In_ PVOID sourceAddress,
    _In_ HANDLE targetProcessId,
    _In_ PVOID targetAddress,
    _In_ SIZE_T size,
    _In_ BOOLEAN silentMode
)
{
    PEPROCESS sourceProcess;
    PEPROCESS targetProcess;
    SIZE_T bytesCopied = 0;
    NTSTATUS status;

    if (sourceAddress == NULL || targetAddress == NULL || size == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    status = LookupProcessPair(sourceProcessId, targetProcessId, &sourceProcess, &targetProcess);
    if (!NT_SUCCESS(status)) {
        LogMemoryOpStatus(L"PsLookupProcessByProcessId", status, L"failed to resolve process pair", silentMode);
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

    if (NT_SUCCESS(status) && bytesCopied != size) {
        status = STATUS_BUFFER_OVERFLOW;
    }

    LogMemoryOpStatus(L"MmCopyVirtualMemory", status, NULL, silentMode);
    return status;
}

NTSTATUS ProtectProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ SIZE_T size,
    _In_ BOOLEAN silentMode
)
{
    OBJECT_ATTRIBUTES attributes;
    CLIENT_ID clientId;
    HANDLE processHandle = NULL;
    PVOID baseAddress = address;
    SIZE_T regionSize = size;
    ULONG oldProtection = 0;
    NTSTATUS status;

    if (address == NULL || size == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    InitializeObjectAttributes(&attributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
    clientId.UniqueProcess = processId;
    clientId.UniqueThread = NULL;

    status = ZwOpenProcess(&processHandle, PROCESS_VM_OPERATION, &attributes, &clientId);
    if (!NT_SUCCESS(status)) {
        LogMemoryOpStatus(L"ZwOpenProcess", status, NULL, silentMode);
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
    LogMemoryOpStatus(L"ZwProtectVirtualMemory", status, NULL, silentMode);
    return status;
}

NTSTATUS PatchProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ BOOLEAN silentMode
)
{
    /* x64: mov eax, 0x500 followed by NOP padding to overwrite 8 bytes predictably. */
    UCHAR patchBytes[] = { 0xB8, 0x00, 0x05, 0x00, 0x00, 0x90, 0x90, 0x90 };

    return CopyProcessMemory(
        PsGetCurrentProcessId(),
        patchBytes,
        processId,
        address,
        sizeof(patchBytes),
        silentMode);
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
    _In_ SIZE_T size,
    _In_ BOOLEAN silentMode
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
        size,
        silentMode);
}

NTSTATUS ReadProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ SIZE_T size,
    _Out_writes_(size) PVOID buffer,
    _In_ BOOLEAN silentMode
)
{
    PEPROCESS process;
    SIZE_T bytesCopied = 0;
    NTSTATUS status;

    if (address == NULL || buffer == NULL || size == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    status = PsLookupProcessByProcessId(processId, &process);
    if (!NT_SUCCESS(status)) {
        LogMemoryOpStatus(L"PsLookupProcessByProcessId", status, L"failed to resolve process", silentMode);
        return status;
    }

    __try {
        status = MmCopyVirtualMemory(
            process,
            address,
            PsGetCurrentProcess(),
            buffer,
            size,
            KernelMode,
            &bytesCopied);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    ObDereferenceObject(process);

    if (NT_SUCCESS(status) && bytesCopied != size) {
        status = STATUS_BUFFER_OVERFLOW;
    }

    LogMemoryOpStatus(L"MmCopyVirtualMemory (Read)", status, NULL, silentMode);
    return status;
}

_Success_(return != FALSE)
static BOOLEAN FindPatternOffset(
    _In_reads_(bufferSize) const UCHAR* buffer,
    _In_ SIZE_T bufferSize,
    _In_reads_(patternSize) const UCHAR* pattern,
    _In_ SIZE_T patternSize,
    _Out_ SIZE_T* matchedOffset
)
{
    if (buffer == NULL || pattern == NULL || matchedOffset == NULL || patternSize == 0 || bufferSize < patternSize) {
        return FALSE;
    }

    *matchedOffset = 0;

    for (SIZE_T index = 0; index + patternSize <= bufferSize; ++index) {
        if (RtlCompareMemory(&buffer[index], pattern, patternSize) == patternSize) {
            *matchedOffset = index;
            return TRUE;
        }
    }

    return FALSE;
}

NTSTATUS ScanProcessMemory(
    _In_ HANDLE processId,
    _In_ PVOID startAddress,
    _In_ SIZE_T size,
    _In_reads_(patternSize) const UCHAR* pattern,
    _In_ ULONG patternSize,
    _Out_ PULONGLONG matchedAddress,
    _In_ BOOLEAN silentMode
)
{
    PEPROCESS process;
    UCHAR* scanBuffer = NULL;
    SIZE_T page;
    SIZE_T bytesCopied = 0;
    NTSTATUS status;

    if (startAddress == NULL || pattern == NULL || patternSize == 0 || size == 0 || matchedAddress == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    *matchedAddress = 0;

    status = PsLookupProcessByProcessId(processId, &process);
    if (!NT_SUCCESS(status)) {
        LogMemoryOpStatus(L"PsLookupProcessByProcessId", status, L"failed to resolve process", silentMode);
        return status;
    }

    scanBuffer = (UCHAR*)ExAllocatePool2(POOL_FLAG_NON_PAGED, READDRIVER_SCAN_BUFFER_SIZE, 'ncsR');
    if (scanBuffer == NULL) {
        ObDereferenceObject(process);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

 page = 0;
    status = STATUS_NOT_FOUND;

    while (page < size) {
        SIZE_T chunkBase = page;
        SIZE_T copySize;
        SIZE_T stepSize = size - page;
        SIZE_T matchedOffset = 0;
        SIZE_T overlap = 0;

        if (stepSize > READDRIVER_SCAN_BUFFER_SIZE) {
            stepSize = READDRIVER_SCAN_BUFFER_SIZE;
        }

        if (page > 0 && patternSize > 1) {
            overlap = patternSize - 1;
            if (overlap > page) {
                overlap = page;
            }
            chunkBase -= overlap;
        }

        copySize = size - chunkBase;
        if (copySize > READDRIVER_SCAN_BUFFER_SIZE) {
            copySize = READDRIVER_SCAN_BUFFER_SIZE;
        }

        bytesCopied = 0;
        __try {
            status = MmCopyVirtualMemory(
                process,
                (PVOID)((ULONG_PTR)startAddress + chunkBase),
                PsGetCurrentProcess(),
                scanBuffer,
                copySize,
                KernelMode,
                &bytesCopied);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
            break;
        }

        if (!NT_SUCCESS(status)) {
            break;
        }

        if (bytesCopied < patternSize) {
            page += (copySize > bytesCopied) ? bytesCopied : 1;
            continue;
        }

        if (FindPatternOffset(scanBuffer, bytesCopied, pattern, patternSize, &matchedOffset)) {
            *matchedAddress = (ULONGLONG)((ULONG_PTR)startAddress + chunkBase + matchedOffset);
            status = STATUS_SUCCESS;
            ExFreePoolWithTag(scanBuffer, 'ncsR');
            ObDereferenceObject(process);
            LogMemoryOpStatus(L"ScanProcessMemory", status, L"pattern matched", silentMode);
            return status;
        }

        page += stepSize;
    }

    ExFreePoolWithTag(scanBuffer, 'ncsR');
    ObDereferenceObject(process);
    LogMemoryOpStatus(L"ScanProcessMemory", status, L"pattern not found", silentMode);
    return status;
}

NTSTATUS BatchReadProcessMemory(
    _In_ HANDLE processId,
    _In_reads_(count) const ULONGLONG* addresses,
    _In_ ULONG count,
    _In_ SIZE_T readSize,
    _Out_writes_(count * readSize) PVOID buffer,
    _In_ BOOLEAN silentMode
)
{
    PEPROCESS process;
    NTSTATUS status;

    if (addresses == NULL || buffer == NULL || count == 0 || readSize == 0 || readSize > sizeof(ULONGLONG)) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory(buffer, count * readSize);

    status = PsLookupProcessByProcessId(processId, &process);
    if (!NT_SUCCESS(status)) {
        LogMemoryOpStatus(L"PsLookupProcessByProcessId", status, L"failed to resolve process", silentMode);
        return status;
    }

    for (ULONG i = 0; i < count; ++i) {
        SIZE_T bytesCopied = 0;
        __try {
            status = MmCopyVirtualMemory(
                process,
                (PVOID)addresses[i],
                PsGetCurrentProcess(),
                (PVOID)((ULONG_PTR)buffer + i * readSize),
                readSize,
                KernelMode,
                &bytesCopied);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
            break;
        }

        if (!NT_SUCCESS(status)) {
            break;
        }

        if (bytesCopied != readSize) {
            status = STATUS_BUFFER_OVERFLOW;
            break;
        }
    }

    ObDereferenceObject(process);
    LogMemoryOpStatus(L"BatchReadProcessMemory", status, NULL, silentMode);
    return status;
}

NTSTATUS ReadWithProtect(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ SIZE_T size,
    _Out_writes_(size) PVOID buffer,
    _Out_opt_ PULONG originalProtection,
    _In_ BOOLEAN silentMode
)
{
    PEPROCESS process;
    SIZE_T bytesCopied = 0;
    NTSTATUS status;

    if (address == NULL || buffer == NULL || size == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    if (originalProtection != NULL) {
        *originalProtection = 0;
    }

    status = PsLookupProcessByProcessId(processId, &process);
    if (!NT_SUCCESS(status)) {
        LogMemoryOpStatus(L"PsLookupProcessByProcessId", status, L"failed to resolve process", silentMode);
        return status;
    }

    __try {
        status = MmCopyVirtualMemory(
            process,
            address,
            PsGetCurrentProcess(),
            buffer,
            size,
            KernelMode,
            &bytesCopied);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    ObDereferenceObject(process);

    if (NT_SUCCESS(status) && bytesCopied != size) {
        status = STATUS_BUFFER_OVERFLOW;
    }

    LogMemoryOpStatus(L"ReadWithProtect", status, NULL, silentMode);
    return status;
}

NTSTATUS ReadProcessMemoryAsHex(
    _In_ HANDLE processId,
    _In_ PVOID address,
    _In_ SIZE_T size,
    _Out_writes_(hexBufferSize) PCHAR hexBuffer,
    _In_ SIZE_T hexBufferSize,
    _In_ BOOLEAN silentMode
)
{
    UCHAR rawBuffer[256];
    NTSTATUS status;
    SIZE_T copySize;
    SIZE_T requiredSize;

    if (address == NULL || hexBuffer == NULL || size == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    copySize = size;
    if (copySize > sizeof(rawBuffer)) {
        copySize = sizeof(rawBuffer);
    }

    status = ReadProcessMemory(processId, address, copySize, rawBuffer, silentMode);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    requiredSize = (copySize * 3) + 1;
    if (hexBufferSize < requiredSize) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    __analysis_assume(hexBufferSize >= requiredSize);

    for (SIZE_T i = 0; i < copySize; ++i) {
        static const CHAR hexChars[] = "0123456789ABCDEF";
        hexBuffer[i * 3]     = hexChars[(rawBuffer[i] >> 4) & 0x0F];
        hexBuffer[i * 3 + 1] = hexChars[rawBuffer[i] & 0x0F];
        hexBuffer[i * 3 + 2] = ' ';
    }

    hexBuffer[requiredSize - 1] = '\0';

    return STATUS_SUCCESS;
}

static VOID FreezeTimerDpc(
    _In_ PRKDPC Dpc,
    _In_opt_ PVOID DeferredContext,
    _In_opt_ PVOID SystemArgument1,
    _In_opt_ PVOID SystemArgument2
)
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    /* Rewrite all active freeze slots. */
    for (ULONG i = 0; i < READDRIVER_MAX_FREEZE_SLOTS; ++i) {
        if (!g_freezeSlots[i].active)
            continue;

        WriteIntegerProcessMemory(
            g_freezeSlots[i].processId,
            (PVOID)g_freezeSlots[i].address,
            g_freezeSlots[i].value,
            g_freezeSlots[i].size,
            TRUE);
    }

    /* Re-arm the timer. */
    LARGE_INTEGER interval;
    interval.QuadPart = -(LONG)(READDRIVER_FREEZE_INTERVAL_MS * 10000);
    KeSetTimer(&g_freezeTimer, interval, Dpc);
}

NTSTATUS InitializeFreezeTimer(VOID)
{
    RtlZeroMemory(g_freezeSlots, sizeof(g_freezeSlots));
    KeInitializeSpinLock(&g_freezeLock);
    KeInitializeTimer(&g_freezeTimer);
    KeInitializeDpc(&g_freezeDpc, FreezeTimerDpc, NULL);

    LARGE_INTEGER interval;
    interval.QuadPart = -(LONG)(READDRIVER_FREEZE_INTERVAL_MS * 10000);
    KeSetTimer(&g_freezeTimer, interval, &g_freezeDpc);

    g_freezeInitialized = TRUE;
    return STATUS_SUCCESS;
}

VOID StopFreezeTimer(VOID)
{
    KIRQL oldIrql;

    KeAcquireSpinLock(&g_freezeLock, &oldIrql);
    RtlZeroMemory(g_freezeSlots, sizeof(g_freezeSlots));
    g_freezeInitialized = FALSE;
    KeSetTimer(&g_freezeTimer, (LARGE_INTEGER){0}, &g_freezeDpc);
    KeReleaseSpinLock(&g_freezeLock, oldIrql);
}

NTSTATUS AddFreezeSlot(
    _In_ HANDLE processId,
    _In_ ULONG_PTR address,
    _In_ ULONGLONG value,
    _In_ SIZE_T size
)
{
    KIRQL oldIrql;

    if (!IsSupportedIntegerSize(size)) {
        return STATUS_INVALID_PARAMETER;
    }

    KeAcquireSpinLock(&g_freezeLock, &oldIrql);

    /* Check if slot for this address already exists, update in-place. */
    for (ULONG i = 0; i < READDRIVER_MAX_FREEZE_SLOTS; ++i) {
        if (g_freezeSlots[i].active && g_freezeSlots[i].address == address) {
            g_freezeSlots[i].value = value;
            g_freezeSlots[i].size = size;
            g_freezeSlots[i].processId = processId;
            KeReleaseSpinLock(&g_freezeLock, oldIrql);
            return STATUS_SUCCESS;
        }
    }

    /* Find a free slot. */
    for (ULONG i = 0; i < READDRIVER_MAX_FREEZE_SLOTS; ++i) {
        if (!g_freezeSlots[i].active) {
            g_freezeSlots[i].active = TRUE;
            g_freezeSlots[i].processId = processId;
            g_freezeSlots[i].address = address;
            g_freezeSlots[i].value = value;
            g_freezeSlots[i].size = size;
            KeReleaseSpinLock(&g_freezeLock, oldIrql);
            return STATUS_SUCCESS;
        }
    }

    KeReleaseSpinLock(&g_freezeLock, oldIrql);
    return STATUS_INSUFFICIENT_RESOURCES;
}

NTSTATUS ClearFreezeSlot(
    _In_ ULONG_PTR address
)
{
    KIRQL oldIrql;

    KeAcquireSpinLock(&g_freezeLock, &oldIrql);

    if (address == 0) {
        /* address 0 = clear all. */
        RtlZeroMemory(g_freezeSlots, sizeof(g_freezeSlots));
    }
    else {
        for (ULONG i = 0; i < READDRIVER_MAX_FREEZE_SLOTS; ++i) {
            if (g_freezeSlots[i].active && g_freezeSlots[i].address == address) {
                g_freezeSlots[i].active = FALSE;
                g_freezeSlots[i].processId = NULL;
                KeReleaseSpinLock(&g_freezeLock, oldIrql);
                return STATUS_SUCCESS;
            }
        }
    }

    KeReleaseSpinLock(&g_freezeLock, oldIrql);
    return STATUS_SUCCESS;
}
