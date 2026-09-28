# TimedTask__Finalize

> Renamed from `Class86668__Finalize` on 2026-09-26 (tools/rename.py). Address 0x8004a228.

> Renamed from `TimedTask__Dtor` on 2026-09-25 (tools/rename.py). Address 0x8004a228.

> Renamed from `func_8004A228` on 2026-09-23 (tools/rename.py). Address 0x8004a228.

**Unit:** DayTaskStageMap · **Size:** 27 words (0x6C bytes) · **Status:** MATCHED (27/27 words)

## What it does

Method-table slot +0x00C (the dtor override) of `gTimedTaskMethods`, a sibling
class of `gDayTaskMethods` (resolved with
`tools/classtable.py 0x800865C8 --vs 0x8006E878`, then cross-checked against
`0x80086668` -- both share the same base, `gIntermediateBaseMethods`). If `self->unk30` is
set, notifies `self->subB` (guarded slot, same pattern as
`include/GameApplication.h`'s `unk18`/`GameApplication__InitSystems` comment). Then chains to the
BASE class's own dtor, fetched through `Get_vtable_IntermediateBase()` (a plain
no-parameter accessor returning `&gIntermediateBaseMethods`, same shape as
`GetDreamSysMethods`).

## Derivation

```
move  $s0, $a0
lw    $v0, 0x30($s0)
beqz  $v0, .L8004A268
 nop
lw    $a0, 0x34($s0)
lw    $v0, 0x0($a0)
lw    $v0, 0x4($v0)
jalr  $v0
 nop
.L8004A268:
jal   Get_vtable_IntermediateBase
 nop
lw    $v0, 0xC($v0)
jalr  $v0
 move $a0, $s0
```

Written as:

```c
void TimedTask__Finalize(Obj865C8 *self) {
    if (self->unk30 != 0) {
        self->subB->methods->slot4(self->subB);
    }
    Get_vtable_IntermediateBase()->dtor(self);
}
```

`IntermediateBaseMethods` (the base table's own type, only slots +0x00C,
+0x048, +0x060 typed -- the three this unit calls through explicitly) and
`SubObjB` (opaque, only slot +0x004 named) are established in
`include/DayTaskStageMap.h`, added this round.

## Proposed learning

**A sibling class can be identified by classtable.py --vs against the SAME
base twice, not just against each other.** `gDayTaskMethods` and `gTimedTaskMethods`
share a long run of identical slot values from +0x058 through +0x070 (not
because one subclasses the other, but because both independently override
those slots with the SAME shared implementation while diverging elsewhere,
e.g. +0x008/+0x00C/+0x040/+0x044/+0x048). Diffing both against their common
base (`gIntermediateBaseMethods`) rather than against each other avoids misreading shared
inherited/override code as a subclass relationship.

## Naming

`TimedTask__Dtor` -- tier A. The sibling class's own dtor override (+0x00C), releasing `subB` when owned and forwarding to the base dtor.

## Track 4

2026-09-25, round 84 (bravo): class unified in `include/TimedTask.h`. Renamed from `TimedTask__Dtor`, tier A: it occupies +0x00C, BasicClass's `finalize` slot, and its body is a finalize: release `sound` when `soundBankPath` is set (the ctor made it), then the base finalize through `Get_vtable_IntermediateBase()->finalize`. `self` is `TimedTask *`; `unk30`/`subB` are `soundBankPath`/`sound`. DayTask__Finalize and ObjM__Finalize reach it as `GetTimedTaskMethods()->finalize((TimedTask *)self)`. Image byte-identical.
