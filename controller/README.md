# Controller

This folder contains the user-mode console application that opens the driver device and sends a single buffered IOCTL request.

The controller project source lives in [G:\workspace\projects\readDriver\controller\controller](/G:/workspace/projects/readDriver/controller/controller), and the solution entry point is [G:\workspace\projects\readDriver\controller\controller.sln](/G:/workspace/projects/readDriver/controller/controller.sln).

## What It Does

The controller:

- parses six numeric command-line arguments
- fills a `READDRIVER_REQUEST`
- opens the driver device at `\\\\.\\PWN29`
- sends `READDRIVER_IOCTL_REQUEST` with `DeviceIoControl`
- prints the request fields and the returned byte count

## Build

Open [G:\workspace\projects\readDriver\controller\controller.sln](/G:/workspace/projects/readDriver/controller/controller.sln) in Visual Studio 2022 and build `Debug|x64`, or build the root solution at [G:\workspace\projects\readDriver\readDriver.sln](/G:/workspace/projects/readDriver/readDriver.sln).

Expected debug output:

- [G:\workspace\projects\readDriver\build\controller\x64\Debug\controller.exe](/G:/workspace/projects/readDriver/build/controller/x64/Debug/controller.exe)

## Usage

The controller expects exactly six arguments:

```text
controller.exe <srcPid> <srcAddress> <targetPid> <targetAddress> <size> <operation>
```

All arguments are parsed as unsigned integers with base auto-detection, so both decimal and `0x` hexadecimal values are accepted.

Examples:

```text
controller.exe 5412 0x6520A9F5 6512 0x0203B3F94940 15 0
controller.exe 5412 0x0203B3F94940 40000 0 0 3
controller.exe 5412 0x0203B3F94940 0 0 0 4
```

## Arguments

The request layout comes from [G:\workspace\projects\readDriver\common\protocol.h](/G:/workspace/projects/readDriver/common/protocol.h).

Fields sent by the controller:

- `srcPid`
  Mapped to `SourceProcessId`
- `srcAddress`
  Mapped to `SourceAddress`
- `targetPid`
  Mapped to `TargetProcessId`
- `targetAddress`
  Mapped to `TargetAddress`
- `size`
  Mapped to `Size`
- `operation`
  Mapped to `Operation`

## Operation Codes

Current operation values:

- `0`
  `ReadDriverOperationCopy`
- `1`
  `ReadDriverOperationRead`
  Declared in the shared enum, but not currently handled by the driver dispatch path.
- `3`
  `ReadDriverOperationProtect`
- `4`
  `ReadDriverOperationPatch`

## Current Semantics

The controller always sends the same six fields, but the driver interprets them differently depending on `operation`.

### Copy (`operation = 0`)

Use:

```text
controller.exe <srcPid> <srcAddress> <targetPid> <targetAddress> <size> 0
```

Meaning:

- `srcPid`: source process ID
- `srcAddress`: source virtual address
- `targetPid`: target process ID
- `targetAddress`: destination virtual address
- `size`: number of bytes to copy

### Protect (`operation = 3`)

Use:

```text
controller.exe <pid> <address> <regionSize> 0 0 3
```

Meaning in the current driver implementation:

- `srcPid`: process ID to open
- `srcAddress`: base address
- `targetPid`: region size
- `targetAddress`: ignored
- `size`: ignored

Example:

```text
controller.exe 5412 0x0203B3F94940 40000 0 0 3
```

### Patch (`operation = 4`)

Use:

```text
controller.exe <pid> <address> 0 0 0 4
```

Meaning in the current driver implementation:

- `srcPid`: process ID to modify
- `srcAddress`: address to patch
- remaining fields: ignored

Example:

```text
controller.exe 5412 0x0203B3F94940 0 0 0 4
```

## Expected Output

On success, the controller prints the request fields and a completion line similar to:

```text
Source PID:    5412
Source Addr:   0x6520A9F5
Target PID:    6512
Target Addr:   0x203B3F94940
Size:          15
Operation:     0
Request completed, bytes returned: 88
```

On failure, it prints one of:

- `CreateFile failed with error <code>`
- `DeviceIoControl failed with error <code>`
- `Failed to parse one or more numeric arguments.`

## Requirements

Before using the controller:

- the driver must already be loaded
- the driver must expose the DOS device name `\\\\.\\PWN29`
- the controller and driver must use the same `READDRIVER_REQUEST` layout from `common/protocol.h`

## Source Layout

- [G:\workspace\projects\readDriver\controller\controller\main.c](/G:/workspace/projects/readDriver/controller/controller/main.c)
  Main flow.
- [G:\workspace\projects\readDriver\controller\controller\cli.c](/G:/workspace/projects/readDriver/controller/controller/cli.c)
  Usage text, argument parsing, request summary printing.
- [G:\workspace\projects\readDriver\controller\controller\transport.c](/G:/workspace/projects/readDriver/controller/controller/transport.c)
  Device open and `DeviceIoControl` transport.
- [G:\workspace\projects\readDriver\controller\controller\controller.h](/G:/workspace/projects/readDriver/controller/controller/controller.h)
  Shared controller declarations.
