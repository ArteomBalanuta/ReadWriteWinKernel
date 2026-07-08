# Protocol Improvement Proposals

Generated: 2026-07-08

Analysis of how the driver reads and writes process memory, with concrete protocol-level improvements to support writing specific values directly instead of always copying between two arbitrary addresses.

## Status Notes

### Addressed (first pass)

- documented the current op `5` field reuse more clearly in the shared header and docs
- documented that op `1` is declared but not currently dispatched
- documented that operation value `2` is intentionally unused in the current numbering
- aligned the C# mirror struct reserved-field naming with the native header
- clarified that the reserved tail is being kept intentionally for ABI stability

### Addressed (second pass)

- **BH-001**: Scan path replaced underflow-prone loop with `FindPatternOffset` helper + explicit bounds + overlap handling across chunk boundaries
- **BH-002**: C# client now serializes response struct as raw bytes via `MemoryMarshal.Write` (no more `Array.Copy` from `ulong[]`)
- **BH-003**: `ReadWithProtect` now restores original page protection on all error paths via `Cleanup` label
- **BH-004**: Silent flag threaded through each operation call (no shared global state)
- Response buffers zero-initialized in dispatch layer
- Tightened validation: null address and zero-size rejections across all ops
- `BatchReadProcessMemory` zeroes destination buffer, fails partial reads with `STATUS_BUFFER_OVERFLOW`
- `ReadProcessMemoryAsHex` buffer sizing includes null terminator
- Logging timestamp fields cast to `USHORT` for format string safety
- Scan buffer moved to non-paged pool (`ExAllocatePool2`)
- `client.Tests` regression test project added (2 tests: `GetResponseBytes`, `GetResponseString`)

### Not Addressed

- no arbitrary byte-array write
- no explicit protection-value expansion
- no full protocol redesign
- no transport-method changes

The remaining sections are preserved as proposals, not implementation status.

---

## Current Read/Write Model (5 operations)

| Op | Name                | What it does                                                     | Kernel primitive                  |
|----|---------------------|------------------------------------------------------------------|-----------------------------------|
| 0  | Copy                | SourceProcess@SrcAddr → TargetProcess@TgtAddr                   | `MmCopyVirtualMemory`             |
| 1  | Read                | Enum exists, no dispatch case wired up                           | —                                 |
| 3  | Protect             | Sets PAGE_EXECUTE_READWRITE on range in one process              | `ZwProtectVirtualMemory`          |
| 4  | Patch               | Injects hardcoded x64 bytes (`B8 00 05 00 00 90 90 90`) from driver space into target | Copy via `MmCopyVirtualMemory` |
| 5  | WriteInteger        | Writes single value to address using field-swapped semantics     | Stages buffer then copies         |

### How writes actually flow today

Every write operation funnels through **one path**: allocate a temporary buffer in driver space, fill it with the payload (constant bytes for Patch, pointer-casted value for WriteInteger), then copy via `MmCopyVirtualMemory`. The two-process hop is always present even when only injecting constant values from kernel address.

### Field semantics today (confusing)

| Field | Copy | Read | Protect | Patch | WriteInteger |
|-------|------|------|---------|-------|--------------|
| `SourceProcessId` | source PID | source PID | target PID | current process (ignored) | PID |
| `SourceAddress` | source address | source address | address | ignored | **integer value to write** |
| `TargetProcessId` | target PID | target PID | **size** | ignored | address |
| `TargetAddress` | target address | target address | ignored | address | ignored |
| `Size` | byte count | byte count | ignored | ignored | byte count |

The field reuse across operations makes the protocol brittle — `SourceAddress` means "read origin" for Copy but "value to inject" for WriteInteger.

---

## Protocol Gaps Identified

### 1. No read-back path to user mode

The Copy operation writes into target process memory but never returns data back through the IOCTL response buffer. Both clients discard it. There's no way for a client to "read X bytes from PID at address Y" and get those bytes printed or stored somewhere useful afterward — only silently copied elsewhere inside kernel space.

**Why:** Every operation is fire-and-forget. The response buffer allocated by the dispatch layer exists but goes unused because user-mode clients pass zero output size (native controller) or ignore the returned struct (C# client).

**Impact:** Users can't verify writes succeeded by reading the same address again immediately after issuing a command.

### 2. Op 1 (Read) is dead code

The enum value exists but there's no dispatch case for it. The client libraries have no read method. This is a half-baked feature.

**Status:** Still true. Only documentation was added; no new dispatch case was implemented.

### 3. WriteInteger field semantics are confusing

For WriteInteger, `SourceAddress` becomes the **value** to write and `TargetAddress` becomes the **destination address**, while `SourceProcessId` is the PID. This inversion is unintuitive and error-prone.

**Status:** Addressed at the documentation/contract level. The runtime behavior is unchanged, but the current field reuse is now explicitly documented in the shared header, docs, and C# mirror contract.

### 4. No arbitrary byte-array write

You can write a single integer value (1/2/4/8 bytes) or a hardcoded 8-byte patch, but there's no way to write an arbitrary sequence of bytes (e.g., a shellcode snippet, a string, a struct) to a target address. The Patch operation is hardcoded to `B8 00 05 00 00 90 90 90`.

### 5. No protection control beyond RWX

Protect always sets `PAGE_EXECUTE_READWRITE`. There's no way to set specific protections like `PAGE_READONLY`, `PAGE_READWRITE`, `PAGE_EXECUTE_READ`, etc.

**Status:** Not implemented.

### 6. Protocol wastes 48 bytes on reserved fields

The `READDRIVER_REQUEST` struct is 96 bytes (12 × ULONGLONG). Four of those fields (`Reserved1`–`Reserved5`, actually 5 reserved but one slot is missing from enum — see gap 7) are unused. That's 48 bytes of wasted space that could hold meaningful data.

### 7. Operation enum has a gap (skips 2)

Operations jump from 1 to 3. Op 2 is undefined. This leaves no room for a new operation without either renumbering (breaking compatibility) or using a higher number.

**Status:** The gap remains by design. Current code and docs now state explicitly that value `2` is intentionally unused to preserve protocol numbering.

---

## Ring 0 Stealth Reading Techniques

The driver runs in kernel mode (Ring 0) but currently exposes only five copy-based operations. These improvements leverage the driver's kernel position to reduce syscall noise, compress operations, and add inspection capabilities unavailable to user-mode clients.

### 1. Wire Op 1 (Read) with response data

**Status:** Not implemented.

**What:** Op 1 (`ReadDriverOperationRead`) exists in the enum but has no dispatch case. Every operation is fire-and-forget — the response buffer allocated by the dispatch layer goes unused.

**How:** Add a dispatch case that copies from target process memory into the `METHOD_BUFFERED` system buffer and returns the byte count. The client receives the read bytes directly from the IOCTL call.

**Why:** Foundation for all stealth reading. Without a working read path, the client cannot verify writes, inspect process state, or chain operations.

**Stealth value:** High — enables in-place inspection without extra round-trips.

### 2. Batch multi-address read

**Status:** Not implemented.

**What:** Each read currently requires one IOCTL = one syscall. The 40 bytes of reserved fields in the request struct can be repurposed as an inline address array.

**How:** Add a new operation that accepts multiple target addresses and reads all of them in a single kernel round-trip, returning concatenated results in the response buffer. The reserved fields become:

```c
UCHAR Payload[40];  // inline address list or read results
```

**Why:** Fewer syscalls = less trace noise. Monitoring `MmCopyVirtualMemory` calls becomes harder when five reads collapse into one.

**Stealth value:** Very high — classic anti-detection technique used in cheat engines.

### 3. Memory scan (pattern search)

**Status:** Not implemented.

**What:** Ring 0 can walk the page table of any target process. Add an operation that scans a memory range for a byte pattern and returns the first matching address.

**How:** New dispatch case that iterates over a memory range, reads pages in chunks, and compares against a supplied byte pattern. Returns the matching virtual address in the response buffer.

**Why:** This is how cheat engines find unknown values. The driver already has `MmCopyVirtualMemory` — it just needs a scan loop.

**Stealth value:** High — eliminates the read-compare loop in user mode, replacing N syscalls with 1.

### 4. Read-with-protect (combined operation)

**Status:** Not implemented.

**What:** Protect and read are two separate operations today. Combine them: protect a range, read it, then restore the original protection — all in one syscall.

**How:** New dispatch case that calls `ZwProtectVirtualMemory` → `MmCopyVirtualMemory` → `ZwProtectVirtualMemory` (restore) atomically. Returns the original protection value and the read bytes.

**Why:** Fewer operations = fewer kernel transitions = harder to detect. Also avoids leaving memory in RWX state between operations.

**Stealth value:** Medium — reduces the operation footprint from 2 syscalls to 1.

### 5. Silent mode flag

**Status:** Not implemented.

**What:** The driver logs every operation to `C:\dummy_logs.txt`. Add a mode where logging is suppressed.

**How:** Repurpose one reserved field as a flags bitmask. Bit 0 = silent mode. The dispatch path checks this flag before calling `LogStatus`.

**Why:** Stealth drivers shouldn't leave log files by default. The log file is a detectable artifact that reveals which operations were performed and when.

**Stealth value:** Medium — removes the most obvious detection vector (disk I/O).

### 6. Read-then-return-hex

**Status:** Not implemented.

**What:** The response buffer returns raw bytes. Add an operation that reads memory and returns a hex-formatted string.

**How:** New dispatch case that reads bytes and formats them as a hex string (`"48 65 6C 6C 6F"`) in the response buffer. Useful for quick inspection without post-processing in user mode.

**Why:** Clients skip the hex conversion step entirely. Reduces the client-side processing chain, keeping the conversion logic in kernel space where it's harder to trace.

**Stealth value:** Low — convenience feature, but keeps the formatting logic out of the client process.

### 7. Fill the enum gap (Op 2)

**Status:** Not implemented. The current repo intentionally keeps the numbering gap for compatibility.

**What:** Operations jump from 1 to 3. Op 2 is undefined.

**How:** Either assign Op 2 to a new operation (e.g., memory scan) or renumber the enum to be contiguous.

**Why:** Leaving gaps wastes protocol space and makes future additions fragile. A contiguous enum is easier to reason about and leaves room for higher-numbered extensions.

**Stealth value:** Low — protocol hygiene, but prevents future breakage.

---

## Proposed Protocol Redesign

### New struct layout (64 bytes, 8 × ULONGLONG)

```c
typedef enum _READDRIVER_OPERATION {
    ReadDriverOperationRead      = 0,  // Read bytes from process → response buffer
    ReadDriverOperationWrite     = 1,  // Write arbitrary bytes to process
    ReadDriverOperationProtect   = 2,  // Change memory protection
    ReadDriverOperationCopy      = 3,  // Copy between processes (existing)
    ReadDriverOperationPatch     = 4,  // Hardcoded patch (existing, keep for compat)
    ReadDriverOperationWriteInt  = 5,  // Write integer value (existing, keep for compat)
    ReadDriverOperationReadBytes = 6,  // Read N bytes, return in response
    ReadDriverOperationWriteBytes= 7,  // Write N bytes from user buffer
} READDRIVER_OPERATION;

typedef struct _READDRIVER_REQUEST {
    ULONGLONG Operation;      // Which operation to perform
    ULONGLONG ProcessId;      // Target process (single field, not source/target)
    ULONGLONG Address;        // Target virtual address
    ULONGLONG Size;           // Byte count
    ULONGLONG Value;          // Integer value (for WriteInt) or byte array pointer (for WriteBytes)
    ULONGLONG Protection;     // New page protection (for Protect)
    ULONGLONG SourceProcessId;// Source PID (for Copy only)
    ULONGLONG SourceAddress;  // Source address (for Copy only)
} READDRIVER_REQUEST, *PREADDRIVER_REQUEST;
```

### Operation semantics (clean)

| Op | Name | Key fields | Returns |
|----|------|-----------|---------|
| 0  | Read | `ProcessId`, `Address`, `Size` | bytes in IOCTL response |
| 1  | Write | `ProcessId`, `Address`, `Size`, `Value` (pointer to buffer in user mode) | nothing |
| 2  | Protect | `ProcessId`, `Address`, `Size`, `Protection` | nothing |
| 3  | Copy | `SourceProcessId`, `SourceAddress`, `ProcessId` (target), `Size` | nothing |
| 4  | Patch | `ProcessId`, `Address` | nothing (hardcoded, kept for compat) |
| 5  | WriteInt | `ProcessId`, `Address`, `Size`, `Value` | nothing |
| 6  | ReadBytes | `ProcessId`, `Address`, `Size` | bytes in IOCTL response |
| 7  | WriteBytes | `ProcessId`, `Address`, `Size`, `Value` (pointer to user buffer) | nothing |

### Key design decisions

1. **Single `ProcessId` field** — eliminates the source/target confusion. Copy is the only operation that needs a second PID.
2. **Read returns data** — use `METHOD_BUFFERED` response buffer. The kernel fills the same buffer the user provided, so the client reads back from the IOCTL return value.
3. **`Value` field repurposed** — for WriteInt it holds the integer; for WriteBytes it holds a user-mode pointer to the byte array (requires `METHOD_NEITHER` or an additional IOCTL for the buffer).
4. **`Protection` field** — explicit page protection for Protect operation instead of always RWX.
5. **Compact 64-byte struct** — frees up 32 bytes, removes all reserved fields.

**Status:** Not adopted. The repo still uses the existing fixed-width request layout.

### Read-back implementation sketch

```c
case ReadDriverOperationRead:
case ReadDriverOperationReadBytes: {
    PEPROCESS process;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)request->ProcessId, &process);
    if (!NT_SUCCESS(status)) return status;

    SIZE_T bytesCopied = 0;
    __try {
        status = MmCopyVirtualMemory(
            process,
            (PVOID)request->Address,
            PsGetCurrentProcess(),
            irp->AssociatedIrp.SystemBuffer,  // <-- fill response buffer
            min(request->Size, sizeof(*request)),
            KernelMode,
            &bytesCopied);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    ObDereferenceObject(process);
    if (NT_SUCCESS(status)) {
        *bytesReturned = (ULONG)bytesCopied;
    }
    return status;
}
```

### Write-bytes from user buffer

This is the trickier one. `METHOD_BUFFERED` limits payload to one page. For larger writes:

**Option A** — inline within buffer limit: pack bytes directly into the request struct by replacing reserved fields with a `BYTE Payload[48]` array. Keeps `METHOD_BUFFERED`.

```c
typedef struct _READDRIVER_REQUEST {
    ULONGLONG Operation;
    ULONGLONG ProcessId;
    ULONGLONG Address;
    ULONGLONG Size;
    ULONGLONG Value;
    ULONGLONG Protection;
    ULONGLONG SourceProcessId;
    UCHAR     Payload[64];  // inline byte data for WriteBytes
} READDRIVER_REQUEST, *PREADDRIVER_REQUEST;
```

**Option B** — switch to `METHOD_NEITHER` for the WriteBytes operation, passing a user-mode pointer in `Value`. Requires `ProbeForRead` in kernel mode. More flexible but slightly more complex.

---

## Implementation Priority

| Priority | Change | Effort | Impact | Stealth Value |
|----------|--------|--------|--------|---------------|
| P0 | Wire up Op 1 (Read) with response data | Low | Enables read-back, foundational | High — all reading depends on this |
| P0 | Fill enum gap (Op 2) | Trivial | Protocol hygiene | Low — prevents future breakage |
| P0 | Fix WriteInteger field semantics | Low | Bug fix, removes confusion | Low — correctness |
| P1 | Silent mode flag | Low | Remove log artifact | Medium — removes disk I/O trace |
| P1 | Batch multi-address read | Medium | Fewer syscalls | Very high — compresses N reads to 1 |
| P1 | Memory scan (pattern search) | Medium | Find values by bytes | High — core cheat-engine capability |
| P1 | Read-with-protect (combined) | Low | Atomic protect+read | Medium — fewer kernel transitions |
| P1 | Add inline byte-array write (Payload field) | Medium | Enables arbitrary writes | Low — convenience |
| P1 | Add explicit protection values | Low | More granular memory control | Low — correctness |
| P1 | Read-then-return-hex | Low | Hex-formatted response | Low — convenience |
| P2 | Full protocol struct redesign | Medium | Long-term maintainability | Low — structural |
| P2 | Add WriteBytes with user buffer pointer | Medium | Larger payload writes | Low — convenience |

---

## Client-Side Changes Required

### Native controller (`controller/controller/`)
- Add `read_process()` function that sends IOCTL with output buffer
- Add `scan_memory()` function for pattern search
- Add `batch_read()` function for multi-address reads
- Update CLI argument parser for new operation syntax
- Update transport layer to handle response data
- Pass silent-mode flag when logging is not needed

### C# client (`client/`)
- Add `ReadAsync(pid, address, size)` method returning `byte[]`
- Add `ScanAsync(pid, address, size, pattern)` method returning matching address
- Add `BatchReadAsync(pid, addresses)` method returning dictionary of results
- Add `WriteBytesAsync(pid, address, bytes)` method
- Update `DriverApi.cs` struct to match new layout
- Update CLI and GUI to support new operations

### Shared header (`common/protocol.h`)
- Redefine `READDRIVER_REQUEST` struct
- Update enum values (fill Op 2 gap, add scan/batch ops)
- Add documentation comments for each field per operation
- Add flags field for silent mode and future extensions

