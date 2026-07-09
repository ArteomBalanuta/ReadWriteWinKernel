# Critical Diagnostic Note: DeviceIoControl Error 87

## Symptom

The native controller can print:

```text
DeviceIoControl failed with error 87
```

Win32 error `87` is `ERROR_INVALID_PARAMETER`. In this project, that usually means the IOCTL reached the driver and the driver completed the request with an invalid-parameter style status.

## Evidence

The controller opens the driver device and sends one buffered request:

- `controller/controller/transport.c` opens `READDRIVER_USER_DEVICE_NAME`.
- `controller/controller/transport.c` calls `DeviceIoControl` with `READDRIVER_IOCTL_REQUEST`.
- `controller/controller/main.c` uses a response buffer sized as `sizeof(READDRIVER_REQUEST)`.

The driver uses `METHOD_BUFFERED`, so the input and output data share `irp->AssociatedIrp.SystemBuffer`.

In `dummy/dispatch.c`, the request pointer is assigned from `SystemBuffer`, and the same `SystemBuffer` is also passed as the response buffer:

```c
*request = (PREADDRIVER_REQUEST)irp->AssociatedIrp.SystemBuffer;
...
status = DispatchReadDriverOperation(request, irp->AssociatedIrp.SystemBuffer, bytesReturned, silentMode);
```

Inside `DispatchReadDriverOperation`, the response buffer is cleared before the request fields are consumed:

```c
if (responseBuffer != NULL) {
    RtlZeroMemory(responseBuffer, sizeof(READDRIVER_REQUEST));
}

switch ((READDRIVER_OPERATION)request->Operation) {
```

Because the request and response pointers alias the same `SystemBuffer`, clearing the response also clears the input request.

## Likely Failure Flow

1. The controller sends a populated `READDRIVER_REQUEST`.
2. The driver maps the request to `irp->AssociatedIrp.SystemBuffer`.
3. The driver clears the response buffer.
4. The request fields are cleared too, because input and output share the same buffer.
5. `Operation` becomes `0`.
6. The dispatcher enters the operation-zero path with zeroed addresses and size.
7. The memory operation rejects the cleared request as invalid.
8. `DeviceIoControl` fails and `GetLastError()` reports `87`.

## Does Error 87 Mean The Driver Loaded?

If the controller printed `DeviceIoControl failed with error 87`, then `CreateFile` already succeeded. That means the user-mode path resolved to an open device handle.

So yes: in this specific controller flow, the driver is very likely loaded, the device object/link is reachable, and the IOCTL dispatch path is being invoked.

It does not prove that every driver operation is correct. It only proves that the controller got past device open and reached a request path that failed validation or request handling.

If the driver were not loaded or the symbolic link were not reachable, the expected failure would usually happen earlier at `CreateFile`, not at `DeviceIoControl`.

## Other Valid Error-87 Sources

After the buffer-aliasing issue is addressed or ruled out, error `87` can still come from invalid request data, for example:

- zero address where an operation requires a non-null address
- zero transfer size
- unsupported integer width for the integer-write operation
- invalid scan pattern length
- request fields that do not match the selected operation's expected contract

Those are secondary possibilities. The shared `SystemBuffer` clearing issue is the first thing to verify for this exact symptom.
