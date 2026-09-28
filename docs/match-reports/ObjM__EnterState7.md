# ObjM__EnterState7

> Renamed from `func_80053D18` on 2026-09-23 (tools/rename.py). Address 0x80053d18.

**Unit:** class_3bb8c_k · **Size:** 33 instructions · **Status:** MATCHED (33/33 words)

## Context

First function of `class_3bb8c_k`, the fourth 20-function slice carved from
the tail of the large `class_3bb8c` block (0x44518..0x44F14). This unit's
`self` object (`ObjM`, `include/class_3bb8c.h`) is a NEW, independent type —
its field offsets (0x10, 0x14, 0x18, 0x20, 0x3C, 0x80, 0x84) and its own
vtable slots (0x10, 0x14, 0x30, 0xB8, 0xD4) don't correspond to anything
already established for `class_3bb8c_b`'s `Obj866E8`, so it is kept
separate per the project's multiple-independent-local-views convention.

`self->unk3C` is a pointer to a second class (`FieldM3C`) with a rich
vtable; this function reaches two of its slots (`0xF0`, `0xFC`).

## What this function does

Sets `self`'s mode/state field to 7, asks `self->unk3C` to resolve some
value into a stack out-param via slot `0xF0`, forwards that value into the
unit's shared helper `ObjM__StartFadeUp` (matched this round, see its own
report) along with fixed literals `(0, 5, 1)`, then tells `self->unk3C` to
do something with no arguments via slot `0xFC`.

## The C

```c
void ObjM__EnterState7(ObjM *self) {
    s32 val;
    self->unk20 = 7;
    self->unk3C->methods->slotF0(self->unk3C, &val, -1);
    ObjM__StartFadeUp(self, val, 0, 5, 1);
    self->unk3C->methods->slotFC(self->unk3C);
}
```

`ObjM__StartFadeUp` is defined later in this same file (ROM order), so a local
forward `extern` prototype is added at the top of `class_3bb8c_k.c` ahead
of this function's definition (calling a not-yet-defined-in-this-TU
function is fine per DECOMPILATION_LEARNINGS' "Calling into a function
that is still INCLUDE_ASM in another unit" note — the same reasoning
applies to a forward reference within one unit).

## Residue

None — matched on the first attempt once `FieldM3C`'s vtable slots
(`0xF0`, `0xFC`) and `ObjM::unk3C`/`unk20` were typed correctly.

## Proposed learning

None beyond what's already recorded; this function's shape is
straightforward once the struct layout is right.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `class_3bb8c_k`.

## Naming

**ObjM__EnterState7** -- tier B. Sets `ObjM::mode = 7`, queries `dreamSys->getSetFlashbackSession(dreamSys, &val, -1)` (offset +0xF0, confirmed via `tools/classtable.py 0x80087BDC`), forwards the result into `ObjM__StartFadeUp`, then calls `dreamSys->blockMovement`. Mechanics are fully pinned down; the game-level meaning of "mode 7" is not, so this stays tier B rather than a guessed purpose name.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the class_3bb8c_k/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and class_39e08.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.

## Track 7 (2026-09-27, round 98, delta)

`self->state = OBJM_STATE_LINK_FLASHBACK` (enum ObjMState, include/ObjM.h,
added this round: every link state is the DreamSys code that started it
less 6, from ObjM__OnDreamSysNotify, and DayTask__OnObjMNotify's cases
agree). The out-parameter is the `DreamColors` getSetFlashbackSession
fills, so the old `s32 val` and its `(DreamColors *)` cast went. Zero bytes.

### The unit banner's history (moved from src/class_3bb8c_m.c)

The banner said: seventh carved slice of the class_3bb8c block
(0x44518..0x44F14, vram 0x80053D18..0x80054714), 20 functions, all 20
matched (0 INCLUDE_ASM, 0 NON_MATCHING). Carved round 15; all four former
toolchain-blocker functions matched round 23/44, once `addiu_at` and
`gp_rel` were resolved project-wide (CLAUDE.md, "Open toolchain
blockers"). The unit owns no switch jump table. Track 4 (round 89) unified
the class_3bb8c_k/_l/_m views of ObjM in include/ObjM.h. The banner also
said "include/class_3bb8c.h is SHARED with every other class_3bb8c_*
slice. Header edits must be strictly ADDITIVE" -- a rule for editors,
which lives in docs, not in the banner.
