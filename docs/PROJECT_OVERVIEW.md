# Project Overview

## Purpose

`readDriver` is a Windows driver workspace for local research and development. It is structured around a kernel-mode driver that exposes a device interface and two user-mode clients that can send one shared request format to that device.

The repository is organized so that:

- protocol changes start in [common/protocol.h](/G:/workspace/projects/readDriver/common/protocol.h)
- driver implementation lives in [dummy](/G:/workspace/projects/readDriver/dummy)
- native user-mode tooling lives in [controller/controller](/G:/workspace/projects/readDriver/controller/controller)
- .NET user-mode tooling lives in [client](/G:/workspace/projects/readDriver/client)

## Main Modules

### Shared protocol

File: [protocol.h](/G:/workspace/projects/readDriver/common/protocol.h)

This file defines:

- the device type and IOCTL number
- the kernel and DOS device names
- the operation enum
- the fixed-width `READDRIVER_REQUEST` structure
- the request flags, including silent-mode support

This is the interface contract for the whole workspace. If the request layout changes, both clients and the driver need to be updated together.

### Driver project

Folder: [dummy](/G:/workspace/projects/readDriver/dummy)

This is the KMDF kernel-mode driver project. Its current source files are split by responsibility:

- [main.c](/G:/workspace/projects/readDriver/dummy/main.c)
  Driver entry point.
- [driver_lifecycle.c](/G:/workspace/projects/readDriver/dummy/driver_lifecycle.c)
  Device creation, global symbolic-link creation, dispatch registration, unload, and decommission behavior.
- [dispatch.c](/G:/workspace/projects/readDriver/dummy/dispatch.c)
  IOCTL validation, request routing, IRP completion.
- [memory_ops.c](/G:/workspace/projects/readDriver/dummy/memory_ops.c)
  Behavior-heavy implementation helpers.
- [logging.c](/G:/workspace/projects/readDriver/dummy/logging.c)
  File-based diagnostic logging.
- [main.h](/G:/workspace/projects/readDriver/dummy/main.h)
  Shared declarations inside the driver.

Current lifecycle notes:

- the internal driver object is `\\Driver\\gammaA####` (randomized each load)
- the internal device object is `\\Device\\gammaA####` (randomized each load)
- the user-visible path remains `\\\\.\\Global\\Ki000`
- the `unload` operation removes the global DOS symbolic link and decommissions request handling; it does not force-delete the kernel driver object

### Native C client

Folder: [controller/controller](/G:/workspace/projects/readDriver/controller/controller)

This is a small command-line tool that:

- parses request arguments
- opens `\\\\.\\Global\\Ki000`
- submits one buffered IOCTL request
- prints request and completion information
- prints operation-specific response data for read-style operations

Main files:

- [main.c](/G:/workspace/projects/readDriver/controller/controller/main.c)
- [cli.c](/G:/workspace/projects/readDriver/controller/controller/cli.c)
- [transport.c](/G:/workspace/projects/readDriver/controller/controller/transport.c)
- [controller.h](/G:/workspace/projects/readDriver/controller/controller/controller.h)

### C# client

Folder: [client](/G:/workspace/projects/readDriver/client)

This project has two modes:

- CLI mode when arguments are passed
- WinForms GUI mode when launched without arguments

Current split:

- CLI mode can submit the expanded operation set and decode response payloads
- GUI mode exposes `Copy`, `Protect`, `Patch`, and `WriteInteger` on the Requests tab
- GUI mode provides a dedicated Freeze tab for registering one to four addresses and clearing selected or all freeze entries

Main files:

- [Program.cs](/G:/workspace/projects/readDriver/client/Program.cs)
  Entry point and CLI flow.
- [DriverApi.cs](/G:/workspace/projects/readDriver/client/DriverApi.cs)
  Shared parsing and transport helpers.
- [ClientForm.cs](/G:/workspace/projects/readDriver/client/ClientForm.cs)
  GUI.

### Managed regression tests

Folder: [client.Tests](/G:/workspace/projects/readDriver/client.Tests)

This project currently provides focused .NET tests around the managed request/response helpers, including byte/string decoding and freeze-request validation and address packing.

## Device Contract

The current device names are:

- internal driver object: `\\Driver\\gammaA####` (randomized each load)
- kernel device: `\\Device\\gammaA####` (randomized each load)
- global DOS symbolic link: `\\DosDevices\\Global\\Ki000`
- user-visible path: `\\\\.\\Global\\Ki000`

Current lifecycle caveat:

- the public path is created as a symbolic link to the internal device object
- removing the public link prevents new opens through `\\\\.\\Global\\Ki000`
- existing handles are rejected after decommission, but the driver object itself remains owned by the I/O manager until a real unload path or reboot

The request transport uses:

- `METHOD_BUFFERED`
- one fixed request structure
- `READDRIVER_IOCTL_REQUEST`
- the same system buffer for request input and response output

## Current Operations

The shared enum currently defines:

- `ReadDriverOperationCopy = 0`
- `ReadDriverOperationRead = 1`
- `ReadDriverOperationScan = 2`
- `ReadDriverOperationProtect = 3`
- `ReadDriverOperationPatch = 4`
- `ReadDriverOperationWriteInteger = 5`
- `ReadDriverOperationBatchRead = 6`
- `ReadDriverOperationReadWithProtect = 7`
- `ReadDriverOperationReadHex = 8`

Not every declared value is necessarily handled in the same way by every module. The authoritative implementation is the driver dispatch path in [dispatch.c](/G:/workspace/projects/readDriver/dummy/dispatch.c).

Current caveats worth knowing:

- `ReadDriverOperationWriteInteger = 5` reuses `SourceAddress` as the integer payload and `TargetAddress` as the write destination.
- `ReadDriverOperationBatchRead = 6` currently gathers addresses from `SourceAddress`, `TargetProcessId`, `TargetAddress`, `Reserved2`, and `Reserved3`.
- read-style operations return data through the same fixed-width buffered IOCTL system buffer, so the current response budget is effectively one `READDRIVER_REQUEST` struct.
- `ReadDriverOperationReadHex = 8` is additionally limited by its `size * 3 + 1` string expansion, so the current request layout only leaves room for a short hex string response.
- `Flags` bit `0` enables silent mode, suppressing driver status logging for that request.

## Build Outputs

When built through the root solution, outputs go to the root [build](/G:/workspace/projects/readDriver/build) folder:

- driver output under `build\driver\...`
- native controller output under `build\controller\...`
- C# client output under `build\client\...`

Managed test output stays under `client.Tests\bin\...` and `client.Tests\obj\...`.

## Recommended Workflow

- open [readDriver.sln](/G:/workspace/projects/readDriver/readDriver.sln) for normal development
- use [build-all.ps1](/G:/workspace/projects/readDriver/build-all.ps1) from the repo root for command-line builds
- use [publish-client.ps1](/G:/workspace/projects/readDriver/publish-client.ps1) when you need a standalone C# client package
- use `dotnet test .\client.Tests\client.Tests.csproj -nologo` when you need a quick managed regression check

## Related Docs

- [BUILD_AND_RUN.md](/G:/workspace/projects/readDriver/docs/BUILD_AND_RUN.md)
- [ARCHITECTURE.md](/G:/workspace/projects/readDriver/docs/ARCHITECTURE.md)
