# SceneNode__CheckBoundsOverlap — MATCHED round 73 (243/243, bravo; was STALL rounds 13-54: 21 words SHORT, built 222/retail 243, 14/243 raw, first diff at vram 0x8001DA38 `addiu $t4, $a3, 4`)

> Renamed from `Class6B5CC__CheckBoundsOverlap` on 2026-09-26 (tools/rename.py). Address 0x8001da28.

> Renamed from `func_8001DA28` on 2026-09-18 (tools/rename.py). Address 0x8001da28.

Unit: `code_d294_b`. Round 13, runner delta. This is the unit's largest
queued function (268 asm lines / 243 words). One real attempt reached
6/243 in-range words with a plausible but not-yet-correct shape (compiled
body 221 words vs retail's 243 — a 22-word/frame-size deficit, not yet a
control-flow mismatch). Restored to `INCLUDE_ASM`. This report exists to
save the next attempt the (substantial) re-derivation cost, not because a
match was reached.

## Signature (as attempted)

```c
s32 SceneNode__CheckBoundsOverlap(SceneNodeObj *self, void *arg1, Vec3S16_d294 *arg2);
```

Confirmed from `SceneNode__TryAttachNearby`'s own call site (this round, also stalled —
see that report): `SceneNodeMethods`'s `+0x0A8` slot, dispatched as
`(self, arg1, &diff)` where `diff` is a `Vec3S16_d294`.

## What it does (derived from full disassembly trace, not guessed)

Two independent "walk an array, accumulate a delta, track a running
min/max" passes, followed by a scrambled AABB-overlap comparison:

**Pass 1** — `arg1` points at a `CornerList_d294`:
```c
typedef struct CornerList_d294 {
    s32 count;         /* +0x000 */
    Vec3S16_d294 hdr;  /* +0x004, corner[0] */
    /* corner[1..count*8-1] follow at +0x00A, stride 6 (Vec3S16) —
       count*8 corners total, 48 bytes per `count` unit */
} CornerList_d294;
```
`arg2` (a `Vec3S16_d294`) is added to EVERY corner (`hdr` directly, the rest
in a loop from `+0x00A` to `&hdr + count*48`), while a running
`BoundsBox_d294` (`mm`, seeded from the post-delta `hdr`) tracks the min and
max of every corner's x/y/z.

**The loop uses TWO pointers into the SAME 6-byte stride, per this round's
head-broadcast Lever 2** (confirmed by tracing retail's own register
allocation, not assumed): `t4`/`cur` at the element's own start (giving `x`
at offset 0), and a SECOND pointer `a0`/`zview` based 4 bytes further into
the SAME element (giving `z` at offset 0 relative to itself, and `y` via
`a0[-2]`/`zview[-1]`, one `s16` step backward). Both pointers advance by the
same 6-byte stride each iteration. This is the exact "N walkers, differently
based views of the same stride" shape the lever describes, confirmed a
second time in this codebase (the lever's own worked example was
`StageMap__ApplyRateEntries`/`ChunkLoadEntryTail`, a different unit).

**Per-iteration order is MIN.x, MIN.y, MIN.z, MAX.x, MAX.y, MAX.z —
sequential, not interleaved per-axis.** Each of the six checks re-reads its
field fresh from the walker pointer (not from a cached local), matching
retail's own double-read of `cur->x` (once for the min check, once for the
max check) — this is the "default value, then conditionally overwritten"
idiom from DECOMPILATION_LEARNINGS, applied six times per iteration.

**Pass 2** — after the corner loop, three PsyQ calls:
```c
TmdModel__UpdateBoundsBuffer(self->unk20);              /* fills PsyQ global gTmdModelBoundsBuf */
arr = TmdModel__GetBoundsBuffer(self->unk20, 0);     /* IGNORES both args, returns &gTmdModelBoundsBuf */
cnt2 = TmdModel__GetBoundsCount(self->unk20);       /* returns a count */
```
**`TmdModel__GetBoundsBuffer`'s whole body is `lui/addiu %hi/%lo(gTmdModelBoundsBuf); jr $ra`** —
MEASURED (`asm/psyq_GsLinkObject4.s`), it is a plain getter for a PsyQ-
internal global that `TmdModel__UpdateBoundsBuffer` fills one instruction earlier via
`TmdModel__ComputeBounds`. This resolved what looked at first like a confusing
"return value used as both a pointer and a scalar simultaneously" — it
isn't; `TmdModel__GetBoundsBuffer`'s return (a pointer) and `TmdModel__GetBoundsCount`'s return (a
count) are two DIFFERENT values that happen to both be freshly in `$v0` at
adjacent points, and a delay-slot register copy that looked like "assign
the NEW call's return" is actually copying the OLD (pre-call) `$v0` — read
the delay slot semantics twice before trusting which call's result a
register holds.

`arr` (typed `Sixteen6_d294 *`, a new all-`s16`, 6-field, 12-byte record —
same "whole-struct-assignment compiles to `lwl`/`lwr`" idiom as
`Vec3S16_d294`) is walked the same two-pointer way as pass 1, tracking a
running per-field MINIMUM only (all six fields, no max) into `track`,
seeded from `*arr` (element 0), walking from element 1 to `cnt2`.

**Tail**: a 5-guard early-exit AABB-overlap test comparing `track`'s 6
fields against `mm`'s 6 fields in a SCRAMBLED pairing (`track.f5` vs
`mm.lo.z`, `track.f2` vs `mm.hi.z`, `track.f3` vs `mm.lo.x`, `track.f4` vs
`mm.lo.y`, `track.f1` vs `mm.hi.y`), returning 0 on the first failure, else
`mm.hi.x >= track.f0` as the final boolean. Consistent with `track`'s 6
fields being some OTHER object's own `[hi.x, hi.y, hi.z, lo.x, lo.y, lo.z]`
in that specific order, though this is not confirmed beyond the offsets
themselves.

## Best attempt reached (preserved literally, positioned to compile)

```c
s32 SceneNode__CheckBoundsOverlap(SceneNodeObj *self, void *arg1, Vec3S16_d294 *arg2) {
    CornerList_d294 *list;
    Vec3S16_d294 *cur;
    s16 *zview;
    u8 *end;
    s32 count;
    BoundsBox_d294 mm;
    Sixteen6_d294 *arr;
    s16 *cur2;
    s16 *view2;
    u8 *end2;
    s32 cnt2;
    Sixteen6_d294 track;
    s16 v;

    list = (CornerList_d294 *)arg1;
    list->hdr.x = list->hdr.x + arg2->x;
    list->hdr.y = list->hdr.y + arg2->y;
    list->hdr.z = list->hdr.z + arg2->z;

    count = list->count;
    end = (u8 *)&list->hdr + count * 48;

    mm.lo = list->hdr;
    mm.hi = list->hdr;

    cur = (Vec3S16_d294 *)((u8 *)&list->hdr + 6);
    zview = (s16 *)((u8 *)cur + 4);

    if ((u8 *)cur < end) {
        do {
            cur->x = cur->x + arg2->x;
            *(zview - 1) = *(zview - 1) + arg2->y;
            *zview = *zview + arg2->z;

            v = cur->x;
            if (v < mm.lo.x) {
                mm.lo.x = v;
            }
            v = *(zview - 1);
            if (v < mm.lo.y) {
                mm.lo.y = v;
            }
            v = *zview;
            if (v < mm.lo.z) {
                mm.lo.z = v;
            }
            v = cur->x;
            if (mm.hi.x < v) {
                mm.hi.x = v;
            }
            v = *(zview - 1);
            if (mm.hi.y < v) {
                mm.hi.y = v;
            }
            v = *zview;
            if (mm.hi.z < v) {
                mm.hi.z = v;
            }

            cur = (Vec3S16_d294 *)((u8 *)cur + 6);
            zview = (s16 *)((u8 *)zview + 6);
        } while ((u8 *)cur < end);
    }

    TmdModel__UpdateBoundsBuffer(self->unk20);
    arr = TmdModel__GetBoundsBuffer(self->unk20, 0);
    cnt2 = TmdModel__GetBoundsCount(self->unk20);

    track = *arr;

    end2 = (u8 *)arr + cnt2 * 12;
    cur2 = (s16 *)((u8 *)arr + 12);
    view2 = cur2 + 5;

    if ((u8 *)cur2 < end2) {
        do {
            v = cur2[0];
            if (v < track.f0) {
                track.f0 = v;
            }
            v = view2[-4];
            if (v < track.f1) {
                track.f1 = v;
            }
            v = view2[-3];
            if (v < track.f2) {
                track.f2 = v;
            }
            v = view2[-2];
            if (v < track.f3) {
                track.f3 = v;
            }
            v = view2[-1];
            if (v < track.f4) {
                track.f4 = v;
            }
            v = view2[0];
            if (v < track.f5) {
                track.f5 = v;
            }

            cur2 = (s16 *)((u8 *)cur2 + 12);
            view2 = (s16 *)((u8 *)view2 + 12);
        } while ((u8 *)cur2 < end2);
    }

    if (track.f5 < mm.lo.z) {
        return 0;
    }
    if (mm.hi.z < track.f2) {
        return 0;
    }
    if (track.f3 < mm.lo.x) {
        return 0;
    }
    if (track.f4 < mm.lo.y) {
        return 0;
    }
    if (mm.hi.y < track.f1) {
        return 0;
    }
    return mm.hi.x >= track.f0;
}
```

## Where it stands and what's actually left

This is NOT a small residue — this attempt only got as far as confirming
the shape is roughly right (correct total word count within ~9%, correct
overall two-pass-plus-comparison structure, `objdump` shows a plausible
prologue) before running out of session budget to iterate on it. Two
concrete, measured leads for the next attempt:

1. **Frame size is 0x98 (152 bytes) vs retail's 0xF8 (248 bytes) — a
   96-byte/24-word deficit**, found from the very first diffed word
   (`addiu $sp,$sp,-0xF8` vs this attempt's `-0x98`). Given this round's
   repeated pattern (`SceneNode__NotifyWithHull`, `SceneNode__TryAttachNearby`) of an "unused-
   looking" local needing to be sized much larger than its own field
   accesses suggest, in order to reproduce retail's frame — check first
   whether `track`, `mm`, or a not-yet-identified THIRD local needs
   padding, using the same technique: adjust one candidate's size, rebuild,
   and watch the frame arithmetic (`addiu $sp,$sp,-N`) converge before
   reading any per-instruction diff. Do this BEFORE reading further
   per-word diffs — a frame-size mismatch shifts every later stack offset
   and makes the rest of the diff unreadable, same lesson as
   `SceneNode__NotifyWithHull`'s report.
2. **The `CornerList_d294`/`Sixteen6_d294` field NAMES here are honestly
   opaque placeholders** (`f0`..`f5`, `hdr`) — real semantic names were not
   recoverable from this one function alone. If a caller of
   `SceneNode__CheckBoundsOverlap` gets matched later and clarifies what `arg1`/`arg2`
   actually represent (geometry? a hit-test volume?), rename these before
   they propagate further.

## Header changes kept

`include/code_d294.h`:
- New `Sixteen6_d294` (12 bytes, 6×`s16`, all-s16-struct-copy idiom) and
  `CornerList_d294` (`{ s32 count; Vec3S16_d294 hdr; }`) types.
- New externs `TmdModel__UpdateBoundsBuffer(void *arg0)` and `TmdModel__GetBoundsBuffer(void *arg0,
  s32 arg1)` returning `Sixteen6_d294 *` (PsyQ library,
  `asm/psyq_GsLinkObject4.s`) — see the `gTmdModelBoundsBuf` finding above.
- Prototype for `SceneNode__CheckBoundsOverlap` itself.
- **No existing declaration was modified** for this function — `SceneNodeMethods::slotA8`
  (typed `s32 (*)(SceneNodeObj*, void*, Vec3S16_d294*)`) was already added
  while working `SceneNode__TryAttachNearby` earlier this round; this attempt only
  consumed that existing type, it did not change it.

## Proposed learning

**Confirms Lever 2 (two walkers, differently-based views of one stride)
generalizes beyond its original worked example.** This is the SECOND
independent function in this codebase (after `StageMap__ApplyRateEntries` in a
different unit) where retail's own register allocation shows a dense
per-element loop split across two pointers into the same array, one offset
from the other by a few bytes, rather than one pointer with field-offset
member access. Worth treating as a standing pattern to check for FIRST
(before attempting a single-pointer version at all) whenever a loop body
reads/writes 3+ fields of a small fixed-stride struct and the disassembly
shows more than one live pointer-class register surviving the loop.

## Round 20 (alpha): frame size fixed exactly, self-materialization
## fixed, 6/243 -> 14/243, substantial structure still needed

**First, a correctness note for whoever resumes this:** while re-deriving
this function, a bug from earlier in this same session was discovered
and fixed -- `SceneNode__TryAttachNearby` (this unit's OTHER round-13 stall, addressed
earlier the same round) had been left LIVE in `src/code_d294_b.c`
(missing its `#if 0`/`INCLUDE_ASM` wrapper) after an experiment, which
silently shifted every function after it in the file by 13 words. This
made `SceneNode__CheckBoundsOverlap`'s own vram address wrong in the LINKED build
(`0x8001D9F4` instead of retail's `0x8001DA28`, confirmed via
`build/lsdde.map`), which in turn made every early diff read against it
meaningless. Fixed by re-wrapping `SceneNode__TryAttachNearby` properly; confirmed via
the map file that `SceneNode__CheckBoundsOverlap` lands at the correct retail address
before trusting any further diff. **Whenever a diff for an
`INCLUDE_ASM`-adjacent function looks nonsensical from word 0 (a
completely different instruction, not a plausible near-miss), check
`build/lsdde.map` for that function's actual linked address against its
retail one before reading anything else** -- this is a variant of
CLAUDE.md's "four ways a score lies" that isn't explicitly named there:
a sibling function accidentally left live/mis-sized poisons every
later function's diff without any compile error or the usual drift
warning necessarily appearing on the LATER function itself (it appeared
here, but the natural instinct is to blame the function being worked on,
not an unrelated sibling).

Dropped the round-13 best-attempt body (never previously committed to
`src/`, only preserved in this report's prose) into `src/code_d294_b.c`
in place of the bare `INCLUDE_ASM`. Baseline reproduced: **5/243, frame
`-0x98` (152 bytes) vs retail's `-0xF8` (248 bytes)**, matching the
report's own "22-word/96-byte deficit" figure closely (96 bytes exactly).

**Frame size, closed exactly.** Per this report's own flagged first lead,
and this unit's now-repeated idiom (`SceneNode__NotifyWithHull`, `SceneNode__TryAttachNearby`):
mapped every `(sp)`-relative load/store in retail's own `.s` and found
the ENTIRE stack usage is two 12-byte structs (`mm` at `+0x10`, `track`
at `+0x20`) plus the standard `0x10` outgoing-arg minimum -- nothing else
in the whole 248-byte frame is ever touched by a load or store. The gap
between the last real use (`+0x2C`) and the saved-register area
(`+0xF0`) is **exactly 96 bytes (`0x60`), matching the deficit exactly**.
Added `u8 pad[0x60];` as a genuinely-unused local: frame became `-0xF8`
EXACTLY, matching retail bit-for-bit in the `addiu $sp` instruction.

**Self-materialization, closed.** With the frame fixed, `funcdiff`'s
per-function window finally aligned to the correct address (confirmed
via `build/lsdde.map`, not just assumed), showing `self` deferred into
the MIDDLE of the corner-loop setup instead of materializing into `$s0`
in the prologue like retail -- the same "deferred parameter copy" shape
this project's OTHER unit (`code_55dd4`) has repeatedly hit. `self` is
not referenced in this function's source until `TmdModel__UpdateBoundsBuffer(self->unk20)`,
deep in pass 2, so GCC defers it. A bare `__asm__("")` as the very first
statement forced early materialization, matching retail's prologue
exactly (`sw s0`/`move s0,a0`/`move a3,a1` all now byte-identical from
word 0): **5/243 -> 14/243.**

**Tried one more source reshape targeting the next visible difference
(retail interleaves `count`/`end`'s computation between the header's y
and z updates; this attempt computes them afterward, as one block) --
moving the `count =`/`end =` statements to sit between the `y` and `z`
header-add lines, textually matching retail's instruction order: NO
CHANGE (still 14/243, byte-identical).** GCC 2.6.3's scheduler here does
not follow source statement order for these independent computations --
consistent with this project's broader finding that only SOME residues
respond to statement reordering. Reverted to the clearer prose order.

**What is left, precisely:** the persistent difference immediately after
the (now-correct) prologue is that retail keeps `arg2` (the per-axis
delta) live in `$a2` for the ENTIRE function, including reuses deep
inside the loop, while this attempt copies it into a scratch register
(`$t2`) early and uses that instead -- a register-allocation choice, not
yet explained by any lever tried this round. Beyond that single
instruction, the diff runs for over 1000 lines (both loop bodies, the
`Sixteen6_d294` min-tracking pass, and the scrambled tail comparison all
still differ substantially) -- this is genuinely NOT a small residue,
consistent with the round-13 report's own framing. A future attempt
should start from THIS round's 14/243, correctly-framed body (preserved
below) rather than re-deriving the frame/materialization fixes from
scratch.

Filing as STALL at 14/243 (up from round 13's 6/243), `INCLUDE_ASM`
restored; confirmed clean rebuild (`build exit=0`, whole-image OK).

## Preserved body (round 20 best, 14/243 -- frame + self-materialization fixed)

```c
#if 0
s32 SceneNode__CheckBoundsOverlap(SceneNodeObj *self, void *arg1, Vec3S16_d294 *arg2) {
    CornerList_d294 *list;
    Vec3S16_d294 *cur;
    s16 *zview;
    u8 *end;
    s32 count;
    BoundsBox_d294 mm;
    Sixteen6_d294 *arr;
    s16 *cur2;
    s16 *view2;
    u8 *end2;
    s32 cnt2;
    Sixteen6_d294 track;
    s16 v;
    u8 pad[0x60];

    __asm__("");
    list = (CornerList_d294 *)arg1;
    list->hdr.x = list->hdr.x + arg2->x;
    list->hdr.y = list->hdr.y + arg2->y;
    list->hdr.z = list->hdr.z + arg2->z;

    count = list->count;
    end = (u8 *)&list->hdr + count * 48;

    mm.lo = list->hdr;
    mm.hi = list->hdr;

    cur = (Vec3S16_d294 *)((u8 *)&list->hdr + 6);
    zview = (s16 *)((u8 *)cur + 4);

    if ((u8 *)cur < end) {
        do {
            cur->x = cur->x + arg2->x;
            *(zview - 1) = *(zview - 1) + arg2->y;
            *zview = *zview + arg2->z;

            v = cur->x;
            if (v < mm.lo.x) {
                mm.lo.x = v;
            }
            v = *(zview - 1);
            if (v < mm.lo.y) {
                mm.lo.y = v;
            }
            v = *zview;
            if (v < mm.lo.z) {
                mm.lo.z = v;
            }
            v = cur->x;
            if (mm.hi.x < v) {
                mm.hi.x = v;
            }
            v = *(zview - 1);
            if (mm.hi.y < v) {
                mm.hi.y = v;
            }
            v = *zview;
            if (mm.hi.z < v) {
                mm.hi.z = v;
            }

            cur = (Vec3S16_d294 *)((u8 *)cur + 6);
            zview = (s16 *)((u8 *)zview + 6);
        } while ((u8 *)cur < end);
    }

    TmdModel__UpdateBoundsBuffer(self->unk20);
    arr = TmdModel__GetBoundsBuffer(self->unk20, 0);
    cnt2 = TmdModel__GetBoundsCount(self->unk20);

    track = *arr;

    end2 = (u8 *)arr + cnt2 * 12;
    cur2 = (s16 *)((u8 *)arr + 12);
    view2 = cur2 + 5;

    if ((u8 *)cur2 < end2) {
        do {
            v = cur2[0];
            if (v < track.f0) {
                track.f0 = v;
            }
            v = view2[-4];
            if (v < track.f1) {
                track.f1 = v;
            }
            v = view2[-3];
            if (v < track.f2) {
                track.f2 = v;
            }
            v = view2[-2];
            if (v < track.f3) {
                track.f3 = v;
            }
            v = view2[-1];
            if (v < track.f4) {
                track.f4 = v;
            }
            v = view2[0];
            if (v < track.f5) {
                track.f5 = v;
            }

            cur2 = (s16 *)((u8 *)cur2 + 12);
            view2 = (s16 *)((u8 *)view2 + 12);
        } while ((u8 *)cur2 < end2);
    }

    if (track.f5 < mm.lo.z) {
        return 0;
    }
    if (mm.hi.z < track.f2) {
        return 0;
    }
    if (track.f3 < mm.lo.x) {
        return 0;
    }
    if (track.f4 < mm.lo.y) {
        return 0;
    }
    if (mm.hi.y < track.f1) {
        return 0;
    }
    return mm.hi.x >= track.f0;
}
#endif
```

### Proposed learning

**An unrelated sibling function left improperly wrapped (missing its
`#if 0`/`INCLUDE_ASM` restore) shifts every later function's LINKED
address, and the symptom shows up on the LATER function's own diff, not
the sibling's** -- `build exit=0`/whole-image checks catch this only if
you run them between every source change; a `funcdiff.py` read on a
function whose own source hasn't changed can still be reading a
meaningless window if something upstream in the same file has drifted.
Check `build/lsdde.map` for the function's actual vs. expected vram
address whenever a diff looks nonsensical from its very first word,
before concluding the function's OWN source is somehow producing
alien code.

Also: this function's frame-size fix is the THIRD instance in this one
unit (`SceneNode__NotifyWithHull`, `SceneNode__TryAttachNearby`, now this) of "an unused-looking
local needs to be sized to close a frame gap, not to what the function's
own visible code needs" -- strong enough now to treat as a standing
first-check for any `code_d294_b` stall with a non-matching frame size,
not just a possibility to consider.

## Round 37 (echo) — title rebuilt with the three required figures; no new lever tried

This function had no LENGTH/RAW-MATCH/FIRST-DIFF figures in its title
(only prose: "structure understood, not yet byte-matched"), which
CLAUDE.md's per-round instructions flagged explicitly as blocking the next
round from ranking it. Per the round's "build the inherited body before
you trust its score" discipline, spliced the round-20 preserved body
(above, 14/243, frame + self-materialization fixes already folded in)
back into `src/code_d294_b.c` verbatim, confirmed `SceneNode__TryAttachNearby`
(the sibling immediately before it in ROM order) was still properly
`#if 0`/`INCLUDE_ASM`-wrapped before trusting the address, and rebuilt.

**Reproduces exactly: 14/243 raw words match, `build exit=2`, no compile
errors.** `build/lsdde.map` confirms `SceneNode__CheckBoundsOverlap` itself lands at the
correct retail address `0x8001da28` (so the earlier frame-size fix still
holds and this function's own window is trustworthy) — but the NEXT
function, `SceneNode__ClassifyAgainstPlanes`, lands at `0x8001dda0` where retail has it at
`0x8001DDF4`, a **0x54-byte / 21-word deficit**. That is this function's
own true LENGTH residue: 222 words built vs 243 retail.

Ran `tools/asm-differ/diff.py SceneNode__CheckBoundsOverlap` to find the first REAL
defect (realigned, not the raw funcdiff list which runs into ripple
almost immediately once the lengths diverge). The prologue
(`addiu $sp,$sp,-0xf8` / `sw $s0` / `move $s0,$a0` / `move $a3,$a1`) is
byte-identical for its first four instructions — confirming the frame-size
and self-materialization fixes from round 20 are both still holding — and
the first divergence is the very next instruction: retail's
`addiu $t4, $a3, 4` (materializing a dedicated pointer for the `cur`
corner-walk, offset 4 into the corner struct) against this build's
`move $t2, $a2` (copying `arg2` into a scratch register instead of
keeping it live in `$a2`, matching round 20's own description of the
residue: "retail keeps `arg2` live in `$a2` for the entire function... this
attempt copies it into a scratch register early"). So the FIRST divergence
is exactly the residue round 20 already named, now pinned to a concrete
file offset/vram for the next attempt to target directly rather than
re-deriving from a raw diff: **file offset `0xE238`, vram `0x8001DA38`.**

No new lever was tried this round — the task here was explicitly to
rebuild the missing figures, not to re-attempt the residue, and this
function's own remaining gap (21 words, a whole second loop-and-comparison
structure still substantially unresolved per round 20's own framing) is
too large to responsibly hand off to the permuter without first closing
the `arg2`-liveness residue by hand; a permuter search seeded from a
14/243 body would be searching a 229-word space essentially blind. Filed
as STALL, figures rebuilt, `INCLUDE_ASM` confirmed restored, whole-image
`build-and-verify.sh` clean afterward.

### Proposed learning

**A match report's title is itself a measurement that goes stale exactly
like a progress count** — this one was accurate prose in round 20 but,
lacking the three-figure format CLAUDE.md's reporting convention
specifies, could not be ranked or compared across rounds without someone
re-deriving it from scratch. Rebuilding a title costs one build and one
`asm-differ` run (a few minutes) against re-deriving the whole function's
structure, and is worth doing as its own deliverable whenever a queued
report predates the three-figure convention, independent of whether any
new attempt is made on the residue itself.

## Round 41 (bravo): first-ever permuter search, clean negative -- the residue is too large/structural for a bounded search, exactly as round 37 predicted

Re-verified the round-20/37 14/243 body live first: reproduces exactly
(`build exit=2`, no compile errors, `funcdiff.py` confirms 14/243 with
the same 340154-byte drift warning this report already documents --
expected, since the function is still a genuine 21-word length deficit,
not a pure register residue).

This was `SceneNode__CheckBoundsOverlap`'s first-ever permuter search, per this round's
assignment. Round 37 explicitly declined to run one ("too large to
responsibly hand off to the permuter without first closing the
arg2-liveness residue by hand... a permuter search seeded from a 14/243
body would be searching a 229-word space essentially blind") -- this
round ran it anyway, per the ordered work list, to get a real data point
rather than defer again.

**Scaffold validation (`--debug --stack-diffs`) confirmed round 37's own
prediction before any search budget was spent:** base score 7871 --
**136 stack-difference points, 123 register-difference points, 7
reorderings, 23 insertions, 44 deletions.** This is not a clean
register-identity residue (contrast `ClipSegmentToBox`'s validated scaffold
the same round: 0 insertions, 0 deletions, a pure 1-instruction
symptom) -- 23 insertions and 44 deletions describe a genuinely different
program shape across a large fraction of the function, matching round
20/37's own characterization of "substantial structural work in the two
loop bodies and tail comparison," not a permutation of an
otherwise-correct body.

**Search:** `-j 6 --stop-on-zero --best-only`, bounded at 900s, rc
captured on the very next command: **rc=124** (the 900-second bound
fired; `--stop-on-zero` never triggered, so no zero was found).
**73273 iterations** -- run under load (other runners' own permuter
searches were concurrently active in their own worktrees, per `ps`, but
this round's "one search at a time" rule is scoped to THIS worktree, not
the whole machine, and was honored: no second search was started here
until this one's `permuter rc=` line appeared).

**Best score found: 4990 (down from base 7735; the permuter's own base
score differs slightly from `--debug`'s 7871, an internal
scoring-mode difference, not a discrepancy in the residue itself).**
Read every one of the eight `output-*` directories `--best-only` wrote
(7075, 6820, 6205, 6200, 5775, 5705, 5655, 4990), not just the final
best, since round 20's own finding on a DIFFERENT function in this unit
(`SceneNode__TryAttachNearby`) was that a permuter's headline score can bundle a
load-bearing change with unrelated noise:

- **None of the eight candidates touch the function's actual first
  divergence** (retail's `addiu $t4,$a3,4` at vram `0x8001DA38`,
  materializing `&list->hdr` immediately after `list` is copied into
  `$a3`, against this build's `move $t2,$a2` -- copying `arg2` into a
  scratch register instead of leaving it live in `$a2`, per round 37's
  own pinned finding). Every candidate's diff is confined to LOOP 2 (the
  `Sixteen6_d294` min-tracking pass) or to trivial, semantically-inert
  rewrites of loop 1's body -- never the prologue-adjacent code where the
  real 21-word gap begins.
- Six of the eight are pure noise with no plausible mechanism: wrapping
  a single field read in a newly-declared `inline` helper function
  (`inline_fn`, three separate candidates, each just `return
  arg0->x;`/`arg0[-arg1];` called once), or introducing an unused
  dummy local that shadows an existing read (`s16 new_var`, `cnt2`
  temporarily aliasing `v`) with no behavioral difference from the
  original expression.
- **One candidate (the final best, 4990) is UNDEFINED BEHAVIOR, not a
  usable lever:** it duplicates the ENTIRE prologue-through-loop-setup
  block into `if (v) { ...block... } else { ...identical block... }`,
  where `v` is read BEFORE its first assignment anywhere in the
  function (it is declared, never initialized, and only ever assigned
  inside the loop body that comes later). Branching on an
  uninitialized value is UB; the permuter's scorer only checks compiled
  bytes against retail; it has no way to know or care that the C it
  emitted has no defined behavior. Not translated into `src/` for this
  reason alone, independent of whether it would have scored well
  against the real build.

**Conclusion: clean negative, exactly the outcome round 37 predicted
rather than a new one.** A validated scaffold with 23 insertions and 44
deletions describes a residue with too much genuine structural
difference for an 900-second, 73273-iteration bounded search to
stumble into the RIGHT set of changes -- the search spent its entire
budget finding cosmetically-different-but-behaviorally-identical
rewrites of code that already matches, or scores that only look better
because of UB the scorer cannot detect, and never touched the real
divergence point at all. This is consistent with, and strengthens, round
37's own reasoning for deferring the search in the first place: **a
permuter search seeded from a body whose residue starts at the very
FIRST instruction after the prologue searches an effectively unbounded
space when 220+ words downstream are also uncertain**, and this round's
result is the concrete confirmation of that prediction rather than a
reason to distrust it.

Filing unchanged as **STALL at 14/243** (built 222/retail 243 words per
`nm -S`, 21 words short; first real diff at file offset `0xE238` / vram
`0x8001DA38`, unchanged from round 37's own pinned figures).
`INCLUDE_ASM` restored, `src/code_d294_b.c` confirmed byte-identical to
the committed state (`git diff --stat` empty) after the check.

### Proposed learning (round 41)

**A `--debug --stack-diffs` scaffold's insertion/deletion count is a
reliable PRE-SEARCH predictor of whether a bounded permuter run will be
productive, and this round is a second, independent data point for it
(after `ClipSegmentToBox`, same round, same unit).** `ClipSegmentToBox`'s
scaffold showed 0/0 insertions/deletions (a pure register/stack
residue) and its search found a zero in 2642 iterations. This
function's scaffold showed 23/44 (a genuinely different program shape
across a large fraction of the body) and its search found nothing
better than a 4990-vs-7735 score in 73273 iterations, entirely via
noise and one UB exploit, having never touched the real divergence.
Worth treating the insertion/deletion count from `--debug` as a
go/no-go signal WORTH RECORDING even when the decision is "run it
anyway" (as this round did, deliberately, to get a data point rather
than defer again) -- the prediction held, and having it recorded means
the next round doesn't have to re-run a 900-second search to learn what
this round's `--debug` output already said before any iteration ran.

Also: **reviewing every `output-*` candidate a `--best-only` search
wrote, not just the final lowest score, caught a UB-exploiting
"improvement" that would otherwise have looked like the round's
headline result.** A permuter's scorer has no way to flag undefined
behavior; a human reading the diff is the only check. Worth stating
alongside CLAUDE.md's existing "a permuter score drop is a LEAD, not a
RESULT" framing: sometimes the lead, once read, is a reason NOT to
translate anything, and that is itself the useful output of the check.

## Naming (round 54, bravo, track 3)

Renamed from `func_8001DA28` via `tools/rename.py` (name only -- still
`INCLUDE_ASM`, not attempted for a match this round). **Tier B** -- slot
`+0x0A8` occupant. Builds an AABB from a `CornerList` (offset by `arg2`,
running min/max over every corner), a second per-plane running minimum
from `self->unk20`'s planes, then a 6-guard scrambled AABB-overlap test
between the two, returning a boolean. "Overlap test" describes the
measured tail comparison; the real identity of the second box (what
`self->unk20`'s planes represent) is not confirmed beyond the field
offsets, per this report's own "What it does" section. Purely local to
this unit + its header for the FUNCTION rename; the vtable FIELD name
(`slotA8`) is proposed, not renamed.

NON_MATCHING body promoted, round 69

## Round 73 (bravo): MATCHED 243/243, first rebuild of a fresh re-read

REVISITED, round 73: MATCHED 243/243; names/types used (the second record is a `BoundsBox_d294`, not six opaque `f0..f5` -- see below).

**Preserved body rebuilt first (round-20 body verbatim from this report, with
its `__asm__("")` and `u8 pad[0x60]`):** 14/243, `insertions 63 / deletions 63`,
positional skeleton diffs 229, built 222 words. Matches the title.

**Then written fresh from the asm, block by block.** Builds, one lever each:

1. Loops as `for (v++; v < end; v++)` over the parameter-derived pointer
   (no hand-built second `zview` walker -- GCC's loop optimiser makes the
   `+4` giv itself), every min/max as a ternary stored back unconditionally
   (`f = (v < f) ? v : f`), `u8 pad[0x60]` kept: 2/243, ins 42/del 42, built
   239 words, frame `-0x158`. **The ternary closed 17 of the 21 missing words**:
   retail stores every box field every iteration (`lhu tK,off(box)` default,
   conditional `move tK,..`, unconditional `sh tK`), which the `if (v < f) f = v;`
   form never emits.
2. `u8 pad[0x60]` deleted: frame `-0xF8` exactly. The pad had only ever
   compensated for the missing ternary temporaries. (Score unreadable, same
   window.)
3. `b = &mm;` pointer local used in the corner loop only: 140/243, ins 22/del
   22, 241 words. Retail's loop-invariant `addiu $t3, $sp, 0x10` is that
   pointer.
4. Min ternaries spelled `f = (f > v) ? v : f` instead of `(v < f) ? v : f`:
   184/243, ins 19/del 19. The comparison's operand LOAD order follows the
   source operand order; retail loads the box default, then the element.
5. Pass 2 fields 3..5 are a running MAX, not a min (the round-13 trace
   misread them), and the tail tests on the lo side spelled `mm.lo.z >
   box.hi.z` etc. (load order again): 224/243, ins 1/del 1, 242 words.
6. Tail as `ret = 0; if (!c1 && !c2 && !c3 && !c4 && !c5) ret = !c6; return ret;`
   (retail: `a0 = 0` in the first delay slot, `move v0,a0` at the join,
   final `xori a0,v0,1`): **243/243, build exit=0, whole-image OK.**
7. Reader form (second record typed `BoundsBox_d294`, fields `lo`/`hi`,
   `Sixteen6_d294 *` return cast): 243/243, whole-image OK. No header edit.

The round-13 pairing note ("scrambled" `f5<->lo.z` ...) is not scrambled:
`TmdModel__GetBoundsBuffer`'s records are `BoundsBox_d294` boxes (`f0..f2` = lo, `f3..f5`
= hi), pass 2 grows their union, and the tail is an ordinary per-axis AABB
overlap in z, x, y order. The header comment on `Sixteen6_d294` still says
otherwise; left untouched (shared header, comment only).

No permuter, no Gate 3 search spent. Signature unchanged.

### Matched body

```c
/* Offsets arg1's corner list by `d` and grows a box `mm` over the moved
 * corners, grows a second box `box` over the model's own bounds records
 * (TmdModel__GetBoundsBuffer's array), and returns 1 if the two boxes overlap on all
 * three axes. Each running min/max is a ternary stored back unconditionally
 * (retail stores every field every iteration), and the source compares
 * with `>` for a min so the slt operands load in retail's order. */
s32 SceneNode__CheckBoundsOverlap(SceneNodeObj *self, void *arg1, Vec3S16_d294 *d) {
    CornerList_d294 *list;
    Vec3S16_d294 *v;
    Vec3S16_d294 *end;
    BoundsBox_d294 mm;
    BoundsBox_d294 *b;
    BoundsBox_d294 *p;
    BoundsBox_d294 *end2;
    s32 n;
    s32 ret;
    BoundsBox_d294 box;

    list = (CornerList_d294 *)arg1;
    v = &list->hdr;
    v->x += d->x;
    v->y += d->y;
    end = v + list->count * 8;
    v->z += d->z;
    mm.lo = *v;
    mm.hi = *v;
    b = &mm;
    for (v++; v < end; v++) {
        v->x += d->x;
        v->y += d->y;
        v->z += d->z;
        b->lo.x = (b->lo.x > v->x) ? v->x : b->lo.x;
        b->lo.y = (b->lo.y > v->y) ? v->y : b->lo.y;
        b->lo.z = (b->lo.z > v->z) ? v->z : b->lo.z;
        b->hi.x = (b->hi.x < v->x) ? v->x : b->hi.x;
        b->hi.y = (b->hi.y < v->y) ? v->y : b->hi.y;
        b->hi.z = (b->hi.z < v->z) ? v->z : b->hi.z;
    }

    TmdModel__UpdateBoundsBuffer(self->unk20);
    p = (BoundsBox_d294 *)TmdModel__GetBoundsBuffer(self->unk20, 0);
    n = TmdModel__GetBoundsCount(self->unk20);
    box = *p;
    end2 = p + n;
    for (p++; p < end2; p++) {
        box.lo.x = (box.lo.x > p->lo.x) ? p->lo.x : box.lo.x;
        box.lo.y = (box.lo.y > p->lo.y) ? p->lo.y : box.lo.y;
        box.lo.z = (box.lo.z > p->lo.z) ? p->lo.z : box.lo.z;
        box.hi.x = (box.hi.x < p->hi.x) ? p->hi.x : box.hi.x;
        box.hi.y = (box.hi.y < p->hi.y) ? p->hi.y : box.hi.y;
        box.hi.z = (box.hi.z < p->hi.z) ? p->hi.z : box.hi.z;
    }

    ret = 0;
    if (!(mm.lo.z > box.hi.z) && !(mm.hi.z < box.lo.z) && !(mm.lo.x > box.hi.x) &&
        !(mm.hi.x < box.lo.x) && !(mm.lo.y > box.hi.y)) {
        ret = !(mm.hi.y < box.lo.y);
    }
    return ret;
}
```

### Proposed learning (round 73)

**A running min/max whose retail form loads the field as a default, moves
the new value in conditionally and stores UNCONDITIONALLY is a ternary
store-back, and the `if (v < f) f = v;` form costs a store and its temps per
field** -- here 21 words over twelve fields, which an earlier round had
"fixed" at the frame level with a fake `u8 pad[0x60]` local. A pad local that
exists only to make `addiu $sp` agree is a symptom to explain, not a fix.
Discriminator: `lhu tK,off(box)` before the compare, `sh tK,off(box)` after
the join, every field. Companion: the compare's two loads come in source
operand order, so a min whose default is loaded first is spelled `f > v`.
