# StepVoiceEnvelope -- STALL: length exact (231/231, round 73), 223/231 raw word-match, first diff at vram 0x8002E700 (the pan split: retail copies the volume into $a1 and multiplies the copy; this body masks val1). libsnd SetAutoVol.

> Renamed from `StepVoiceEnvelope` on 2026-09-24 (tools/rename.py). Address 0x8002e4d8.

> Renamed from `func_8002E4D8` on 2026-09-20 (tools/rename.py). Address 0x8002e4d8.

Unit: `src/code_179d8_m.c`. Round 26 (second pass), runner bravo, applying
the HEAD's "split scaled index" diagnosis per the work order.

## Screens (clean)

```
grep -n 'gp_rel' asm/nonmatchings/code_179d8_m/SetAutoVol.s            -> no hits
grep -A2 -nE '\b(mflo|mfhi)\b' ... | grep -E '\b(mult|multu|div|divu)\b'  -> no hits
```

## Result

`./build-and-verify.sh` GREEN with `INCLUDE_ASM` restored. **The
halfword-index idiom the HEAD diagnosed (and this round confirmed
byte-exact for `SpuVmInit`'s loop) DOES reproduce the early `sll #3`
here too** — confirmed via `asm-differ`, the function's very first
instruction now matches retail's `sll t0,v1,0x3` shape (register-renamed
but structurally identical), closing the specific gap the HEAD's message
was about. But the OVERALL score moved from the first pass's stall (236/231,
5 words long) to 237/231 (6 words long) — a net regression of one word,
because this function calls the idiom from a SINGLE, non-looping call site
rather than a loop body, and that context surfaces a DIFFERENT residual
issue the loop case does not have.

## What the head's fix looks like applied here

Two single-field, 0x10-stride arrays (`D_8008D7F2`, `D_8008D7F0`, each with
only one `s16` field at offset 0) replace the earlier attempt's
`Rec16D7F0`-typed `array[idxCopy].unk0` struct-cast form:

```c
s16 woff;
...
idxCopy = a0;
woff = idxCopy << 3;              /* idx*8, computed at the very top */
...
((s16 *) D_8008D7F2)[(u16) woff] = val2;   /* mask applied at the point of use */
((s16 *) D_8008D7F0)[(u16) woff] = val1;
```

This alone reproduced the missing early `sll #3` — confirmed directly (the
prior attempt's `<` line for `sll t0,v1,0x3` and its paired `>` line for the
single-shot `sll v0,v1,0x1` are BOTH gone from the diff once this is
applied). **The idiom generalizes correctly across the record family**, as
the HEAD predicted.

## Why this is NOT a loop, and why that matters

`SpuVmInit`'s version of this idiom lives inside a per-channel `for`
loop, where `i` changes every iteration and `woff` is naturally recomputed
each time as the loop body's first statement — there is no "which value is
`woff` derived from" ambiguity, because `i` IS the loop induction variable
and nothing else touches it.

`SetAutoVol` runs once per call with a single fixed `a0`. Here, `idx*8`
has to be derived from the PARAMETER, which is ALSO used (via `idxCopy`) to
index the unrelated 0x34-stride record family the rest of the function
reads. Three placements were tried for where this derivation should read
from and where it should sit textually, all producing DIFFERENT counts:

1. **`woff` derived from `idxCopy` (a local already equal to `a0`), computed
   immediately after `idxCopy = a0;`, at the very top of the function**:
   238 words (7 long) — introduced a spurious `move v1,a0; andi
   v0,v1,0xffff` pair (2 extra words) BEFORE the sign-extension the 0x34-
   chain needs, because a `(u16)` cast on the still-raw parameter forces
   its own zero-extension materialization independent of the later
   sign-extension the record-index chain performs.
2. **`woff` derived from `a0` directly** (not through `idxCopy`), same
   position: 239 words (8 long) — worse; the same spurious pair appeared,
   PLUS retail's actual dependency (its early `sll #3` reads the value
   AFTER $a0 has been sign-extended into $v1, not the raw parameter) was
   still not reproduced.
3. **`woff` kept `s16` (not masked to `u16` until point of use) and
   computed as `idxCopy << 3`**, matching retail's own type discipline
   (the multiply stays a plain register value; the `(u16)` mask is applied
   only where the array is indexed): 237 words (6 long) — the best reached,
   and the one preserved below. This closed the specific "early
   materialization + zero-extend" bug from attempts #1/#2, and reproduced
   the early `sll #3` correctly. The remaining 6-word gap is a SEPARATE,
   pre-existing issue (below), not newly introduced by this idiom.

## The pre-existing residual: a redundant sign-extension, already flagged in the prior stall

The prior round's report on this function already documented an
UNRESOLVED redundant-sign-extension bug independent of the split-index
issue: comparing a `u16` accumulator sum against a `s16` limit forces a
spurious `sll`/`sra` re-extension pair at each comparison site, because
the compiler will not treat a value it derived through an unsigned load as
already sign-clean for a later signed comparison. This round tried the
next lever on that specific bug — reading `accumS` back from MEMORY
(`D_8008D9AC[idxCopy].unk0`, a genuine fresh `lh`) instead of assigning it
from the already-computed `accumU` register (a register-level reinterpret
cast) — on the theory that a fresh signed load, rather than a same-register
reinterpretation, is what retail's disassembly actually shows at the
analogous point. **This did not move the score** (237 words either way):
the extra `sll`/`sra` pair persists in the same place regardless of which
of the two forms produces `accumS`. This confirms the redundant-extension
bug is NOT about how `accumS` is derived (register cast vs. fresh reload)
— something else about the surrounding accumulator-add sequence is
triggering it, not yet identified.

### Axes tried this round, in order, effect on length (retail 231)

1. Baseline `array[idxCopy].unk0`-style indexing for `D_8008D7F2`/
   `D_8008D7F0` (carried over from the prior stall): 236/231 (5 long) --
   this is the number the PRIOR report closed on; preserved here as the
   comparison point.
2. Applied the halfword-index idiom, `woff` derived from `idxCopy`,
   computed at function top: 238/231 (7 long) -- worse, see reason #1 above.
3. `woff` derived from raw `a0`, same position: 239/231 (8 long) -- worse
   still, see reason #2 above.
4. `woff` kept `s16`, derived from `idxCopy`, u16-cast only at the two
   point-of-use array subscripts: 237/231 (6 long) -- best result, closes
   the early-`sll`-#3 gap, one word worse overall than the pre-idiom
   baseline because of the pre-existing sign-extension bug documented
   above (unaffected either way by this idiom).
5. Replaced `D_8008D9A8`/`D_8008D9AA`/etc. array indices from the raw
   parameter `a0` to the local `idxCopy` throughout the function (matching
   retail's apparent single-variable-for-everything usage): no change
   (still 237/231) -- ruled out as a lever for the remaining gap.
6. `accumS` derived via fresh memory reload instead of register-cast
   (see above): no change (still 237/231) -- ruled out for the specific
   sign-extension residue.
7. A bare `__asm__("")` scheduling barrier immediately after `idxCopy =
   a0;` (before `woff`'s computation): no change (still 237/231) -- the
   ordering issue observed for the sign-extend/split-index registers is
   not fixable by a barrier at this position; not further explored given
   the round's time budget.

## Round 27 update: callee-saved lever CLOSED (negative), and two more axes tried on the sign-extension residue

Head broadcast round 27's callee-saved-register lever (a promoted
parameter reaching a home-slot spill vs. a dedicated `$sN`, see
`_card_clear`'s writeup) with this function named as the live
candidate. Checked directly rather than inferred from the frame-size gap:

```sh
grep -oE 'sw +\$(s[0-7]|fp),' asm/nonmatchings/code_179d8_m/SetAutoVol.s
# -> no hits at all
```

**Retail saves ZERO callee-saved registers for this function** (frame is
`0x18`, just `$ra`-worth-of-locals, no `$s0`-`$s7`/`$fp` anywhere). Spliced
this round's 237-word preserved body into a standalone build to check the
other side: its compiled frame is `0x10`, and it ALSO saves zero
callee-saved registers. **Both sides save nothing, so there is no
promotion to undo — the lever does not apply here, and not because retail
happens to match my register SET (the two-way case the broadcast
described) but because neither side has one to compare.** This is a third
outcome distinct from "lever applies" / "same set, genuine identity", and
head confirmed it as a new case worth recording in
`DECOMPILATION_LEARNINGS.md`.

With the lever closed, two more axes were tried directly on the
already-diagnosed residue (a spurious `sll`/`sra` sign-extend pair
appearing right after `accumU = accumU + incU;`, before the `incS>0`/
`incS<0` branch, where retail's disassembly shows the SAME pair appearing
TWICE instead -- once inside EACH branch, computed fresh from the just-
stored sum, rather than once before either):

1. **Moved `accumS`'s declaration and derivation inside each of the two
   branches** (`if (incS > 0) { s16 accumS = (s16) accumU; ... }`, and
   the same inside the `else if`), instead of one shared `accumS` computed
   once before the `if`/`else if` and read from both: **no change (still
   237/231)**. The extra `sll`/`sra` pair still lands in the SAME spot,
   immediately after the add and before either branch, confirming this is
   not about WHERE `accumS` is lexically scoped or how many times it is
   spelled in the source -- the compiler hoists the sign-extend to that
   point regardless.
2. **A bare `__asm__("")` between `incU`'s read and `incS`'s read** (both
   read the SAME `D_8008D9A6[idxCopy]` address, one unsigned one signed --
   retail's disassembly shows this as two SEPARATE loads, `lhu` then `lh`,
   while my build shows only the `lhu` with `incS` derived from it via a
   register-level sign-extend instead of its own `lh`): **no change
   (still 237/231)**. The barrier did not force a second, independent
   load of `incS`.

**Both axes now ruled out cheaply** (previously-untried per the round-26
report's axis list, which had tried the reverse -- deriving `accumS` from
a fresh memory reload instead of a register cast -- with the same null
result). The residue increasingly looks like a genuine GCC 2.6.3
instruction-selection preference for THIS multiply-add-then-branch
shape (reuse the already-materialized unsigned sum/load rather than
re-fetching a signed copy), not something reachable by varying which C
statement produces the value. Not spending further attempts on this axis;
next attempt should look for a DIFFERENT lever entirely (e.g. the
`incS`/`incU` ORDER swapped, or `incU` computed FROM `incS` via a cast
rather than as an independent load, untried this round).

## Round 30 (charlie) update: rebuilt, confirmed accurate

Re-spliced this exact preserved body (typedef names local to the splice
were renamed only to route around unrelated duplicate-declaration
conflicts with OTHER still-`INCLUDE_ASM` functions sharing the file; the
reported C itself is unchanged) and rebuilt from scratch. **All title
figures reconfirmed:** `objdump -t build/src/code_179d8_m.c.o` shows
`SetAutoVol` at `0x3b4` bytes = **237 words**, against retail's 231 (6
long, exactly as titled). No new axis attempted this round: this function
was already worked twice in the immediately preceding rounds (26 and 27)
with the callee-saved lever explicitly closed as inapplicable, and the
`SetAutoPan` sibling attempt this round (applying THIS function's own
"woff" idiom back to `SetAutoPan`) surfaced that the idiom's correct
placement is more context-sensitive than either report currently
documents — see `SetAutoPan.md`'s round-30 update for the negative
result and its diagnosis, which is relevant background for anyone
revisiting either function's split-index code next.

### Proposed learning

**The `woff`-halfword-index idiom the HEAD diagnosed for a small-struct
array write is CONFIRMED to generalize across this record family and
across at least two different sibling functions, but its correct SOURCE
FORM depends on whether the write site is inside a loop or not.** Inside a
loop (`SpuVmInit`), declaring `woff` fresh each iteration as the loop
body's first statement, computed from the loop induction variable directly,
reproduces retail exactly. Outside a loop (`SetAutoVol`, this report),
the SAME idiom needs the intermediate value kept at its NATURAL type (here
`s16`, matching the parameter's own type, with the `(u16)` mask deferred
to the point of use) rather than materialized as `u16` immediately — an
early unsigned cast on a not-yet-sign-extended value introduces a spurious
zero-extension the loop case never triggers, because the loop's induction
variable is already being independently zero-extended for OTHER purposes
(the `for (i = 0; (u16) i < count; ...)` bound check) that a straight-line
function has no equivalent of. **Whoever attempts this idiom next on a
non-loop call site should start from `s16 woff = idx << 3;` with the mask
deferred to use, not from `u16 woff = (u16) idx * 8;` — the latter is right
for a loop and wrong for straight-line code.**

This function ALSO still carries the pre-existing sign-extension residue
from its prior stall report, now confirmed independent of both the
split-index bug and of how the reload is spelled — worth flagging for the
next attempt as a genuinely separate, still-unidentified cause, distinct
from the class this round's fix addressed.

## Round 32 (bravo) update: reconfirmed, the round-27-flagged "untried lever" TRIED and REGRESSED sharply

Re-spliced this exact preserved body and rebuilt from scratch. Reconfirmed
237/231 built words (6 long), split-index gap still closed, sign-extension
residue still present, matching round 30's own reconfirmation exactly.

Round 27's update named a specific untried next step for the remaining
sign-extension residue: swap the `incS`/`incU` derivation order, or derive
`incU` from `incS` via a cast rather than as an independent load. Read the
raw `.s` directly first to confirm the exact shape at this site (lines
1ED70-1EDB0 of `asm/nonmatchings/code_179d8_m/SetAutoVol.s`): retail
issues TWO separate loads of `D_8008D9A6[a1]` — one `lhu` into `incU`, one
`lh` into `incS` (a different register) — immediately adjacent, then adds
`incU` into `accumU`.

**Tried deriving `incU` from `incS` via a cast** (`s16 incS = ...; u16 incU
= (u16) incS;`) instead of the existing two-independent-loads form: this
built clean but REGRESSED SHARPLY, to 31/231 raw match with the whole
function's register layout diverging from the very first instruction (not
just the targeted add/branch site) — worse than doing nothing. **This
appears to have collapsed the two loads into one** (a cast-derivation gives
the compiler no reason to re-fetch the value from memory a second time),
which is the wrong shape entirely: retail's two separate loads are load-
bearing and the existing preserved body (which already reproduces them
correctly, one `lhu` one `lh`) is the right form. The round-27 suggestion
to "derive incU from incS via a cast" is now tried and REJECTED, not merely
unexplored — the existing two-independent-loads structure should not be
touched further on this specific axis.

This closes the round-27-flagged lever as tried and failed. The
sign-extension residue (paragraph above, "reuse the already-materialized
unsigned sum/load rather than re-fetching a signed copy") remains
unidentified; per round 27's own conclusion, the next attempt needs a
genuinely different idea, not a further variant of how `incS`/`incU` are
spelled or ordered — that specific axis is now exhausted across three
distinct triable forms (fresh-reload cast, scope-per-branch, and this
round's cast-derivation-instead-of-independent-load).

## Round 37 (bravo) update: rebuilt (confirmed accurate), header reorganized to allow the splice, permuter searched for the first time

Round 37's designated permuter-priority item 4 (this unit's five stalls are
the largest never-permuter-searched block in the corpus this round). Per
the round's "build the inherited body before you trust its score"
instruction, this exact preserved body was re-spliced into the live unit
and rebuilt from scratch.

**This function sits FIRST in `code_179d8_m.c`**, so the shared
`Rec34Half`/`Rec34HalfU`/`Rec16D7F0`/`ObjE970`/scratch-global declarations
this body needs (originally written for a standalone splice with their
own flat externs) are declared LATER in the file, after `SeAutoPan`
and `SetAutoPan`'s own stall bodies. Re-declaring them again here under
the same names is a hard conflict (duplicate typedef names, and for
`D_8008E970`/`D_8008D7F0`/`D_8008D7F2` a redeclaration of the same extern
symbol under an incompatible pointee type) -- not a new finding, but the
concrete case the project's own "one extern symbol cannot carry two
conflicting C types in one file" rule warns about, hitting a typedef this
time rather than a symbol.

**Fix: moved the shared typedef block (`Rec34Half` and its
`D_8008D9B0`.`D_8008D9BA` externs, `Rec34HalfU`, `Rec16D7F0` +
`D_8008D7F0`/`D_8008D7F2`, `D_8008D970`, `ObjE970` + `D_8008E970`, the six
`D_8008EA1*` scratch bytes, `D_8008E8C0`) from its old position (between
`SeAutoPan` and `SetAutoPan`) up to right after `#include
"common.h"`, adding this function's own `D_8008D9A4`/`A6`/`A8`/`AA`/`AC`/`AE`
externs (same `Rec34Half` shape, disjoint symbols) alongside the existing
ones.** Declaration order carries no code -- only DEFINITIONS need strict
ROM order, which this move does not disturb (no function moved). This is
believed to generalize as a documented pattern below.

**All title figures reconfirmed exactly:** `objdump -t
build/src/code_179d8_m.c.o` shows `SetAutoVol` at `0x3b4` bytes = **237
words** (retail 231, 6 words LONG, exactly as titled).

### Permuter search

`tools/setup-permuter.sh SetAutoVol <seed>` -- seed built from this
report's preserved body (standalone, with its own flat-extern
declarations, since the seed is compiled in isolation and does not need
the in-file reorganization above). See the Permuter result subsection for
the base `--debug --stack-diffs` score and the real search's outcome
(iteration count and `rc`).

#### Permuter result

`--debug --stack-diffs` base score: **4155** (Register Differences 71 x 5
= 355; Reorderings 10 x 60 = 600; Insertions 19 x 100 = 1900; Deletions 13
x 100 = 1300; Stack Differences 0 and Branch Differences 0 -- same pattern
as `SetAutoPan`'s sibling search: this function's own documented frame
gap, `0x10` built vs retail's `0x18`, is likewise not reflected as a
stack-slot difference, only as an immediate-operand difference on the
`addiu sp,sp,-N` line itself).

Real search: `timeout 900 permuter.py -j 6 --stop-on-zero --best-only
--stack-diffs` via the harness's `run_in_background`. **Completed cleanly,
`rc=124`** (own bound) after **80,882 iterations** -- the deepest search
of the five functions in this unit this round. Best score: **2105** (from
base 4155), saved at `permuter-work/SetAutoVol/output-2105-1/`; no zero
reached, and none of the eight other intermediate best-score directories
saved along the way (`2275`, `2310`, `2415`, `2935`, `3015`, `3315`,
`3340`) reached one either. **Not closed; both open residues this report
already diagnoses -- the redundant sign-extension pair around the
`incU`/`incS` accumulator add, and the callee-saved-register question
already closed as inapplicable in round 27 -- are consistent with what
this search found: steady incremental score improvement with no
qualitative jump, the signature this project's own learnings associate
with a register-identity plateau rather than a reachable one-instruction
fix.**

### Proposed learning

**When the first function in a file needs a shared record-family typedef
that a LATER function in the same file already declares (for different,
same-shaped symbols), the fix is to MOVE the typedef block earlier in the
file, not to re-declare it under a renamed local typedef.** A renamed
local typedef works for a throwaway verification splice (as prior rounds
did, explicitly to route around exactly this collision) but is a
regression if it is what ends up committed, since it duplicates a type the
file already has a name for. Declarations carry no code and are free to
reorder; only function DEFINITIONS need to stay in ROM address order. This
generalizes beyond this file: whenever the *first* matched function in a
unit turns out to need a type a *later*, still-unmatched function's stall
report already introduced, move the type's declaration rather than
re-deriving or renaming it.

## Round 48 update (runner echo): tested charlie's frame-padding lever -- FOURTH confirmed negative for length closure, unit-wide verdict now 4/4

Round 48's designated test of charlie's `ContDataEntry` frame-padding
discovery. This function's frame gap was already ON FILE (round 27's
callee-saved-lever check, above): built `-0x10` vs retail `-0x18`, an
8-byte gap. Direct grep confirms the textbook shape:

```sh
grep -oE '0x[0-9a-fA-F]+\(\$sp\)|\$sp,\$sp,' asm/nonmatchings/code_179d8_m/SetAutoVol.s
# -> only the prologue "addiu $sp,$sp,-0x18" and epilogue "addiu $sp,$sp,0x18"
```

Nothing in retail addresses the frame via `$sp` at all beyond the
immediate adjustment itself, and (per round 27's own finding) both sides
save zero callee-saved registers — the same pure-unaddressed-padding shape
already confirmed 3/3 on this unit's other stalls.

Applied `u8 dead[8];` under the established `if (0) { dead[0] = 0; }`
guard. **Result: frame realigns byte-exactly** (`addiu sp,sp,-0x18`,
confirmed via objdump) **but built length is UNCHANGED at 237/231 (still
6 words LONG).** With the frame realigned, `tools/asm-differ/diff.py`
shows the SAME already-diagnosed residue this report's own "Axes tried"
section already explored from three different placements: this build's
`woff = idxCopy << 3;` materializes as `sll t0,a0,0x3` immediately at
function entry (word 2), before the sign-extension chain (`sll/sra v1`)
the 0x34-stride record accesses need; retail computes the equivalent
value only AFTER that sign-extension, sharing it with the record-index
multiply chain. **No new residue surfaced** — unlike `SpuVmFlush`
(this unit's other post-realignment win), realigning this function's
frame did not reveal anything beyond what was already on file.

**This unit is now 4-for-4 this round: charlie's `dead[N]`/`if(0)`
padding idiom recovers frame byte-alignment exactly every time (four
measured cases: `SpuVmFlush`, `SetAutoPan`, `SpuVmKeyOn`, and
this function), and has closed a missing-WORD-COUNT gap on none of them.**
Every one of `code_179d8_m`'s frame gaps is pure unaddressed
register-save-area padding — confirmed directly by grep in three of the
four cases (`SetAutoVol`, `SpuVmFlush` here; `SetAutoPan` and
`SpuVmKeyOn`'s own permuter `--stack-diffs` runs independently
confirmed zero stack differences) — with each function's real content
residue (a redundant mask, a persisted early value, an addressing-cost
difference, an early-materialization placement) living entirely
independently of the frame allocation. `ContDataEntry`'s original length
recovery in a DIFFERENT unit came from a coincidentally-paired
tail-duplication fix, not from the padding move itself.

**Housekeeping note on this splice**: this function sits FIRST in ROM
order in the unit, so its shared record-family types (`Rec34Half` and
`D_8008D9A4`/`A6`/`A8`/`AA`/`AC`/`AE`, `Rec34HalfU`, `Rec16D7F0` +
`D_8008D7F0`/`D_8008D7F2`, `D_8008D970`, `ObjE970` + `D_8008E970`, the six
`D_8008EA1*` scratch bytes, `D_8008E8C0`) had to be declared BEFORE this
function rather than duplicated under function-local names — duplicating
them (tried first, see the compile errors this produced) hits the
project's "one extern symbol/typedef cannot carry two conflicting
declarations in one file" rule, exactly the same shape round 37 already
hit and fixed the same way. The shared prelude now lives right after
`#include "common.h"`, and the later, now-redundant typedef definitions
(previously positioned for `SetAutoPan`'s isolated splice) were
removed, leaving only their accompanying `extern` lines in place (which
remain valid: same already-declared type, referenced from a later point
in the file). Reverted the function itself to `INCLUDE_ASM`; the shared
prelude relocation is KEPT in `src/` since it is declaration-only (no
code) and matches round 37's own precedent. Whole-image SHA1 reconfirmed
green after the revert.

### Proposed learning (fourth data point, unit-wide conclusion)

**`code_179d8_m` closes out this round's frame-padding-lever test at 0-for-4
on length closure, 4-for-4 on frame-byte-alignment recovery.** The lever's
reliable, repeatable value on this unit was diagnostic — it makes an
otherwise length-misaligned diff readable — and it directly PAID OFF once
(`SpuVmFlush`'s `andi 0xff` mask, found only after realignment). But
treating it as a length-closing move in its own right would have been
wrong all four times here. The generalizable rule for the next runner:
apply the padding cheaply whenever a frame gap is confirmed pure
unaddressed padding (no `$sp`-relative accesses beyond the prologue/
epilogue immediate, ideally cross-checked with a permuter `--stack-diffs`
run showing zero Stack Differences), then re-read the diff for genuinely
NEW residues — but do not expect the built LENGTH to move unless a
SEPARATE piece of evidence (a duplicated tail, a missing call, extra
addressed content) is also found.

## Superseded preserved body (pre-round-73 best attempt, 237/231 built words -- 6 words long; split-index gap CLOSED, sign-extension residue REMAINS)

```c
#if 0
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34Half;
extern Rec34Half D_8008D9A4[];
extern Rec34Half D_8008D9A6[];
extern Rec34Half D_8008D9A8[];
extern Rec34Half D_8008D9AA[];
extern Rec34Half D_8008D9AC[];
extern Rec34Half D_8008D9AE[];

/* Same 0x34-stride record family, UNSIGNED 16-bit view -- D_8008D9A6 and
 * D_8008D9AC each need this width (`lhu`) at least once in this function,
 * on top of the plain signed `Rec34Half` view above (which D_8008D9A6
 * ALSO needs, at a DIFFERENT read site: retail issues TWO separate loads
 * of the same address, one `lhu` and one `lh`). */
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34HalfU;

typedef struct {
    u8 pad[0x18];
    u8 unk18; /* +0x18 */
} ObjE970;
extern ObjE970 *D_8008E970;

extern u8 D_8008EA10;
extern u8 D_8008EA11;
extern u8 D_8008EA16;
extern u8 D_8008EA17;
extern u8 D_8008EA19;
extern u8 D_8008EA1A;

extern s16 D_8008E8C0;

/* Same 0x10-byte-stride record family code_179d8_j.c documents as
 * Rec16D7F0 -- accessed here through a flat `s16 *` halfword-indexed
 * cast (the `woff` idiom below), so only a plain extern is needed. */
extern s16 D_8008D7F0[];
extern s16 D_8008D7F2[];

extern u8 D_8008D970[];

void SetAutoVol(s16 a0) {
    s16 idxCopy;
    s16 accum;
    u8 flagByte;
    u16 val1;
    u16 val2;
    s32 product1;
    s32 q1;
    s32 q1b;
    u32 q1c;
    u32 q2;
    s32 tmp;
    u8 tableval;
    s16 limit;
    s16 woff;

    /* Halfword-indexed byte-offset for the D_8008D7F2/D_8008D7F0 stores
     * near the end of this function: idx*8 computed here, kept `s16`
     * (matching retail's dependency -- its early `sll #3` reads the
     * sign-extended parameter and stays a plain register value, with no
     * separate unsigned materialization at this point), before the
     * 0x34-stride record accesses even start. Masked to `u16` only at
     * the point of use (indexing an `s16 *`, which then scales by 2),
     * reaching idx*16 -- the real per-channel byte stride for these two
     * 0x10-stride, single-field arrays. Same base idiom as
     * SpuVmInit's D_8006DAD4 fix, but kept `s16` (not `u16`) until
     * point of use -- see this report's "non-loop" analysis for why the
     * loop-context version of the idiom does not transfer directly. */
    idxCopy = a0;
    woff = idxCopy << 3;

    if (D_8008D9A8[idxCopy].unk0 != 0) {
        s16 orig = D_8008D9AA[idxCopy].unk0;

        D_8008D9AA[idxCopy].unk0 = orig - 1;
        if (orig > 0) {
            return;
        }
        D_8008D9AA[idxCopy].unk0 = D_8008D9A8[idxCopy].unk0;
    }

    {
        u16 accumU = ((Rec34HalfU *) D_8008D9AC)[idxCopy].unk0;
        u16 incU = ((Rec34HalfU *) D_8008D9A6)[idxCopy].unk0;
        s16 incS = D_8008D9A6[idxCopy].unk0;
        s16 accumS;

        accumU = accumU + incU;
        D_8008D9AC[idxCopy].unk0 = accumU;
        accumS = D_8008D9AC[idxCopy].unk0;

        if (incS > 0) {
            limit = D_8008D9AE[idxCopy].unk0;
            if (accumS >= limit) {
                accumU = limit;
                D_8008D9AC[idxCopy].unk0 = accumU;
                D_8008D9A4[idxCopy].unk0 = 0;
            }
        } else if (incS < 0) {
            limit = D_8008D9AE[idxCopy].unk0;
            if (limit >= accumS) {
                accumU = limit;
                D_8008D9AC[idxCopy].unk0 = accumU;
                D_8008D9A4[idxCopy].unk0 = 0;
            }
        }
    }

    accum = ((Rec34HalfU *) D_8008D9AC)[idxCopy].unk0;
    D_8008EA10 = (u8) accum;
    tableval = D_8008E970->unk18;

    product1 = accum * (tableval * 0x3FFF);
    q1 = product1 / 16129;
    q1b = q1 * D_8008EA16;
    q1c = q1b * D_8008EA19;
    q2 = q1c / 16129u;

    if (D_8008EA1A < 0x40) {
        tmp = q2 * D_8008EA1A;
        val2 = (u32) tmp >> 6;
        val1 = q2;
    } else {
        tmp = q2 * (0x7F - D_8008EA1A);
        val1 = (u32) tmp >> 6;
        val2 = q2;
    }

    if (D_8008EA17 < 0x40) {
        tmp = val2 * D_8008EA17;
        val2 = tmp / 64;
    } else {
        tmp = val1 * (0x7F - D_8008EA17);
        val1 = tmp / 64;
    }

    if (D_8008EA11 < 0x40) {
        tmp = val2 * D_8008EA11;
        val2 = tmp / 64;
    } else {
        tmp = val1 * (0x7F - D_8008EA11);
        val1 = tmp / 64;
    }

    if (D_8008E8C0 == 1) {
        if (val1 < val2) {
            val1 = val2;
        } else {
            val2 = val1;
        }
    }

    ((s16 *) D_8008D7F2)[(u16) woff] = val2;
    flagByte = D_8008D970[idxCopy];
    ((s16 *) D_8008D7F0)[(u16) woff] = val1;
    flagByte |= 3;
    D_8008D970[idxCopy] = flagByte;
}
#endif
```

## Naming

**SetAutoVol** (was `func_8002E4D8`) -- Tier B. Same shape as
SetAutoPan (accumulate-until-limit, throttled by an interval/countdown
pair, clear an active flag on reaching the limit, then compute and write
a stereo output level from the result) but over its own `_svm_voice +0x1C..+0x26`
family, and with no "Begin"-style setup function in this unit -- nothing
here writes `D_8008D9A4`, `D_8008D9A6` or `D_8008D9AE`.
`SeAutoVol` in `code_179d8_l` opens with the identical prologue and
argument-narrowing shape this unit's header already calls out as a
register-pressure sibling, not a coincidence worth re-deriving; worth
checking directly whether it is the missing "BeginVoiceEnvelope".
"Envelope" rather than "Fade" is the mechanical distinction (no saved
start/end pair, just increment-until-limit), not a claim about which one
is ADSR-shaped in the audio sense.

## NON_MATCHING body promoted, round 67

Placed in `src/code_179d8_m.c` under `#ifdef NON_MATCHING`, `INCLUDE_ASM`
kept in `#else`. This function is first in ROM order in the unit, so its
own preserved body's local `Rec34Half`/`Rec34HalfU`/plain-byte-global
declarations duplicate the unit's shared prelude that already sits above
it (moved up for exactly this reason); the duplicates were dropped. One
real field-name update: the body's own local `ObjE970` (`unk18`) collided
with the shared `ObjE970` the prelude already declares at the same offset
under the name `masterVolume`; changed the access to
`D_8008E970->masterVolume`, same offset, no behavior change. The body's
own flat `extern s16 D_8008D7F0[]/D_8008D7F2[]` declarations were also
dropped -- the shared prelude already declares both as `Rec16D7F0[]`, and
the body already casts to `(s16 *)` before indexing, so no code change was
needed there. `./build-and-verify.sh` green (zero bytes changed) and
`tools/check-nonmatching.sh code_179d8_m` green.

## Round 73 (delta): REVISIT -- 238/231 (7 long) -> 231/231 length-exact, ins 1 / del 1

REVISITED, round 73: STALL improved to length-exact 223/231 (ins 1 / del 1), residue is the same single pan-split register copy as SetAutoPan; names/types used (same locals as SetAutoPan's round-73 body).

### Ownership

`sdkname.py SetAutoVol`: **masked 0.00, shape 0.99** against libsnd
`SetAutoVol` (3.3 `vmanager`, 227w). This is Sony's `SetAutoVol` in a build
no disc carries (the `seqread` situation; matching as C is the project's
practice for those, counted as library by address). See `SpuVmKeyOn.md`,
round 73, for the rest of the unit.

### Preserved body rebuilt first

The round-67 `#ifdef NON_MATCHING` body, switched live: `build exit=2`, no
compile-error hits, built length **238 words** (`objdump -t`: 0x3b8 bytes)
-- 7 long, not the 6 the title carried -- `funcdiff`: **43/231**,
**insertions 28 / deletions 28**, positional skeleton diffs 185, drift
warning firing (278258 bytes).

### What moved it

Retail SetAutoVol and SetAutoPan are the same code with a
different array family (a normalised `diff` of the two `.s` files differs
only in: `$t0`/`$t1` naming, `lhu` of the accumulator stored to
`D_8008EA10` and multiplied as `s16`, and a reload of `D_8008EA11` for the
third pan test). So this body is SetAutoPan's round-73 body ported with:

- `acc` is `s16`, `acc = D_8008D9AC[v].unk0; D_8008EA10 = acc;`
- first quotient `q2 = (acc * vol) / 16129;`
- third pan test `p = D_8008EA11;` (the global, reloaded -- retail's
  `lui a0; lbu a0; nop`).

Every lever in SetAutoPan.md's round-73 list applies unchanged (field ops
instead of cached locals, `v` for the tail, `off = voice * 8`,
`D_8008D7F0[off + 1]`, reused `s32 p` with `(u32)` bound tests, `q2` reused
for both quotients, `val2 > val1`). The old report's "redundant
sign-extension around the incU/incS accumulator add" was the cached-local
residue: it disappears with lever 1. Frame is 0x18 without padding.

In-tree: `build exit=2`, no grep hits, **223/231**, **insertions 1 /
deletions 1**, positional skeleton diffs 7, no out-of-range drift.

### Residue

Identical to SetAutoPan's and at the same place (vram 0x8002E6FC..
0x8002E724): retail copies the volume into `$a1` in the `else` arm of the
first pan test and multiplies that copy unmasked; this body masks `val1`
(`andi`). The shapes tried against it are tabulated in SetAutoPan.md's
round-73 section; they were not re-run here, since the two functions'
section 1 is byte-identical in retail and in both bodies. No permuter spent
on this function (SetAutoPan's search covers the shared residue).

## Preserved body (round 73 best -- compiles standalone through the pinned pipeline)

```c
#if 0
#include "common.h"
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34Half;
typedef struct {
    u8 pad[0x18];
    u8 masterVolume; /* +0x18 */
} ObjE970;
extern ObjE970 *D_8008E970;
extern s16 D_8008D7F0[];   /* SPU voice-register shadow, 8 halfwords per voice */
extern u8 D_8008D970[];
extern u8 D_8008EA10;
extern u8 D_8008EA11;
extern u8 D_8008EA16;
extern u8 D_8008EA17;
extern u8 D_8008EA19;
extern u8 D_8008EA1A;
extern s16 D_8008E8C0;
extern Rec34Half D_8008D9A4[];
extern Rec34Half D_8008D9A6[];
extern Rec34Half D_8008D9A8[];
extern Rec34Half D_8008D9AA[];
extern Rec34Half D_8008D9AC[];
extern Rec34Half D_8008D9AE[];

void SetAutoVol(s16 voice)
{
    s16 v;
    s16 off;
    s32 p;
    s16 acc;
    s32 vol;
    s32 q1;
    u16 val1;
    u16 val2;
    u32 q2;
    s32 tmp;

    v = voice;
    off = voice * 8;
    if (D_8008D9A8[voice].unk0 != 0) {
        if (D_8008D9AA[voice].unk0-- > 0) {
            return;
        }
        D_8008D9AA[voice].unk0 = D_8008D9A8[voice].unk0;
    }
    D_8008D9AC[voice].unk0 += D_8008D9A6[voice].unk0;
    if (D_8008D9A6[voice].unk0 > 0) {
        if (D_8008D9AC[voice].unk0 >= D_8008D9AE[voice].unk0) {
            D_8008D9AC[voice].unk0 = D_8008D9AE[voice].unk0;
            D_8008D9A4[voice].unk0 = 0;
        }
    } else if (D_8008D9A6[voice].unk0 < 0) {
        if (D_8008D9AC[voice].unk0 <= D_8008D9AE[voice].unk0) {
            D_8008D9AC[voice].unk0 = D_8008D9AE[voice].unk0;
            D_8008D9A4[voice].unk0 = 0;
        }
    }

    acc = D_8008D9AC[v].unk0;
    D_8008EA10 = acc;

    vol = D_8008E970->masterVolume * 0x3FFF;
    q2 = (acc * vol) / 16129;
    q2 = (q2 * D_8008EA16 * D_8008EA19) / 16129u;

    p = D_8008EA1A;
    val1 = q2;
    if ((u32) p < 0x40) {
        val2 = (q2 * p) >> 6;
        val1 = q2;
    } else {
        val2 = val1;
        val1 = (val1 * (0x7F - p)) >> 6;
    }

    p = D_8008EA17;
    if ((u32) p < 0x40) {
        val2 = (val2 * p) / 64;
    } else {
        val1 = (val1 * (0x7F - p)) / 64;
    }

    p = D_8008EA11;
    if ((u32) p < 0x40) {
        val2 = (val2 * p) / 64;
    } else {
        val1 = (val1 * (0x7F - p)) / 64;
    }

    if (D_8008E8C0 == 1) {
        if (val2 > val1) {
            val1 = val2;
        } else {
            val2 = val1;
        }
    }

    D_8008D7F0[off + 1] = val2;
    D_8008D7F0[off] = val1;
    D_8008D970[v] |= 3;
}#endif
```

## Track 2 (round 86, 2026-09-26, alpha)

The per-field symbols this report names (`D_8008D988`..`D_8008D9BA` at a 0x34 stride, and the twelve game names over +0x1C..+0x33 that earlier rounds gave `D_8008D9A4`..`D_8008D9BA`) are ONE Sony table: libsnd/vmanager.o (disc 3.5) bss puts `_svm_voice` at +0x198 of the block anchored at 0x8008D7F0, so `_svm_voice` = 0x8008D988, 24 voices x 0x34 = 0x4E0 bytes, ending exactly at `_svm_envx_ptr`. The symbols file now carries `_svm_voice` (size:0x4E0); the record type is `include/SvmData.h` (fields by offset only, Sony's rule). Field map: +0x00 `unk00` (was `D_8008D988`), +0x02 `unk02` (`D_8008D98A`), +0x04 `unk04` (`D_8008D98C`), +0x06 `unk06` (`D_8008D98E`), +0x08 `unk08` (`D_8008D990`), +0x0A `unk0A` (`D_8008D992`), +0x0C `unk0C` (`D_8008D994`), +0x0E `unk0E` (`D_8008D996`), +0x10 `unk10` (`D_8008D998`), +0x12 `unk12` (`D_8008D99A`), +0x14 `unk14` (`D_8008D99C`), +0x16 `unk16` (`D_8008D99E`), +0x18 `unk18` (`D_8008D9A0`), +0x1B `unk1B` (`D_8008D9A3`), +0x1C..+0x26 `unk1C`..`unk26` (the SeAutoVol/SetAutoVol ramp: active, step, interval, countdown, accum, limit; `D_8008D9A4`..`D_8008D9AE`), +0x28..+0x32 `unk28`..`unk32` (the SeAutoPan/SetAutoPan ramp, same order; `D_8008D9B0`..`D_8008D9BA`). Preserved bodies in this report keep the per-address `D_` spellings, which still link (except `D_8008D988`, which is now `_svm_voice` itself) (splat keeps them as auto-symbols, since the table lies past the global segment's vram range and splat does not fold them into `_svm_voice`).

The NON_MATCHING body now uses `_svm_voice[voice].unk1C`..`unk26`; its normalized disassembly is identical to the per-field version, so the stall and its residue are unchanged.
