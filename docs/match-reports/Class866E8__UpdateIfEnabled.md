# Class866E8__UpdateIfEnabled — MATCH

> Renamed from `func_8004AB24` on 2026-09-22 (tools/rename.py). Address 0x8004ab24.

**Unit:** class_3ac78 · **Size:** 25 instructions · **Result:** 25/25 words

## What it does

A guard-and-dispatch method: if `self->unk70` is set, calls two of its own
vtable slots (`+0xF4`, `+0x13C`) back to back with just `self`.

Slot resolution used `tools/classtable.py D_800866E8` (the whole vtable was
resolved in the previous commit for this unit): `+0xF4` -> `func_8004B5BC`,
`+0x13C` -> `func_8004D028`. Neither is decompiled yet; only the slot
existence/signature (self-only) was needed here.

## Final source

```c
void Class866E8__UpdateIfEnabled(Class866E8 *self)
{
    if (self->unk70) {
        self->methods->slotF4(self);
        self->methods->slot13C(self);
    }
}
```

`self->unk70` (a plain `s32`, offset 0x6C..0x74 previously undifferentiated
padding) is new struct knowledge, added to `include/class_3ac78.h`.

## Residue

None — matched on the first attempt. The one thing worth noting: the first
call's `$a0` is never re-set before the `jalr` (the compiler reuses the
still-live entry value, matching `self` being unmodified since function
entry); the SECOND call explicitly reloads `$a0` from `$s0` even though it
holds the identical value, presumably because `$a0` is caller-saved across
the intervening call and the compiler doesn't track its liveness past a
`jalr`. Writing the two calls as plain sequential statements reproduced
this without any special handling.

## Provenance

round 2026-09-02, runner ALPHA, unit class_3ac78.
