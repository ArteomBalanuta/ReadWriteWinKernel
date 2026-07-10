# Build And Run

## Requirements

This workspace currently assumes:

- Windows
- Visual Studio 2026
- Windows 11 SDK `10.0.28000.x`
- Windows Driver Kit `10.0.28000.x`
- .NET 8 SDK

For managed regression coverage, the repo also now includes [client.Tests](/G:/workspace/projects/readDriver/client.Tests).

## Install Prerequisites

Install the current Visual Studio 2026 toolchain with C++ support. The repository build script still contains a fallback path for older MSBuild + WDK combinations, but the preferred setup is now Visual Studio 2026 with the `28000` SDK and WDK line.

Install .NET 8 SDK:

```powershell
winget install --id Microsoft.DotNet.SDK.8 -e
```

Install Visual Studio Community 2026:

```powershell
winget install --id Microsoft.VisualStudio.Community -e
```

Install Windows Driver Kit:

```powershell
winget install --id Microsoft.WindowsWDK.10.0.28000 -e
```

Install Windows 11 SDK if needed:

```powershell
winget install --id Microsoft.WindowsSDK.10.0.28000 -e
```

## Visual Studio Components

Open Visual Studio Installer and verify these are installed:

- `Desktop development with C++`
- `MSVC v145` toolset when building from Visual Studio 2026
- `Windows 11 SDK (10.0.28000.x)`
- `Windows Driver Kit`

If you want to build the driver directly from Visual Studio 2026, make sure your WDK installation includes:

- `Microsoft.DriverKit.Build.Tasks.18.0.dll`

If your machine only has an older WDK task assembly such as `Microsoft.DriverKit.Build.Tasks.17.0.dll`, use the repo build script and let it fall back to a compatible MSBuild version.

## Verify the Toolchain

Check .NET:

```powershell
dotnet --info
```

Check that Visual Studio can be discovered:

```powershell
& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath
```

## Open the Correct Solution

For normal development, use:

- [readDriver.sln](/G:/workspace/projects/readDriver/readDriver.sln)

Use these smaller entry points only when you intentionally want them:

- [dummy.sln](/G:/workspace/projects/readDriver/dummy.sln)
- [controller.sln](/G:/workspace/projects/readDriver/controller/controller.sln)

For managed-only verification, use:

- [client.Tests.csproj](/G:/workspace/projects/readDriver/client.Tests/client.Tests.csproj)

## Build Everything

From the repository root:

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

What this does:

- finds `MSBuild.exe`
- prefers Visual Studio 2026 when WDK 18 task binaries are installed
- otherwise falls back to an older compatible MSBuild path for driver compatibility
- builds [readDriver.sln](/G:/workspace/projects/readDriver/readDriver.sln)

## Manual Builds

If you are already in a Visual Studio developer environment:

```powershell
msbuild G:\workspace\projects\readDriver\readDriver.sln /t:Build /p:Configuration=Debug;Platform=x64 /m
```

Build only the C# client:

```powershell
dotnet build G:\workspace\projects\readDriver\client\client.csproj -c Debug -p:Platform=x64
```

Run the managed regression tests:

```powershell
dotnet test G:\workspace\projects\readDriver\client.Tests\client.Tests.csproj -nologo
```

## Build Outputs

Root solution builds emit:

- driver: [dummy.sys](/G:/workspace/projects/readDriver/build/driver/x64/Debug/dummy.sys)
- native controller: [controller.exe](/G:/workspace/projects/readDriver/build/controller/x64/Debug/controller.exe)
- C# client: [client.exe](/G:/workspace/projects/readDriver/build/client/x64/Debug/net8.0-windows/client.exe)

Release outputs follow the same directory layout under `Release`.

## Publish the C# Client

If you want to move the C# client outside the build folder and run it as a standalone application, publish it first:

```powershell
.\publish-client.ps1
```

or

```powershell
.\publish-client.cmd
```

Custom publish example:

```powershell
.\publish-client.ps1 -Configuration Release -Runtime win-x64
```

Published output path:

- `client\bin\Release\net8.0-windows\win-x64\publish\`

## Running the Native Client

Executable:

- [controller.exe](/G:/workspace/projects/readDriver/build/controller/x64/Debug/controller.exe)

CLI forms:

```text
controller.exe <srcPid> <srcAddress> <targetPid> <targetAddress> <size> <operation>
controller.exe <processPid> <integerValue> <targetAddress> <size> 5
```

Supported operations currently include:

- `0` copy
- `1` read bytes back into the response buffer
- `2` scan for a 1-8 byte pattern
- `3` protect
- `4` patch
- `5` write integer
- `6` batch read up to five addresses
- `7` read with temporary RWX protection
- `8` read and return a hex string

The controller prints a usage screen if parsing fails and prints response data for read-style operations.

## Running the C# Client

Executable:

- [client.exe](/G:/workspace/projects/readDriver/build/client/x64/Debug/net8.0-windows/client.exe)

CLI behavior:

```text
client.exe <srcPid> <srcAddress> <targetPid> <targetAddress> <size> <operation>
client.exe <processPid> <integerValue> <targetAddress> <size> 5
```

The C# CLI prints response data for read-style operations and can use the expanded operation set exposed by `DriverApi.cs`.

GUI behavior:

- start `client.exe` with no arguments
- the WinForms UI opens
- choose an operation
- enter request values
- click `Send Request`

Current GUI note:

- the WinForms UI currently exposes `Copy`, `Protect`, `Patch`, and `WriteInteger`
- newer read/scan-style operations exist in the protocol and CLI path, but are not yet surfaced in the GUI

## Driver Runtime Notes

The shared protocol constants define:

- `\\Driver\\gammaA####` (randomized each load, e.g., `\\Driver\\gammaA8973`)
- `\\Device\\gammaA####` (randomized each load)
- `\\DosDevices\\Global\\Ki000`
- `\\\\.\\Global\\Ki000`

Current lifecycle caveat:

- `driver_lifecycle.c` creates a global DOS symbolic link for the stable client path
- the `unload` command and GUI button remove that public link, stop background freeze work, and mark the driver surface as decommissioned
- this prevents new opens and causes later IOCTLs from existing handles to fail, but it is not a forced unload of the kernel driver object

The driver writes diagnostic files to:

- `C:\dummy_logs.txt`
- `C:\dummy_conversion_logs.txt`

Silent mode:

- `READDRIVER_REQUEST.Flags` bit `0` suppresses driver status logging for that request
- the C# helper methods for read-style operations currently set this flag automatically

Current response-buffer note:

- read-style operations reuse the same buffered IOCTL system buffer for output
- that means one response is currently limited to `sizeof(READDRIVER_REQUEST)`
- `ReadHex` expands bytes into text, so its practical limit is smaller than raw-byte reads

## What This Repo Does Not Yet Automate

The repository currently does not include:

- a full driver install/uninstall script
- a full signing and deployment workflow
- a packaged service or installer

That means this repo is strongest as a buildable local development workspace rather than a finished deployment product.

## Troubleshooting

### `dotnet` app fails after copying only `client.exe`

Use `publish-client.ps1` or copy the entire build output directory. A framework-dependent .NET build needs its companion files unless it is published as self-contained.

### `CreateFile` fails against `\\\\.\\Global\\Ki000`

The shared protocol and both clients target `\\\\.\\Global\\Ki000`. If `CreateFile` fails after running `client.exe unload`, the public symbolic link was intentionally removed. Remap or reload a driver build that creates `\\DosDevices\\Global\\Ki000`, or reboot if the existing manually mapped driver object is still occupying its internal name.

### `build-all.ps1` cannot find MSBuild

Check that Visual Studio is installed and that `vswhere.exe` exists under:

```text
C:\Program Files (x86)\Microsoft Visual Studio\Installer\
```

### `ValidateNTTargetVersion` fails under Visual Studio 2026

That means MSBuild 18 was selected but your WDK install does not contain `Microsoft.DriverKit.Build.Tasks.18.0.dll`. In that case:

- build through `.\build-all.ps1`
- or install `Microsoft.WindowsWDK.10.0.28000`
- or install the matching SDK `Microsoft.WindowsSDK.10.0.28000`
- or use an older Visual Studio + WDK pair that ships matching DriverKit task binaries

### Root build works but controller-only solution writes new artifacts

That is expected if you build [controller.sln](/G:/workspace/projects/readDriver/controller/controller.sln) directly. The repo ignore rules already account for nested build output.

## Related Docs

- [PROJECT_OVERVIEW.md](/G:/workspace/projects/readDriver/docs/PROJECT_OVERVIEW.md)
- [ARCHITECTURE.md](/G:/workspace/projects/readDriver/docs/ARCHITECTURE.md)
