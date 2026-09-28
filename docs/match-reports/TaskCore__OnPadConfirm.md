# TaskCore__OnPadConfirm — MATCH (30/30 words)

> Renamed from `Obj86B60__func_8003C858` on 2026-09-25 (tools/rename.py). Address 0x8003c858.

> Renamed from `func_8003C858` on 2026-09-24 (tools/rename.py). Address 0x8003c858.

**Unit:** task · **Size:** 30 instructions

## What it does

```c
void TaskCore__OnPadConfirm(Obj86B60 *self, s32 a1)
{
    s32 reason;

    if (self->unk4C != NULL) {
        self->methods->slot70(self, 0x10);
        reason = 0xF;
        if (self->unk3C == 1) {
            reason = 0xB;
        }
        self->methods->slot60(self, reason);
    }
}
```

Same gate-then-forward family as `TaskCore__OnPadStart`, but the reason code passed
to `slot60` is picked between two literals based on `self->unk3C`.

**First attempt had the two literals backwards (28/30, two words swapped)**
-- I initially wrote "default `0xB`, override to `0xF` when `unk3C==1`"
by misreading which value the `bne`'s delay slot sets. Re-reading: the delay
slot of `bne $v1,$v0,.L8003C8A4` (branch taken when `unk3C != 1`) is
`ori $a1,$zero,0xF` -- delay slots execute UNCONDITIONALLY, so `reason = 0xF`
is the actual default set every time, and it is the FALL-THROUGH path
(`unk3C == 1`) that overwrites it to `0xB`. This is the documented
"default value, then conditionally overwritten" idiom
(DECOMPILATION_LEARNINGS.md), and the fix was purely swapping which literal
is the default vs. the override -- one attempt.

## Struct knowledge established

Nothing new beyond `TaskCore__OnPadStart`'s `unk4C`/`unk3C`/`slot70`/`slot60`;
confirms `unk3C == 1` is a real, meaningful state value (not just a
`!= 0` gate as `TaskCore__OnPadEvent`, STALL, treats it).

## Provenance

round 2026-09-02, runner echo, unit task. 2 attempts.

## Naming (round 78, delta)

**Tier C.** `func_8003C858` -> `Obj86B60__func_8003C858`. Message-0x19
handler (slot78, corrected occupant -- see the header). Body: when
`self->unk4C` is set, calls `slot70(self, 0x10)` then `slot60(self, reason)`
with `reason` = 0xB when `unk3C == 1`, else 0xF. Same "gate on unk4C, forward
to child, transition state" shape as its siblings; the specific reason codes
and message code have no independent evidence of game meaning. Tier C.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__func_8003C858 (tools/rename.py). Occupant of +0x078 (`onPadConfirm`, onPadEvent's 0x19 case). Named from what it reaches: setState(0xB) runs tick (inputMode 1: begin scrolling or finish on the target's unkC slot) and setState(0xF) commitElementScroll (inputMode 2). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/task_core.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 98, alpha)

`reason` -> `state`; `0x10`/`0xF`/`0xB`/`1` -> TASKCORE_TONE_BUTTON, TASKCORE_STATE_ITEM_CONFIRMED, TASKCORE_STATE_SLOT_CONFIRMED, TASKCORE_INPUT_CHOOSING_SLOT (include/task_core.h). Byte-identical.
