# Safe Protocol Clarity Pass Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Clarify the current protocol contract and docs without changing runtime behavior or adding new memory-access capabilities.

**Architecture:** Keep the existing IOCTL contract and operation numbering intact, then improve maintainability at the boundaries where developers consume that contract: the shared C header, the C# mirror struct, and the protocol docs. This is a compatibility-safe cleanup, not a feature expansion.

**Tech Stack:** C, C#/.NET 8, KMDF/WDK, PowerShell, MSBuild 18

## Global Constraints

- Preserve the current wire layout of `READDRIVER_REQUEST`.
- Do not add new operations, read-back paths, batch requests, scanning, stealth behavior, or transport changes.
- Do not renumber existing operation values.
- Keep `IMPROVE.md` untouched.
- Verify with `./build-all.ps1 -Configuration Debug -Platform x64`.

---

### Task 1: Clarify the shared native contract

**Files:**
- Modify: `G:\workspace\projects\readDriver\common\protocol.h`

**Interfaces:**
- Consumes: Existing `READDRIVER_OPERATION` and `READDRIVER_REQUEST`
- Produces: Clear native comments about operation numbering, fixed payload intent, and field reuse

- [ ] Add a short comment noting that operation value `2` is intentionally unused in the current numbering.
- [ ] Add concise field comments to `READDRIVER_REQUEST` describing the generic meaning and reserved-tail intent.
- [ ] Add a short comment near the reserved fields that this tail exists for ABI stability rather than current runtime use.

### Task 2: Align the managed mirror contract

**Files:**
- Modify: `G:\workspace\projects\readDriver\client\DriverApi.cs`

**Interfaces:**
- Consumes: The layout in `common/protocol.h`
- Produces: Matching managed field names and comments that reduce drift risk

- [ ] Rename `Reserved7` through `Reserved11` to `Reserved1` through `Reserved5`.
- [ ] Add a short comment on the enum gap and on the fixed-layout struct mirror.
- [ ] Keep the marshaled field order unchanged.

### Task 3: Update protocol docs

**Files:**
- Modify: `G:\workspace\projects\readDriver\docs\PROJECT_OVERVIEW.md`
- Modify: `G:\workspace\projects\readDriver\docs\ARCHITECTURE.md`

**Interfaces:**
- Consumes: Current protocol semantics
- Produces: Docs that explain what is implemented today versus merely declared

- [ ] Add a note in `PROJECT_OVERVIEW.md` that op `1` is declared but not dispatched and op `2` is intentionally unused.
- [ ] Add a compact operation-semantics note in `PROJECT_OVERVIEW.md` for the `WriteInteger` field reuse.
- [ ] Extend `ARCHITECTURE.md` so the protocol caveats mention the op `2` gap and current op `1` status explicitly.

### Task 4: Verify build integrity

**Files:**
- Modify: none
- Test: workspace build

**Interfaces:**
- Consumes: Root build script
- Produces: Fresh evidence that the documentation/contract cleanup did not break the repo

- [ ] Run `G:\workspace\projects\readDriver\build-all.ps1 -Configuration Debug -Platform x64`.
- [ ] Confirm the build succeeds with no errors before reporting completion.
