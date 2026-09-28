# TaskCore__SetFadeOutCallbackEnabled — MATCH (14/14 words)

> Renamed from `Obj86B60__func_8003CB30` on 2026-09-25 (tools/rename.py). Address 0x8003cb30.

> Renamed from `func_8003CB30` on 2026-09-24 (tools/rename.py). Address 0x8003cb30.

**Unit:** Task · **Size:** 14 instructions

## What it does

Structurally identical to `TaskCore__SetFadeCallbackEnabled` (see that report for the residue
and its fix, applied directly here on the first attempt):

```c
void TaskCore__SetFadeOutCallbackEnabled(Obj86B60 *self, s32 a1)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    switch (a1) {
    case 0:
        self->unk8C = NULL;
        break;
    case 1:
        self->unk8C = methods->slotC4;
        break;
    }
}
```

Sets the `unk8C` callback (paired with `TaskCore__TickFadeOutCallback`, which invokes it)
to this class's own vtable slot `+0xC4` (external `TaskCore__TickFadeOut`).

## Struct knowledge established

- `Obj86B60::unk8C` (`s32 (*)(Obj86B60*)`, +0x08C) -- OBSERVED here.
- `Obj86B60Methods::slotC4` (+0x0C4, external `TaskCore__TickFadeOut`) -- read as
  DATA here, same idiom as `TaskCore__SetFadeCallbackEnabled`'s `slotB0`.

## Provenance

round 2026-09-02, runner echo, unit Task. 1 attempt.

## Naming (round 78, delta)

**Tier C.** `func_8003CB30` -> `Obj86B60__func_8003CB30`. Same
enable/disable-toggle SHAPE as `TaskCore__SetFadeCallbackEnabled` immediately
above (`self->unk8C = NULL` or `= methods->slotC4`), but `slotC4`'s occupant
(`TaskCore__TickFadeOut`) is EXTERNAL -- not in this unit, its body has not been
read here -- so unlike `unk88`/`TickColorFade` there is no basis to call this
one a "fade" callback too. Kept at the tier-C class-scoped form rather than
reusing "Fade" on a guess.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__func_8003CB30 (tools/rename.py). Occupant of +0x0A0 (`setFadeOutCallbackEnabled`). Named on the cross-unit evidence the round-78 naming pass lacked: the function it installs at +0x08C (now `fadeOutCallback`), +0x0C4's TaskCore__TickFadeOut, computes 0x80 - frameCounter * fadeRate, the mirror of the fade-in callback's base + frameCounter * fadeRate. The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 98, alpha)

The early `methods` load (the residue above, or its sibling's) carries a MATCHING line. Byte-identical.
