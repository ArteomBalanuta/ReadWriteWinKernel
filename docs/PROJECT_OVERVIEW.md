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

This is the interface contract for the whole workspace. If the request layout changes, both clients and the driver need to be updated together.

### Driver project

Folder: [dummy](/G:/workspace/projects/readDriver/dummy)

This is the KMDF kernel-mode driver project. Its current source files are split by responsibility:

- [main.c](/G:/workspace/projects/readDriver/dummy/main.c)
  Driver entry point.
- [driver_lifecycle.c](/G:/workspace/projects/readDriver/dummy/driver_lifecycle.c)
  Device creation, symbolic link creation, dispatch registration, unload.
- [dispatch.c](/G:/workspace/projects/readDriver/dummy/dispatch.c)
  IOCTL validation, request routing, IRP completion.
- [memory_ops.c](/G:/workspace/projects/readDriver/dummy/memory_ops.c)
  Behavior-heavy implementation helpers.
- [logging.c](/G:/workspace/projects/readDriver/dummy/logging.c)
  File-based diagnostic logging.
- [main.h](/G:/workspace/projects/readDriver/dummy/main.h)
  Shared declarations inside the driver.

### Native C client

Folder: [controller/controller](/G:/workspace/projects/readDriver/controller/controller)

This is a small command-line tool that:

- parses request arguments
- opens `\\\\.\\PWN29`
- submits one buffered IOCTL request
- prints request and completion information

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

Main files:

- [Program.cs](/G:/workspace/projects/readDriver/client/Program.cs)
  Entry point and CLI flow.
- [DriverApi.cs](/G:/workspace/projects/readDriver/client/DriverApi.cs)
  Shared parsing and transport helpers.
- [ClientForm.cs](/G:/workspace/projects/readDriver/client/ClientForm.cs)
  GUI.

## Device Contract

The current device names are:

- kernel device: `\\Device\\PWN29`
- DOS device: `\\DosDevices\\PWN29`
- user-visible path: `\\\\.\\PWN29`

The request transport uses:

- `METHOD_BUFFERED`
- one fixed request structure
- `READDRIVER_IOCTL_REQUEST`

## Current Operations

The shared enum currently defines:

- `ReadDriverOperationCopy = 0`
- `ReadDriverOperationRead = 1`
- `ReadDriverOperationProtect = 3`
- `ReadDriverOperationPatch = 4`
- `ReadDriverOperationWriteInteger = 5`

Not every declared value is necessarily handled in the same way by every module. The authoritative implementation is the driver dispatch path in [dispatch.c](/G:/workspace/projects/readDriver/dummy/dispatch.c).

## Build Outputs

When built through the root solution, outputs go to the root [build](/G:/workspace/projects/readDriver/build) folder:

- driver output under `build\driver\...`
- native controller output under `build\controller\...`
- C# client output under `build\client\...`

## Recommended Workflow

- open [readDriver.sln](/G:/workspace/projects/readDriver/readDriver.sln) for normal development
- use [build-all.ps1](/G:/workspace/projects/readDriver/build-all.ps1) from the repo root for command-line builds
- use [publish-client.ps1](/G:/workspace/projects/readDriver/publish-client.ps1) when you need a standalone C# client package

## Related Docs

- [BUILD_AND_RUN.md](/G:/workspace/projects/readDriver/docs/BUILD_AND_RUN.md)
- [ARCHITECTURE.md](/G:/workspace/projects/readDriver/docs/ARCHITECTURE.md)
