# StageMap__UpdateIfEnabled — MATCH

> Renamed from `Class866E8__UpdateIfEnabled` on 2026-09-26 (tools/rename.py). Address 0x8004ab24.

> Renamed from `func_8004AB24` on 2026-09-22 (tools/rename.py). Address 0x8004ab24.

**Unit:** class_3ac78 · **Size:** 25 instructions · **Result:** 25/25 words

## What it does

A guard-and-dispatch method: if `self->unk70` is set, calls two of its own
vtable slots (`+0xF4`, `+0x13C`) back to back with just `self`.

Slot resolution used `tools/classtable.py gStageMapMethods` (the whole vtable was
resolved in the previous commit for this unit): `+0xF4` -> `StageMap__UpdateFootprintTracking`,
`+0x13C` -> `StageMap__StepScaleRamp`. Neither is decompiled yet; only the slot
existence/signature (self-only) was needed here.

## Final source

```c
void StageMap__UpdateIfEnabled(StageMap *self)
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

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004AB24` | `StageMap__UpdateIfEnabled` | B | Occupant of vtable slot `+0x098`. The gate field is `+0x070`, and `class_3bb8c`'s two matched accessors pin its meaning exactly: `StageMap__Enable` sets it to 1 and `StageMap__Disable` runs `slotC0` and then clears it to 0 -- an enable/disable pair. When enabled this function dispatches `slotF4` then `slot13C`; `slot13C` is `class_3bb8c_b`'s matched `StageMap__StepScaleRamp`, which decrements a per-object countdown and sweeps every element's cells, i.e. periodic work. Tier B: "update" describes what the two dispatched slots do, not a purpose anyone has established. |

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `StageMap+0x070` | `enabled` | A | Set to 1 / cleared to 0 by a matched setter pair in `class_3bb8c`, and used as a plain boolean gate here. A pure flag whose mechanics are its purpose. |

`slotF4` and `slot13C` keep their `slotNN` names: their occupants
(`StageMap__UpdateFootprintTracking`, `StageMap__StepScaleRamp`) are still `func_` in `class_3bb8c`, and
the convention is to name a slot after the method it dispatches to.
