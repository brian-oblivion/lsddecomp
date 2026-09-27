# StepVoiceFade -- STALL: length exact (228/228, round 73), 220/228 raw word-match, first diff at vram 0x8002EC6C (the pan split: retail copies the volume into $a1 and multiplies the copy; this body masks val1). libsnd SetAutoPan.

> Renamed from `StepVoiceFade` on 2026-09-24 (tools/rename.py). Address 0x8002ea44.

> Renamed from `func_8002EA44` on 2026-09-20 (tools/rename.py). Address 0x8002ea44.

Unit: `src/code_179d8_l.c`. Round 24 (second pass), runner bravo.

## Screens (clean)

```
grep -n 'gp_rel' asm/nonmatchings/code_179d8_l/SetAutoPan.s            -> no hits
grep -A2 -nE '\b(mflo|mfhi)\b' ... | grep -E '\b(mult|multu|div|divu)\b'  -> no hits
```

## Result

`./build-and-verify.sh` GREEN with `INCLUDE_ASM` restored. Best attempt built
clean (no compile error) but is the WRONG SIZE: 223 words against retail's
228 (measured directly off `objdump -d build/src/code_179d8_l.c.o`, since
`funcdiff`'s own in-range count is unreliable once drift appears -- see
CLAUDE.md's four-ways-a-score-lies list). `funcdiff.py` reports `13/228`
in-range and 238767 bytes of out-of-range drift, both **not fully
trustworthy** given the length mismatch, but the raw word-match figure and
the first-diff location are still worth recording as a starting point for
the next attempt.

## Round 30 (charlie) update: rebuilt (confirmed accurate), one new axis tried (REGRESSION)

Re-spliced this exact preserved body and rebuilt from scratch. **All title
figures reconfirmed:** built length **223 words** (`objdump -t
build/src/code_179d8_l.c.o` confirms `SetAutoPan` at `0x37c` bytes = 223
words, retail 228, so 5 short), `funcdiff.py`'s in-range figure **13/228**
with drift (238724 bytes this run, matching the report's own figure to
within measurement noise).

**New axis: applied `SetAutoVol`'s "woff" idiom** (declare `s16 woff;`,
set `woff = idxCopy << 3;` as one of the function's FIRST statements,
before any of the 0x34-stride record accesses, then store the two final
results through flat `((s16 *) D_8008D7F2)[(u16) woff]` / `((s16 *)
_svm_sreg_buf)[(u16) woff]` casts instead of the `Rec16D7F0`-typed
`array[idxCopy].unk0` form) — this is exactly the SAME "persisted `idx*8`,
consumed once near the end for these same two arrays" shape this report's
own "What's missing" section already named as the open mystery, and it is
the identical fix `SetAutoVol` (this unit's own sibling, same round)
used to close its analogous early-`sll`/mid-block-mask gap. **This
REGRESSED the score**: still 223/228 built length (unchanged), but the
in-range match dropped from 13/228 to 7/228 and out-of-range drift grew
(291307 bytes). Reverted; the ORIGINAL (non-woff) body below is confirmed
the better of the two and is what is preserved.

**Why it regressed, read off `asm-differ` directly — a THIRD placement
variant beyond `SetAutoVol`'s own loop-vs-non-loop distinction.**
Retail's actual dependency chain here is: sign-extend `a0` ONCE into a
temp (`v1 = (s16) a0`), THEN derive `t1 = v1 << 3` (the woff-equivalent)
FROM that already-sign-extended value, and separately reuse the SAME `v1`
to start the 0x34-stride multiply chain — i.e. retail computes the
sign-extension exactly once and shares it between both consumers. Placing
`woff = idxCopy << 3;` at the very top (mirroring `SetAutoVol`'s
winning placement verbatim) instead computes it from the RAW,
not-yet-sign-extended parameter as a standalone early instruction, which
`asm-differ` shows landing as `sll t1,a0,0x3` / `move t0,a0` / `sll
a0,a0,0x10` / `sra a0,a0,0x10` (a SEPARATE zero-cost copy plus a SEPARATE
later sign-extension) rather than retail's `sll v1,a0,0x10` / `sra
v1,v1,0x10` / `sll t1,v1,0x3` (sign-extend first, shift second, one
shared value). This is a strictly worse structural match than the
original (non-woff) attempt, which never introduced the extra early
`move`/re-extension pair in the first place.

**So the idiom's correct placement is a THIRD, function-specific variant,
not just "top of function, `s16`, mask deferred to use" as
`SetAutoVol`'s report generalised it.** Here the shift must be derived
from the ALREADY-sign-extended working value the 0x34-stride chain also
uses (i.e. computed AFTER whatever sign-extends the parameter for its
first record-index use, not before it) — untried this round given the
budget, and worth the first attempt for whoever picks this back up next,
ahead of any further work on the pre-existing frame-size/`$t1`-persistence
mystery this report already documents below.

**This is a substantially harder function than anything else in this unit
so far, and it did not reach a "clean length, register-only residue" state
the way `SpuVmPBVoice` did.** What follows is a full structural derivation
(believed correct in its BROAD SHAPE -- every branch target and field
access matches something in the disassembly) with a genuine, unresolved gap
in one specific spot (see "What's missing" below).

## The one genuinely hard-won piece: BOTH magic-multiply constants identify the SAME divisor, 16129 = 127²

This function contains two divisions by constants whose magic-multiply
encodings looked, at first glance, unrelated:

- `lui v0,(0x82061029>>16); ori v0,v0,(0x82061029&0xffff)` (signed, shift 13)
- `lui v0,(0x040C2051>>16); ori v0,v0,(0x040C2051&0xffff)` (**unsigned**,
  via `multu`, shift 13, with the extra `subu`/`srl 1`/`addu` correction
  step this compiler's unsigned-division idiom needs)

Reverse-engineering a divisor from a magic multiplier by hand is
impractical, but the pinned toolchain IS the ground truth for what any
given divisor compiles to, so the practical move was to ask it directly:
generate one tiny function per candidate divisor, run the WHOLE PINNED
PIPELINE ONCE over the batch (cheap: cc1 -> maspsx -> as in one pass over
thousands of functions took a few seconds), and grep the disassembly for a
match:

```sh
python3 -c "
for d in range(2, 20000):
    print(f'int f_{d}(int n) {{ return n / {d}; }}')" > /tmp/batch.c
# ... run through tools/gcc263/cpp | cc1 | maspsx | as, then:
objdump -d out.o | grep -B3 '8206'      # -> f_16129 matches 0x82061029 exactly
```

The same technique, with `unsigned int` operands and a `u` suffix, found
the SAME divisor (16129) for the second constant (`0x040C2051`). **Both
magic constants are for the SAME divisor, 16129 = 127², just signed vs
unsigned division of it.** This matches the `ori v0,zero,0x7F` (127)
constant appearing twice elsewhere in the same function (in `127 -
byteVal`-shaped expressions), confirming a 7-bit (0-127) percentage-style
fixed-point scale used throughout: this looks like a chained
volume/pan/attenuation calculation where several 0-127 "percentage" byte
values get multiplied together and renormalized by 127² partway through.

**Do not brute-force one candidate at a time through the pipeline** -- each
invocation pays full process-spawn overhead and 20000 iterations one at a
time timed out. Batching thousands of tiny functions into ONE source file
and running the pipeline ONCE is fast; this is the technique to reach for
whenever a magic-multiply constant needs identifying and the divisor isn't
obvious from context.

## Struct/global knowledge derived this round

- `Rec34HalfU`: the SAME 0x34-stride record family already declared
  (`Rec34Half`, `s16 unk0`), but `D_8008D9B2`, `D_8008D9B6` and `D_8008D9B8`
  each need an UNSIGNED (`lhu`) read at least once in this function, on top
  of (for `D_8008D9B2`) a plain SIGNED read at a different point --
  reinterpreted through a cast (`((Rec34HalfU *) D_8008D9B2)[a0].unk0`)
  rather than redeclared, same rule as everywhere else in this file.
- `Rec16D7F0` / `_svm_sreg_buf[]`, `D_8008D7F2[]`: the SAME 0x10-byte-stride
  record family `code_179d8_j.c` documents as `Rec16D7F0`.

  > **HEAD ADJUDICATION, round 24: this bullet's "important correction" is
  > WITHDRAWN. `_svm_sreg_buf`/`D_8008D7F2` remain the two FIELDS of one
  > 0x10-stride record, as `code_179d8_j.c`'s `Rec16D7F0` models them.**
  >
  > The observation was accurate and the inference from it was not. What the
  > disassembly shows here (measured again by the head at lines 230-239 of
  > this function's `.s`) is a separate full `lui`/`addiu` pair per symbol,
  > each then indexed by the same `a0 * 16` offset with
  > `addu $at, $at, $a0`. **That is the NON-CONCLUSIVE case**, and it was
  > established earlier this same round -- see
  > `DECOMPILATION_LEARNINGS.md`, "Two globals a few bytes apart with a
  > COMMON stride are one array":
  >
  > | evidence | conclusion |
  > | --- | --- |
  > | shared base register, small positive fold | ONE object — conclusive |
  > | separate `lui`/`addiu` per symbol | **nothing either way** |
  >
  > "Each field gets its own `lui`/`addiu`" measures whether GCC CHOSE to
  > share a base register at that one site -- a register-pressure question --
  > not whether the symbols ARE one object. A merely-tested or separately
  > stored member routinely gets its own materialisation inside the same
  > function that proves the record is one object elsewhere.
  >
  > Against that, round 23 adjudicated these same two symbols from
  > `SpuVmSetSeqVol`/`SsUtSetDetVVol` on CONCLUSIVE evidence: an
  > `addiu $t2, $a3, 0x2` folding the second base off the first symbol's
  > already-materialised address, which cc1 can only emit if the two are one
  > object. **Conclusive evidence in one function beats non-conclusive
  > evidence in another about the same symbols**, so the one-record model
  > stands and this function's stores are `_svm_sreg_buf[a0].unk0` and
  > `.unk2`.
  >
  > Corroborating, though not by itself decisive: this report's own analysis
  > notes that `a0 * 16` is "the shared byte offset for BOTH stores". One
  > record with two fields explains a shared 0x10 stride directly; two
  > independent arrays would have to coincide in both stride and index.
  >
  > Kept rather than deleted because the OTHER half of the bullet is right
  > and useful: `code_179d8_j.c`'s `_svm_sreg_buf`/`D_8008D7F4` genuinely are
  > two independent arrays of that record shape, which is what makes this
  > family easy to misread in either direction.
- `ObjE970` / `_svm_vh` (pointer variable, `lw`-loaded): only the byte
  field at `+0x18` is read here.
- `D_8008EA10`, `D_8008EA11`, `D_8008EA16`, `D_8008EA17`, `D_8008EA19`,
  `D_8008EA1A`: plain byte scratch globals feeding the percentage-chain
  calculation.
- `_svm_stereo_mono`: an `s16` mode flag; when it equals `1`, both output values
  get forced to their shared maximum.

## Shape (believed correct)

Given a record index `a0` (this function's only parameter): a
countdown/reset gate (`D_8008D9B4[a0]` step, `D_8008D9B6[a0]` countdown,
same "unsigned re-read after unsigned decrement, early-return if still
positive, else reset from `D_8008D9B4`" shape as the OTHER interpolation
functions in this unit), then an accumulator update against a clamped limit
(`D_8008D9B2`/`D_8008D9B8`/`D_8008D9BA`, the SAME "increment, clamp against
a limit whose comparison direction depends on the increment's sign" shape
already documented for `SeAutoPan`'s and `SpuVmKeyOff`'s siblings in
this record family). Then a four-stage 7-bit-percentage blend chain:
combine two byte-scratch globals through the identified 16129 divisor
(twice, once signed once unsigned) to get a base level `q2`, then apply
THREE successive "if (byte < 0x40) scale channel A by byte/64 else scale
channel B by (127-byte)/64" passes (against `D_8008EA1A`, then
`D_8008EA17`, then the just-clamped accumulator's own low byte), optionally
force both channels to their shared max if `_svm_stereo_mono == 1`, then store
the two results into `D_8008D7F2`/`_svm_sreg_buf` and OR a flag bit into
`_svm_sreg_dirty[a0]`.

## What's missing: a persisted early value that doesn't reduce to anything I could name

Retail computes `$t1 = (s16) a0 << 3` as its SECOND instruction (right
after sign-extending the parameter, BEFORE even reading `D_8008D9B4[a0]`),
keeps it alive across the ENTIRE function body (confirmed by `grep '\$t1\b'`
against the `.s` file: exactly two occurrences, the initial computation and
one consuming use ~200 instructions later), and consumes it at the very end
as `(($t1 << 16) >> 15)` -- algebraically `$t1 * 2`, i.e. `a0 * 16`, used as
the shared byte offset for BOTH the `D_8008D7F2` and `_svm_sreg_buf` stores.

**Reproducing this exact split (compute `a0*8` early, double it at the
point of use) did not work as a direct transcription.** Declaring an
explicit `s16 idx8 = a0 << 3;` early and using it via raw pointer
arithmetic at the end (`*(s16 *) ((u8 *) D_8008D7F2 + idx8 * 2) = val2;`)
made the built function SHORTER (223 -> that attempt measured worse, more
drift, 7/228 in-range) rather than closer -- the opposite of what an
accurate reconstruction should do. Reverted.

**A minimal reproducer rules out "this is just how GCC 2.6.3 multiplies an
`s16` by 16"**: a small test function (parameter `s16 idx`, an early-return
branch, a few unrelated statements, then `tbl[idx].unk0 = 5;` on a
0x10-stride array) compiles the index as ONE instruction pair
(`sll v0,a0,16; sra v0,v0,12`, i.e. `idx * 16` directly), not the two-part
`sll 3` / `sll 16, sra 15` split retail shows. So the split is NOT a generic
codegen quirk for multiplying a narrow value by a power-of-two constant --
something specific to THIS function's actual source causes GCC to split the
computation and preserve the partial result across the whole body in a
DEDICATED register.

The clearest signal of a real, unresolved gap: **retail's stack frame is
`0x18` bytes; every attempt this round produced `0x10`** -- 2 fewer words
of frame, meaning my C consistently uses fewer simultaneously-live
callee-saved registers than the real source does. Since the diagnosed `$t1`
value is exactly one such extra live-across-the-whole-function register,
the frame-size gap and the `$t1` mystery are almost certainly the SAME
underlying cause: there is a value in the real source, computed very early
and consumed only once near the very end, that my derivation does not
account for. Whether it is genuinely `a0 * 8` for some purpose distinct
from indexing (used to compute `a0 * 16` as a byproduct, rather than
`a0 * 16` being computed directly), or represents a different quantity that
merely happens to have the same numeric relationship to `a0`, is open.

### Proposed learning

**A register that is defined once, very early, and consumed only once,
much later, with nothing else touching it in between, is a real signal
worth chasing before writing off a stall -- but transcribing it as a
literal early local computing the SAME arithmetic is not sufficient on its
own to reproduce it, and can make the match WORSE if the surrounding
expression doesn't match how the value is actually consumed.** Confirmed
with an isolated reproducer that a plain "index a 0x10-stride array by an
s16 parameter" does NOT produce this split naturally at any function size
tested -- ruling out "this is just GCC's own multiply-by-16 idiom" as an
explanation and confirming SOMETHING about the real source structure
causes it. This is worth flagging for whoever attempts this function next:
the frame-size mismatch (`0x18` vs `0x10`) is the cheapest single check to
confirm whether a future attempt has resolved this class of problem, faster
than reading the full instruction diff.

## Round 32 (bravo) update: reconfirmed, the round-30-flagged "untried lever" TRIED and REGRESSED identically

Re-spliced this exact preserved body and rebuilt from scratch. **All title
figures reconfirmed:** built length 223/228 words (5 short), 13/228 raw
in-range match with drift, first diff at the function's very first
instruction (frame `-0x10` vs retail's `-0x18`).

Round 30's own update named a specific untried next step: derive the
persisted `idx8`/`t1` value (`idx << 3`) from the SAME already-sign-extended
working value the 0x34-stride record-index chain also uses, rather than
from a fresh copy — i.e. compute `idx8 = a0 << 3;` using bare `a0` directly,
placed as the FIRST statement (before `idxCopy = a0;`), so the compiler
shares the sign-extension between `idx8`'s computation and the record-index
multiply chain the very next statement (`D_8008D9B4[a0]`) also needs.
Confirmed directly against the raw `.s` first (not just the report's prose)
that this IS retail's actual dependency shape: `sll v1,a0,16 / sra
v1,v1,16 / sll t1,v1,3 / sll v0,v1,1 / ...` — `v1` (the sign-extended `a0`)
feeds BOTH the `t1` computation and the record-index chain.

**Tried exactly this, twice** (once via a struct-typed final store, once via
a flat `s16 *` cast at the point of use, matching the two ways the value
could plausibly be consumed): **both reproduce round 30's own already-
documented regression, byte for byte** — frame stays `-0x10` (still 2 words
short of retail's `-0x18`), and the WHOLE function's register layout
diverges starting at the very first instruction (not just the specific
`idx8`-consuming tail), landing at 7/228 and 35/228 raw match respectively
depending on exactly how `idxCopy`'s own placement was written (both worse
than the existing 13/228 baseline, and both under drift so not directly
comparable in the ordinary sense — but neither reaches a longer, more
plausible length, both still 5 words short at best in one variant and
mismatched length in the other before being discarded).

**This closes the round-30-flagged lever as tried and failed, not merely
under-explored.** The report's own hypothesis (share the sign-extension
between `idx8` and the record-index chain) is CONFIRMED as retail's actual
mechanism by direct disassembly reading, but transcribing it literally
in C does not make the compiler reproduce it — GCC 2.6.3 does not persist
`t1` across the whole function merely because the source computes it early
from a shared sign-extended value; something else about retail's real
source (not yet identified) is what forces the register to stay live for
~200 instructions rather than being recomputed cheaply at the point of use,
which is exactly what my C's compiled output keeps choosing to do instead
(hence the smaller, wrong-shape frame). Not a lever to retry without a new
idea about what that "something else" is.

## Round 37 (bravo) update: rebuilt, LENGTH CORRECTION (222 not 223), permuter searched for the first time

This is round 37's designated permuter-priority item 3 (this unit's stalls
were flagged as the largest never-permuter-searched block in the corpus --
73/111 near-misses in the project already have a search, none of this
unit's five do). Per this round's own instruction, the preserved body was
spliced back into the live unit and rebuilt from scratch BEFORE trusting
its recorded score.

**The rebuild reproduces the exact same C, but the true built length is
222 words, not 223.** `objdump -t build/src/code_179d8_l.c.o` gives
`SetAutoPan` at `0x378` bytes = 222 words (confirmed independently by
counting disassembled instructions from the function's `addiu sp,sp,-0x18`
line to its final `nop`, inclusive: 222 lines). Retail is 228, so this is
**6 words short, not 5** as every prior round (26, 30, 32) recorded. The
raw word-match figure (13/228) and the first-diff location are UNCHANGED
and still reconfirmed exactly. A byte-for-byte diff of this round's spliced
C against the report's own preserved body (below) found ZERO textual
difference -- so this is not a regression introduced this round, it is a
correction to a figure every prior round re-asserted without re-deriving:
0x37c (223 words) was written once (round 26) and then carried forward as
"reconfirmed" three times without anyone recomputing `0x378` vs `0x37c` by
hand. Filed here per the round's own "build the inherited body before you
trust its score" instruction, which is exactly the class of drift that
instruction exists to catch, even though the underlying C was never
wrong -- only the arithmetic converting its object size to a word count
was.

### Permuter search

`tools/setup-permuter.sh SetAutoPan <seed>` (seed: the preserved body
below, `Rec34HalfU`'s three unsigned-view symbols routed through
`__asm__`-aliased C names to avoid clashing with the plain signed
declarations the same file needs elsewhere -- a scaffold-only device, not
a change to the reported C). `--debug --stack-diffs` base score: see the
Permuter result subsection below for the exact figure and the search
outcome (iteration count and `rc`, captured on the very next command per
this round's own rule).

#### Permuter result

`--debug --stack-diffs` base score: **4125** (Register Differences 81 x 5
= 405; Reorderings 7 x 60 = 420; Insertions 20 x 100 = 2000; Deletions 13
x 100 = 1300; **Stack Differences 0** despite this report's own documented
frame gap (`0x10` built vs retail's `0x18`) -- the permuter's stack-diff
detector evidently tracks addressed stack SLOTS, not the raw
`addiu sp,sp,-N` immediate by itself, and nothing in either version
addresses the extra 8 bytes via `$sp`-relative load/store, consistent with
this report's own read of the frame gap as an unidentified LIVE-REGISTER
difference rather than a stack-variable difference).

Real search: `timeout 900 permuter.py -j 6 --stop-on-zero --best-only
--stack-diffs` via the harness's `run_in_background`. **Completed cleanly,
`rc=124`** (own bound) after **63,989 iterations**. Best score: **2430**
(from base 4125), saved at
`permuter-work/SetAutoPan/output-2430-1/`; no zero reached.

The 2430 candidate's only content change from the scaffold is dropping the
intermediate `limit = D_8008D9BA[a0].unk0;` assignment in the
`incrementS < 0` branch and inlining the array read directly into the
comparison (`if (D_8008D9BA[a0].unk0 >= (s16) accum)`), leaving a stray
empty statement (`;`) where the assignment was removed -- a permuter
scope/statement-elision artifact, not a naming of the actual missing
early-persisted value this report's own "What's missing" section
describes. **Not closed; the persisted-register/frame-gap mystery this
report already diagnoses (a value computed early and consumed once near
the end, forcing a wider retail frame than any tried derivation
reproduces) did not move in ~64k iterations.**

## Round 48 update (runner echo): tested charlie's frame-padding lever -- second confirmed negative for length closure

Round 48's designated test of charlie's `ContDataEntry` frame-padding
discovery (`u8 dead[N];` under `if (0) { dead[0] = 0; }`, sized to THIS
BUILD's frame gap vs retail, not retail's raw unaddressed-byte count).
Rebuilt the round-37 preserved body first, in isolation: **reconfirmed
exactly** -- 222/228 built words, 13/228 raw match, matching this report's
own prior figures.

**This function's frame gap is the same 8 bytes the report's own "What's
missing" section already names**: this build's frame is `-0x10`, retail's
is `-0x18`. Applied `u8 dead[8];` under the established `if (0)` guard.

**Result: frame realigns byte-exactly (`addiu sp,sp,-0x18`, confirmed via
objdump), but built LENGTH is UNCHANGED at 222/228 (still 6 words
short).** Raw word-match barely moved (13/228 -> 14/228, noise-level).
This is the SAME negative shape found this round on this unit's sibling
`SpuVmFlush`: charlie's padding idiom rewrites only the `addiu
sp,sp,-N` / `sw $sN,N(sp)` IMMEDIATE operands of already-existing
prologue/epilogue instructions, which costs the same instruction count
regardless of the immediate value -- it cannot by itself manufacture
missing content words. It only recovers frame BYTE-ALIGNMENT, which makes
the rest of the disassembly comparison meaningful again.

With the frame realigned, `tools/asm-differ/diff.py SetAutoPan` shows
retail computing `t1 = v1 << 3` (the sign-extended `a0`, shifted) as its
THIRD instruction and holding it live, exactly as this report's own "What's
missing" section already diagnosed from the un-realigned disassembly —
**the frame fix did not surface any NEW residue here** (unlike
`SpuVmFlush`, where realignment revealed a fresh, fixable `andi 0xff`
mask). The already-tried-and-regressed `$t1` transcription (rounds 30 and
32, both confirmed negative) remains the actual open gap; frame padding
does not touch it and this report's existing conclusion stands unchanged.
**Not a new lead — confirms the existing "persisted early value" diagnosis
is still the real blocker, now cross-checked with the frame byte-exact.**

Reverted to `INCLUDE_ASM`; whole-image SHA1 reconfirmed green.

### Proposed learning (same lever, second data point)

**Two-for-two this round: charlie's `dead[N]`/`if(0)` frame-padding idiom
recovered frame byte-alignment exactly on both `SpuVmFlush` and
`SetAutoPan`, and closed the missing-WORD-count gap on NEITHER.** Both
functions' extra retail frame bytes are pure unaddressed register-save-area
padding with no companion missing-instruction elsewhere in THIS unit,
unlike `ContDataEntry` (a different unit), whose length recovery came from
a separate, coincidental tail-duplication fix applied alongside the
padding. **The lever's reliable value is diagnostic (realign the frame so
the rest of the diff reads true), not a length-closing move by itself** --
whether it ALSO closes a length gap depends on whether retail's extra bytes
correlate with extra addressed content elsewhere in the SAME function,
which has to be checked separately, not assumed from the frame gap itself.

## Superseded preserved body (pre-round-73 best attempt, 222/228 built words -- 6 short, structurally believed correct except for the missing early-persisted value noted above)

```c
#if 0
/* Same 0x34-stride record family, UNSIGNED 16-bit view -- D_8008D9B2,
 * D_8008D9B6 and D_8008D9B8 each need this width (`lhu`) at least once
 * in this function, on top of the plain signed Rec34Half view
 * declared above (which some of these same symbols also need, at a
 * DIFFERENT read site in this same function). Reinterpreted through a
 * cast rather than redeclared, per this project's rule that one
 * extern symbol cannot carry two conflicting C types in one file. */
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34HalfU;

/* Same 0x10-byte-stride record family code_179d8_j.c documents as
 * Rec16D7F0 (that unit's own _svm_sreg_buf/D_8008D7F4 pair); local view.
 * _svm_sreg_buf and D_8008D7F2 here are TWO INDEPENDENT arrays of this
 * shape (each gets its own %hi/%lo pair in the disassembly), not one
 * record's two fields. */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x10 - 0x2];
} Rec16D7F0;
extern Rec16D7F0 _svm_sreg_buf[];
extern Rec16D7F0 D_8008D7F2[];

extern u8 _svm_sreg_dirty[];

/* Pointer to an object; only the byte field this function reads is
 * named. */
typedef struct {
    u8 pad[0x18];
    u8 unk18; /* +0x18 */
} ObjE970;
extern ObjE970 *_svm_vh;

/* Scratch bytes for a chained percentage-of-percentage volume/pan
 * calculation -- both magic-multiply divisions in this function are
 * by 16129 (=127*127), confirmed by brute-forcing candidate divisors
 * through the pinned toolchain until the exact magic constants
 * (0x82061029 signed, 0x040C2051 unsigned) appeared. */
extern u8 D_8008EA10;
extern u8 D_8008EA11;
extern u8 D_8008EA16;
extern u8 D_8008EA17;
extern u8 D_8008EA19;
extern u8 D_8008EA1A;

/* Mode flag: forces both output channels to the same (maximum) level
 * when set to 1. */
extern s16 _svm_stereo_mono;

void SetAutoPan(s16 a0) {
    s16 idxCopy;
    s16 step;
    u16 increment;
    s16 incrementS;
    u16 accum;
    s16 limit;
    u8 accumByte;
    u8 tableval;
    s32 product1;
    s32 q1;
    s32 q1b;
    u32 q1c;
    u32 q2;
    u16 val1;
    u16 val2;
    s32 tmp;
    u8 v0;

    idxCopy = a0;
    step = D_8008D9B4[a0].unk0;
    if (step != 0) {
        u16 current = ((Rec34HalfU *) D_8008D9B6)[a0].unk0;

        ((Rec34HalfU *) D_8008D9B6)[a0].unk0 = current - 1;
        if ((s16) current > 0) {
            return;
        }
        ((Rec34HalfU *) D_8008D9B6)[a0].unk0 = D_8008D9B4[a0].unk0;
    }

    increment = ((Rec34HalfU *) D_8008D9B2)[a0].unk0;
    accum = ((Rec34HalfU *) D_8008D9B8)[a0].unk0;
    incrementS = D_8008D9B2[a0].unk0;
    accum = accum + increment;
    ((Rec34HalfU *) D_8008D9B8)[a0].unk0 = accum;

    if (incrementS > 0) {
        limit = D_8008D9BA[a0].unk0;
        if ((s16) accum >= limit) {
            accum = limit;
            ((Rec34HalfU *) D_8008D9B8)[a0].unk0 = accum;
            D_8008D9B0[a0].unk0 = 0;
        }
    } else if (incrementS < 0) {
        limit = D_8008D9BA[a0].unk0;
        if (limit >= (s16) accum) {
            accum = limit;
            ((Rec34HalfU *) D_8008D9B8)[a0].unk0 = accum;
            D_8008D9B0[a0].unk0 = 0;
        }
    }

    accumByte = *(u8 *) &D_8008D9B8[a0].unk0;
    D_8008EA11 = accumByte;
    tableval = _svm_vh->unk18;

    product1 = D_8008EA10 * (tableval * 0x3FFF);
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

    if (accumByte < 0x40) {
        tmp = val2 * accumByte;
        val2 = tmp / 64;
    } else {
        tmp = val1 * (0x7F - accumByte);
        val1 = tmp / 64;
    }

    if (_svm_stereo_mono == 1) {
        if (val1 < val2) {
            val1 = val2;
        } else {
            val2 = val1;
        }
    }

    D_8008D7F2[idxCopy].unk0 = val2;
    v0 = _svm_sreg_dirty[idxCopy];
    _svm_sreg_buf[idxCopy].unk0 = val1;
    v0 |= 3;
    _svm_sreg_dirty[idxCopy] = v0;
}
#endif
```

## Naming

**SetAutoPan** (was `func_8002EA44`) -- Tier B. Companion to
SeAutoPan: advances `D_8008D9B8` toward `D_8008D9BA` by
`D_8008D9B2`, throttled by `D_8008D9B4`/`D_8008D9B6`,
clears `D_8008D9B0` on reaching the limit, then computes and writes
this voice's stereo output level from the resulting percentage (same
final block as SetAutoVol, confirmed near-identical between the
two reports). "Fade" rather than a more specific name for the same reason
as SeAutoPan: what value is actually being interpolated in-game is
not established from this function's body alone.

## NON_MATCHING body promoted, round 67

Placed in `src/code_179d8_l.c` under `#ifdef NON_MATCHING`, `INCLUDE_ASM`
kept in `#else`. All of the preserved body's own local `Rec34HalfU`/
`Rec16D7F0`/plain-byte-global declarations were already present in the
unit's shared prelude (moved up for `SetAutoVol`, first in ROM
order) and were dropped here rather than re-typedef'd. One real field-name
update: the body's own local `ObjE970` (`unk18`) collided with the shared
`ObjE970` the prelude already declares with the same offset under the name
`masterVolume`; the access was changed to `_svm_vh->masterVolume`, same
offset, no behavior change. `./build-and-verify.sh` green (zero bytes
changed) and `tools/check-nonmatching.sh code_179d8_l` green.

## Round 73 (delta): REVISIT -- 13/228 (6 short) -> 220/228 length-exact, ins 1 / del 1

REVISITED, round 73: STALL improved to length-exact 220/228 (ins 1 / del 1), residue is one register copy in the pan split; names/types used (param renamed `voice`, locals `v`/`off`/`acc`/`vol`/`q2`/`p`/`val1`/`val2`; SPU shadow stores spelled through `_svm_sreg_buf[off + 1]`).

### Ownership, read before spending more on this

`sdkname.py` puts its sibling `SetAutoVol` at **shape 0.99** against
libsnd `SetAutoVol` (3.3 `vmanager`), and Sony's 3.3 `SetAutoPan` has this
function's opening skeleton (224w; the 0x30 voice stride there vs 0x34 here
accounts for the length). This is libsnd's `SetAutoPan` in a build no disc
carries -- the `seqread` situation, where the project does match as C and
counts it as library by address. `SpuVmKeyOn.md` (round 73) has the unit-wide
evidence (StartNote is `SpuVmKeyOn`).

### Preserved body rebuilt first

The round-67 `#ifdef NON_MATCHING` body, switched live: `build exit=2`, no
compile-error hits, built length **222 words** (6 short, confirmed),
`funcdiff`: **13/228**, **insertions 39 / deletions 39**, positional
skeleton diffs **212**, out-of-range drift warning firing (238642 bytes).

### What moved it, one lever per line

Measured with a standalone scaffold through the pinned pipeline (the
CLAUDE.md recipe, maspsx flags read from the Makefile), scored by a
line-level diff against the retail `.s` ("lines" below is that scaffold
metric, out of 228; not funcdiff words), then anchored in-tree.

1. Every record access written as a plain field op, no cached u16 locals:
   `if (D_8008D9B6[voice].unk0-- > 0) return;`,
   `D_8008D9B8[voice].unk0 += D_8008D9B2[voice].unk0;`,
   `if (accum[voice] >= limit[voice]) { accum[voice] = limit[voice]; ... }`.
   **222 -> 228 words, length exact.** Cause, checked in the `-ds` dump: the
   old body's `current`/`accum` locals made CSE share the element ADDRESS
   (`lui/addiu/addu aN` once, then `0(aN)`); retail re-derives
   `sym(off)` through `$at` at every access, i.e. no address pseudo exists.
2. Separate index names: `voice` for the head, `v = voice` for the tail.
   Retail recomputes `idx*0x34` from the raw copy in `$t0` for the byte
   reload of the accumulator and for `_svm_sreg_dirty` -- the tail really uses a
   second variable.
3. SPU-shadow stores as `_svm_sreg_buf[off + 1] = val2; _svm_sreg_buf[off] = val1;`
   (the `D_8008D7F2` of the asm is the `+2` of the same record) and
   `_svm_sreg_dirty[v] |= 3` last: the `_svm_sreg_dirty` load then stays AFTER the
   first store, as retail has it (this morning's const-plus-address lever).
4. `off = voice * 8;` instead of `voice << 3`: retail's `$t1` is shifted from
   the NARROWED voice (`sll t1,v1,3`); `<< 3` let combine read raw `$a0`.
   Same edit took the frame from 0x10 to retail's **0x18**: the leaf's dead
   frame is one stack slot per sign-extension pseudo that combine folded
   away (`-dg`: pseudos with zero conflicts and no hard reg, "ST_REGS or
   none"), and `* 8` leaves one more of them. No `dead[]` padding needed.
5. Pan is a reused `s32 p` (`p = D_8008EA1A; ... p = D_8008EA17; ...
   p = acc;`) with `(u32) p < 0x40` bound tests: the third test reading the
   byte through `p` is what produces retail's `move a0,a3` join copy, and
   `(u32)` keeps `sltiu`. `u8` p gave `andi a0,a3,0xff`, u16 `andi 0xffff`,
   u32 lost the signed rounding of the `/ 64`s (216 words).
6. `acc = *(u8 *) &D_8008D9B8[v].unk0` into an `s32` (plain `lbu`);
   using the global `D_8008EA11` for the third test reloads it (230 words).
7. Volume chain `vol = _svm_vh->masterVolume * 0x3FFF;
   q2 = (D_8008EA10 * vol) / 16129; q2 = (q2 * D_8008EA16 * D_8008EA19) / 16129u;`
   -- `vol` as its own statement fixes the load order (`lw` ptr, `lbu +0x18`
   before `lbu D_8008EA10`), reusing `q2` for both quotients fixes the
   `subu v1`/`mult v1` register of the first quotient. Associativity inside
   one expression changes nothing (three variants, identical).
8. `if (val2 > val1)` rather than `val1 < val2` for the stereo max: fixes the
   order of the two `andi 0xffff`.

In-tree with all of it (the body now in the unit's `#ifdef NON_MATCHING`):
`build exit=2`, no grep hits, **220/228**, **insertions 1 / deletions 1**,
positional skeleton diffs 7, no out-of-range drift.

### The residue (checked hypothesis, not closed)

Only the pan split's first `if` differs. Retail:

```
beqz  v0, ARM2
 addu a1,v1,zero        ; stolen by reorg from ARM2's first insn
mult  v1,a0 ; addu a1,v1,zero ; mflo v0 ; j JOIN ; srl a2,v0,6
ARM2: li v0,127 ; subu v0,v0,a0 ; mult a1,v0 ; addu a2,a1,zero ; mflo v0 ; srl a1,v0,6
```

The `else` arm copies the volume (`v1`) into `a1` -- the register that then
receives `val1` -- and multiplies THAT, unmasked. So the multiply's operand
is a 32-bit SImode pseudo equal to `q2` that is not `q2`, and that pseudo
won CSE's canonical-register choice (cse.c `make_regs_eqv`: the copy's
destination becomes canonical only if it lives outside the CSE block and
dies after the source). Every C spelling tried either lets CSE fold the
copy away (the multiply reads `v1`) or reads `val1` as u16 (an `andi`).
The Sony 3.3 `SetAutoVol` object shows the identical sequence, so it is
the natural compile of Sony's source, not a toolchain effect.

Negatives, each in scaffold lines/228 (length in words):

| lever tried on section 1 | words | lines |
| --- | --- | --- |
| best: `val1 = q2;` before the if, `else { val2 = val1; val1 = (val1*(0x7F-p))>>6; }` | 228 | 219 |
| plain two-arm `val2=...; val1=q2` / `val1=...; val2=q2` (either order) | 227 | 220 |
| same with an `s32 tmp` holding each product | 227 | 220 |
| `x = q2` (u32) in the else arm, or before the if | 227 | 217 |
| copy through `vol` (set earlier, so CSE-canonical): copy folds into `v1` | 227 | 216 |
| copy through `q1` / `tmp` (first set there) | 227 | 220 |
| `val1 = val2 = q2;` then one-sided updates | 227 | 216-219 |
| u32/s32 `L`,`R` for section 1 then `val1 = L; val2 = R` (4 shapes) | 227-228 | 200-218 |
| val1/val2 as u32/s32 with `(u16)` casts in sections 2/3 | 226 | 211 |
| assignment-as-value `((val2 = q2) * (0x7F - p))` (3 shapes) | 228-229 | 200-219 |
| q2 in {u32,s32,u16,s16} x val in {u16,s16} | 227-239 | 205-220 |
| `tmp` reused as the section 2/3 product temporary | 228 | 185-211 |

That is more than 30 consecutive builds after the best body without
improvement: this function stopped by the budget rule.

### Permuter search (one, bounded)

Gate 3, all three checks run: (1) scaffold from this body via
`tools/setup-permuter.sh` compiles and scores, base 235; (2) its
`--debug --stack-diffs`: insertions 1 / deletions 1, register differences
7; (3) in-tree `funcdiff`: insertions 1 / deletions 1, 7 positional
skeleton diffs -- AGREE, search meaningful. `timeout 1500 permuter.py -j 6
--stop-on-zero --best-only`: **rc 124 (timeout), 232,988 iterations, no
zero.** Outputs: 225 (`p = 0x7F - p; val1 = (val1 * p) >> 6;` -- legal,
but rebuilt it keeps the same `andi`, only the subtraction's register
moves: not adopted), 195 (drops the pre-branch `val1 = q2`, leaving val1
uninitialised on the else path: UB, rejected), 120 (`val1 = p`: changes
semantics, rejected).

### Proposed learning

- **A load/store that retail reaches through `lui at/addiu at/addu at,at,aK`
  at every access, where yours hoists one `addu aN,aK,vX` and then uses
  `0(aN)`, means your C cached a VALUE in a local and CSE then shared the
  ADDRESS.** Write the field access each time (`x[i].f--`, `x[i].f += ...`).
  Closed 6 words here and 6 in `SetAutoVol`.
- **A leaf function's dead frame (no `$sp` access, frame > 0) counts the
  sign-extension pseudos that combine folded into loads/compares.** Each
  keeps its register-allocation record and gets a stack slot. A frame
  smaller than retail's = one such pseudo missing; `x * 8` vs `x << 3` was
  one of them here. Prefer this to `dead[]` padding, which only fixes the
  immediate.

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
extern ObjE970 *_svm_vh;
extern s16 _svm_sreg_buf[];   /* SPU voice-register shadow, 8 halfwords per voice */
extern u8 _svm_sreg_dirty[];
extern u8 D_8008EA10;
extern u8 D_8008EA11;
extern u8 D_8008EA16;
extern u8 D_8008EA17;
extern u8 D_8008EA19;
extern u8 D_8008EA1A;
extern s16 _svm_stereo_mono;
extern Rec34Half D_8008D9B0[];
extern Rec34Half D_8008D9B2[];
extern Rec34Half D_8008D9B4[];
extern Rec34Half D_8008D9B6[];
extern Rec34Half D_8008D9B8[];
extern Rec34Half D_8008D9BA[];

void SetAutoPan(s16 voice)
{
    s16 v;
    s16 off;
    s32 p;
    s32 acc;
    s32 vol;
    s32 q1;
    u16 val1;
    u16 val2;
    u32 q2;
    s32 tmp;

    v = voice;
    off = voice * 8;
    if (D_8008D9B4[voice].unk0 != 0) {
        if (D_8008D9B6[voice].unk0-- > 0) {
            return;
        }
        D_8008D9B6[voice].unk0 = D_8008D9B4[voice].unk0;
    }
    D_8008D9B8[voice].unk0 += D_8008D9B2[voice].unk0;
    if (D_8008D9B2[voice].unk0 > 0) {
        if (D_8008D9B8[voice].unk0 >= D_8008D9BA[voice].unk0) {
            D_8008D9B8[voice].unk0 = D_8008D9BA[voice].unk0;
            D_8008D9B0[voice].unk0 = 0;
        }
    } else if (D_8008D9B2[voice].unk0 < 0) {
        if (D_8008D9B8[voice].unk0 <= D_8008D9BA[voice].unk0) {
            D_8008D9B8[voice].unk0 = D_8008D9BA[voice].unk0;
            D_8008D9B0[voice].unk0 = 0;
        }
    }

    acc = *(u8 *) &D_8008D9B8[v].unk0;
    D_8008EA11 = acc;

    vol = _svm_vh->masterVolume * 0x3FFF;
    q2 = (D_8008EA10 * vol) / 16129;
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

    p = acc;
    if ((u32) p < 0x40) {
        val2 = (val2 * p) / 64;
    } else {
        val1 = (val1 * (0x7F - p)) / 64;
    }

    if (_svm_stereo_mono == 1) {
        if (val2 > val1) {
            val1 = val2;
        } else {
            val2 = val1;
        }
    }

    _svm_sreg_buf[off + 1] = val2;
    _svm_sreg_buf[off] = val1;
    _svm_sreg_dirty[v] |= 3;
}#endif
```

## Track 2 (round 86, 2026-09-26, alpha)

The per-field symbols this report names (`D_8008D988`..`D_8008D9BA` at a 0x34 stride, and the twelve game names over +0x1C..+0x33 that earlier rounds gave `D_8008D9A4`..`D_8008D9BA`) are ONE Sony table: libsnd/vmanager.o (disc 3.5) bss puts `_svm_voice` at +0x198 of the block anchored at 0x8008D7F0, so `_svm_voice` = 0x8008D988, 24 voices x 0x34 = 0x4E0 bytes, ending exactly at `_svm_envx_ptr`. The symbols file now carries `_svm_voice` (size:0x4E0); the record type is `include/SvmData.h` (fields by offset only, Sony's rule). Field map: +0x00 `unk00` (was `D_8008D988`), +0x02 `unk02` (`D_8008D98A`), +0x04 `unk04` (`D_8008D98C`), +0x06 `unk06` (`D_8008D98E`), +0x08 `unk08` (`D_8008D990`), +0x0A `unk0A` (`D_8008D992`), +0x0C `unk0C` (`D_8008D994`), +0x0E `unk0E` (`D_8008D996`), +0x10 `unk10` (`D_8008D998`), +0x12 `unk12` (`D_8008D99A`), +0x14 `unk14` (`D_8008D99C`), +0x16 `unk16` (`D_8008D99E`), +0x18 `unk18` (`D_8008D9A0`), +0x1B `unk1B` (`D_8008D9A3`), +0x1C..+0x26 `unk1C`..`unk26` (the SeAutoVol/SetAutoVol ramp: active, step, interval, countdown, accum, limit; `D_8008D9A4`..`D_8008D9AE`), +0x28..+0x32 `unk28`..`unk32` (the SeAutoPan/SetAutoPan ramp, same order; `D_8008D9B0`..`D_8008D9BA`). Preserved bodies in this report keep the per-address `D_` spellings, which still link (except `D_8008D988`, which is now `_svm_voice` itself) (splat keeps them as auto-symbols, since the table lies past the global segment's vram range and splat does not fold them into `_svm_voice`).

The NON_MATCHING body now uses `_svm_voice[voice].unk28`..`unk32` (and `*(u8 *) &_svm_voice[v].unk30`); normalized disassembly identical to before, stall unchanged.

**_svm_sreg_buf / _svm_sreg_dirty (same round).** `D_8008D7F0` (0x180 bytes, 24 voices x 0x10, halfwords at +0x0..+0xA spelled `D_8008D7F0`..`D_8008D7FA` by splat) is Sony's `_svm_sreg_buf` and `D_8008D970` (24 bytes) is `_svm_sreg_dirty`: libsnd/vmanager.o bss +0x000 and +0x180, anchored at 0x8008D7F0. Both are in the symbols file; the record type is `SvmSreg` in `include/SvmData.h` (fields by offset). The NON_MATCHING body keeps `((s16 *) _svm_sreg_buf)[off]`/`[off + 1]` and `_svm_sreg_dirty`; normalized disassembly identical.

## Track 6 (round 96, charlie)

Round 96 (charlie, track 6) moved `src/code_179d8_l.c` onto Sony's headers (`<libsnd.h>`, `<libspu.h>`) and Sony's types; zero bytes changed, whole-image SHA1 green, NON_MATCHING bodies compile. `ObjE970` is `VabHdr` (`_svm_vh`): this body reads `D_8008E970->mvol` (+0x18, was `masterVolume`). Normalized disassembly unchanged.
