# Actor__FindNearbyLink -- MATCHED (71/71)

> Renamed from `DreamSys__FindNearbyLink` on 2026-09-25 (tools/rename.py). Address 0x80057668.

> Renamed from `func_80057668` on 2026-09-19 (tools/rename.py). Address 0x80057668.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0BC`'s
caller? No -- checked all 6 method tables reachable from this unit's
addresses with `tools/classtable.py`: no hit. Plain internal helper,
called by this unit's own `Actor__MoveOrFindNearbyLink` (already matched).

## Signature

```c
s32 Actor__FindNearbyLink(DreamSys *self);
```

## Body

```c
s32 Actor__FindNearbyLink(DreamSys *self) {
    LinkQueryBuf sp18;
    GridQuery sp48[3];
    /* No known field needs this gap; empirically required to reproduce
     * retail's exact stack layout for sp78/sp88 below (round 2026-09-04,
     * see below). */
    u8 pad48Tail[8];
    GridArrElem *sp78[3];
    DreamSysVec3 sp88;

    if (self->unk_0x4C != NULL) {
        void *pos = (u8 *) self->unk_0x14 + 0x18;

        if (self->unk_0x4C->methods->queryLinkAtPos(self->unk_0x4C, &sp18, pos) == 0) {
            s32 count = Actor__BuildLinkQueries(self, sp48, sp78, &sp18, 1);
            void *result = Actor__ScanLinkCandidates(self, &sp88, pos, count, sp48, sp78);

            self->unk_0x28 = result;
            if (result != NULL) {
                self->vt->Actor__AddTranslation(self, &sp88);
                self->vt->DreamSys__NotifyLinkAttempt(self, -1);
                return 1;
            }
            self->vt->DreamSys__NotifyLinkAttempt(self, -2);
            return 0;
        }
    }
    return 0;
}
```

Fetches an output position (`self->unk_0x14 + 0x18`, raw byte offset --
`unk_0x14` is `DreamSysUnk14 *`, already established in
`include/DreamSys.h`) via `self->unk_0x4C`'s own vtable slot `+0x110`
(`DreamSysUnk4CMethods::queryLinkAtPos`, newly named this round, splitting the
existing `pad_0x110[0x11C-0x110]`), then -- only if that call signals
success (`== 0`) -- builds a query (`Actor__BuildLinkQueries`, THIS unit's own,
currently STALLED -- see its report; still linkable and correct as an
`INCLUDE_ASM` symbol) and runs it (`Actor__ScanLinkCandidates`, this unit's own,
already matched). Stores the search result into `self->unk_0x28`
(`DreamSysUnk28Target *`), and on success dispatches `self->vt->slotBC`
(`Actor__AddTranslation`, already named) and `self->vt->slot0x88`
(`DreamSys__NotifyLinkAttempt`, already named) with `-1`; on failure, only the latter
with `-2`.

## The stack-layout gap

`sp48` (a `GridQuery[3]`) and `sp78` (a `GridArrElem *[3]`) are separated
by 8 bytes with no known field to account for them. This was found
empirically: `tools/funcdiff.py` scored 55/71 with a 0x30-byte
`LinkQueryBuf` (positions `sp18` and `sp48` correctly, per the address
computation `addiu s1, sp, 0x48` matching) but `sp78`/`sp88` and the
overall frame size (`-0xB0`) still 8 bytes short; growing `LinkQueryBuf`
to 0x38 fixed the frame size and `sp78`/`sp88` but then made `sp48`'s OWN
address 8 bytes too high. The two needs are independent -- an explicit
8-byte gap declared between `sp48` and `sp78` (not folded into
`LinkQueryBuf`) satisfies both simultaneously. `LinkQueryBuf` itself grew
by 8 bytes too (`0x28` -> `0x30`, still additive/local -- it's this
unit's own type, not a shared header), matching the stack gap observed
between `sp18` and `sp48` before this fix. Neither gap has a known field;
both are declared as opaque `u8` padding.

## New shared types/fields

- `LinkQueryBuf` grown from `0x28` to `0x30` bytes (this file's own
  local type, first introduced in `Actor__BuildLinkQueries`'s stalled report).
- `include/DreamSys.h`: `DreamSysUnk4CMethods::queryLinkAtPos` (already added
  alongside `getGridArrElemAt` in the previous commit, for this function).
- The forward declaration `s32 Actor__FindNearbyLink(DreamSys *self);` (added
  earlier for `Actor__MoveOrFindNearbyLink`'s call) had to be updated from a stale
  `void` return type once this function's real signature was known --
  C89 caught the mismatch as a hard `conflicting types` error, not a
  silent problem.

## Naming

**`Actor__FindNearbyLink` -- tier B.** Mechanics fully confirmed
(71/71 byte-exact): fetches a `LinkQueryBuf` via
`self->unk_0x4C->methods->queryLinkAtPos` for a fixed position
(`self->unk_0x14 + 0x18`), builds and scans grid candidates
(`Actor__BuildLinkQueries` + `Actor__ScanLinkCandidates`), stores
the result into `self->unk_0x28`, and on success applies the found
position (`Actor__AddTranslation`) and tags the outcome via `DreamSys__NotifyLinkAttempt`
(`-1` success / `-2` failure). "Find a nearby link" is squarely what the
grid search does; the surrounding game purpose (why search for one here)
is not established, which keeps this tier B rather than A.

## Proposed field names

`DreamSys::unk_0x28` and `DreamSys::unk_0x4C` are accessed from OTHER
units too (`src/DreamSys.c`, per `grep -rn -- '->unk_0x28\b\|->unk_0x4C\b'
src/`), so per FINISHING-PLAN.md track 3 step 3 they are proposed here,
not renamed, and posted to the broadcast for the head to apply by type
scope at merge.

- **`DreamSys::unk_0x28` -> `linkTarget`** (type `DreamSysUnk28Target *`,
  tier B). Evidence: `Actor__MoveOrFindNearbyLink` clears it to
  NULL before invoking a per-axis callback and treats a still-NULL value
  afterward as "no link found, fall back to a grid search"; this function
  itself stores the grid search's own result into it. Consistently
  "the currently attached/found link object", never anything else, across
  every access in this unit.
- **`DreamSys::unk_0x4C` -> `linkMgr`** (type `DreamSysUnk4CObj *`, tier
  B). Evidence: this function calls `unk_0x4C->methods->queryLinkAtPos`
  to look up a link at a position; `include/DreamSys.h`'s own existing
  comments show it used the same way by `DreamSys__WallLink`,
  `DreamSys__TryInstantTeleportLink` and `DreamSys__DetachFromParent` in `src/DreamSys.c` -- every access
  across every unit is link-related.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__FindNearbyLink   # 71/71
```

### Proposed learning

**An empirically-required stack padding gap with no known source field is
a legitimate, documentable finding -- not a sign the struct reading is
wrong.** When a local buffer's declared size change fixes ONE downstream
address but breaks ANOTHER, the two needs may be independent (one struct
genuinely grew, and a SEPARATE gap exists elsewhere that isn't part of
that struct at all). Isolate them: pin the struct size to whatever
satisfies the addresses immediately adjacent to it, then look for a
second, separately-declared gap to satisfy what remains. Guessing a
single combined size change to satisfy both trades one correct offset
for another.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__FindNearbyLink`. Reached only from Actor__MoveOrFindNearbyLink (base slots +0x0D0/+0x0D4), so the method is Actor's. Accessors now: grid (+0x04C, the StageMap child; cast to DreamSys.h's DreamSysUnk4CObj view for queryLinkAtPos), &coord2->tx, linkTarget, addTranslation, notifyIfUnk20Active (-1 found, -2 not). The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_p.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 96, bravo)

Comments quoted below are verbatim as the file stood before this round's
comment pass, i.e. with this round's renames already applied (the
`LinkQueryBuf` one as it stood before step 2).

- Locals: `sp18` -> `desc` (a `Descriptor10Ext` now, StageMap.h, so the
  `(Descriptor10Ext *)` cast at computeFootprintDescriptor goes; the old
  0x30-byte `LinkQueryBuf` view is retired, and dropping its 4 trailing
  bytes is byte-identical, measured), `sp48` -> `queries`, `sp78` ->
  `slots`, `sp88` -> `offset` (SceneNode__RaycastVertical's output, the
  hit less the ray's start, then addTranslation'd).
- `pad48Tail[8]` is still needed (without it, 55/71). `GridQuery
  queries[4]` in its place also matches (0x24 + 8 rounds to the same 0x30),
  but BuildLinkQueries fills at most 3, so the pad stays and keeps one
  `/* MATCHING: */` line.
- Proposed: the notifyWithHull events -1/-2 (linked / not linked) and
  MoveAlongLocalAxis's 6/7/8 as one enum in Actor.h (head; class_3bb8c_k
  would change too).

The pad's comment, verbatim:

```c
    /* No known field needs this gap; empirically required to reproduce
     * retail's exact stack layout for slots/offset below (round 2026-09-04,
     * see this function's match report). */
```

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.
