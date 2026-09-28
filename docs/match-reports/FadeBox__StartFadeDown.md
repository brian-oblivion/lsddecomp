# FadeBox__StartFadeDown -- MATCHED (35/35, round 73; was STALL "scheduling residue, 29/35")

> Renamed from `Class6E99C__StartFadeDown` on 2026-09-26 (tools/rename.py). Address 0x80040024.

> Renamed from `FadeBox__StartFadeToIndex` on 2026-09-26 (tools/rename.py). Address 0x80040024.

REVISITED, round 73: MATCHED 35/35 (whole-image SHA1 green); names/types used

## Round 73 (runner bravo): MATCHED -- the residue was ARITY, not scheduling

**Preserved body rebuilt first, unchanged** (the `#ifdef NON_MATCHING` block,
identical to "Best body reached" below): `29/35 words match`,
`insertions 2 / deletions 2`, `positional skeleton diffs 6`, no outside-range
drift. So the round-14 description "zero insertions/deletions" was a reading
of the diff, not funcdiff's opcode-level figure: at 2/2 the `ori $a1` and the
`lw` of `self->methods` genuinely sit in different slots of the sequence.

**Re-derived class.** Retail sets NO argument register before the
`configure` `jalr` except `$a0` (which already holds `self`), and
`configure`'s occupant, `FadeBox__Configure`, reads `$a1`..`$a3` (it is
defined `(self, a1, a2, a3)`: a2 is the returned index, a3 is stored to
`unk7C`). Per LEARNINGS 3f ("MIPS o32 fills argument registers strictly left
to right... untouched `$a1` with `$a2`/`$a3` set PROVES a forwarded
parameter; the converse fails, so type a slot from its CALL SITES" -- here,
from the callee's BODY), untouched `$a1`-`$a3` at a call whose callee reads
them means the caller forwards its own parameters. The slot and this
function had both been typed `(self)` only since round 14.

**The one lever, one build:** declare `(self, s32 a1, s32 a2, s32 a3)` and
call `configure(self, a1, a2, a3)`. 35/35, ins 0 / del 0, skeleton 0,
`build exit=0` (whole image). No other change to the body. The 29/35 body's
compiled instructions are the SAME SET; only the order moved, which is why
this read as scheduling for 59 rounds and why ~94,000 permuter iterations
could not find it (a permuter never changes a function's parameter list or
a call's arity).

The shared `FadeBoxMethods::configure` slot (`include/Task.h`) is
NOT retyped: the call goes through a file-local
`typedef s32 (*Configure6E99CFn)(FadeBoxObj *, s32, s32, s32)` cast, per
3f's "prefer a LOCAL function-pointer view over retyping a shared slot". A
comment-only note was added under that slot in the header.

### Matched body

```c
typedef s32 (*Configure6E99CFn)(FadeBoxObj *self, s32 a1, s32 a2, s32 a3);

void FadeBox__StartFadeDown(FadeBoxObj *self, s32 a1, s32 a2, s32 a3) {
    s32 idx;

    if (self->state != 0) {
        return;
    }
    idx = ((Configure6E99CFn)self->methods->configure)(self, a1, a2, a3);
    self->methods->slotB8(self, 1, &gFadeBoxMaskColors[idx * 3]);
    self->state = 1;
    self->step = -self->step;
}
```

### Why the arity moves the `li` (checked hypothesis, not measured further)

With the forwarded parameters, `$a1`-`$a3` are USES of incoming pseudos at
the first call, so the hard registers are live up to that `jalr` and the
parameter pseudos have real live ranges; without them the call has only a
`$a0` use. That changes pseudo numbering and the scheduler's dependence
graph in the block after the call, which is where the `ori $a1,1` landed.
Discriminator for the class: an argument register the callee's body READS
is not written before the call. This instance has it (`$a1`-`$a3`, callee
`FadeBox__Configure`); the sibling `FadeBox__StartFadeUp` has it
too and closed on the same lever plus parameter reuse.

### Proposed learning

**A "literal argument scheduled late" residue after a call that sets only
`$a0` can be a FORWARDED-PARAMETER arity defect.** Before calling a residue
scheduling, check the PREVIOUS call in the function: if its callee reads
`$aN` and the caller never writes `$aN`, the caller forwards its own
parameter -- declare it and pass it. Same instruction set, different order,
so it reads as pure scheduling at ins/del 2/2 and is invisible to the
permuter (it never edits a parameter list). `FadeBox__StartFadeDown`
35/35 and `FadeBox__StartFadeUp` 41/41, both in one build each,
after 94k and 280 s of search respectively.


> Renamed from `func_80040024` on 2026-09-20 (tools/rename.py). Address 0x80040024.

## Round 46 (runner delta): drift-checked fresh, no new attempt -- DELIBERATE SKIP

Re-spliced the exact preserved body (unchanged) in isolation and rebuilt:
**29/35 words match (file 0x30824-0x308B0)**, no outside-range drift --
identical to every prior measurement, not stale. Deliberate skip: this
"literal materialised late" residue has already absorbed ~94,000
unguided permuter iterations (round 20) plus four hand attempts spanning
naming the table address, naming the literal, and a scheduling barrier
(the last of which actively regressed). The two statement POSITIONS a
fresh attempt would naturally try (naming `one = 1` before vs. after the
`idx = ...` dispatch) are both already in that attempt list. No new
lever occurred to me. Restored to `INCLUDE_ASM`; full oracle
re-confirmed green.

## Round 21 (runner delta): re-verified fresh, no new attempt

Restored the exact 29/35 body and rebuilt fresh, isolated (with
`FadeBox__StartFadeUp` reverted to `INCLUDE_ASM` at the time, so this
function's own window cannot be contaminated by that sibling's own
drift -- see that function's own report for a real instance of this
project's "one inherited body in six carries a false drift-free claim"
warning). Reproduces `29/35 words match (file 0x30824-0x308B0)` exactly,
**no outside-range drift warning** -- this function's own length really
does match retail, unlike its neighbour. Also re-ran with the
coordinator's corrected oracle grep
(`error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]`, round 21
broadcast) instead of the three-pattern one: zero hits, confirming this
was never a masked compile error.

Round 20 already spent ~94,000 unguided permuter iterations on this
exact residue with zero improvement (see that round's own section
below) and this round's assignment explicitly flagged the class as
confirmed-negative; not re-spending a further unguided search on the
identical mechanism without a new lever. Verdict unchanged: STALL,
29/35, restored to `INCLUDE_ASM`.

## Round 20 (runner delta): permuter search, confirmed negative

Drift check: rebuilt the exact 29/35 body above verbatim, reproduced
cleanly. Ran `permuter.py --debug --stack-diffs` first to confirm the
scaffold scores the reported residue exactly: **0 stack/branch
differences, 5 register differences, 60 reorderings(2), base score
120** -- matches "pure instruction-scheduling difference, zero
insertions/deletions" from the original report precisely.

Ran the real search twice (unguided randomization -- no `PERM_VAR`/
`PERM_LINESWAP` macros used, since the report's own "materialize the
literal in a local" and "reorder the two `self->methods` reads" levers
are exactly what unguided mutation explores anyway): first bounded run
`timeout 280`, ~9,900 iterations; second bounded run `timeout 500`,
~84,438 iterations. **Neither ever beat the base score of 120** -- no
candidate came within any measurable distance of zero across ~94,000
combined iterations. Both runs used `--stack-diffs` throughout (per the
project's own tooling-bug broadcast), so a false zero from the missing
flag is ruled out.

**This closes the "not attempted due to time budget" note in the
original report as a genuine negative**, not merely an unexplored lead.
Restored to `INCLUDE_ASM`; full oracle re-confirmed green
(`build exit=0` on this unit's baseline before and after).

### Proposed learning (round 20)

A "retail materializes a literal argument immediately after an unrelated
dispatch, before computing an index expression" residue does not yield
to ~94,000 unguided permuter iterations either, on top of the four
hand-reshaping attempts the original report already spent. Combined with
`FadeBox__StartFadeUp`'s identical-shape residue (see that report), this is now
a **confirmed-negative class**, not merely an untried one -- a future
runner should not re-spend permuter budget on this specific
`slotXX(self, 1, tableEntry)`-after-a-fresh-dispatch shape without a new
lever (e.g. a `PERM_LINESWAP`-guided search, which requires bypassing
`setup-permuter.sh`'s own cc1 sanity-check per `func_8003F764.md`'s
round-19 finding -- not attempted this round either, same tooling
boundary).

> **CROSS-REFERENCE CHECKED, STANDS (round 39, head).** `func_8003F764` is
> **`select_max_param`** (`libgs/gs_131.o`), now a linked Sony object, so
> do not read that report for a game-code residue precedent. The thing
> cited here is different in kind and survives the reclassification: it is
> a finding about `setup-permuter.sh`'s own behaviour, which is a property
> of the TOOL and not of whose code it was pointed at.

Unit `ScreenWidgets`, carved round 14. `FadeBoxMethods::startFadeToIndex` (`+0x0D4`).

**Correction to an earlier version of this report**, which claimed a full
35/35 match under the stale-build window described in `New_FadeBox.md`
(same cause, cross-referenced there). Once the build was confirmed genuinely
fresh (every symbol's linked address checked against `build/lsdde.map`),
this function turned out to still have a real residue.

## Best body reached (29/35)

```c
#if 0
void FadeBox__StartFadeDown(FadeBoxObj *self) {
    s32 idx;

    if (self->state != 0) {
        return;
    }
    idx = self->methods->configure(self);
    self->methods->slotB8(self, 1, &gFadeBoxMaskColors[idx * 3]);
    self->state = 1;
    self->step = -self->step;
}
#endif
```

## Residue

Pure instruction-scheduling difference, zero insertions/deletions. Retail
materialises the `slotB8` call's literal `1` argument (`li $a1,0x1`)
IMMEDIATELY after the `configure` dispatch's own delay slot -- before computing
`idx*3` -- and reloads `self->methods` a second time (fresh, not cached)
right before the `slotB8` dispatch itself. This body's compiled form
computes `idx*3` first and defers the `li $a1,0x1` to just before the call.
Same total instruction count, same registers, purely reordered.

## Attempts (4)

1. Base body above: 29/35, the "li a1,1 late" residue.
2. Materialize the table address into a named local (`entry = &gFadeBoxMaskColors[...]; slotB8(self, 1, entry);`)
   before the call: no change.
3. Materialize the literal into a named local (`one = 1;`) assigned
   immediately after `idx = ...`, before computing the table address: no
   change -- GCC still floats the `li` to just before the call regardless
   of where the C source puts it.
4. Same as (3) plus a bare `__asm__("")` barrier right after the `one = 1;`
   statement, to physically pin its position: made it WORSE (3/35) --
   the barrier perturbed something else's scheduling elsewhere in the
   function, consistent with the project's documented "`__asm__("")` is
   not a local lever -- it perturbs the WHOLE function's register
   allocation" caution.

## What I did NOT try, and why

- **The permuter.** This is a small (35-word), pure-reordering residue with
  zero structural difference -- close to the ideal permuter target. Not run
  due to time budget in this round; worth a `PERM_VAR`-guided search on a
  re-attempt, seeded from the base body above.
- **Reordering the two `self->methods` reads** (the `configure` dispatch's own
  read vs the `slotB8` dispatch's fresh reload) relative to the `idx*3`
  computation, independently of the literal's position. Only the literal's
  position was varied across all 4 attempts; the reload's position was left
  alone since it already matches retail's own placement in every attempt.

## Proposed learning

A trivial constant argument (`1`) to a call reached immediately after a
DIFFERENT dispatch through the same vtable pointer can resist being pinned
to retail's early position by any combination of source reordering, naming
it in a local, or a bare scheduling barrier (which actively regresses it).
`FadeBox__StartFadeUp` in this same unit shows the identical pattern on the exact
same call shape (`slotB8(self, 1, tableEntry)`) -- worth treating as one
class rather than two coincidences; see that function's own report.

## Round 59 (runner charlie): NON_MATCHING body promoted

NON_MATCHING body promoted, round 59. The exact preserved body above (29/35
words, length exact, instruction-scheduling residue on the `li $a1,1`
materialization) is now live in `src/ui/ScreenWidgets.c` under `#ifdef
NON_MATCHING`, with the verified build still taking the `#else INCLUDE_ASM`
branch. `./build-and-verify.sh` and `tools/check-nonmatching.sh` both green.

## Naming (round 61, track 3)

**`FadeBox__StartFadeDown`** -- tier B (STALL, preserved body
unchanged by this rename). `FadeBoxMethods::startFadeToIndex` (`+0x0D4`). Guards
on `state == 0` (idle), looks up an index via `configure`, dispatches the
`slotB8` color-set slot with `&gFadeBoxMaskColors[idx * 3]` (an INDEXED table
entry), sets `state = 1`, and negates `step`. Named opposite
`FadeBox__StartFadeUp` (`startFadeDefault`, `state = 2`, the FIXED
`gFadeBoxBlackColors` table) -- the two are a matched pair distinguished by which
color source they select. "Fade" is inferred from `step` accumulating into
color-channel bytes over time in `FadeBox__Update`; "index" from this
function's own `idx`-based table lookup versus its sibling's fixed one.
Game-level purpose (what is fading, and why) is not established.

## Track 4 (2026-09-26, round 87, echo)

Renamed from `FadeBox__StartFadeToIndex` to `FadeBox__StartFadeDown`.
"ToIndex" said the fade goes TO the indexed colour; the body does the
opposite. It sets the box colour to `gFadeBoxMaskColors[channels]` (overwrite, via
`setColor`) and NEGATES `step`, and `FadeBox__Update` then adds `(u8)step`
to each selected channel byte once per tick for `0x100 / step` ticks: the
channels count DOWN from the table colour (0xFF over 25 ticks at the
default step of 10). `gFadeBoxMaskColors` is eight 3-byte RGB entries indexed by a
channel mask (1 = 0000FF, 2 = 00FF00, 4 = FF0000, 7 and 0 = FFFFFF; read
from the retail bytes), the same mask `Update` tests (4 = r, 2 = g, 1 = b).
Its sibling counts up; see `FadeBox__StartFadeUp`. The arguments are
`configure`'s, forwarded (`source`, `channels`, `unk7C`), and the slot is
now typed with them. Callers, all through Entity's `unk100`:
Entity__MoodCue57 (Entity) passes (companion2, 4, 0), Entity__MoodCue85
(Entity) and Entity__MoodCue98 (Entity_g) (companion2, 7, 0), and
Entity__MoodCue91 (Entity) (companion2, 0, 0). Tier B: what the fade is for is not shown.

## Track 6 (2026-09-26, round 93, charlie)

Renamed with the class: `Class6E99C` is now `FadeBox`
(`python3 tools/renametype.py Class6E99C FadeBox`, table
`python3 tools/rename.py D_8006E99C gFadeBoxMethods`), tier A; the evidence
is in New_FadeBox.md's Track 6 section. The method's own name was kept: it
already says what the body does. renametype.py rewrote the old class name in
this report's earlier history too (known, pending an operator decision).


## Track 7 (round 100, charlie)

- Parameter `arg3` -> `mode` (FadeBox::mode, see FadeBox__Configure.md); local `idx` -> `mask`, configure's return, the mask it stored.
