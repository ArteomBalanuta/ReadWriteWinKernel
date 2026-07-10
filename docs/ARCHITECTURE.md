# Architecture

This document explains how the codebase is structured today and where each responsibility lives.

## High-Level Flow

1. The controller parses command-line arguments.
2. The controller creates a `READDRIVER_REQUEST`.
3. The controller attempts to open the protocol-defined path `\\\\.\\Global\\Ki000`.
4. The controller sends `READDRIVER_IOCTL_REQUEST` through `DeviceIoControl`.
5. The driver receives the IRP in its device-control dispatch path.
6. The driver validates the request and routes it by `Operation`.
7. The selected driver helper performs the requested work.
8. The driver completes the IRP and returns a status to user mode.

Current lifecycle note:

- the internal driver object is `\\Driver\\gammaA####` (randomized each load)
- the internal device object is `\\Device\\gammaA####` (randomized each load)
- the public client path remains `\\\\.\\Global\\Ki000`
- the `unload` operation removes the public symbolic link and decommissions request handling; it is not a forced driver-object unload

## Module Responsibilities

### `common/protocol.h`

Shared protocol boundary for both projects.

Owns:

- device and symbolic-link names
- IOCTL code
- operation enum
- request struct definition
- request flags such as silent mode

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
- global symbolic-link creation
- dispatch table registration
- decommission behavior after link removal

Why it exists:

- lifecycle code changes for different reasons than request handling
- it is easier to review device setup when it is isolated from IOCTL logic

Current behavior note:

- the driver creates `\\Device\\gammaA####` with a random 4-digit suffix (e.g., `\\Device\\gammaA8973`)
- the driver creates `\\DosDevices\\Global\\Ki000` as the stable user-mode path
- the link-removal operation deletes the global DOS symbolic link, stops freeze timer activity, and makes existing handles fail future IOCTLs
- randomized internal names avoid collisions on repeated mapping without reboot

### `dummy/dispatch.c`

Request validation and IRP routing.

Owns:

- IOCTL code validation
- buffer size validation
- conversion from `SystemBuffer` to `READDRIVER_REQUEST`
- operation switch and dispatch
- response-buffer population for read-style operations
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
- read helper
- scan helper
- batch-read helper
- read-with-protect helper
- read-as-hex helper

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
- response-buffer printing handoff

### `controller/controller/cli.c`

Native client argument parsing and usage text.

Owns:

- command-line parsing
- usage/help output
- request summary printing
- response formatting for read, scan, hex, and batch-read operations

### `controller/controller/transport.c`

Native client device transport.

Owns:

- `CreateFileW` against the device path
- `DeviceIoControl` request submission
- caller-provided response-buffer transport

### `client/`

C# client project with both CLI and GUI entry points.

Owns:

- command-line submission parity with the native client
- WinForms UI for interactive requests
- shared .NET transport wrapper in `DriverApi.cs`
- managed decoding helpers for read responses

### `client.Tests/`

Managed regression-test project.

Owns:

- focused tests for the C# request/response helpers
- verification of raw-byte and string decoding behavior in the managed client layer

## Request Semantics

`READDRIVER_REQUEST` is generic, but the operations interpret fields differently.

Current protocol caveats:

- the protocol still uses one generic fixed-width request instead of operation-specific payload structs
- some operations reuse fields in non-obvious ways, especially `WriteInteger` and `BatchRead`
- read-style operations reuse the buffered IOCTL system buffer as both request input and response output
- because that response buffer is only `sizeof(READDRIVER_REQUEST)`, all read-style operations are bounded by the request-struct size on the current implementation

### Copy

- `SourceProcessId`: source process
- `SourceAddress`: source address
- `TargetProcessId`: target process
- `TargetAddress`: target address
- `Size`: transfer size

### Read

- `SourceProcessId`: process to read
- `SourceAddress`: address to read
- `Size`: byte count
- return value: raw bytes copied back into the IOCTL response buffer

### Scan

- `SourceProcessId`: process to scan
- `SourceAddress`: scan start address
- `TargetProcessId`: scan range size
- `TargetAddress`: byte pattern packed into the low 8 bytes
- `Size`: pattern length in bytes (`1`-`8`)
- return value: first matched address as `ULONGLONG`

### Protect

- `SourceProcessId`: process to open
- `SourceAddress`: base address
- `TargetProcessId`: region size

### Patch

- `SourceProcessId`: process to modify
- `SourceAddress`: address to patch

### WriteInteger

- `SourceProcessId`: process to modify
- `SourceAddress`: integer value to write
- `TargetAddress`: destination address
- `Size`: integer width in bytes (`1`, `2`, `4`, or `8`)

### BatchRead

- `SourceProcessId`: process to read
- `SourceAddress`: first address
- `TargetProcessId`: second address if non-zero
- `TargetAddress`: third address if non-zero
- `Reserved2`: fourth address if non-zero
- `Reserved3`: fifth address if non-zero
- `Size`: byte count per address (`1`-`8`)
- return value: concatenated read chunks in the response buffer

### ReadWithProtect

- `SourceProcessId`: process to read
- `SourceAddress`: address to read
- `Size`: byte count
- behavior: temporarily sets `PAGE_EXECUTE_READWRITE`, reads bytes, then restores the original protection
- return value: raw bytes copied back into the IOCTL response buffer

### ReadHex

- `SourceProcessId`: process to read
- `SourceAddress`: address to read
- `Size`: byte count, capped by the implementation and by the fixed response-buffer budget
- return value: ASCII hex string written into the response buffer

Current implementation note:

- the hex formatter expands each byte to `"XX "` plus a trailing `\0`
- with the current fixed-width response buffer, only a short hex string fits in one request/response cycle

### Flags

- `Flags` bit `0`: silent mode, which suppresses `LogStatus` writes for that request

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

Additional current note:

- the root solution includes a `client.Tests` project that is easiest to run with `dotnet test`

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
- remember that the current WinForms GUI does not expose every protocol operation the CLI/transport layers support
- remember that the clients target `\\\\.\\Global\\Ki000`, while the internal kernel names are `\\Driver\\gammaA####` and `\\Device\\gammaA####` (randomized each load)

## Suggested Next Refactors

1. Rename `dummy` to a driver-specific project name.
2. Replace field overloading in `READDRIVER_REQUEST` with operation-specific payloads.
3. Move hard-coded log paths behind a single configuration layer.
4. Rename the `unload` operation to `decommission` in the protocol and clients so command wording matches behavior.
5. Bring the WinForms GUI operation list into parity with the current protocol if interactive support for read/scan modes is desired.
