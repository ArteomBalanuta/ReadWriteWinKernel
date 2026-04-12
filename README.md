# readDriver

This repository contains a Windows kernel driver and a small user-mode controller that sends a single buffered IOCTL request format to that driver.

The codebase is organized as a Visual Studio C/C++ workspace and currently builds successfully with:

- Visual Studio 2022
- Windows 11 SDK `10.0.26100.0`
- Windows Driver Kit `10.0.26100.0`

## Repository Layout

- `common/`
  Shared protocol definitions used by both the driver and the controller.
- `dummy/`
  Kernel-mode driver project.
- `controller/controller/`
  User-mode console application that opens the device and sends IOCTL requests.
- `readDriver.sln`
  Root solution that builds driver + both clients together.
- `dummy.sln`
  Driver-only solution.

## Main Components

### Shared protocol

[common/protocol.h](/G:/workspace/projects/readDriver/common/protocol.h) defines:

- the device type and IOCTL code
- the kernel and DOS device names
- the `READDRIVER_OPERATION` enum
- the `READDRIVER_REQUEST` struct shared by kernel-mode and user-mode code

This file is the contract between the controller and the driver. If the request layout changes, both sides must be updated together.

### Driver

The driver source is intentionally split by responsibility:

- [dummy/main.c](/G:/workspace/projects/readDriver/dummy/main.c)
  Driver entry point.
- [dummy/driver_lifecycle.c](/G:/workspace/projects/readDriver/dummy/driver_lifecycle.c)
  Device creation, symbolic link setup, dispatch registration, unload.
- [dummy/dispatch.c](/G:/workspace/projects/readDriver/dummy/dispatch.c)
  IOCTL validation, request routing, IRP completion.
- [dummy/memory_ops.c](/G:/workspace/projects/readDriver/dummy/memory_ops.c)
  Process lookup, memory copy, page protection, patch helper.
- [dummy/logging.c](/G:/workspace/projects/readDriver/dummy/logging.c)
  Timestamp formatting and file-based status logging.
- [dummy/main.h](/G:/workspace/projects/readDriver/dummy/main.h)
  Shared driver declarations.

### Controller

[controller/controller/Source.c](/G:/workspace/projects/readDriver/controller/controller/Source.c) is a minimal console client that:

- parses six numeric arguments
- fills a `READDRIVER_REQUEST`
- opens `\\\\.\\PWN29`
- sends `READDRIVER_IOCTL_REQUEST` through `DeviceIoControl`
- prints basic request and completion information

## Setup

Follow these steps on Windows before first build.

### 1. Install required tools

Install Visual Studio 2022 Community:

```powershell
winget install --id Microsoft.VisualStudio.2022.Community -e
```

Install .NET 8 SDK:

```powershell
winget install --id Microsoft.DotNet.SDK.8 -e
```

Install Windows Driver Kit (WDK):

```powershell
winget install --id Microsoft.WindowsDriverKit -e
```

Install Windows 11 SDK (if not already present):

```powershell
winget install --id Microsoft.WindowsSDK.10.0.26100 -e
```

### 2. Configure Visual Studio components

Open Visual Studio Installer and make sure these workloads/components are installed:

- `Desktop development with C++`
- `MSVC v143` toolset
- `Windows 11 SDK (10.0.26100.x)`
- `Windows Driver Kit`

### 3. Verify toolchain from terminal

```powershell
dotnet --info
```

```powershell
& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath
```

### 4. Clone and enter the repository

```powershell
git clone <your-repo-url>
cd readDriver
```

### 5. Build everything from the parent folder

```powershell
.\build-all.ps1
```

or

```powershell
.\build-all.cmd
```

Release build:

```powershell
.\build-all.ps1 -Configuration Release -Platform x64
```

### 6. Build output locations

- Driver: `build\driver\x64\<Configuration>\dummy.sys`
- Native client: `build\controller\x64\<Configuration>\controller.exe`
- C# client: `build\client\x64\<Configuration>\net8.0-windows\client.exe`

## Build

### Recommended

Open [readDriver.sln](/G:/workspace/projects/readDriver/readDriver.sln) in Visual Studio 2022 and build `Debug|x64`.

### Command line

From the repository root:

```powershell
.\build-all.ps1
```

or

```powershell
.\build-all.cmd
```

To select configuration:

```powershell
.\build-all.ps1 -Configuration Release -Platform x64
```

You can still build manually from a Visual Studio 2022 Developer Command Prompt:

```powershell
msbuild G:\workspace\projects\readDriver\readDriver.sln /t:Build /p:Configuration=Debug;Platform=x64 /m
```

## Build Outputs

Successful `Debug|x64` builds currently emit:

- controller: [build/controller/x64/Debug/controller.exe](/G:/workspace/projects/readDriver/build/controller/x64/Debug/controller.exe)
- driver: [build/driver/x64/Debug/dummy.sys](/G:/workspace/projects/readDriver/build/driver/x64/Debug/dummy.sys)
- C# client: [build/client/x64/Debug/net8.0-windows/client.exe](/G:/workspace/projects/readDriver/build/client/x64/Debug/net8.0-windows/client.exe)

The individual project files also support project-local build folders under each project when built directly.

## Request Format

The request layout is fixed-width and uses `ULONGLONG` fields for compatibility between user mode and kernel mode:

- `SourceProcessId`
- `SourceAddress`
- `TargetProcessId`
- `TargetAddress`
- `Size`
- `Operation`
- `Reserved7` through `Reserved11`

Current semantics in the driver:

- `ReadDriverOperationCopy`
  Uses source PID/address, target PID/address, and size.
- `ReadDriverOperationProtect`
  Uses `SourceProcessId` as the target process, `SourceAddress` as the base address, and `TargetProcessId` as the region size.
- `ReadDriverOperationPatch`
  Uses `SourceProcessId` as the target process and `SourceAddress` as the patch address.

This field reuse works, but it is one of the main readability issues left in the project. If you extend the protocol, prefer operation-specific naming or separate request payloads.

## Runtime Notes

- The driver creates:
  - kernel device: `\\Device\\PWN29`
  - DOS device: `\\DosDevices\\PWN29`
  - user-visible path: `\\\\.\\PWN29`
- IOCTL transfer method is `METHOD_BUFFERED`.
- The driver currently uses buffered I/O via `DO_BUFFERED_IO`.
- File-based logging writes to:
  - `C:\dummy_logs.txt`
  - `C:\dummy_conversion_logs.txt`

## Project Conventions

- Shared protocol code belongs in `common/`.
- Driver code should stay split by responsibility instead of returning to a single large source file.
- Build outputs should go to `build/` rather than source folders.
- Generated Visual Studio artifacts such as `.vs/`, `x64/`, `Debug/`, `Release/`, and `output/` are ignored and should not be committed.

## Documentation

- [docs/ARCHITECTURE.md](/G:/workspace/projects/readDriver/docs/ARCHITECTURE.md)
  Internal structure, module responsibilities, request flow, and maintenance notes.

## Known Cleanup Opportunities

- Rename `dummy/` to a more descriptive driver project name.
- Replace the generic `READDRIVER_REQUEST` field overloading with clearer per-operation semantics.
- Move hard-coded log file paths into a configuration or compile-time option if the project is going to be reused on other machines.
