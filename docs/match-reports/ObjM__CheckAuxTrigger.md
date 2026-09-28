# ObjM__CheckAuxTrigger

> Renamed from `func_80054120` on 2026-09-23 (tools/rename.py). Address 0x80054120.

**Unit:** ObjMStyleActor · **Size:** 43 instructions · **Status:** MATCHED (43/43 words)

## Context

Establishes several new types: `self->unk14` (`FieldM14`, vtable slot
`0x114`, returning a new `ChildM114*`), `ChildM114`'s own fields (`unk4`
a `SubM4*`, `unk14` an `s32` set from this function's own external call),
`SubM4` (`unk34` an `s32`, vtable slot `0x84`), and calls the uncarved
sibling `TryDreamAuxTrigger` (`src/world/dream_aux.c`, still `INCLUDE_ASM`) declared
via a local `extern` prototype in `include/class_3bb8c.h`, typed purely
from this call site's own register setup.

## What this function does

```c
s32 ObjM__CheckAuxTrigger(ObjM *self) {
    s32 out;
    s32 result;
    ChildM114 *child = self->unk14->methods->slot114(self->unk14, &out);
    void *thing = self->unk3C->methods->slot1A0(self->unk3C, 0);
    result = TryDreamAuxTrigger(child->unk4->unk34, &out, thing);
    child->unk14 = result;
    if (result != 0) {
        return 0;
    }
    child->unk4->methods->slot84(child->unk4);
    return 1;
}
```

`self->unk3C->methods->slot1A0(self->unk3C, 0)`'s return value is used
directly (never named — it's `TryDreamAuxTrigger`'s own 3rd argument), while
`self->unk14->methods->slot114(...)`'s return value (`child`) is the one
that gets a real local and is used repeatedly across the intervening
call.

## Two residues, found in order

1. **Register identity, not a real defect — CORRECT variable, WRONG
   register source.** First draft passed `child` itself (the `slot114`
   return, already named and stored to a local) as `TryDreamAuxTrigger`'s 3rd
   argument. That compiled to `move $a2, $s0` (reloading `child` from its
   callee-saved home). Retail instead has `move $a2, $v0` — reading the
   value straight out of `$v0`, where it happens to still be sitting
   because `self->unk3C->methods->slot1A0(...)`'s OWN return (a
   *different*, never-named value) was the last thing written to `$v0`
   and nothing overwrote it before this point. **The two values are not
   the same thing that happens to share a register by luck** — `$v0` at
   the call site is `slot1A0`'s return, not `child`. Naming that return in
   a local (`thing`) and passing IT (not `child`) as the 3rd argument
   reproduced retail's register exactly, because it matches what the
   source actually computes and uses there.

   **General lesson, adds to the existing "value in an argument register
   survives an intervening call" family**: when a register holds a
   plausible value right where an argument is needed, check what
   COMPUTED it, not just whether the value would work. Two different
   call returns can occupy the same register at nearby points without
   being interchangeable in the source.

2. **Branch polarity — the early-exit-guard idiom, confirmed again.**
   Writing the tail as `if (result == 0) { call; return 1; } return 0;`
   compiled with the guard branch aimed at a small near offset (testing
   into the body) instead of retail's far skip-branch past the whole
   block. Rewriting as the inverted guard-clause form —
   `if (result != 0) { return 0; } call; return 1;` — matched immediately.
   This is the same idiom already recorded in DECOMPILATION_LEARNINGS
   ("Write a small early exit as an inverted guard clause, not as the
   `else` of a big `if`"), now confirmed once more on a bare `if`/`return`
   with no `else` at all.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `ObjMStyleActor`.

## Naming

**ObjM__CheckAuxTrigger** -- tier B. Fetches a `ChildM114` via `self->unk14`'s `slot114`, reads the current day/year from `dreamSys->getCurrentDayAndYear`, calls the uncarved `TryDreamAuxTrigger` with both, stores the result, and finalizes the child (`slot84`) on failure. Named for its one external call; the gameplay trigger itself is still uncarved (`src/world/dream_aux.c`), so this stays tier B.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the ObjMStyleActor/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and DayTaskStageMap.h's Obj4C/SubObjB/EventArg are gone. Byte-identical. `unk14` is the StageMap: +0x114 is getLastTargetRateSplit, returning a ChunkSlot whose +0x004 is `loader` (the LbdFile) and +0x014 `heldObj`, which takes TryDreamAuxTrigger's result (cast to BasicClass *).

## Track 7 (2026-09-27, round 98, delta)

The out-parameter getLastEventSlotChunk fills was an `s32` passed as
`(u8 *)&out`; StageMap__SplitChunkIndex writes two bytes into it (column,
row), and TryDreamAuxTrigger reads the pair as one `s16` key. It is now a
unit-local `ChunkCoord { u8 column; u8 row; }` passed as `&coord.column`,
and TryDreamAuxTrigger's local prototype takes `(s32 data, ChunkCoord
*coord, s32 day)` -- its definition (src/world/dream_aux.c) takes `(s32, s16 *,
s32)`, and the third argument is getCurrentDayAndYear's s32, not the
`void *` the old prototype said. Locals `elem`/`result`/`thing` became
`slot`/`held`/`day`. Byte-exact on the first build: the two-byte struct
takes the same stack slot as the s32.

Moved from the source comment: "ObjM__CheckAuxTrigger's one external call,
src/world/dream_aux.c (MATCHED; its definition reads the second argument as
`s16 *`)."
