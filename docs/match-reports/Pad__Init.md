# Pad__Init

> Renamed from `func_80025C84` on 2026-09-24 (tools/rename.py). Address 0x80025c84.

**Unit:** `src/app/pad.c` (runner ALPHA, `runner/alpha`)
**Status:** MATCHED (16/16 words, full build verified byte-exact)
**Vtable slot:** `gPadMethods+0x40` (`PadMethods.init`)

## Context

Called from the constructor (`Pad__Pad`) as the last step. Boolifies
the incoming port argument into `self->port`, zeroes the three mask fields,
then tail-calls `self->methods->loadButtonTable()` (slot `+0x50`,
`Pad__LoadButtonTable`) through the vtable.

## Final C

```c
void Pad__Init(Pad *self, s32 port) {
    self->port = (port != 0);
    self->heldMask = 0;
    self->releasedMask = 0;
    self->pressedMask = 0;
    self->methods->loadButtonTable();
}
```

## Derivation notes

- Retail's `sltu $a1, $zero, $a1` is exactly `(port != 0)` computed as an
  unsigned 0/1 result, stored with `sh` into the `u16 Pad.port` field --
  matches directly.
- The trailing call is a true tail call (`jalr`, then straight to the
  epilogue with no move into `$v0`): retail's own `$v0` at return is
  whatever `Pad__LoadButtonTable` (`loadButtonTable`) left there, which is itself a
  loop-condition artifact, not an authored return value (see
  `Pad__LoadButtonTable`'s report). Declared `init` and `loadButtonTable` both
  `void` in `PadMethods` -- consistent with the two already-matched no-op
  slots in this same table (`Pad__NoOpSlot4C`/`Pad__NoOpSlot54`, `+0x4C`/`+0x54`,
  both `void`). First-try full match with this typing; no register or
  reshaping fight needed.
- Confirms field zeroing order (`heldMask`, `releasedMask`, `pressedMask` --
  offsets 0x10, 0x14, 0x18 in that order) matches the struct layout derived
  from `Pad__UpdateMasks`.

### Proposed learning

None beyond what's already recorded for `Pad__LoadButtonTable`/`Pad__UpdateMasks`.

## Naming

**Tier A.** Vtable slot `+0x40`, already named `init` in the struct (the
project's `Class__Method` convention where `Method` matches the slot name).
Called once, as the last step of `Pad__Pad`. Mechanics (boolify port, zero
the three edge masks, load the button table) and purpose (finish setting up
one Pad instance for a given controller port) are both evident from the body.
