# StageMap__UnloadAllSlots — MATCHED round 71 (charlie): 74/74, byte-exact, whole-image SHA1 green

> Renamed from `StageMap__ResetAllElements` on 2026-09-26 (tools/rename.py). Address 0x8004abd0.

> Renamed from `Class866E8__ResetAllElements` on 2026-09-26 (tools/rename.py). Address 0x8004abd0.

REVISITED, round 71: MATCHED (first build of a rewrite); names/types used
(existing `UnkSlotEntry_3ac78` / `UnkSlotListObj_3ac78` fields, no struct edit).

## Round 71 (charlie) — the match

**Rebuilt as given first.** The preserved `#if 0` body from `src/` rebuilt
at **9/74, insertions 45 / deletions 45, 64 positional skeleton diffs**,
correct length. The title's "correct size" was current; its CAUSE
("scheduling-only, the offset increment keeps landing in the wrong delay
slot") was wrong. Reading the diff: the whole register assignment was
shifted by one saved register (retail uses five: self, i, offset, entry,
list; the old body used four), and `entry->list` was reloaded after the
release call instead of held.

**Two levers, both source-shape, no permuter:**

1. Write `entry = &self->elems[i];`, not
   `entry = (UnkSlotEntry_3ac78 *)((u8 *)self + offset); offset += 0x1C;`.
   GCC 2.6.3's loop strength reduction turns the array index into exactly
   retail's `addu s0, s2, s4` with a running `0xEC + 0x1C*i` in `s4`, and
   it schedules the `addiu s4, s4, 0x1C` into the `beqz` delay slot on its
   own. The hand-rolled offset was a transcription of the strength-reduced
   asm, and GCC does not re-derive the same schedule from it.
2. Hold `list = entry->list;` in a local: retail loads it once into `s1`
   and reuses it for the store-back after `release`.

The matched C is the live definition in `src/world/DayTaskStageMap.c`.

### Proposed learning

A base-plus-running-offset loop in the asm (`addu sN, self, sOff` with
`sOff` stepping by the element size) is GCC's strength reduction of
`&self->array[i]`, and the source should say `&self->array[i]`. Writing the
byte offset by hand reproduces the arithmetic but not the schedule. (Candidate
for DECOMPILATION_LEARNINGS if it has not already been written down; check
for an existing entry on strength reduction first.)

---

## Historical record (superseded by the match above)

### (old title) StageMap__UnloadAllSlots — STALL

> Renamed from `func_8004ABD0` on 2026-09-22 (tools/rename.py). Address 0x8004abd0.

**Unit:** class_3ac78 · **Size:** 74 instructions · **Best reached:** 9/74
words, correct size, no address drift

## What it does

Walks a 7-element array of 0x1C-byte "slot" records starting at
`self+0xEC` (new struct knowledge: `StageMap::unkEC[7]`, typed
`UnkSlotEntry_3ac78`). For each slot entry: calls the "child" object's
(`entry->unk4`) own slot `+0x74`; zeroes the entry's `unk0` (u16); calls
`self->methods->slot108(self, entry)`; if the "list" object's
(`entry->unk8`) field `+0x2C` is non-NULL, refreshes it through its own
base-class `unk04` slot (same `GenericObject`/`unk04` pattern already
established by `StageMap__OnSlotEvent`); calls `self->methods->slot88(self, 6,
entry, i)` (dispatches to `StageMap__OnSlotEvent`, which per its own already-
matched signature only reads 2 of these 4 args — the extra ones are dead
at the callee, consistent with the "argument register carries no meaning
if unused by callee" case in DECOMPILATION_LEARNINGS); calls
`entry->unk4->methods->slot84(entry->unk4)` again. After the loop:
zeroes `self->unk1B8`/`unk1B4` and calls `self->methods->slot140(self)`.

New struct/vtable knowledge added regardless of the stall (all verified
straight from the disassembly): `StageMapMethods::slot88` (declared with
a 4th `s32 arg3` parameter that the occupant, `StageMap__OnSlotEvent`, doesn't
read — same "field type need not match every occupant's real signature"
precedent as `StageMap__OnNotify`/`StageMap__OnSlotEvent`'s `GetSceneNodeMethods`), `slot108`
(`StageMap__ClearSlotCells`, not decompiled), `slot140` (`StageMap__EndScaleRamp`, not
decompiled — this promotes what was previously just end-of-struct
padding into a real slot), `StageMap::unkEC[7]` (`UnkSlotEntry_3ac78`,
0x1C bytes each), `unk1B4`/`unk1B8`, and three new opaque types
(`UnkSlotEntry_3ac78`, `UnkSlotChildObj_3ac78`/`Methods`,
`UnkSlotListObj_3ac78`).

## Best-reached body (does NOT compile to retail bytes)

```c
#if 0
void StageMap__UnloadAllSlots(StageMap *self)
{
    s32 i;
    s32 offset;
    UnkSlotEntry_3ac78 *entry;

    offset = 0xEC;
    for (i = 0; i < 7; i++) {
        GenericObject *check;

        entry = (UnkSlotEntry_3ac78 *)((u8 *)self + offset);
        offset += 0x1C;
        entry->unk4->methods->slot74(entry->unk4);
        entry->unk0 = 0;
        self->methods->slot108(self, entry);
        check = entry->unk8->unk2C;
        if (check != NULL) {
            entry->unk8->unk2C = check->methods->unk04(check);
        }
        self->methods->slot88(self, 6, entry, i);
        entry->unk4->methods->slot84(entry->unk4);
    }

    self->unk1B8 = 0;
    self->unk1B4 = 0;
    self->methods->slot140(self);
}
#endif
```

## The residue: the loop-offset increment keeps landing in the WRONG
delay slot

Retail recomputes the slot pointer fresh each iteration from two
registers added together (`self` + a running byte offset, `addu
$s0,$s2,$s4`) rather than carrying an incrementing pointer across
iterations, and the offset's own increment (`addiu $s4,$s4,0x1C`) sits in
the delay slot of the `beqz $a0,...` branch that guards the `unk2C`
refresh — i.e. textually in the MIDDLE of the loop body, right after
reading `entry->unk8->unk2C`.

**Every reconstruction tried puts the increment in the FIRST available
call's delay slot (`slot74`'s `jalr`) instead, regardless of where the
increment statement sits in the C source:**

1. `entry = &self->unkEC[i];` (array indexing, fresh index each
   iteration): 8/74. Compiler additionally split the walk into TWO
   separate persisted induction-variable registers (one for
   `entry`/`unk0`, one specifically for `&entry->unk4`), which retail does
   not do — retail re-reads `entry->unk4` twice through the SAME pointer.
2. `entry++` (persisted incrementing pointer, C89-idiomatic "increment
   pointer" per the project's own `TodActor__ApplyTodPacket` lesson): 8/74. Same
   general shape as (1) but with a single induction variable; still not
   retail's "fresh add each iteration" shape.
3. `offset` accumulator (plain `s32`), `entry` recomputed from
   `self+offset` each iteration, increment written at the END of the loop
   body: regressed HARD — 0/74 with a 196KB whole-image drift. GCC
   strength-reduced this into the exact same single-incrementing-pointer
   shape as (2), i.e. the accumulator didn't survive as a literal
   register-pair recomputation.
4. Same accumulator, increment moved to right after reading `check =
   entry->unk8->unk2C;` (retail's approximate textual position): 9/74, no
   drift. This is the first attempt where `addu $s0,$s1,$s3` (recompute,
   not increment-in-place) appears at the top of the loop, matching
   retail's overall SHAPE — but the increment itself still schedules into
   `slot74`'s `jalr` delay slot, not the `beqz`'s.
5. Same, with a bare `__asm__("");` scheduling barrier inserted between
   the `slot74` call and `entry->unk0 = 0;` (attempting to block the
   later-appearing increment from floating backward across it): no
   change — 9/74, identical instruction layout. The barrier did not
   prevent the delay-slot fill from reaching backward past it, which is
   itself worth recording (see below).
6. Same, increment moved to immediately after computing `entry` (the very
   TOP of the loop, before `slot74` at all): 9/74, unchanged — the
   increment still ends up in `slot74`'s delay slot either way, meaning
   its FINAL position is apparently independent of where between "entry
   computed" and "check read" it's written in source.

**None of the four listed word-counts should be read as size-verified
except (4) and (6)** — (1)/(2) showed no `WARNING: differs OUTSIDE this
range` at 8/74 either, so those are also drift-safe; only (3) drifted.

### What's actually going on (best guess, unconfirmed)

The delay-slot filler for `slot74`'s `jalr` is choosing the offset
increment because it is the FIRST independent, side-effect-only
instruction reachable in the block, and GCC 2.6.3's `dbr_schedule` pass
here appears to search past several intervening independent instructions
(not just the immediately-next one — contrast with `StageMap__SetFootprintFromCell`'s
residue, which stayed local) to find and hoist it, regardless of textual
distance from where the C source puts it. A bare `__asm__("")` scheduling
barrier did NOT block this hoist, which contradicts the working
assumption from `TodActor__CreateParts`/`TodActor__SetLightMode` that the barrier is a
reliable local lever — worth flagging for whoever revisits the barrier's
actual scope in this compiler. The remaining structural difference (two
separate registers recomputed by addition vs. one incrementing pointer)
was successfully reproduced (attempt 4/6); only the SCHEDULING of the one
increment instruction resists.

### Proposed learning

**A bare `__asm__("")` barrier does not reliably block a later
instruction from being hoisted BACKWARD into an earlier delay slot** —
at least not past multiple intervening independent statements. This
narrows the barrier's documented scope (CLAUDE.md's "if removing it
changes ordering only, it's allowed" test already implies it's a
reordering tool, not an ordering GUARANTEE) and should be checked before
reaching for it as a fix for a "value hoisted too early" residue class,
which up to this point had only been solved by splitting COMPUTE from
STORE (`StageMap__SetFootprintFromCell`), not by barriers.

## Provenance

round 2026-09-02 (head-requested extension), runner ALPHA, unit
DayTaskStageMap. Six attempts across two structural strategies (array
indexing / incrementing pointer / offset accumulator), best 9/74 with the
loop's overall two-register shape reproduced but one instruction's
delay-slot placement unmoved. Moved on to stay within budget for the
remaining assigned functions. Restored to `INCLUDE_ASM`.

## Round 19 (echo): permuter pass (negative), on this report's own
## flagged residue class

Re-verified 9/74 first (matches exactly). Sanity-checked the permuter
scaffold with `--debug --stack-diffs`: base score 1056 (`Stack
Differences: 56` -- despite the report's own confirmed "no address
drift" via the real oracle; treating this as a scorer artifact rather
than a real frame mismatch, consistent with `--stack-diffs`' known
imprecision noted elsewhere this round for other functions).

Ran the bounded search WITHOUT `--stack-diffs` (matching this report's
own residue, a pure delay-slot-fill/scheduling question with no stack
component per the real-oracle confirmation): `timeout 600 permuter.py -j
8 --stop-on-zero --best-only`. **`permuter exit=124`** (bound fired).
**140,928 iterations**, best score progression `1000 -> 900 -> 685 ->
385 -> 240`, **no zero found**. Per the project's standing rule, this is
phrased as "not closed in 140,928 iterations," not evidence of
exhaustion -- the search improved on the permuter's OWN scoring metric
without reaching the real oracle's zero.

Did not attempt to translate the score-240 candidate to `src/` and
re-verify against the real oracle -- per CLAUDE.md, a nonzero permuter
score is never itself a result, only a lead, and with six real manual
attempts already having established the residue's exact shape (a single
instruction's delay-slot placement), a partial permuter improvement
without reaching zero does not change this function's filed status.
Remains a STALL at 9/74, `INCLUDE_ASM` restored (permuter work confined
to `permuter-work/`, gitignored, `src/` untouched this round beyond the
earlier body-preservation commit).

### Proposed learning

No new learning beyond what this report and this round's other permuter
passes already establish -- filed here mainly so the next round does not
re-run an unhinted blind search on this exact residue without first
checking whether a source-level hint (the "reduce the increment's
apparent independence to the scheduler" direction this report's own
analysis suggests, still untried) could produce a more targeted future
search than another blind 140K-iteration pass.

## ROUND 20 (runner echo): lever transferability test plus two fresh C attempts, all negative

**Per the coordinator's explicit instruction, no permuter search this
round -- time spent entirely on the C.**

**Lever transferability (the coordinator's cross-unit question):** this
function's `entry = (u8 *)self + offset` recompute is the closest
analogue in this unit to `GetRCnt`'s `base`/`entry` split, but the
lever does not map onto it the way it did there. `GetRCnt`'s
starting shape was a single expression built from a GLOBAL table
(`D_8006DCB0[idx]`) that had never been given its own name; splitting the
global reference out into its own `base` local is what created slack.
Here, `self` is already a distinct, separately-named parameter (not a
global folded into one expression), and the existing best body already
combines `self`+`offset` as ONE statement, which is independently
required (this report's own attempts 1-3 already show splitting THIS
computation into a persisted/accumulated form regresses badly). There is
no unnamed global or struct-base reference in this function's body to
extract into a second independently-live local -- **the lever has
nothing to apply to here; not tested for lack of an applicable shape**,
which is itself the answer to the transferability question for this
function specifically.

**Two fresh structural ideas tried instead, re-derived from the raw
disassembly (not resumed from the preserved body verbatim):**

1. **Moved `offset += 0x1C;` into the `for` loop's own increment clause**
   (`for (i = 0; i < 7; i++, offset += 0x1C)`), untried by any prior
   attempt (1-6 all keep it as a body statement in various positions).
   **Regressed hard: 0/74, 196263-byte drift.** GCC strength-reduced/
   restructured this into a single incrementing-pointer shape (the same
   failure mode already documented for attempt 3's plain accumulator-at-
   loop-end placement) -- confirms the for-clause position is not a
   free variant of the accumulator idea, it triggers the identical
   collapse.
2. **Removed the `check` local entirely**, reading
   `entry->unk8->unk2C` three separate times inline (once for the test,
   twice more for the refresh's LHS/RHS) instead of caching the loaded
   value in a named variable at all -- on the theory that a genuinely
   short-lived named local, rather than the increment's position, might
   be what changes how eagerly the scheduler searches backward past it.
   **Byte-identical to the existing 9/74 body, in every single word** --
   confirmed via direct `objdump` diff of the two compiled `.o` outputs
   line for line, not just the funcdiff score. This is a clean negative
   on a genuinely new axis: the named intermediate `check` is not what
   licenses the backward hoist; removing it changes nothing at all.

**Confirmed independently (via `objdump` on the actual compiled object,
not just inference from the report's prose) that the hoist is real and
exactly as described:** the increment lands in `slot74`'s own `jalr`
delay slot (`798: jalr v0` / `79c: addiu s3,s3,0x1c`), 13 instructions
and one full `jalr` call earlier than its written position, while
retail's OWN slot74 delay slot is a genuine `nop` and the increment sits
in the `beqz`'s delay slot instead -- exactly where the C statement is
textually written. GCC 2.6.3's backward delay-slot search is reaching
across an entire unrelated call to grab an available, dependency-free
instruction that retail's own build did not have available at that
point (or chose not to take), and neither changing where the increment
is worded, how it's computed (accumulator vs. for-clause), nor whether
its consumer is cached in a named local moves that choice.

**Disposition unchanged: STALL at 9/74, `INCLUDE_ASM` restored.** No new
lever found this round; the residue remains a pure GCC-internal
delay-slot-fill preference with, per this round's and the prior round's
combined evidence (6 manual attempts, 2 barrier placements, 1 permuter
pass at 140,928 iterations, and now 2 more fresh structural axes this
round), no known C-level control.

### Proposed learning

**A named intermediate local's PRESENCE OR ABSENCE is not, by itself, a
lever against a backward delay-slot hoist** -- removing `check` entirely
(reading its source expression three times inline instead) produced a
byte-IDENTICAL compiled object to keeping it as a local, confirmed via
direct object-file diff. This narrows the search space for whoever
revisits this residue: the scheduler's choice of WHICH instruction fills
`slot74`'s delay slot does not depend on how the later-consumed value is
named or cached, only on its (in this case, real) independence from
everything between its natural position and the earlier call.

## ROUND 48 (runner delta): re-verified, no new work

Rebuilt the preserved 9/74 body in-tree before touching anything (per this
round's "rebuild every recorded figure before you trust it" instruction):
`build exit=2` (no compile-error grep hits — clean compile, SHA1 mismatch as
expected), `funcdiff.py` reports **9/74 words, correct size, no address
drift** — matches the report exactly. `INCLUDE_ASM` restored immediately
after, `git diff --stat` confirmed clean.

Not re-searched or re-attempted this round: this round's assignment staffed
`StageMap__SetFootprintRect` as the priority (a proven-representative permuter target,
per its own report), and this function's own history — 8 manual attempts
across 3 prior rounds plus a 140,928-iteration permuter search that
improved on its own score but never reached zero — already exhausts every
axis this round's runner could identify without a genuinely new idea.
Flagged on the broadcast that this round's staffing table understated the
prior search depth ("~928 iters" reads like a truncation of "140,928").
Disposition unchanged: STALL at 9/74, `INCLUDE_ASM` in place.

## Naming

Round 67 (track 3, naming pass). This function is still a documented STALL;
naming applies to the report and to the preserved body's field references,
not to the shipped bytes.

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004ABD0` | `StageMap__UnloadAllSlots` | B | Occupant of vtable slot `+0x0C0`, and `DayTaskStageMap`'s matched `StageMap__Disable` calls exactly that slot immediately before clearing `enabled` -- so this is the shutdown/clear path. The body walks all seven `elems[]` entries and for each: dispatches the target's `slot74`, zeroes the entry's `flag`, dispatches `slot108`, releases the list's held object, raises `onElementEvent(self, 6, entry, i)`, dispatches the target's `slot84`. Then zeroes `unk1B4`/`unk1B8` and dispatches `slot140`. Tier B: "reset all elements" is what the loop does; why the object is reset is not established. |

The preserved `#if 0` body in `src/world/DayTaskStageMap.c` was updated to the current
field names in the same round (`entry->unk0` -> `flag`, `entry->unk4` ->
`target`, `entry->unk8` -> `list`, `self->unkEC` -> `self->elems`). Its score
and residue are unchanged -- no code was altered, only identifiers.

### Field names in the preserved bodies above

Round 67 renamed this unit's struct fields. The preserved bodies in THIS
report are left in their original spelling -- preserved code is a record of
what was tried, not doctrine -- but they will not compile as written against
the current `include/DayTaskStageMap.h`. The mapping, for whoever rebuilds one:

| old | current |
| --- | --- |
| `self->unk88` | `self->rectCount` |
| `self->unk8C` | `self->rects` |
| `self->unkEC` | `self->elems` |
| `self->unk54` | `self->origin` |
| `self->unk68` | `self->config` |
| `self->unk70` | `self->enabled` |
| `self->unkBC` | `self->cellTag` |
| `self->unk1C0` / `unk1C2` / `unk1C3` | `curCellTag` / `curCellCol` / `curCellRow` |
| `self->unk1BC` | `self->lastEventElem` |
| `entry->unk0` / `unk2` / `unk4` / `unk8` / `unkC` / `unk10` / `unk14` | `flag` / `key` / `target` / `list` / `cellParent` / `cells` / `heldObj` |
| `HistoryEntry_3ac78` / `HistoryBlock_3ac78` | `GridRect_3ac78` / `GridRectList_3ac78` |
| `->methods->unk04(...)` | `->methods->release(...)` |

The `#if 0` copy that lives in `src/world/DayTaskStageMap.c` WAS updated to the current
names in the same round, so that one still compiles; only identifiers changed
and the recorded score is unaffected.

## Track 6 (2026-09-26, round 93, alpha)

The class `Class866E8` (table `gClass866E8Methods`, id 0x114, LightRig's
subclass) is now `StageMap` (`python3 tools/renametype.py Class866E8
StageMap`, tier B): it keeps seven slots loaded with map chunks of the
current stage (LbdFile, `STGnn\Mnnn.LBD`) around a tracked target, the
centre chunk and its six staggered neighbours (`sChunkNeighbourDeltas`), laid
out by the stage's `StageGridDimensions` (`setConfig`, from ObjM's
`GetStageGridDimensions(stage)`), each slot's placements linked into a 20 x
20 lattice of GridCells whose drawn window follows the target. Tier B: the
mechanics are established; "the stage's map" rests on the files it loads and
the per-stage config. Header now `include/StageMap.h`; evidence in its banner.

Member types, same pass: `Unk68Struct` is `StageGridDimensions`
(include/StageGrid.h), `Unk54Struct` is `LongVec3` (include/SceneNode.h),
`EntryDesc866E8` is `Ratio16[3]` (include/SceneNode.h), all by layout and
use; `Class866E8Elem` -> `ChunkSlot`, `QueryPos866E8` -> `SplitLongVec3`,
`SetupEntry866E8` -> `ChunkLoadEntry`, `SetupSub866E8` ->
`ChunkLoadEntryTail`, `TargetSpec866E8` -> `ChunkSlotSpec`, `GridSlot866E8`
-> `CellRect`, `GridSlotList866E8` -> `CellRectSet`, `Bounds866E8_3bb8c_b`
-> `CellBounds`, `Class866E8ValueFn` -> `ChunkFileFn`,
`Class866E8OnElementEventFn` -> `StageMapOnSlotEventFn`,
`Class866E8ElemFn` -> `ChunkSlotFn`, `Class866E8CellFn` -> `StageMapCellFn`;
new `ChunkNeighbourDelta` for `sChunkNeighbourDeltas` (was typed as the
3-word placeholder). renametype.py also rewrote the old names inside
earlier sections' history prose in this and sibling reports (known, pending
an operator decision; not hand-reverted).

This function: `StageMap__ResetAllElements` -> `StageMap__UnloadAllSlots` (`python3 tools/rename.py StageMap__ResetAllElements StageMap__UnloadAllSlots`, tier B): every slot: cancel loads, clear cells, release the placements' LinkResource, event 6, release the data block; then `chunksLoaded` 0 and EndScaleRamp.

## Track 7 (2026-09-27, round 98, charlie)

Moved here from the `.c` comment: "Reset every one of the seven grid elements, then the two counters. Matched round 71: `&self->slots[i]` is what produces retail's base + running-offset walk (GCC's strength reduction), not a hand-rolled byte offset." The comment now says what the function does. Locals: `entry` -> `slot`, `list` -> `placements`; the loop bound is `ARRAY_COUNT(self->slots)` and the event `STAGEMAP_EVENT_SLOT_RELEASE` (was 7 and 6). Zero bytes.
