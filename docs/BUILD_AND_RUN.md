# Build And Run

## Requirements

This workspace currently assumes:

- Windows
- Visual Studio 2026
- Windows 11 SDK `10.0.28000.x`
- Windows Driver Kit `10.0.28000.x`
- .NET 8 SDK

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

The controller prints a usage screen if parsing fails.

## Running the C# Client

Executable:

- [client.exe](/G:/workspace/projects/readDriver/build/client/x64/Debug/net8.0-windows/client.exe)

CLI behavior:

```text
client.exe <srcPid> <srcAddress> <targetPid> <targetAddress> <size> <operation>
client.exe <processPid> <integerValue> <targetAddress> <size> 5
```

GUI behavior:

- start `client.exe` with no arguments
- the WinForms UI opens
- choose an operation
- enter request values
- click `Send Request`

## Driver Runtime Notes

The current code creates:

- `\\Device\\PWN29`
- `\\DosDevices\\PWN29`
- `\\\\.\\PWN29`

The driver writes diagnostic files to:

- `C:\dummy_logs.txt`
- `C:\dummy_conversion_logs.txt`

## What This Repo Does Not Yet Automate

The repository currently does not include:

- a full driver install/uninstall script
- a full signing and deployment workflow
- a packaged service or installer

That means this repo is strongest as a buildable local development workspace rather than a finished deployment product.

## Troubleshooting

### `dotnet` app fails after copying only `client.exe`

Use `publish-client.ps1` or copy the entire build output directory. A framework-dependent .NET build needs its companion files unless it is published as self-contained.

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
