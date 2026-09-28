# Actor__MoveLocalX -- MATCHED (14/14)

> Renamed from `DreamSys__ApplyOffsetSlot0` on 2026-09-25 (tools/rename.py). Address 0x800574c4.

> Renamed from `func_800574C4` on 2026-09-19 (tools/rename.py). Address 0x800574c4.

Unit: `src/ObjMStyleActor.c`. Class: `DreamSys`, own vtable slot `+0x0C8`
(base-class-inherited; resolved via `tools/classtable.py gDreamSysMethods`
and confirmed unchanged in the DreamSys-level table too).

## Signature

```c
void Actor__MoveLocalX(DreamSys *self, s32 val, void *extra);
```

## Body

```c
void Actor__MoveLocalX(DreamSys *self, s32 val, void *extra) {
    Actor__MoveAlongLocalAxis(self, &gActorLocalMove[0], val, extra, 7);
}
```

A thin wrapper: forwards to `Actor__MoveAlongLocalAxis` (this unit's own helper, see
its own match report) with a fixed pointer into element 0 of a
newly-identified 2-element `s16` array `gActorLocalMove`, and the constant `7`.

`val` is `s32`, not `s16`, even though `Actor__MoveAlongLocalAxis` only ever uses it
truncated to 16 bits -- see `Actor__MoveAlongLocalAxis`'s report for why: typing it
`s16` here forces a spurious `sll`/`sra` re-sign-extend pair at this call
site that retail does not have.

## `gActorLocalMove` is a 2-element `s16` array, not a lone `s32`

splat's single-word `dlabel gActorLocalMove` (`asm/data/7B008.sdata.s`) is really
`s16 gActorLocalMove[2]`: this function writes element 0
(`%hi/%lo(gActorLocalMove)`), its sibling `Actor__MoveLocalY` writes element 1
(`%hi/%lo(gActorLocalMove + 0x2)`). Not referenced anywhere else in the repo
(checked with `grep -rn gActorLocalMove src/ include/` before this round), so
declared locally in `src/ObjMStyleActor.c` rather than added to a shared
header.

## Naming

**`Actor__MoveLocalX` -- tier B.** Mechanics fully confirmed
(forwards to `Actor__MoveAlongLocalAxis` with a fixed pointer
into element 0 of the local `gActorLocalMove[2]` array and the constant `7`);
purpose of "why element 0, why 7" is not established. `Slot0` names the
array element this wrapper owns -- an objective, code-confirmed fact --
rather than guessing which axis or game concept it represents.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__MoveLocalX   # 14/14
```

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__ApplyOffsetSlot0`. Occupant of +0x0C8 in gActorMethods (the BASE table, so the method is Actor's, not DreamSys's): writes gActorLocalMove[0], the x of the local move vector (see Actor__MoveLocalZ), event 7. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/ObjMStyleActor.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 96, bravo)

Comments quoted below are verbatim as the file stood before this round's
comment pass, i.e. with this round's renames already applied (the
`LinkQueryBuf` one as it stood before step 2).

The unit banner of `src/ObjMStyleActor.c` was rewritten to say what the file
holds. The old one, verbatim:

```c
/*
 * ObjMStyleActor -- vram 0x800574C4..0x80057DBC, carved round 17
 * (2026-09-04), immediately behind ObjMStyleActor. Actor methods
 * (include/Actor.h; before round 82 they carried DreamSys's name, but they
 * are occupants of the BASE table gActorMethods, +0x0C8..+0x0EC) plus one
 * unrelated constructor:
 *
 *  - The local-axis moves (Actor__MoveLocalX/Y, the shared
 *    Actor__MoveAlongLocalAxis, and Actor__MoveLocalZOrFindLink /
 *    MoveLocalXOrFindLink through Actor__MoveOrFindNearbyLink): write one
 *    component of the local move vector gActorLocalMove, apply it through
 *    addLocalTranslation, clear it, and fall back to a grid-based
 *    nearby-link search (Actor__FindNearbyLink, Actor__BuildLinkQueries,
 *    Actor__ScanLinkCandidates, Actor__ScanGridWindow, AcceptGridElem) when
 *    the move alone did not set linkTarget.
 *  - The link-command pair (Actor__OnActorLinkCommand,
 *    Actor__OnGridCellLinkCommand) forwarding through the SceneNode base
 *    table and, for an event in [5,9), the object's own tryAttachNearby.
 *  - Actor__SetLastOffsetValue/SetPendingExtra, GetActorMethods: plain
 *    setters/getter.
 *  - New_VariantSprite + VariantSprite__VariantSprite: allocator and
 *    constructor of an unrelated class, VariantSprite (a Sprite subclass,
 *    include/VariantSprite.h).
 *
 * No stalls: Actor__BuildLinkQueries, the last one, matched in round 75
 * (2-argument method call, see its report). No switch jump table in this slice, and no gp_rel/addiu_at/
 * nop_mflo_mfhi anywhere in it (all three are resolved toolchain
 * constructs anyway, CLAUDE.md "Open toolchain blockers").
 */
```

### Naming

- **`D_8008ABA4` -> `gActorLocalMove`, tier A** (`python3 tools/rename.py`).
  Evident from the bodies: MoveLocalX/Y write element 0/1, the shared
  Actor__MoveAlongLocalAxis passes `&gActorLocalMove[0]` to
  addLocalTranslation (which rotates a local s16 vector by the actor's
  orientation, Actor.h) and clears the element again; ObjMStyleActor's
  MoveLocalZ does the same through the next halfword, `gActorLocalMoveZ` (not
  renamed here: not this unit's; proposed as `gActorLocalMoveZ`).

The declaration's comment lost its history; what it said, verbatim:

```c
/* Two-element s16 array -- Actor__MoveLocalX and Actor__MoveLocalY each write one
 * element (index 0 and 1 respectively) via a plain `sh` through a pointer
 * computed as %hi/%lo of `gActorLocalMove + 2*index`, so splat's single-word
 * dlabel is really this 2-element array, not a lone s32 (round 2026-09-04).
 * Not referenced anywhere else in the repo (checked with grep), so this is
 * this unit's own reading -- kept local rather than added to a shared
 * header. It is the x and y of Actor's local move vector: the z is the next
 * halfword, gActorLocalMoveZ, which ObjMStyleActor's Actor__MoveLocalZ writes, and
 * SceneNode__RotateLocalVector reads src[0..2]. */
```
