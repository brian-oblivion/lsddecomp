# FindNextStyleCueInRange -- MATCHED round 48 (alpha), 111/111 words

> Renamed from `FindNearestStyleCueEntry` on 2026-09-26 (tools/rename.py). Address 0x80055620.

> Renamed from `func_80055620` on 2026-09-23 (tools/rename.py). Address 0x80055620.

## Round 48 (alpha): Check 3 AGREE, permuter search led to a hand-applied fix, closed byte-exact

**Check 3, both checks run before searching:**

- Scaffold `--debug --stack-diffs`: `Stack Differences: 0`, `Branch
  Differences: 0`, `Register Differences: 8 (5)`, `Reorderings: 4 (60)`,
  `Insertions: 5 (100)`, `Deletions: 5 (100)` -- base score 1280.
- Rebuilt round 47's preserved body in-tree: `build exit=2`, no
  compile-error grep hits, `funcdiff.py` reads 95/111, no out-of-range
  drift (window `0x45E20-0x45FDC` matches the expected 111-word/0x1BC size
  exactly). The raw diff shows the same shifted-word pattern the scaffold's
  nonzero insertions/deletions predict.
- **AGREE**: nonzero insertions AND deletions on both scaffold and in-tree
  sides, same shape -- a meaningful search candidate per this round's
  Check-3 guidance (nonzero ins/del on both is a reason to search, not
  decline).

**Permuter search run**: `-j 6 --stop-on-zero --best-only`, bounded at 900s.
96,740 iterations completed within the wall-clock bound; best score reached
10 (down from the base-line 1280) but the search itself never found a zero.
The wrapping shell that was to append `permuter rc=$?` was reaped before
that line landed; log growth stopped at iteration 96740 in step with the
900s wall-clock and the worker pool's own normal-shutdown
`resource_tracker` warning, which is `timeout`'s SIGTERM signature --
treating the raw search as rc=124-equivalent (bound fired).

**But the score-10 candidate (`permuter-work/FindNextStyleCueInRange/output-10-1`) was
a genuine, correct lead, and applying it by hand closed the function.** Two
changes, read straight off that candidate's mutated source:

1. **`if (n <= 0) goto fail;` is REDUNDANT and can simply be dropped.** The
   candidate's mutation turned it into an empty `if (n <= 0) { }` (plus an
   inert duplicate `goto fail; goto fail;` elsewhere, a pure scheduling
   nudge in the same spirit as `TickStyle`'s dead `i++; i--;`). Tracing
   it through: when `n <= 0`, the very next statement is
   `for (j = 0; j < n; ...)`, whose own condition `j < n` is `0 < n`, false
   immediately -- so the loop body never runs and execution falls straight
   through to `fail: return 0;` regardless. The explicit early exit was
   never doing anything the loop guard didn't already do; retail simply
   never wrote it, and my build's `goto` was adding a real word.
2. **The `entry` pointer's address computation needed a different operand
   ORDER, not a different value.** The one remaining word after removing
   the redundant guard was `addu $s1,$a0,$v0` (retail) vs `addu
   $s1,$v0,$a0` (mine) -- `$a0` held `gStyleCueRecordIndex * 8` and `$v0` held
   `base`, both correctly, just encoded in the opposite operand order.
   **Four different C spellings of `base + gStyleCueRecordIndex * 8` (the original
   form, `gStyleCueRecordIndex * 8 + base`, `(EntrySlot *) base + gStyleCueRecordIndex`, and
   `&((EntrySlot *) base)[gStyleCueRecordIndex]`) all produced the SAME operand
   order** -- this is not a simple "write the addends in the other order"
   fix. What worked: `(EntrySlot *) (gStyleCueRecordIndex * 8 + (s32) base)`, casting
   `base` to `s32` explicitly before the addition rather than letting the
   pointer-plus-integer arithmetic happen implicitly. Confirmed byte-exact:
   `build exit=0`, `OK: build matches retail SLPS_015.56`,
   `funcdiff.py`: 111/111.

**Final body (matched):**

```c
void *FindNextStyleCueInRange(void *arg0, s32 *arg1, void *arg2) {
    s32 j, n;
    u8 *base;
    EntrySlot *entry;
    LocalBuf buf;
    s32 d1, d2, dist;
    void *self;

    if (arg2 == 0) {
        goto fail;
    }
    base = gStyleCueRecordLists[gStyleStage];
    n = sStyleCueRecordCounts[gStyleStage] - gStyleCueRecordIndex;
    entry = (EntrySlot *) (gStyleCueRecordIndex * 8 + (s32) base);
    for (j = 0; j < n; j++, entry++) {
        gStyleCueRecordIndex++;
        if (entry->count > 0) {
            buf.pos = entry->pos;
            buf.tab = *(TabEntry *) (sStyleCueOffsets + entry->idx * 6);
            self = (void *) gStyleGrid;
            ((ObjAB4C *) self)->methods->slotE8((ObjAB4C *) self, arg0, &buf);
            d1 = *(s32 *) arg0 - *(s32 *) arg2;
            if (d1 < 0) {
                d1 = ~d1 + 1;
            }
            d2 = *(s32 *) ((u8 *) arg0 + 8) - *(s32 *) ((u8 *) arg2 + 8);
            if (d2 >= 0) {
                dist = d1 + d2;
            } else {
                dist = d1 - d2;
            }
            *arg1 = dist;
            if (dist < sStyleCueDistanceTable[entry->count]) {
                return entry;
            }
        }
    }
fail:
    return 0;
}
```

Needs (already present earlier in the unit, in strict ROM order): the
`ObjAB4C`/`ObjAB4CMethods`/`Pos4`/`TabEntry`/`EntrySlot`/`LocalBuf` types and
`extern s32 gStyleStage, gStyleCueRecordIndex, gStyleGrid, sStyleCueDistanceTable[];`,
`extern u8 *gStyleCueRecordLists[], sStyleCueRecordCounts[], sStyleCueOffsets[];` (all already
declared in `src/world/ObjMStyleActor.c` ahead of this function).

### Proposed learning

**"Two operands of a commutative add both hold the right value, but the
generated instruction encodes them in the wrong order" is a real, fixable
residue class -- but the fix is not "swap the order you wrote them in the
C".** Four textually-different-but-semantically-identical spellings of
`base + offset` (reordering the addends, rewriting as pointer-plus-array-
index, rewriting as an explicit array-subscript-of-cast) all produced the
IDENTICAL wrong operand order. What actually flipped it was forcing the
pointer through an explicit `(s32)` cast before the addition, changing how
GCC 2.6.3's tree builder sees the expression's types (pointer-plus-int vs
int-plus-int) rather than which addend is written first. Worth trying
`(s32) ptr + int` as its own idiom, distinct from reordering, the next time
a pure-operand-order residue shows up.

**A near-zero (not literally 0) permuter score is still worth reading, not
just a strict zero** -- this project's own "a permuter zero is a lead, not
an answer" rule already implies verifying candidates, but this round's
score-10 candidate (not a 0) contained a genuinely correct structural
insight (the redundant early-exit) mixed with an inert scheduling nudge
(the dead duplicate `goto`) and a residue the search itself never actually
resolved (the operand order, fixed instead by hand once the redundant-guard
insight cleared the fog around it). Reading a low-score candidate's diff
against its parent, rather than requiring literal 0, surfaced a real
8/9-of-the-gap lever the search's own scoring couldn't finish closing on
its own.

## Round 47 (bravo). Cold fresh, never worked before. Length matches retail
exactly (0x1BC bytes / 111 words both sides, confirmed via `build/lsdde.map`).
First real diff off `asm-differ`: word 15 (`0x45e58`, vram `0x80055658`),
retail `move $s2,zero` vs built (same instruction, present, but scheduled
several instructions later).

## Signature (recovered with confidence) -- widened from the existing 2-arg forward declaration

```c
void *FindNextStyleCueInRange(void *arg0, s32 *arg1, void *arg2);
```

The pre-existing forward declaration in this unit (`TryStartStyleCue`'s block,
itself already MATCHED) declared only `(s32 *arg0, s32 *arg1)` -- but the
raw disassembly plainly reads and branches on `$a2` (`beqz $s4,...` as the
very first real instruction), and objdump of the ALREADY-MATCHED
`TryStartStyleCue.o` shows **nothing sets `$a2` before that call** -- `$a2`
still holds whatever `TryStartStyleCue` itself received as ITS OWN third
argument (`ctx`, a genuinely dead-looking parameter per
`TickStyle`'s report) when `TryStartStyleCue` was entered. So `ctx` is
silently forwarded as `FindNextStyleCueInRange`'s third argument, at zero cost (no
instruction sets `$a2`, since it already holds the right value from
function entry).

**Updated `TryStartStyleCue`'s call site to make this explicit**
(`FindNextStyleCueInRange(&arg0->unk4, &arg0->unk10, arg2)`), and confirmed this does
NOT disturb `TryStartStyleCue`'s own already-matched bytes: rebuilt the whole
image with the change and `TryStartStyleCue` still scores 45/45 with a clean
`OK: build matches retail SLPS_015.56`. This is the "a call argument that's
already resident needs no source-level move" case, not a widening that
costs the caller anything.

## What the function does (recovered from the raw disassembly, high confidence)

Scans a run of 8-byte records starting at a base pointer for one whose
"count" field is positive and whose Manhattan-style distance to `arg2`
(passed to a dispatch callback first) is under a per-record threshold,
returning the first such record or `NULL`.

```c
extern s32 gStyleStage;
extern s32 gStyleCueRecordIndex;
extern u8 *gStyleCueRecordLists[];    /* word array of base pointers, indexed by gStyleStage */
extern u8 sStyleCueRecordCounts[];    /* byte array of counts, same index */
extern u8 sStyleCueOffsets[];    /* table, 6-byte stride entries */
extern s32 sStyleCueDistanceTable[];   /* word table, indexed by entry->count */

typedef struct Pos4 { s16 hi, lo; } Pos4;              /* 4B, alignment 2 */
typedef struct TabEntry { Pos4 head; s16 tail; } TabEntry;   /* 6B */
typedef struct EntrySlot {                              /* 8B stride */
    Pos4 pos; u8 idx; u8 pad5; s8 count; u8 pad7;
} EntrySlot;
typedef struct LocalBuf { Pos4 pos; TabEntry tab; } LocalBuf;   /* 10B */

void *FindNextStyleCueInRange(void *arg0, s32 *arg1, void *arg2) {
    s32 j, n;
    u8 *base;
    EntrySlot *entry;
    LocalBuf buf;
    s32 d1, d2, dist;
    void *self;

    if (arg2 == 0) {
        goto fail;
    }
    base = gStyleCueRecordLists[gStyleStage];
    n = sStyleCueRecordCounts[gStyleStage] - gStyleCueRecordIndex;
    if (n <= 0) {
        goto fail;
    }
    entry = (EntrySlot *) (base + gStyleCueRecordIndex * 8);
    for (j = 0; j < n; j++, entry++) {
        gStyleCueRecordIndex++;
        if (entry->count > 0) {
            buf.pos = entry->pos;
            buf.tab = *(TabEntry *) (sStyleCueOffsets + entry->idx * 6);
            self = (void *) gStyleGrid;
            ((ObjAB4C *) self)->methods->slotE8((ObjAB4C *) self, arg0, &buf);
            d1 = *(s32 *) arg0 - *(s32 *) arg2;
            if (d1 < 0) {
                d1 = ~d1 + 1;
            }
            d2 = *(s32 *) ((u8 *) arg0 + 8) - *(s32 *) ((u8 *) arg2 + 8);
            if (d2 >= 0) {
                dist = d1 + d2;
            } else {
                dist = d1 - d2;
            }
            *arg1 = dist;
            if (dist < sStyleCueDistanceTable[entry->count]) {
                return entry;
            }
        }
    }
fail:
    return 0;
}
```

Notes on the recovery:

- **The two unaligned 4-byte struct copies (`buf.pos = entry->pos;` and
  `buf.tab = *(TabEntry *) (...)`) are the CLAUDE.md idiom**: a struct whose
  members are all `s16` has alignment 2, forcing GCC 2.6.3 to emit
  `lwl`/`lwr` + `swl`/`swr` for a whole-struct assignment instead of an
  aligned `lw`/`sw` pair -- confirmed byte-for-byte against retail's
  `lwl 3(s1)/lwr 0(s1)` + `swl 0x13(sp)/swr 0x10(sp)` sequences.
- **The `ObjAB4C`/`ObjAB4CMethods` local view (self-dispatch through method
  slot `+0xE8`) is the SAME idiom `TickStyle` establishes** for
  `gStyleGrid` -- moved that typedef earlier in the unit (it originally sat
  just before `TickStyle`, which is ROM-later) since this function,
  ROM-earlier, also needs it. No behavioural change, pure reordering of a
  type declaration.
- **The `~x + 1` / `if (d2 >= 0) dist = d1+d2; else dist = d1-d2;` distance
  computation reuses two already-established levers from this unit's round
  46 report** (`~x+1` instead of `-x` for the delay-slot-friendly negate;
  the explicit if/else "combine" shape `IsStyleCueNear` already uses for an
  identical Manhattan-distance pattern in this same unit).
- `entry->count` is read from memory **twice** by retail (once for the
  `> 0` guard, again for the `sStyleCueDistanceTable[entry->count]` index) rather than
  cached in a local -- matched by not caching it in the C either.

## The stall: preamble scheduling order

Retail places `j = 0` (as `move $s2,zero`) as the literal first instruction
after the `ctx == 0` guard, BEFORE either of the two `gStyleStage`-indexed
lookups. My build computes both lookups first and initializes `j` last
(as part of the `for`'s own init clause), which is semantically identical
but produces a different instruction SCHEDULE around the two lookups (see
`asm-differ`, words 15-30).

**Two variations tried:**

1. Hoisting `j = 0;` to right after the `ctx == 0` guard (before `base`/`n`):
   made it WORSE and reintroduced a genuine SIZE drift (0x1C4 vs 0x1BC, 8
   bytes over) -- reverted.
2. Swapping the order of `base = ...;` and `n = ...;`: no change either way
   (byte-identical to not swapping) -- reverted, kept the order that reads
   more naturally against retail's own literal instruction sequence.

Neither variation closed the gap; the 95/111 body above is the best
reached. No `register T v asm("$N")` or operand constraint was tried or
used, per HARD RULE 6.

## Attempts

5 real builds: (1) initial body (guard as plain `if/return`), badly wrong
control flow (GCC tail-duplicated the two early returns instead of sharing
one exit, per the diff showing a spurious inline `move v0,zero`/jump) --
root-caused via `asm-differ` reading the actual jump targets; (2) rewrote
both early exits as `goto fail;`/`fail: return 0;` (this project's
documented "unify identical early-exit tails" idiom): fixed the size to
match exactly and the shared-exit structure, 1/111 (everything after the
preamble shifted by one register/instruction due to the preamble
scheduling difference); (3) hoisted `j = 0` earlier: worse, reintroduced
drift, reverted; (4) reverted to (2)'s statement order, computed `base`
before `n` (matching retail's literal lookup order): 95/111, current best;
(5) swapped `n`/`base` order: no change, reverted to (4)'s order for
readability. Restored `INCLUDE_ASM`; (4)'s body is preserved in `#if 0`.

### Proposed learning

**Two early exits returning the SAME value need an explicit `goto`-to-a-
shared-label, not two separate `return 0;` statements, even when GCC could
in principle tail-merge them.** Without it, GCC 2.6.3 tail-duplicated the
second guard's `return 0;` into its own inline copy (`move v0,zero` + `j`
to the epilogue) instead of sharing the first guard's exit point --
producing a SIZE change (4 extra bytes) rather than a near-miss. This is a
same-value variant of the already-documented "different value needs
`goto`" lever (`New_Pad`): here the values are identical, but the
shared-label unification still doesn't happen for free.

## Naming

**`FindNextStyleCueInRange`, tier B.**

Scans a run of 8-byte `EntrySlot` records (`gStyleCueRecordIndex` onward)
for one whose `count` field is positive and whose Manhattan-style distance
to `arg2` is under a per-record threshold (`sStyleCueDistanceTable[entry->count]`),
returning the first such record or `NULL` and writing the computed distance
through `arg1`. "Nearest" is a simplification: it is actually the FIRST
record under threshold in scan order, not a true nearest-of-all-candidates
search -- named for the dominant behaviour (early-return on first hit) since
no caller distinguishes "first under threshold" from "globally nearest".
MATCHED, 111/111.

## Round 93 polish (delta, track 7)

### Naming

**Renamed from `FindNearestStyleCueEntry`, tier A.** The loop returns the FIRST unclaimed record (cue > 0) whose X+Z distance to the target is under its cue's `sStyleCueDistanceTable` entry, advancing `gStyleCueRecordIndex` past every record it looks at; it never compares candidates, so "nearest" said more than the body does.

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_800876B4` | `gStyleCueRecordLists` | A | per-stage pointer to the stage's 8-byte cue records. |
| `D_800876EC` | `sStyleCueRecordCounts` | A | per-stage record count, the loop bound. |
| `D_800874EC` | `sStyleCueOffsets` | A | 6-byte s16 x/y/z entries, indexed by the record's byte 4; copied after the record's 4 cell bytes to make the 10-byte cell key (StageMap's Descriptor10 shape) computeCellOffsets turns into a world position. |

The `+0x0E8` local view on `gStyleGrid` is StageMap's computeCellOffsets. Locals: `pos`, `outDist`, `target`, `remaining`, `records`, `dx`, `dz`, `grid`.

### Comments moved here from src/world/ObjMStyleActor.c

Verbatim as they stood before the round-93 comment pass (identifiers already carry this round's renames).

```c
/* MATCHED round 48 (alpha), 111/111 -- see docs/match-reports/FindNextStyleCueInRange.md
 * for the round 47 (bravo) recovery and the round 48 permuter lead that
 * closed it: the `if (n <= 0) goto fail;` early exit is redundant (the
 * `for (j = 0; j < n; ...)` loop already falls through to the same
 * `fail: return 0;` when n <= 0) and dropping it, plus writing the
 * `entry` pointer's address computation as `offset + (s32) base` instead
 * of `base + offset`, closed the last word (a pure commutative-operand
 * encoding-order residue in the `addu`). */
```
