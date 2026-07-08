# Bug Hunt Report

Date: 2026-07-08
Scope: `common/`, `dummy/`, `controller/controller/`, `client/`
Excluded: `_inspect_wdk_zip/`, `tmp_wdk_vsix/`, `tmp_wdk_build_vsix/`, `vm_extract/`, build outputs and vendor-like unpacked artifacts.

## Findings

### 1. Kernel scan loop can underflow and read past `scanBuffer`
- Severity: High
- Files: `dummy/memory_ops.c:296`
- Summary: `ScanProcessMemory` iterates with `for (SIZE_T i = 0; i <= bytesCopied - patternSize; ++i)` without first checking `bytesCopied >= patternSize`.
- Impact: When the copied chunk is smaller than the requested pattern, `bytesCopied - patternSize` underflows to a very large `SIZE_T`, and the loop walks past the end of the 4 KB stack buffer. In kernel mode this can turn a short scan request into invalid memory reads or a crash.
- Repro shape: Any scan where the final chunk size is smaller than the pattern length, or where `size < patternSize`.

### 2. C# client response decoding throws for read-style responses
- Severity: Medium
- Files: `client/DriverApi.cs:39`, `client/Program.cs:115`, `client/Program.cs:125`, `client/Program.cs:136`
- Summary: `GetResponseBytes` uses `Array.Copy` from a `ulong[]` into a `byte[]`.
- Impact: Read-related CLI paths call this helper and can fail at runtime instead of printing the returned data. A direct runtime check in this session confirmed `ArrayTypeMismatchException` and `ArgumentException` depending on the requested byte count.
- Repro shape: Any successful response path that calls `GetResponseBytes`, including `Read`, `ReadWithProtect`, `BatchRead`, and the default hex dump branch.

### 3. `ReadWithProtect` can leave target pages writable/executable on an error path
- Severity: Medium
- Files: `dummy/memory_ops.c:396`, `dummy/memory_ops.c:415`
- Summary: `ReadWithProtect` changes the target region protection to `PAGE_EXECUTE_READWRITE`, then returns early if `PsLookupProcessByProcessId` fails.
- Impact: On that path, the function closes the handle and exits without restoring the original protection. A failed read can therefore leave the target memory region permanently more permissive than intended until something else changes it back.
- Repro shape: Any case where `ZwProtectVirtualMemory` succeeds but `PsLookupProcessByProcessId` fails afterward.

### 4. Silent-mode logging flag is shared globally across concurrent requests
- Severity: Low
- Files: `dummy/logging.c:8`, `dummy/logging.c:10`, `dummy/dispatch.c:220`
- Summary: The driver stores per-request logging intent in a single global `g_isSilentMode` boolean.
- Impact: Concurrent IOCTLs can race and change each other’s logging behavior. One request can unintentionally suppress or enable logs for another, which makes debugging and audit trails unreliable under parallel use.
- Repro shape: Two overlapping requests with different `READDRIVER_FLAG_SILENT` values.

## Notes

- `client/client.csproj` builds successfully with `dotnet build client\\client.csproj -nologo`.
- I treated this as a scan-only pass and did not modify project source files.
