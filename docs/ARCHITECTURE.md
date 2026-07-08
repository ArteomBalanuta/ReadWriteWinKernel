# Architecture

This document explains how the codebase is structured today and where each responsibility lives.

## High-Level Flow

1. The controller parses command-line arguments.
2. The controller creates a `READDRIVER_REQUEST`.
3. The controller opens `\\\\.\\PWN29`.
4. The controller sends `READDRIVER_IOCTL_REQUEST` through `DeviceIoControl`.
5. The driver receives the IRP in its device-control dispatch path.
6. The driver validates the request and routes it by `Operation`.
7. The selected driver helper performs the requested work.
8. The driver completes the IRP and returns a status to user mode.

## Module Responsibilities

### `common/protocol.h`

Shared protocol boundary for both projects.

Owns:

- device and symbolic-link names
- IOCTL code
- operation enum
- request struct definition

Design rule:

- Changes to this file are cross-cutting and should be treated as interface changes, not local implementation edits.

### `dummy/main.c`

Small bootstrap file that only handles `DriverEntry` and delegates actual initialization to `DriverInitialize`.

Why it exists:

- keeps the formal entry point trivial
- avoids mixing bootstrapping with runtime logic

### `dummy/driver_lifecycle.c`

Driver lifecycle and object registration.

Owns:

- `DriverInitialize`
- `DriverUnload`
- device creation
- symbolic-link creation
- dispatch table registration

Why it exists:

- lifecycle code changes for different reasons than request handling
- it is easier to review device setup when it is isolated from IOCTL logic

### `dummy/dispatch.c`

Request validation and IRP routing.

Owns:

- IOCTL code validation
- buffer size validation
- conversion from `SystemBuffer` to `READDRIVER_REQUEST`
- operation switch and dispatch
- IRP completion helpers

Why it exists:

- it is the translation boundary between Windows I/O plumbing and internal driver helpers
- it keeps request handling centralized without mixing it with device creation

### `dummy/memory_ops.c`

Process and memory helper implementation.

Owns:

- process lookup
- `MmCopyVirtualMemory` wrapper
- `ZwProtectVirtualMemory` wrapper
- patch helper

Notes:

- this file is the most behavior-heavy part of the driver
- if the driver grows further, this file is the next likely candidate for deeper refactoring

### `dummy/logging.c`

File-based diagnostic logging.

Owns:

- timestamp formatting
- status formatting
- append-to-file helper
- user-facing driver log messages

Tradeoff:

- this is simple to inspect during debugging
- it also hardcodes log file destinations and keeps logging tightly coupled to the local filesystem

### `controller/controller/main.c`

Small console entry point for the native client.

Owns:

- top-level request flow
- CLI parsing handoff
- request summary output
- transport submission handoff

### `controller/controller/cli.c`

Native client argument parsing and usage text.

Owns:

- command-line parsing
- usage/help output
- request summary printing

### `controller/controller/transport.c`

Native client device transport.

Owns:

- `CreateFileW` against the device path
- `DeviceIoControl` request submission

### `client/`

C# client project with both CLI and GUI entry points.

Owns:

- command-line submission parity with the native client
- WinForms UI for interactive requests
- shared .NET transport wrapper in `DriverApi.cs`

## Request Semantics

`READDRIVER_REQUEST` is generic, but the operations interpret fields differently.

### Copy

- `SourceProcessId`: source process
- `SourceAddress`: source address
- `TargetProcessId`: target process
- `TargetAddress`: target address
- `Size`: transfer size

### Protect

- `SourceProcessId`: process to open
- `SourceAddress`: base address
- `TargetProcessId`: region size

### Patch

- `SourceProcessId`: process to modify
- `SourceAddress`: address to patch

This is functional but not self-describing. A future redesign should either:

- split the request into separate structs per operation, or
- keep one envelope struct with an explicit union payload

## Dependency Direction

Dependency flow is intentionally simple:

- `controller` depends on `common`
- `dummy` depends on `common`
- `common` depends on neither project

Within the driver:

- `main.c` depends on lifecycle declarations
- `driver_lifecycle.c` depends on dispatch and logging declarations
- `dispatch.c` depends on memory-operation declarations
- `memory_ops.c` depends on logging declarations

This keeps the driver mostly layered:

- lifecycle
- dispatch
- domain helpers
- diagnostics

## Build and Tooling Notes

The repository currently assumes:

- Visual Studio 2026
- MSVC `v145` for the controller when building under Visual Studio 2026
- `WindowsKernelModeDriver10.0` for the driver
- SDK/WDK version `10.0.28000.0`

Current local build policy:

- driver test-signing is disabled in the project so local builds can complete cleanly
- build output goes to `build/` rather than source folders
- the root build script prefers MSBuild 18 when WDK 18 task binaries are installed and otherwise falls back to an older compatible MSBuild version

## Maintenance Guidance

When making changes, prefer these rules:

### Protocol changes

- start in `common/protocol.h`
- update both the controller and the driver in the same change
- document any changed field meaning in `README.md` and this file

### Driver changes

- keep dispatch validation in `dispatch.c`
- keep object/device lifecycle in `driver_lifecycle.c`
- keep implementation helpers in `memory_ops.c`
- avoid adding unrelated helpers back into `main.c`

### Controller changes

- keep `main.c` small unless the tool gains multiple commands or modes
- keep parsing in `cli.c` and device submission in `transport.c`
- keep the C# client transport logic centralized in `client/DriverApi.cs`

## Suggested Next Refactors

1. Rename `dummy` to a driver-specific project name.
2. Replace field overloading in `READDRIVER_REQUEST` with operation-specific payloads.
3. Move hard-coded log paths behind a single configuration layer.
4. Add a small verification or smoke-test script for common local build and deployment checks.
