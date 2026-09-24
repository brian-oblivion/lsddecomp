# Class866E8__DispatchToRectCells — MATCHED round 71 (charlie): 117/117, byte-exact, whole-image SHA1 green

REVISITED, round 71: MATCHED (3 builds); names/types not relevant (the
fix was the order of loop increments; no field, type or name changed).

## Round 71 (charlie) — the match

**Rebuilt as given first.** The preserved body rebuilt at **95/117,
insertions 3 / deletions 3, 21 positional skeleton diffs**, correct length.
Both the score and the "instruction-scheduling residue" class were
accurate; what nobody had tried is that the residue was the ORDER of the
comma-separated `for` increments, which GCC 2.6.3 emits in source order and
which the scheduler then cannot fully undo.

Controlled, one change per build:

| build | change | result |
| --- | --- | --- |
| 1 | preserved body as given | 95/117, ins/del 3/3 |
| 2 | outer `i++, entry++` -> `entry++, i++` | outer tail exact; ins/del 1/1 (one extra `nop` in the inner tail, which drifts the rest of the function, hence a raw 74/117) |
| 3 | inner `col++, cell++` -> `cell++, col++` | **117/117, 0/0, whole image green** |

Also measured: moving `cell++` to the END of the inner body (after the
chain walk) instead of the increment clause is ALSO byte-exact. The live
source uses the increment-clause spelling because it matches the outer
loop's.

What the two orders do to the asm:

- **Outer:** with `entry++` first, the spilled `entry` (`0x10($sp)`) is
  reloaded on the fall-through path BEFORE the join label and incremented
  first; `i++` follows the `rectCount` load and `s2 += 0xC` lands in the
  delay slot. With `i++` first, the reload sits after the label and the
  store lands in the delay slot.
- **Inner:** with `cell++` first, the `lh width` reload is scheduled ahead
  of `addiu s1, s1, 1` and fills its own load delay; with `col++` first
  the load follows the increment and costs a `nop`.

The matched C is the live definition in `src/class_3ac78.c`.

### Proposed learning

The order of comma-separated expressions in a `for` increment clause is a
scheduling lever in GCC 2.6.3: the increments are emitted in source order,
and when one of them is a spilled pointer or feeds a reload of the loop
bound, the scheduler does not reorder it back. A residue of "one
increment on the wrong side of a load / label" at a loop tail should try
swapping the clause's order before any permuter run. (Round 19's permuter
ran 140k+ iterations on a sibling without finding this; a permuter mutates
expressions, and may not reorder a comma expression.)

---

## Historical record (superseded by the match above)

### (old title) Class866E8__DispatchToRectCells

> Renamed from `func_8004B100` on 2026-09-22 (tools/rename.py). Address 0x8004b100.

**Unit:** class_3ac78 · **Size:** 117 words · **Status:** STALL, instruction-
scheduling residue · best reached 95/117.

## What it does

Walks `self->unk8C` (up to `self->unk88` `HistoryEntry_3ac78` entries).
For each entry: resolves `self->unkEC[entry->elemIdx]` and, if that slot's
child object's `+0x2C` field is nonzero, walks a `entry->width` x
`entry->height` rectangle of a 2D pointer grid (`slot->unk10`, row stride
20 cells) starting at `(entry->col, entry->row)`. For every grid cell
visited: records a "current pass" tag (`self->unkBC` copied to
`self->unk1C0`) and the cell's column/row (`self->unk1C2`/`unk1C3`,
truncated to bytes), then calls `NotifyGridCell` on the cell's own value AND
on every object chained off it via `->unk38` (a singly-linked "several
objects share one grid cell" list).

## The C (closest attempt, 95/117 -- correct branches, correct registers
almost everywhere, one scheduling residue in the inner loop tail)

```c
extern void NotifyGridCell(Class866E8 *self, UnkListObj_3ac78 *arg1, s32 arg2);

void Class866E8__DispatchToRectCells(Class866E8 *self, UnkListObj_3ac78 *arg1, s32 arg2)
{
    s32 i;
    s32 row;
    s32 col;
    HistoryEntry_3ac78 *entry;
    UnkSlotEntry_3ac78 *slot;
    Class866E8 **cell;
    Class866E8 *obj;

    entry = self->unk8C.e;
    for (i = 0; i < self->unk88; i++, entry++) {
        slot = &self->unkEC[entry->elemIdx];
        if (slot->unk4->unk2C != 0) {
            cell = (slot->unk10 + entry->col) + entry->row * 20;
            for (row = 0; row < entry->height; row++) {
                for (col = 0; col < entry->width; col++, cell++) {
                    self->unk1C0 = self->unkBC;
                    self->unk1C2 = entry->col + col;
                    self->unk1C3 = entry->row + row;
                    NotifyGridCell(*cell, arg1, arg2);
                    for (obj = (*cell)->unk38; obj != NULL; obj = obj->unk38) {
                        NotifyGridCell(obj, arg1, arg2);
                    }
                }
                cell += 20 - entry->width;
            }
        }
    }
}
```

## New/corrected header content (`include/class_3ac78.h`)

This unit has no additive-only constraint (nobody else holds
`class_3ac78.h`), so these are real corrections, not just additions.
Every one was independently reverified after the change: `build exit=0`
and `Class866E8__ApplyToSenderFootprint`/`NotifyGridCell` (the two ALREADY-MATCHED functions
whose bytes depend on these types) stayed at their full-match scores
throughout.

- **`HistoryEntry_3ac78` shrunk from 0x10 bytes (3-element array,
  4 bytes of guessed trailing padding per element) to 0xC bytes
  (4-element array, zero padding), and its fields renamed
  (`unk0`->`elemIdx` retyped `void*`->`s32`, `unk4`->`col`, `unk6`->`row`,
  `unk8`->`width`, `unkA`->`height`).** Two independent pieces of
  evidence: `Class866E8__FindElemIndexByUnk30` (slot124's real occupant, already retyped
  round 8 to return an `s32` index) feeds `Class866E8__SetFootprintRect`'s write into
  this field; THIS function reads that same first field back and uses it
  exactly as an index -- `&self->unkEC[elemIdx]`, reproduced by retail as
  a multiply-by-0x1C (`UnkSlotEntry_3ac78`'s own confirmed size). The
  four `s16`s that follow are read as a dense, gapless run (offsets
  +4/+6/+8/+0xA, next element starts at +0xC) -- the same shape as the
  SIBLING unit's `GridSlot866E8` (`include/class_3bb8c.h`: index + col +
  row + width + height), independent view, not a shared C type.
  `HistoryBlock_3ac78`'s total size stays 0x30 (4 x 0xC == 3 x 0x10), so
  `Class866E8__ApplyToSenderFootprint`'s whole-struct copy is unaffected -- confirmed, still
  79/79.
- **`UnkSlotEntry_3ac78::unk10` retyped from `GenericObject **` to
  `Class866E8 **`.** This function forwards a grid cell's raw value
  straight into `NotifyGridCell`, which dereferences `self->flags36` (a
  genuine `Class866E8` field) and (via the chain below) `->unk38` --
  both real `Class866E8` fields, not `GenericObject`'s (which has only a
  `methods` pointer). Safe: the old typing was evidence-only prose in a
  comment, attached to `Class866E8__Finalize` which is still `INCLUDE_ASM` and
  has never compiled against it.
- **`Class866E8::unk38` added** (`Class866E8 *`, +0x038, carved out of
  what was `pad038`): a singly-linked "several instances share one grid
  cell" chain pointer, walked by this function -- the same idiom as the
  sibling unit's `EntryChildObj::unk38` (`include/class_3bb8c.h`).
- **`Class866E8::unkBC` added** (`u16`, +0x0BC, carved out of the start of
  `pad0BC_tail`): copied verbatim into `unk1C0` on every grid cell
  visited.
- **`Class866E8::unk1C0`/`unk1C2`/`unk1C3` added** (`u16`/`u8`/`u8`,
  +0x1C0/+0x1C2/+0x1C3, carved out of the front of the previously fully
  opaque `unk1C0[0x1E8-0x1C0]` byte array). `Class866E8__GetCurrentCellKey`
  (`return &self->unk1C0;`) is unaffected -- it only ever takes the
  ADDRESS, which is identical regardless of the pointee's declared type.
- **`UnkSlotChildObj_3ac78::unk2C` added** (`s16`, +0x2C): gates the
  whole per-history-entry grid walk (nonzero test).
- **`NotifyGridCell` widened from 1 param to 3**
  (`Class866E8 *self, UnkListObj_3ac78 *arg1, s32 arg2`). This function's
  own call sites set up real, explicit `move` instructions for `$a1`/`$a2`
  before every call (unlike `GetClass6B5CCMethods`'s "leftover, already-there"
  args elsewhere in this unit -- see that symbol's own do-not-reconcile
  note), so the call needs a matching 3-param prototype to compile at
  all. Confirmed harmless to `NotifyGridCell`'s own already-matched body:
  neither extra param is read, and this target does not reserve stack
  space for unused trailing args, so its bytes are unaffected --
  reverified 18/18 after the widening.

## The residue: one scheduling swap at the inner loop's tail, resistant to
every restructuring tried

Everything else about this function's shape is right: the outer/middle/
inner loop nesting, every branch target, every register identity
(including the trickiest one -- see below), the whole grid-cell/chain-walk
dispatch block, and the epilogue all match byte-for-byte. The ONLY
remaining difference is at the column loop's tail:

```
retail:  lh v0,-2(s2)      ; reload entry->width
         addiu s1,s1,1     ; col++            (fills the lh's delay slot)
         slt v0,s1,v0
         bnez v0,<loop>
          addiu s3,s3,4    ; cell++

mine:    addiu s1,s1,1     ; col++
         lh v0,-2(s2)      ; reload entry->width
         nop               ; <- one extra instruction, nothing to fill the delay slot
         slt v0,s1,v0
         bnez v0,<loop>
          addiu s3,s3,4    ; cell++
```

Retail schedules the (independent) `col++` INTO the `lh`'s load-delay
slot; mine emits `col++` first and cannot fill the following `lh`'s delay
slot with anything, so retail's word count is one instruction shorter than
what any of my attempts's tail produced. This single extra word shifts
every following branch target (and offset-difference report) by 4 bytes,
which is why the raw diff below word ~80 looks large -- it is the SAME
content, shifted by one word.

**A register-identity swap this same function had (and which WAS
fixable) should not be confused with the residue above.** The very first
attempt at this function's grid-cell pointer computation (writing it as
two separate statements, `cell = slot->unk10 + entry->col; cell +=
entry->row * 20;`) put the whole computation in a callee-saved register
one statement too early, landing it in `$s2` where retail keeps it in a
transient `$a0`/`$v0` pair until the very last `addu`, committing only
the FINAL value to `$s3`. Writing it as ONE combined statement,
`cell = (slot->unk10 + entry->col) + entry->row * 20;`, fixed this
completely (47/117 -> 95/117 in one attempt) -- this was NOT a
register-identity stall, just an artifact of splitting one retail
expression across two C statements. Recorded here so nobody re-derives
it: **when a pointer computation is one retail expression, keep it one
C statement, even when a two-statement form reads more clearly** -- the
extra statement boundary can force an early commit to a permanent
register.

**Attempts against the ACTUAL residue (col-loop tail), all no-ops or
regressions:**

1. Combined `for` increment (`col++, cell++`) vs `cell++` moved to a
   separate trailing statement in the loop body: the latter REGRESSED
   (70-90/117 depending on combination) and reintroduced whole-function
   size drift -- the combined-increment form is strictly better.
2. Swapping the increment-clause order (`cell++, col++` vs `col++,
   cell++`): regressed with drift.
3. Rewriting the column loop as an explicit `while` with `col++;
   cell++;` as trailing statements: byte-IDENTICAL output to the
   combined-`for` form (95/117, same residue) -- confirms this isn't a
   for-vs-while shape question.
4. Rewriting as `if (col < width) { do { ... col++; cell++; } while
   (col < width); }` (spelling out what GCC does internally for a
   pretest loop): also byte-identical, 95/117.
5. A bare `__asm__("")` scheduling barrier tried at THREE positions: as
   the function's first statement (regressed hard, 72/117, perturbed
   unrelated register allocation elsewhere -- do not use this
   placement); as the last statement of the column-loop body; as the
   first statement of the column-loop body. Neither of the latter two
   changed the score at all.
6. Reordering the grid-pointer statement (`entry->row * 20 + entry->col`
   vs `entry->col + entry->row * 20`): the row-first order reintroduced
   drift; col-first (kept) is correct.
7. Declaration-order permutation of the local pointer variables
   (`cell` before `slot`): no effect on the residue (as expected --
   CLAUDE.md's documented "declaration order" lever is for a DIFFERENT
   residue class than this one).

## Proposed learning

> **A "one extra instruction fills what would otherwise be a load-delay
> slot" residue at a loop's tail can be immune to for/while/do-while
> restructuring AND to `__asm__("")` placement** -- tried all of the
> above on this function's column-loop tail with zero effect either way
> (same score for every control-flow shape, and the barrier only ever
> hurt, never helped, at every position tried). Only one thing in this
> function actually moved the needle: keeping a SPLIT-EXPRESSION pointer
> computation as ONE C statement instead of two (see above) -- worth
> trying that lever FIRST on a similar "computed pointer used across a
> loop, register lands in the wrong callee-saved slot" residue, since it
> is cheap and, here, closed 48 of the function's 70 remaining words in
> one attempt. The load-delay-slot-fill class itself remains open; this
> function is the second STALL this round in `class_3ac78`/`class_3bb8c_c`
> where a source-level restructuring provably could not reach a
> retail-chosen instruction SCHEDULE, even though the SHAPE (branches,
> registers) was already fully correct -- worth a permuter pass (see
> `docs/PARALLEL-RUNS.md` Gate 3) rather than more manual attempts.

## Round 19 (echo): permuter pass on this report's own recommendation (negative)

Re-verified the 95/117 claim first (matches exactly, `Stack Differences:
0` confirmed via `permuter.py --debug --stack-diffs`, so no frame-size
component hiding in this residue). Followed this report's own
recommendation and ran the permuter -- as background work, not blocking
on it, per this round's instructions -- while doing hand work on other
functions in this round's list.

`timeout 600 permuter.py -j 8 --stop-on-zero --best-only`, uncontended
(system load average ~1-2, not round-18's five-runner saturation).
**`permuter exit=124`** (bound fired, not an external kill).
**122,037 iterations, best score NEVER moved off the base 330 even
once** (`grep "found new best score"` on the full log: zero matches) --
a stronger negative than the usual "best score improved but no zero"
outcome, since here the random mutator could not find ANY improvement
at all, not even a partial one, in over 100,000 tries.

Filing unchanged as STALL at 95/117, `INCLUDE_ASM` restored (all
experimentation happened in `permuter-work/`, `src/` untouched this
round beyond inlining the preserved body as `#if 0` for durability).

### Proposed learning

**A permuter run that finds literally zero improvement across 120K+
iterations (not just zero MATCHES, zero improvements of any size) is a
different and stronger signal than the usual "best score dropped some
but not to zero" result**, and is worth distinguishing in future reports:
it suggests the specific residue (here, a single delay-slot-fill
scheduling choice at a loop's tail) sits in a part of the search space
GCC-2.6.3-targeted random statement-level mutation essentially never
reaches -- consistent with this report's own characterization of the
residue as a scheduler-internal decision with no direct C-source lever,
now with a large-sample-size permuter result agreeing rather than
merely a manual attempt list agreeing with itself.

## ROUND 20 (runner echo): tested the `GetRCnt` two-independently-live-locals lever -- regressed, negative

Per the coordinator's cross-unit transferability question (does the lever
that closed 5/7 residue words on `code_179d8_c`'s `GetRCnt` --
splitting a combined `base = tableBase; entry = &base[idx];` into two
independently-live locals instead of one combined expression -- transfer
to `class_3ac78`), tested it against this function's own base+index
pointer computation, the closest analogue in this unit:

```c
Class866E8 **base;
...
base = slot->unk10;
cell = (base + entry->col) + entry->row * 20;
```

(kept as one combined STATEMENT for `cell` itself, per this report's own
already-established finding that splitting `cell`'s assignment across two
statements regresses -- only `slot->unk10` was pulled out into its own
named local, exactly mirroring `GetRCnt`'s `base`/`entry` shape.)

**Result: regressed, 95/117 -> 92/117, same size (no drift, confirmed --
`0x3B900-0x3BAD4`, 117 words both sides).** The extra live pointer
disturbed register allocation elsewhere in the function without touching
the actual residue (the column-loop tail's delay-slot fill, still
present, still the same shape) -- a strictly worse result, not a neutral
rephrasing. Reverted immediately; `git diff --stat` confirmed clean.

**Transferability verdict for this function: negative.** This is the
SECOND unit-specific negative for the lever (after `ResetRCnt` in
`code_179d8_c` itself), and the failure mode is the same shape both
times: introducing a new independently-live local costs register
pressure this function's existing allocation doesn't have slack for, even
though the underlying address computation LOOKS structurally identical
to `GetRCnt`'s. This function's own already-established fix (keep
the base+index arithmetic as ONE C statement, no intermediate name at
all) is the opposite lever from the one that helped `GetRCnt` --
worth noting as a discriminator: `GetRCnt`'s starting point was a
single combined expression that NEEDED splitting to free up allocator
slack; this function's starting point already required staying combined
to AVOID an early register commit. The two functions differ in which
failure mode their existing register pressure is closer to, and the
lever only helps the first kind.

## ROUND 48 (runner delta): re-verified, no new work

Rebuilt the preserved 95/117 body in-tree before touching anything: `build
exit=2` (no compile-error grep hits), `funcdiff.py` reports **95/117 words,
correct size, no address drift** — matches the report exactly. `INCLUDE_ASM`
restored immediately after, `git diff --stat` confirmed clean.

Not re-searched or re-attempted this round: this round's assignment staffed
`Class866E8__SetFootprintRect` as the priority. Flagged on the broadcast that this round's
staffing table called this function's prior search "shallow" when its own
report already records a 122,037-iteration permuter run that never moved
off the base score even once — the deepest completed search in this unit's
history, and about as strong a negative as a permuter run produces.
Disposition unchanged: STALL at 95/117, `INCLUDE_ASM` in place.

## Naming

Round 67 (track 3, naming pass). Still a documented STALL; naming applies to
the report and to the preserved body's identifiers, not to the shipped bytes.

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B100` | `Class866E8__DispatchToRectCells` | B | Walks `rects[0 .. rectCount)`; for each rectangle selects `elems[rect->elemIdx]`, skips it unless the element's target is live, then walks the rectangle's cells with a row stride of 20, and for each cell writes the current-cell record (`curCellTag`/`curCellCol`/`curCellRow`) and calls `NotifyGridCell` on the cell and on every cell chained behind it. Tier B: the traversal is certain, the purpose of the notification is not. |

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `Class866E8+0x0BC` | `cellTag` | C-ish/B | Copied verbatim into `curCellTag` before every cell visit and never otherwise touched here. `class_3bb8c` reads the same offset as the first halfword of a 10-byte `Descriptor10` that `func_8004B38C` block-copies in. The name records only "the tag stamped onto each visited cell". |
| `Class866E8+0x1C0` | `curCellTag` | B | Written per cell visit; `Class866E8__GetCurrentCellKey` returns its address. |
| `Class866E8+0x1C2` | `curCellCol` | A | Written per cell visit with the rectangle's start column plus the inner loop offset. |
| `Class866E8+0x1C3` | `curCellRow` | A | Same, row. |
| `Class866E8+0x038` | `nextInCell` | B | The chain this function walks off each grid cell; `class_3bb8c`'s `Class866E8__SetFootprintCellFlag` walks the identical chain off `EntryChildObj::unk38`. |

**Type caveat, recorded not fixed.** This unit declares the cell type as
`Class866E8 *`. `class_3bb8c`'s independently derived view says
`EntryChildObj *`, and its evidence is better: the ctor here ORs `0x80000000`
into each freshly built cell's `+0x010`, which is `EntryChildObj::unk10`
exactly (`func_8004C0AC` sets the same bit, matched `Class866E8__SetFootprintCellFlag` clears
it), and `flags36`/`nextInCell` line up with `EntryChildObj::unk36`/`unk38`.
Unifying the two views is track-4 work, so the declared type is unchanged and
a note sits on the field in `include/class_3ac78.h`. Posted to the broadcast.

### Field names in the preserved bodies above

Round 67 renamed this unit's struct fields. The preserved bodies in THIS
report are left in their original spelling -- preserved code is a record of
what was tried, not doctrine -- but they will not compile as written against
the current `include/class_3ac78.h`. The mapping, for whoever rebuilds one:

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

The `#if 0` copy that lives in `src/class_3ac78.c` WAS updated to the current
names in the same round, so that one still compiles; only identifiers changed
and the recorded score is unaffected.
