# Driver Second Pass

Date: 2026-07-08
Scope: `dummy/` only
Focus: correctness and error handling

## Fixed bugs carried into this pass

- `BH-001`: the scan path no longer uses the underflow-prone `bytesCopied - patternSize` loop. It now scans with explicit bounds and overlap handling across chunk boundaries.
- `BH-003`: `ReadWithProtect` now restores the original page protection on all post-protect error paths and can surface restore failures when they are the only failure.
- `BH-004`: the request-level silent flag is no longer stored in shared global driver state. Logging behavior is now threaded through each operation call.

## Client-side fix that shipped with the same patch set

- `BH-002`: the C# client now serializes the response struct as raw bytes instead of trying to `Array.Copy` from `ulong[]` into `byte[]`.
- Added a new regression test project, `client.Tests`, covering:
  - `GetResponseBytes`
  - `GetResponseString`

## What I changed in the second pass

- Added explicit zero-initialization for driver response buffers and `bytesReturned` in the dispatch layer.
- Tightened validation in memory operations:
  - `CopyProcessMemory` now rejects null addresses and zero-length copies.
  - `ProtectProcessMemory`, `ReadProcessMemory`, `ReadWithProtect`, and `ReadProcessMemoryAsHex` now reject null addresses up front.
  - `BatchReadProcessMemory` now zeroes the destination buffer and fails partial reads with `STATUS_BUFFER_OVERFLOW`.
- Fixed `ReadProcessMemoryAsHex` buffer sizing to require room for the null terminator.
- Cast timestamp fields in logging to unsigned widths expected by the format string.
- Moved the scan buffer to nonpaged pool, which removes the large per-call kernel stack allocation.

## Verification

- `dotnet test client.Tests\client.Tests.csproj -nologo --no-restore`
  - Result: success, 2 tests passed
- `B:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe controller\controller\controller.vcxproj /p:Configuration=Debug /p:Platform=x64 /nologo`
  - Result: success
- `B:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe dummy\dummy.vcxproj /t:Rebuild /p:Configuration=Debug /p:Platform=x64 /p:RunCodeAnalysis=true /nologo`
  - Result: success

## Residual warnings after the second pass

- `dummy/driver_lifecycle.c`
  - PREfast still warns about the generic `UnsupportedDispatch` annotation and direct `MajorFunction` assignments. This is structural/annotation noise around the dispatch table setup rather than a new functional regression from this patch set.
- `dummy/memory_ops.c`
  - PREfast still warns on the broad `__except (EXCEPTION_EXECUTE_HANDLER)` filters. Those are real style concerns, but changing them safely would be a broader error-handling refactor rather than a surgical bug fix.
  - PREfast still reports a possible `hexBuffer` overrun in `ReadProcessMemoryAsHex`. The code now performs an explicit `requiredSize` check before writing and passes native builds, so this appears to be analyzer conservatism rather than an observed runtime break.

## Left to do if we want a deeper cleanup

- Replace the remaining broad SEH handlers with narrower exception strategy where the WDK/API contract makes that practical.
- Revisit dispatch registration annotations in `driver_lifecycle.c` to reduce PREfast noise around `UnsupportedDispatch` and `MajorFunction` assignment.
- If desired, add a small native/unit-test seam for some pure helper logic in the driver so more of the correctness rules can be regression-tested outside full driver loading.

## Bottom line

The verified runtime bugs from the first pass are fixed. The remaining driver findings are mostly analyzer warnings about broad SEH and dispatch-table annotations, plus one conservative buffer warning that does not reproduce as a build failure after the bounds fix.
