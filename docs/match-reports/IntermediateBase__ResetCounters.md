# IntermediateBase__ResetCounters — MATCH (3/3 words)

> Renamed from `Obj86B60__ResetCounters` on 2026-09-25 (tools/rename.py). Address 0x8003e100.

> Renamed from `func_8003E100` on 2026-09-19 (tools/rename.py). Address 0x8003e100.

**Unit:** code_2cc8c_c · **Size:** 3 instructions

## What it does

Zeroes two `Obj86B60` fields, both already documented in
`include/code_2cc8c.h` from other functions in this class family.

## The C

```c
void IntermediateBase__ResetCounters(Obj86B60 *self)
{
    self->unk1C = 0;
    self->unk20 = 0;
}
```

## Provenance

round 12 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.

## Naming

**IntermediateBase__ResetCounters** (renamed from `func_8003E100`, round 55, runner
alpha). Tier B: mechanics fully known (zeroes `self->unk1C` and
`self->unk20`) and dispatched as `Obj86B60Methods::resetCounters`
(`+0x040`, exclusive to this unit, renamed from `slot40`) right after the
vtable pointer is installed, i.e. a post-ctor reset hook. `unk1C` is
independently established elsewhere (`TaskCore__TickColorFade`, code_2cc8c.c) as "a
running count/frame value", supporting "counters" as the mechanic; the
in-game PURPOSE of that counter (what visual/timing effect it drives) is
not established, hence tier B rather than A.

## Track 4 (2026-09-25, round 82, charlie)

The class is IntermediateBase (class id 0x30, gIntermediateBaseMethods; `tools/classtable.py gIntermediateBaseMethods` lists this function as one of its own occupants), declared once in include/IntermediateBase.h. `self` is now `IntermediateBase *`, not TaskCore's `Obj86B60` view; byte-identical. Renamed from Obj86B60__ResetCounters (class prefix). Occupies +0x040 (slot `resetCounters`); the ctor's last call. Clears frameCounter (+0x01C) and state (+0x020, was unk20: IntermediateBase__SetState stores its argument there). Overrides: TaskCore__Reset, TimedTask__CancelTimeout, DayTask__ResetPhase, StreamTask__Reset.
