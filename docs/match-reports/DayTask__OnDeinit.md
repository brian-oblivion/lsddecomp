# DayTask__OnDeinit

> Renamed from `Class865C8__OnDeinit` on 2026-09-26 (tools/rename.py). Address 0x80049c50.

> Renamed from `Obj865C8__RunSubUpdates` on 2026-09-26 (tools/rename.py). Address 0x80049c50.

> Renamed from `func_80049C50` on 2026-09-23 (tools/rename.py). Address 0x80049c50.

**Unit:** DayTaskStageMap · **Size:** 22 words (0x58 bytes) · **Status:** MATCHED (22/22 words)

## What it does

Method-table slot +0x050 of `gDayTaskMethods` (`Obj865C8`, see
`include/DayTaskStageMap.h`). Reads `self->subA`, then dispatches two calls
through THAT sub-object's own vtable, slots +0x090 then +0x074, both with
just the sub-object as argument.

## Derivation

```
lw    $s0, 0x18($a0)     ; s0 = self->subA
lw    $v0, 0x0($s0)      ; v0 = subA->methods
lw    $v0, 0x90($v0)
jalr  $v0
 move $a0, $s0
lw    $v0, 0x0($s0)      ; reload subA->methods
lw    $v0, 0x74($v0)
jalr  $v0
 move $a0, $s0
```

Written as:

```c
void DayTask__OnDeinit(Obj865C8 *self) {
    SubObjA *sub = self->subA;

    sub->methods->slot90(sub);
    sub->methods->slot74(sub);
}
```

`SubObjA` is an opaque, minimally-typed view (vtable pointer at offset 0,
only the two slots this function dispatches through named) -- same policy as
`DreamSysEntityObj` in `include/dream_sys.h`. Nothing here identifies which
concrete class `subA` points to; both slot numbers exceed `gDayTaskMethods`'s own
33-slot table, so it is a genuinely different class, not a self-dispatch.

## Proposed learning

None beyond what's already documented.

## Naming

`DayTask__OnDeinit` -- tier A. Ticks `subA` (`slot90`/`slot74`) every call; matches the existing `runSubUpdates` field name already on file. A pure per-frame forwarding leaf: mechanics are its purpose.

## Track 4 (2026-09-26, round 88, DayTask)

The class (table D_800865C8, id 0x1F230, TimedTask's subclass) is unified as DayTask in include/DayTask.h; the Obj865C8/DayTaskMethods views in DayTaskStageMap.h are gone. Renamed from Obj865C8__RunSubUpdates, which misdescribed it: it is the +0x050 onDeinit override, and the two calls are Viewport's deinitOt and detachViewChild.
