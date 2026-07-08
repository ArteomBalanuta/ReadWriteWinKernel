# readDriver

`readDriver` is a Windows research workspace built around a kernel-mode driver and two user-mode clients that speak the same IOCTL request format.

At a high level, the repository contains:

- a KMDF driver in [dummy](/G:/workspace/projects/readDriver/dummy)
- a native C console client in [controller/controller](/G:/workspace/projects/readDriver/controller/controller)
- a C# client with both CLI and WinForms GUI modes in [client](/G:/workspace/projects/readDriver/client)
- a shared request contract in [common/protocol.h](/G:/workspace/projects/readDriver/common/protocol.h)

This repo is best treated as a local development and research workspace. It includes build helpers and documentation, but it does not include a full deployment or service-install story for the driver.

The workspace is now prepared to open in Visual Studio 2026. Driver builds still require a matching WDK task assembly for the MSBuild version you use.

The current protocol includes copy, read, scan, protect, patch, write-integer, batch-read, read-with-protect, and read-as-hex operations. The shared request still uses one fixed-width struct, so operation-specific field meanings are documented in [docs/ARCHITECTURE.md](/G:/workspace/projects/readDriver/docs/ARCHITECTURE.md).

The root solution also now includes a managed regression-test project in [client.Tests](/G:/workspace/projects/readDriver/client.Tests).

## What This Project Is

The project is a Visual Studio solution for experimenting with a Windows driver and small user-mode tools that send `DeviceIoControl` requests to it. The shared request structure lives in `common/protocol.h`, and both clients use that same contract to talk to the driver device `\\\\.\\Global\\Ki000`.

The current codebase is organized as one root workspace with three main executable modules:

- `dummy`
  KMDF kernel driver
- `controller`
  native C command-line client
- `client`
  .NET client with command-line and GUI entry points
- `client.Tests`
  Managed regression tests focused on the C# client request/response helpers

Today the client surfaces are intentionally uneven:

- the native controller can print response data returned by the driver
- the C# CLI can submit the expanded operation set and decode response payloads
- the WinForms GUI still exposes the older core operations (`Copy`, `Protect`, `Patch`, `WriteInteger`)

The driver lifecycle is also intentionally non-standard right now:

- it creates the `\\DosDevices\\Ki000` symbolic link and then immediately deletes it
- it changes the created device object type to `FILE_DEVICE_NULL`
- it clears `DriverObject` and `NextDevice` references on the created device object
- it zeroes the `DRIVER_OBJECT` at the end of initialization

That means the protocol still names `\\\\.\\Ki000`, but the current driver source does not behave like a normal persistently exposed user-mode device path.

## Start Here

If you are new to the repo, use this order:

1. Read [PROJECT_OVERVIEW.md](/G:/workspace/projects/readDriver/docs/PROJECT_OVERVIEW.md)
2. Follow [BUILD_AND_RUN.md](/G:/workspace/projects/readDriver/docs/BUILD_AND_RUN.md)
3. Use [ARCHITECTURE.md](/G:/workspace/projects/readDriver/docs/ARCHITECTURE.md) when you need code-level structure

## Repository Layout

- [common](/G:/workspace/projects/readDriver/common)
  Shared protocol definitions.
- [dummy](/G:/workspace/projects/readDriver/dummy)
  Driver project source.
- [controller](/G:/workspace/projects/readDriver/controller)
  Native client project and controller-only solution.
- [client](/G:/workspace/projects/readDriver/client)
  C# client project.
- [docs](/G:/workspace/projects/readDriver/docs)
  Project documentation.
- [readDriver.sln](/G:/workspace/projects/readDriver/readDriver.sln)
  Root solution for day-to-day development.
- [build-all.ps1](/G:/workspace/projects/readDriver/build-all.ps1)
  Root build helper for all projects.
- [publish-client.ps1](/G:/workspace/projects/readDriver/publish-client.ps1)
  Standalone publish helper for the C# client.

## Solution Layout

The root Visual Studio solution currently contains:

- the driver project
- the native controller project
- the C# client project
- the `client.Tests` project
- helper solution folders such as `Applications`, `Driver`, `Shared`, `Scripts`, and `Docs`

Those helper folders exist in the solution file, but the current `.sln` no longer nests every project underneath them.

## Build Summary

From the repository root:

```powershell
.\build-all.ps1
```

Release build:

```powershell
.\build-all.ps1 -Configuration Release -Platform x64
```

Standalone C# client publish:

```powershell
.\publish-client.ps1
```

Managed regression tests:

```powershell
dotnet test .\client.Tests\client.Tests.csproj -nologo
```

For the full setup, build, output, and run workflow, see [BUILD_AND_RUN.md](/G:/workspace/projects/readDriver/docs/BUILD_AND_RUN.md).
