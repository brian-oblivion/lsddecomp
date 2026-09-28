# Actor__MoveLocalZ -- MATCHED (14/14 words)

> Renamed from `BaseObjO__func_5748c` on 2026-09-25 (tools/rename.py). Address 0x8005748c.

> Renamed from `func_8005748C` on 2026-09-18 (tools/rename.py). Address 0x8005748c.

Unit: `ObjMStyleActor` (round 17). A shared `BasicClass`-inherited slot
occupant (`slotC4`), already independently confirmed `void` from BOTH
`Entity.h` and `TodActor.c`'s `TodActorMethods::slotC4` (both tables
hold this exact function at `+0xC4`, per `Entity.h`'s own comment). Tail-
calls `Actor__MoveAlongLocalAxis` (still `INCLUDE_ASM`, sibling unit
`ObjMStyleActor`) with a fixed global address and a literal `6`.

## Final source

```c
extern s32 gActorLocalMoveZ;
extern void Actor__MoveAlongLocalAxis(BaseObjO *self, void *arg0, s32 arg1, s32 arg2, s32 arg3);

void Actor__MoveLocalZ(BaseObjO *self, s32 arg1, s32 arg2) {
    Actor__MoveAlongLocalAxis(self, &gActorLocalMoveZ, arg1, arg2, 6);
}
```

## Derivation

Register setup before the `jal`: `$a0` untouched (still `self`), `$a1` =
`&gActorLocalMoveZ` (freshly computed, overwriting the incoming `arg1`'s old
register), `$a2` = the ORIGINAL `arg1` (saved into `$a2` before `$a1` is
overwritten), `$a3` = the original `arg2`, and one stack word (`$sp+0x10`)
= the literal `6`. This is a genuine 5-argument call (4 registers + 1
stack slot, o32 ABI), matching `Actor__MoveAlongLocalAxis(self, &gActorLocalMoveZ, arg1,
arg2, 6)` written left-to-right in C.

**Kept `void`, a bare statement call rather than `return Actor__MoveAlongLocalAxis(...)`
-- per CLAUDE.md's own explicit warning about this exact slot.**
`Entity.h`'s comment on `EntityMethods::slotC4` documents that retyping
this SHARED slot to a non-`void` return breaks an ALREADY-MATCHED sibling
function (`Entity__MoodCue00`, which relies on GCC tail-merging two identical
`void`-typed `slotC4(this,0x32,0)` call sites reached from different
branches -- a non-`void` return stops the merge and costs that function 4
words). `Actor__MoveAlongLocalAxis` itself is declared `void` here purely as a local
call-site typing choice consistent with that constraint; its own real
return type (if any) is unconfirmed and irrelevant to this call site, which
discards it either way.

`gActorLocalMoveZ` is declared as an arbitrary scalar (`extern s32 gActorLocalMoveZ;`)
since only its address is ever taken here, never its value.

### Proposed learning

None -- a direct application of the already-documented `slotC4`-must-stay-
`void` constraint to a NEW occupant of the same shared slot, confirming it
generalizes past the two instances (`Entity__MoodCue32`, `Entity__MoodCue00`)
`Entity.h` already names.

## Naming

**`Actor__MoveLocalZ` -- tier C.** Class is known (occupies the shared
`slotC4` `BasicClass`-inherited slot, confirmed by `tools/classtable.py`
against `Entity.h`/`TodActor.c`'s independent readings of the same
address), but the function tail-calls a still-`INCLUDE_ASM` sibling-unit
function (`Actor__MoveAlongLocalAxis`, `ObjMStyleActor.c`) with a fixed global address
and a literal mode value `6`, and no occupant of `slotC4` anywhere in the
codebase has an established purpose either (`Entity.h`'s own comment on
this exact slot only documents a MUST-STAY-`void` return-type constraint,
not what the slot means). Kept the tier-C `Class__func_xxxxx` form.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `BaseObjO__func_5748c`. Occupant of +0x0C4: Actor__MoveAlongLocalAxis(self, &gActorLocalMoveZ, val, notify, 6). gActorLocalMove is an s16 vector passed whole to addLocalTranslation (RotateLocalVector reads src[0..2]), so ABA4/ABA6/ABA8 are x/y/z and this is the z move. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/ObjMStyleActor.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 99, alpha)

`python3 tools/rename.py D_8008ABA8 gActorLocalMoveZ`, **tier A**: the s16 at 0x8008ABA8, right after gActorLocalMove's x (ABA4) and y (ABA6), used as the axis here as they are in MoveLocalX/Y. The event is ACTOR_EVENT_MOVED_Z (enum ActorMoveEvent, include/Actor.h).
