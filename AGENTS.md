# AGENTS

## Purpose

This repository is a Windows driver research workspace with one kernel-mode driver, two user-mode clients, shared protocol definitions, and helper scripts for build and publish workflows.

Use this file as the quick operator guide before editing code.

## Start Here

Read these in order when you need more detail:

1. [README.md](/G:/workspace/projects/readDriver/README.md)
2. [docs/PROJECT_OVERVIEW.md](/G:/workspace/projects/readDriver/docs/PROJECT_OVERVIEW.md)
3. [docs/BUILD_AND_RUN.md](/G:/workspace/projects/readDriver/docs/BUILD_AND_RUN.md)
4. [docs/ARCHITECTURE.md](/G:/workspace/projects/readDriver/docs/ARCHITECTURE.md)

## Repo Layout

- [common/protocol.h](/G:/workspace/projects/readDriver/common/protocol.h)
  Shared request contract. Treat changes here as cross-cutting interface changes.
- [dummy](/G:/workspace/projects/readDriver/dummy)
  KMDF driver project.
- [controller/controller](/G:/workspace/projects/readDriver/controller/controller)
  Native C console client.
- [client](/G:/workspace/projects/readDriver/client)
  C# client with CLI and WinForms GUI entry points.
- [client.Tests](/G:/workspace/projects/readDriver/client.Tests)
  Managed regression tests for the C# client helpers.
- [build-all.ps1](/G:/workspace/projects/readDriver/build-all.ps1)
  Preferred root build entry point.
- [publish-client.ps1](/G:/workspace/projects/readDriver/publish-client.ps1)
  Preferred way to produce a standalone C# client package.
- [readDriver.sln](/G:/workspace/projects/readDriver/readDriver.sln)
  Preferred solution for day-to-day work.

## Preferred Workflow

- Prefer the root solution over sub-solutions unless the task is intentionally scoped to one project.
- Prefer `.\build-all.ps1` from the repo root over ad hoc Visual Studio builds.
- Use `.\publish-client.ps1` when the C# client needs to run outside the build tree.
- Keep edits aligned with current module boundaries instead of collapsing logic back into larger files.

## Current Toolchain

The workspace is currently aligned to:

- Visual Studio 2026
- MSBuild 18 when WDK 18 task binaries are installed
- Windows SDK `10.0.28000.x`
- Windows WDK `10.0.28000.x`
- .NET 8 SDK

`build-all.ps1` already contains fallback logic for older compatible MSBuild + WDK combinations. Do not remove that fallback casually.

## Build Commands

Build everything:

```powershell
.\build-all.ps1
```

Release build:

```powershell
.\build-all.ps1 -Configuration Release -Platform x64
```

Publish the C# client:

```powershell
.\publish-client.ps1
```

Run managed regression tests:

```powershell
dotnet test .\client.Tests\client.Tests.csproj -nologo
```

Build only the C# client:

```powershell
dotnet build .\client\client.csproj -c Debug -p:Platform=x64
```

## Verification

Before claiming work is done, run the smallest relevant verification command and report the actual result.

Preferred checks:

- Full workspace build:
  ```powershell
  .\build-all.ps1 -Configuration Debug -Platform x64
  ```
- C# client only:
  ```powershell
  dotnet build .\client\client.csproj -c Debug -p:Platform=x64
  ```

If a task changes the shared protocol, verify both the driver-facing and client-facing build paths.

## Module-Specific Guidance

### Shared protocol

- Start in [common/protocol.h](/G:/workspace/projects/readDriver/common/protocol.h).
- Update both user-mode clients and the driver in the same change.
- Reflect changed semantics in the docs when field meanings change.

### Driver

- Keep lifecycle code in [dummy/driver_lifecycle.c](/G:/workspace/projects/readDriver/dummy/driver_lifecycle.c).
- Keep request validation and routing in [dummy/dispatch.c](/G:/workspace/projects/readDriver/dummy/dispatch.c).
- Keep implementation-heavy helpers in [dummy/memory_ops.c](/G:/workspace/projects/readDriver/dummy/memory_ops.c).
- Keep logging concerns in [dummy/logging.c](/G:/workspace/projects/readDriver/dummy/logging.c).

### Native controller

- Keep top-level flow in [controller/controller/main.c](/G:/workspace/projects/readDriver/controller/controller/main.c).
- Keep parsing and usage text in [controller/controller/cli.c](/G:/workspace/projects/readDriver/controller/controller/cli.c).
- Keep device transport in [controller/controller/transport.c](/G:/workspace/projects/readDriver/controller/controller/transport.c).

### C# client

- Keep CLI entry flow in [client/Program.cs](/G:/workspace/projects/readDriver/client/Program.cs).
- Keep transport and parsing helpers centralized in [client/DriverApi.cs](/G:/workspace/projects/readDriver/client/DriverApi.cs).
- Keep GUI concerns in [client/ClientForm.cs](/G:/workspace/projects/readDriver/client/ClientForm.cs).

## Artifacts And Ignore Rules

- Do not edit generated files under `build/`, `client/bin/`, or `client/obj/`.
- Do not re-add generated NuGet cache files from `client/obj/` to version control.
- Temporary WDK inspection folders are intentionally ignored:
  - `_inspect_wdk_build_zip/`
  - `_inspect_wdk_zip/`
  - `tmp_wdk_build_vsix/`
  - `tmp_wdk_vsix/`
  - `vm_extract/`

The repo root also contains binary/reference artifacts that are not part of the normal source-edit workflow. Do not assume root-level `.dll`, `.sys`, `.vsix`, `.zip`, or screenshot files are authoritative source inputs unless the task explicitly depends on them.

## Known Behavior Notes

- The driver device path is `\\\\.\\Global\\Ki000`.
- The driver hides from `\\DosDevices` by creating the link under `\\Global` instead, changes the device type to `FILE_DEVICE_NULL`, clears the device chain, and zeroes the `DRIVER_OBJECT`. Both clients target `\\\\.\\Global\\Ki000`.
- The driver writes diagnostic logs to `C:\dummy_logs.txt` and `C:\dummy_conversion_logs.txt`.
- The C# client launches the WinForms GUI when started without arguments.
- The CLI/transport layers now support operations `0` through `8`, including read, scan, batch-read, read-with-protect, and read-as-hex paths.
- The C# client and native controller both support:
  - standard 6-argument request form
  - 5-argument write-integer shortcut ending in operation `5`
- `READDRIVER_REQUEST.Flags` bit `0` enables silent mode and suppresses driver status logging for that request.
- Read-style operations currently return through the fixed-width request-sized system buffer, so response sizes are tightly bounded.
- The current WinForms GUI does not expose every protocol operation that the CLI/transport layers support.

## Editing Rules

- Prefer small, targeted edits.
- Preserve the current solution organization and output layout.
- Do not rename major projects or move source trees unless the task explicitly calls for it.
- Avoid broad refactors that are unrelated to the requested change.
- Keep documentation consistent with the actual repo state, not an aspirational redesign.

## When In Doubt

- Prefer the root solution.
- Prefer the root build script.
- Prefer updating docs together with protocol or workflow changes.
- Prefer verifying with a real build instead of assuming the change is safe.
