# Pad__UpdateMasks

> Renamed from `func_80025CC4` on 2026-09-24 (tools/rename.py). Address 0x80025cc4.

**Unit:** `src/class_16334.c` (runner ALPHA, `runner/alpha`)
**Status:** MATCHED (19/19 words, full build verified byte-exact)
**Vtable slot:** `D_8006D370+0x44` (`PadMethods.updateMasks`)

## Context

Reads the current button state via `func_80025EFC(self->port)` (a Psy-Q
`psyq_PadInit` helper, uncarved -- it internally calls `func_80025F4C` then
returns `~D_8008B3C8`, i.e. the raw hardware bits are active-low and this
negates them to active-high), then derives the classic
held/pressed/released edge masks against the previous held mask.

## Final C

```c
u32 Pad__UpdateMasks(Pad *self) {
    u32 newMask;
    u32 oldMask;
    u32 changed;

    newMask = func_80025EFC(self->port);
    oldMask = self->heldMask;
    self->heldMask = newMask;
    changed = newMask ^ oldMask;
    self->releasedMask = changed & oldMask;
    self->pressedMask = changed & newMask;
    return newMask;
}
```

## Derivation notes

- `self->port` (`Pad.port`, offset 0xC, `u16`) is read with `lhu` in retail
  and passed as the port/pad-index argument -- confirms the ctor's boolified
  field (see `Pad__Pad`'s report) really is used as a small integer
  selector downstream, not just a flag.
- The retail body never re-derefs `self->heldMask` after the assignment; the
  return value is the same register the new mask was computed into
  (`func_80025EFC`'s return, `$v0`, is never clobbered before the epilogue).
  Writing the C to compute `newMask` into a local (not re-reading
  `self->heldMask` for the return) reproduced that directly -- first-try
  full match, no register-identity fighting needed.
- Confirms the `Pad.heldMask/releasedMask/pressedMask` (0x10/0x14/0x18)
  layout guessed from `Pad__Init`.

### Proposed learning

None beyond what's already in `Pad__LoadButtonTable`'s report.
