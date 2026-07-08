# Cleanup Findings Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Apply the cleanup items from `CLEAN_UP.md` without changing the repo's functional scope.

**Architecture:** Keep the existing module boundaries intact while removing dead code, centralizing shared constants, tightening request/documentation semantics, and simplifying driver/controller plumbing. Limit changes to small, reviewable edits that preserve the current request flow and build layout.

**Tech Stack:** C, KMDF/WDK, Win32, C#/.NET 8, PowerShell, MSBuild 18

## Global Constraints

- Preserve the current root solution structure and output paths.
- Do not introduce new runtime features while performing cleanup.
- Treat `common/protocol.h` as a shared contract and update docs when semantics change.
- Prefer the root build script `./build-all.ps1` for final verification.
- Keep edits focused on the files called out by `CLEAN_UP.md`.

---

### Task 1: Shared contract cleanup

**Files:**
- Modify: `G:\workspace\projects\readDriver\common\protocol.h`
- Modify: `G:\workspace\projects\readDriver\dummy\main.c`

**Interfaces:**
- Consumes: Existing device and IOCTL naming in `common/protocol.h`
- Produces: Shared driver-name constant and documented reserved-field intent

- [ ] Add a shared `READDRIVER_DRIVER_NAME` macro to `common/protocol.h`.
- [ ] Replace the hardcoded `L"\\Driver\\PWN29"` literal in `dummy/main.c` with the shared macro.
- [ ] Replace the anonymous reserved tail fields in `READDRIVER_REQUEST` with clearly named reserved slots and a short comment explaining ABI padding intent.

### Task 2: Driver cleanup

**Files:**
- Modify: `G:\workspace\projects\readDriver\dummy\dispatch.c`
- Modify: `G:\workspace\projects\readDriver\dummy\memory_ops.c`
- Modify: `G:\workspace\projects\readDriver\dummy\logging.c`
- Modify: `G:\workspace\projects\readDriver\dummy\dummy.vcxproj`
- Delete: `G:\workspace\projects\readDriver\dummy\r3request.c`

**Interfaces:**
- Consumes: `READDRIVER_REQUEST`, driver logging helpers, current copy/protect/patch/write helpers
- Produces: Clearer operation-5 semantics, quieter success logging, broader status labels, no orphaned driver source file

- [ ] Delete the uncompiled `dummy/r3request.c` file.
- [ ] Add a short comment in `dispatch.c` explaining that operation `5` reuses `SourceAddress` as the integer value and `TargetAddress` as the write destination.
- [ ] Add a short comment in `memory_ops.c` explaining the patch byte sequence.
- [ ] Gate routine success-path logging in `memory_ops.c` behind a compile-time `READDRIVER_VERBOSE_LOG` flag while preserving failure logging.
- [ ] Expand `DescribeStatus` in `logging.c` with a few common NTSTATUS values used in this code path.

### Task 3: Native controller cleanup

**Files:**
- Modify: `G:\workspace\projects\readDriver\controller\controller\cli.c`
- Modify: `G:\workspace\projects\readDriver\controller\controller\transport.c`

**Interfaces:**
- Consumes: `READDRIVER_REQUEST`, current controller CLI forms
- Produces: Cleaner argument parsing constants and slimmer transport wrapper

- [ ] Introduce named constants for the two supported CLI argument counts in `cli.c`.
- [ ] Deduplicate the parse-failure text in `cli.c`.
- [ ] Remove the redundant `TargetProcessId = 0` assignment in the 5-argument form after `ZeroMemory`.
- [ ] Remove the unused response buffer from `transport.c` and send a write-only request with `lpOutBuffer = NULL` and `nOutBufferSize = 0`.

### Task 4: Docs and verification

**Files:**
- Modify: `G:\workspace\projects\readDriver\docs\ARCHITECTURE.md`
- Modify: `G:\workspace\projects\readDriver\controller\README.md`

**Interfaces:**
- Consumes: Current operation semantics and cleanup decisions
- Produces: Updated docs that match code behavior

- [ ] Document the operation `5` field reuse in `docs/ARCHITECTURE.md`.
- [ ] Update any controller-facing wording that still implies the field mapping is obvious without the operation-specific note.
- [ ] Run `./build-all.ps1 -Configuration Debug -Platform x64`.
- [ ] Confirm the build succeeds before reporting completion.
