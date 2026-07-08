# Safe Protocol Clarity Pass Design

This design implements only the maintainability-oriented portions of `IMPROVE.md`.

## Scope

Included:

- clarify the shared protocol contract in `common/protocol.h`
- document the operation gap and current per-operation field reuse
- align the C# mirror request struct with the native header naming
- update docs so the current protocol behavior is explicit and easier to maintain

Excluded:

- new read-back operations
- new write operations
- pattern scanning
- batching
- silent or stealth-focused behavior changes
- changing IOCTL transport method
- enum renumbering or compatibility-breaking protocol redesign

## Approach Options

### Option 1: Docs-only

Update markdown docs and leave source comments unchanged.

Pros:
- zero runtime risk
- smallest change set

Cons:
- source-level confusion remains in the header and C# mirror struct

### Option 2: Compatibility-safe contract clarification

Update the shared header comments, the managed mirror struct names/comments, and supporting docs without changing operation values or runtime behavior.

Pros:
- removes current ambiguity where developers actually read the contract
- keeps wire format and behavior intact
- low risk

Cons:
- does not solve the deeper protocol-design limitations

### Option 3: Full protocol redesign

Adopt the redesigned struct and operation semantics proposed in `IMPROVE.md`.

Pros:
- cleaner long-term shape

Cons:
- breaks compatibility across driver and both clients
- materially expands capability surface
- out of scope for a safe cleanup pass

## Recommendation

Use Option 2.

It addresses the real maintainability problem without changing the repo's behavior or adding new memory-access capabilities.

## Design

### Shared Contract

Keep the existing `READDRIVER_REQUEST` layout and operation numbers. Add concise comments that explain:

- operation `2` is intentionally unused in the current protocol numbering
- the request remains fixed-width for ABI stability
- operation `5` reuses `SourceAddress` as the integer payload

### Managed Mirror

Rename the reserved fields in `client/DriverApi.cs` to match `common/protocol.h` and add short comments around the enum/struct so future updates do not drift between the native and managed contract definitions.

### Docs

Update `docs/PROJECT_OVERVIEW.md` and `docs/ARCHITECTURE.md` to describe:

- the current operation table
- the fact that op `1` is declared but not dispatched today
- the fact that op `2` is left unused
- the exact current operation `5` field mapping

## Verification

Run the root build script after edits:

```powershell
.\build-all.ps1 -Configuration Debug -Platform x64
```
