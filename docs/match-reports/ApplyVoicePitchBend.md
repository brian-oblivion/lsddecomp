# ApplyVoicePitchBend -- STALL: exact length (138/138 words), 77/138 raw word-match, first diff at file 0x1FBEC / vram 0x8002F3EC (word 1, `move t2,a0` vs `move t1,a0`)

> Renamed from `func_8002F3E8` on 2026-09-20 (tools/rename.py). Address 0x8002f3e8.

Unit: `src/code_179d8_m.c`. Round 24 (second pass), runner bravo.

## Screens (clean)

```
grep -n 'gp_rel' asm/nonmatchings/code_179d8_m/ApplyVoicePitchBend.s            -> no hits
grep -A2 -nE '\b(mflo|mfhi)\b' ... | grep -E '\b(mult|multu|div|divu)\b'  -> no hits
```

## Result

`./build-and-verify.sh` GREEN with the `INCLUDE_ASM` restored (this report's
body is not in `src/`). Best attempt built clean (no compile error) and
matched **exactly retail's length** (`0x228` bytes = 138 words, confirmed
zero out-of-range drift every time this specific body was measured) at
**77/138 raw words**. `asm-differ`'s realigned diff puts the first real
divergence at the SECOND instruction of the function:

```
1fbe8:  addiu sp,sp,-0x18      1fbe8:  addiu sp,sp,-0x18     (match)
1fbec:  move  t2,a0            1fbec:  move  t1,a0            <- first diff
```

## Round 30 (charlie) update: rebuilt (confirmed accurate), one new axis tried (no change)

Re-spliced this exact preserved body and rebuilt from scratch (typedef
names local to this splice were renamed only to route around unrelated
duplicate-declaration conflicts with OTHER still-`INCLUDE_ASM` functions
sharing the file; the reported C itself is unchanged). **All three title
figures reconfirmed:** `funcdiff.py` reports **77/138 words match, file
0x1FBE8-0x1FE10** (zero out-of-range drift, i.e. genuinely exact length),
and `asm-differ` confirms the first real divergence is still the function's
second instruction, `move t2,a0` (retail) vs `move t1,a0` (built).

**New axis 12: applied this unit's own `idxCopy = a0;`-at-the-top idiom**
(the pattern already established and proven in `StepVoiceEnvelope`/
`StepVoiceFade`, sibling functions in this same unit) — declare `s16
idxCopy;`, assign it from `a0` as the function's first statement, and use
`idxCopy` in place of `a0` for every one of the record-index accesses
(`D_8008D996[idxCopy]`, `D_8008D99E[idxCopy]`, etc., through to the final
`D_8008EA26 = idxCopy;` and `D_8008D7F4[idxCopy].unk0 = ...`). **No change
in score (still 77/138, still zero drift)** — but the SPECIFIC register
assignment shifted in an informative way. Before this change, `asm-differ`
showed a near-total register-CLASS renumbering starting at the function's
second instruction. After it, the very first temp (idxCopy's own dedicated
copy) still lands one register off retail (`t1` vs retail's `t2`), but the
multiply-chain index term (`idx*52`, retail's dedicated `t0`) now lands in
plain `$a0` instead of a `$t`-class register at all:

```
1fc20:    sll     t0,v0,0x2     (retail)      1fc20:    sll     a0,v0,0x2     (built, this axis)
```

This is diagnostic: introducing `idxCopy` as a plain alias of `a0` lets
GCC's optimizer notice the two are value-equivalent immediately after the
assignment and back-substitute the ALREADY-sign-extended `a0` register
into the multiply-chain expression, rather than allocating it a fresh
dedicated temp the way retail's compiled code does. Retail, by contrast,
keeps the multiply-chain result in `$t0` — a register genuinely
INDEPENDENT of both the raw index copy (`$t2`) and the sign-extended
working value — which is NOT what a simple `idxCopy = a0;` alias produces
in this compiler. This rules out "a plain top-of-function alias" as the
missing lever and refines the residue's likely cause: retail's real
source probably references the ORIGINAL parameter in a way that keeps
`a0`'s sign-extended value and the multiply-chain index as two textually
and semantically SEPARATE quantities (not one value copied into a second
name), which is exactly the shape a caller-visible or cross-call-clobbered
value would have — reinforcing, rather than resolving, the open
"undentified caller-visible register" hypothesis already in this report.
Reverted (no improvement to keep).

## Signature (derived, and cross-checked against `ApplyPitchBendToAllVoices`'s call site)

```c
s16 ApplyVoicePitchBend(s16 a0, s16 a1, s16 a2, s16 a3, u16 a4);
```

`ApplyPitchBendToAllVoices` (matched 60/60 this round, unchanged) calls this function as
`ApplyVoicePitchBend(i, a0, a1, a2, a3)` and its own report originally reasoned the
second parameter must be `s32` (to explain a per-iteration re-sign-extension
at the call site). **That reasoning was re-examined and retracted this
round**: the MIPS o32 ABI represents an `s16` argument as a properly
sign-extended 32-bit value regardless of whether the callee declares it
`s16` or `s32`, so the caller-side widening instructions do not distinguish
the two. Declaring it `s16` here (matching every neighbouring parameter, and
matching this function's own single, once-only use of it) does not change
`ApplyPitchBendToAllVoices`'s compiled bytes at all -- re-verified 60/60 with zero drift
after the change. The comparison against the field it is checked against
needs an explicit `(s16)` cast on the OLD `s32` typing; once retyped `s16`
the cast is redundant and was dropped. This did not move `ApplyVoicePitchBend`'s
own score (77/138 before and after), so it is recorded as a correction to
`ApplyPitchBendToAllVoices`'s report, not a lever for this one.

## Struct/global knowledge derived this round (kept in `src/`, unused while stalled)

- `Rec34U16` / `D_8008D99C[]`: the SAME 0x34-stride record family as
  `Rec34S16` (already declared above for `D_8008D994`/`D_8008D996`/
  `D_8008D99A`/`D_8008D99E`/`D_8008D988`), but this function reads
  `D_8008D99C` UNSIGNED (`lhu`) where `StopNote` (same unit, already
  matched) reads it -- and `D_8008D994` -- SIGNED (`lh`). Since one extern
  symbol cannot carry two conflicting C types in one translation unit, the
  unsigned view is reached with a pointer-cast reinterpretation:
  `((Rec34U16 *) D_8008D994)[a0].unk0`. `D_8008D99C` is ALSO read at a
  SECOND, BYTE width (`lbu`, same offset 0) later in this same function --
  the established `*(u8 *)&sym` idiom (see `PlayFixedSound`'s report)
  applies again, on top of the signed/unsigned split.
- `Tbl32E978` / `D_8008E978` (pointer variable, `lw`-loaded): a 0x20
  (32)-byte-stride table; only two trailing byte fields are read,
  `unkC`/`unkD`. **Getting the stride wrong (14 bytes, from
  `{u8 pad[0xC]; u8 unkC; u8 unkD;}` with no trailing pad) was the single
  highest-value fix this round** -- it turned a `sll #1` (multiply by 2, an
  outright wrong scale) into retail's `sll #5` (multiply by 32) and moved
  the score from 1/138 to 38/138 in one edit. Padded to `0x20 - 0xE` bytes
  after `unkD` to get the right stride.
- `Rec16D7F4` / `D_8008D7F4[]`: same 0x10-byte-stride record family
  `code_179d8_j.c` already documents as `Rec16D7F0` (its own
  `D_8008D7F0`/`D_8008D7F4` pair) -- local view, `s16 unk0`.
- `D_8008D970[]`: plain byte-stride flags array (no per-record multiply in
  its own addressing, unlike every 0x34/0x10-stride array above).
- `D_8008EA13`, `D_8008EA18`: plain byte globals.
- `func_8002E038` (defined in `code_179d8_l`, still `INCLUDE_ASM` there):
  called as `func_8002E038(outA2 & 0xFFFF, outA1 & 0xFFFF)`, guessed
  `extern s16 func_8002E038(u16 a0, u16 a1);` from the call-site register
  widths and the fact its return value gets stored into a `s16`-shaped
  record field.

## Shape

Given a record index `a0` and three `s16` key values (`a1`, `a2`, `a3`),
verify they match `D_8008D996[a0]`/`D_8008D99E[a0]`/`D_8008D99A[a0]`
respectively (three early-return guard clauses, same idiom as
`StopNote`'s key-match loop). `threshold = a4 - 0x40` is computed
BEFORE these checks (an early, unconditional local -- moving it there from
an initial after-the-checks placement was itself a real fix, see below). If
all three match: combine a debug byte into an index (`someTotal =
D_8008D99C[a0] + (D_8008EA13 << 4)`), read a `baseValue` from `D_8008D994`
(unsigned view), then based on `threshold`'s sign, look up a per-table byte
(`D_8008E978[someTotal].unkD` for positive, `.unkC` for negative) and
combine it with `threshold` through a division (`/63` positive, `/64`
negative, each producing a quotient/remainder pair fed into `outA2`/`outA1`
-- see below); `threshold == 0` just leaves `outA2 = baseValue`, `outA1 =
0`. Finally: read `D_8008D99C[a0]`'s low BYTE, stash `a0` into the
"currently selected channel" scratch `D_8008EA26`, store the byte into
`D_8008EA18`, call `func_8002E038(outA2 & 0xFFFF, outA1 & 0xFFFF)` and store
its return into `D_8008D7F4[a0]`, OR a flag bit into `D_8008D970[a0]`, and
return `1`.

## Axes tried, roughly in order, with effect on the in-range word count (drift in parens where nonzero)

1. **Baseline derivation** (flat `if/else if/else`, `baseValue`/`outA2` etc.
   all `s32`, `Tbl32E978` sized 14 bytes, `threshold` computed after the
   3 key checks): **1/138**, gross structural mismatch (275286-byte
   drift) -- `threshold`'s LATE placement shifted the whole computation to
   a different program point than retail entirely.
2. **Moved `threshold = a4 - 0x40;` before the three key checks** (matching
   retail's early, unconditional computation of it): **38/138** (226697
   drift) -- fixed the `Tbl32E978` stride bug's symptom became visible here
   too, see next.
3. **Fixed `Tbl32E978`'s stride** (added `padE[0x20-0xE]` so `sizeof==0x20`
   matching the observed `sll #5`, not the wrong 14-byte size that produced
   `sll #1`): jumped straight to **51/138** (287573 drift, WORSE numeric
   drift but the STRUCTURAL shape corrected -- see next axis for why the
   drift count alone is not the right read here).
4. **Reordered `q`/`outA2`/`r`/`outA1` inside each division branch**
   (compute the ADD into `outA2` immediately after `q`, before computing
   `r`, matching retail's instruction order exactly): **50/138** (287573
   drift) -- essentially unchanged; this specific reorder did not matter on
   its own.
5. **Restructured the three-way `threshold` dispatch from a flat
   `if/else if/else` into a nested `if (>0) {...} else { outA2 = baseValue;
   if (<0) {...} else {outA1=0;} }`** (matching retail's ACTUAL branch
   structure: a `blez` test, then inside its else-arm a SECOND `bgez` test
   whose delay slot unconditionally sets `outA2 = baseValue` regardless of
   which of the two remaining paths is taken): **50/138** (287573 drift,
   unchanged) -- correct in isolation but masked by axis 6's defect.
6. **Retyped `baseValue` from `s32` to `u16`**: **77/138, DRIFT DROPS TO
   ZERO.** This was the single largest and most important lever. `baseValue`
   is loaded via `lhu` (an unsigned 16-bit value, 0..65535) and used again
   later (read a second time in the negative-threshold branch) -- declaring
   it `s32` let the register allocator coalesce it with `outA2` in a way
   that skipped the explicit `addu $a2,$a1,$zero` copy retail's disassembly
   shows; declaring it its true narrow unsigned width forced the genuine
   copy to reappear, closing the drift entirely and moving the score from
   50 to 77 in one change.
7. **Retyped `ApplyVoicePitchBend`'s own second parameter from `s32` to `s16`**
   (see the signature section above): **no change (77/138)**, confirmed
   `ApplyPitchBendToAllVoices` unaffected. Kept as the more honest typing regardless.
8. **Retyped `outA2`/`outA1` from `s32` to `u16`**: **WORSE, 67/138 with
   280490-byte drift reappearing.** Reverted.
9. **Hoisted the `outA2 & 0xFFFF` call-argument mask into an explicit named
   temporary computed earlier** (to try to match retail's asymmetric
   scheduling, where the FIRST call argument's mask is computed early but
   the SECOND is deferred into the `jal`'s delay slot): **no change
   (77/138)**.
10. **Reordered the `someTotal`/`baseValue`/`threshold`/`outA2`/etc. local
    DECLARATIONS** (several permutations): **no change (77/138)** in every
    ordering tried -- consistent with this project's standing finding that
    declaration order alone does not move register allocation.
11. **A bare `__asm__("")` scheduling barrier** immediately before the
    `func_8002E038` call statement: **80/138 in-range, but reintroduced
    280490 bytes of drift** -- the barrier's ordering constraint forced the
    compiler to emit at least one genuinely different (extra) instruction
    elsewhere, which is disqualifying (a barrier that changes code SIZE is
    not the permitted "instruction order only" case CLAUDE.md allows).
    Reverted.

## What's left, precisely

Two independent problems, both confirmed still present at the 77/138
plateau:

- **A near-total register RENUMBERING, starting at the function's SECOND
  instruction and touching almost every register-bearing line thereafter**:
  retail's `$t2`/`$a1` in several spots read as `$t1`/`$a0` in the built
  code (and vice versa), consistently offset by one throughout entire
  register CLASSES (both the `$t`-temporaries and the `$a`-argument
  registers), while every VALUE and every CONTROL-FLOW EDGE is identical.
  This is the textbook "same instructions, different registers" shape
  CLAUDE.md's residue list already names -- explicitly NOT fixable with a
  `register T v asm("$N")` pin or an operand constraint (that IS the banned
  fix), and no combination of type or statement-order reshaping tried this
  round moved it even slightly, which is itself informative: the two
  register CLASSES shift together and from the very first instruction,
  before ANY of this function's own locals have been computed, suggesting
  whatever determines it is a whole-function register-count decision made
  before RTL for the body is even reached -- plausibly the callee's own
  overall register demand (this function ties up two dedicated `$t`-class
  registers across its whole body: one for the record index `a0`, one for
  `threshold`), where retail's real source needs a caller-visible register
  we have not identified.
- **One genuine extra instruction**, isolated to a single spot: the SECOND
  call argument's `& 0xFFFF` mask (`outA1`) is computed as an ordinary
  instruction ahead of the `jal`, where retail defers the identical
  operation into the `jal`'s own delay slot. Every other instruction
  around it is otherwise identical. `asm-differ`, run with `--first-diff`-
  style output, shows this as the ONLY point where the word COUNT itself
  differs (all other differences in the function are 1-for-1 register
  swaps, not insertions/deletions); the reported 77/138 already reflects
  this, and everything downstream of that one word realigns cleanly
  through to the function's end once this exact spot is passed.

## Preserved body (stalled at 77/138, exact length, zero drift)

```c
#if 0
/* Same 0x34-stride record family, UNSIGNED 16-bit view -- D_8008D99C
 * needs this width (`lhu`) here, and BOTH this width and a plain byte
 * width (`lbu`, same offset) elsewhere in this same function; the byte
 * view is reached via a plain pointer cast, same idiom as D_8008EA26's
 * mixed sh/lbu access.  D_8008D994 needs the same unsigned re-reading
 * here even though StopNote (above) reads the SAME symbol signed
 * (`lh`) -- reinterpreted through a cast rather than redeclared, since
 * one extern symbol cannot carry two conflicting C types in one file. */
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34U16;
extern Rec34U16 D_8008D99C[];

/* Debug/selected-difficulty byte, read fresh each call. */
extern u8 D_8008EA13;

/* Pointer to a 0x20-byte-stride table; only the two trailing byte
 * fields this function reads are named. */
typedef struct {
    u8 pad[0xC];
    u8 unkC; /* +0xC */
    u8 unkD; /* +0xD */
    u8 padE[0x20 - 0xE];
} Tbl32E978;
extern Tbl32E978 *D_8008E978;

/* Same 0x10-byte-stride record family code_179d8_j.c documents as
 * Rec16D7F0 (that unit's D_8008D7F0/D_8008D7F4 pair); local view. */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x10 - 0x2];
} Rec16D7F4;
extern Rec16D7F4 D_8008D7F4[];

/* Plain byte-stride flags array (no per-record multiply in its own
 * addressing -- unlike every 0x34/0x10-stride array above). */
extern u8 D_8008D970[];

/* Selected-channel debug byte, write-only here. */
extern u8 D_8008EA18;

extern s16 func_8002E038(u16 a0, u16 a1);

s16 ApplyVoicePitchBend(s16 a0, s16 a1, s16 a2, s16 a3, u16 a4) {
    s16 threshold;
    u16 someTotal;
    u16 baseValue;
    s32 outA2;
    s32 outA1;
    s32 product;
    s32 q;
    s32 r;
    u8 tableByte;
    u8 byteVal;

    threshold = a4 - 0x40;
    if (D_8008D996[a0].unk0 != a1) {
        return 0;
    }
    if (D_8008D99E[a0].unk0 != a2) {
        return 0;
    }
    if (D_8008D99A[a0].unk0 != a3) {
        return 0;
    }

    someTotal = D_8008D99C[a0].unk0 + (D_8008EA13 << 4);
    baseValue = ((Rec34U16 *) D_8008D994)[a0].unk0;

    if (threshold > 0) {
        tableByte = D_8008E978[someTotal].unkD;
        product = threshold * tableByte;
        q = product / 63;
        outA2 = baseValue + q;
        r = product % 63;
        outA1 = r * 2;
    } else {
        outA2 = baseValue;
        if (threshold < 0) {
            tableByte = D_8008E978[someTotal].unkC;
            product = threshold * tableByte;
            q = product / 64;
            outA2 = baseValue + q - 1;
            r = product % 64;
            outA1 = r * 2 + 0x7F;
        } else {
            outA1 = 0;
        }
    }

    byteVal = *(u8 *) &D_8008D99C[a0].unk0;
    D_8008EA26 = a0;
    D_8008EA18 = byteVal;
    D_8008D7F4[a0].unk0 = func_8002E038(outA2 & 0xFFFF, outA1 & 0xFFFF);
    D_8008D970[a0] |= 4;
    return 1;
}
#endif
```

## Round 32 (bravo) update: reconfirmed, no new axis found

Re-spliced this exact preserved body and rebuilt from scratch (routing around
duplicate-declaration conflicts by reusing the ALREADY-declared `Rec34U16`/
`Tbl32E978`/etc. types this unit's later functions define, rather than
redeclaring — this unit's own functions appear in ROM order after this one's
declaration site, so the existing types were directly reusable this time,
unlike `StepVoiceEnvelope`, which sits FIRST in the file and needs its own local
copies). **All three title figures reconfirmed exactly:** `funcdiff.py`
reports **77/138 words match, file 0x1FBE8-0x1FE10** (zero out-of-range
drift, genuinely exact length), first real diff still the function's second
instruction (`move t2,a0` retail vs `move t1,a0` built).

Read the raw disassembly directly this round (rather than re-deriving from
the report's prose) to check whether the persisted-register class this
unit's OTHER two large stalls (`StepVoiceFade`, `StepVoiceEnvelope`) show is the
same mechanism here. It is not quite the same shape: this function's
register renumbering starts at instruction 2 and is a near-total CLASS
shift (both `$t`-temporaries and `$a`-arguments renumbered together) rather
than one dedicated persisted value computed early and consumed once late.
No new lever identified this round; not spending further attempts here
given `StepVoiceFade` and `StepVoiceEnvelope` (this unit's two other open
stalls) each had a specific, named, untried lever this round, and both of
those also failed to move when tried (see their own round-32 updates) —
consistent with all three of this unit's remaining stalls being the same
underlying "whole-function register-count decision predates any of this
function's own locals" class CLAUDE.md already treats as banned-to-fix.

### Proposed learning

**A struct's stride (element size) computed from an OBSERVED shift amount
is a hard constraint, not a rounding choice: getting it wrong doesn't just
mis-scale one access, it changes the SHAPE of the multiply itself** (a
power-of-two stride compiles to one `sll`; a non-power-of-two stride needs
the `sll`/`addu`/`sll` sequence this project already knows from the
0x34-stride family). Padding a struct to the wrong SIZE showed up here as
the single biggest false lead of the whole attempt: 14 bytes (the sum of
the NAMED fields with no trailing pad) versus the real 32-byte stride
produced a visibly different multiply instruction (`sll #1` vs `sll #5`),
not just a wrong constant -- which made it easy to spot once diffed, but
would have been easy to miss if the two shapes had coincidentally produced
outputs of the same instruction COUNT. Always cross-check a derived
stride's shift amount against the ACTUAL disassembly line, not just against
"whatever offsets I've named so far plus one".

**A local that gets read a SECOND time in only ONE of several branches
needs its true narrow type, or the register allocator may coalesce it with
its own destination and silently drop a real copy instruction retail
has.** `baseValue` (`s32`, read once, used in all three branches, and read
a SECOND time nowhere -- but ITS DESTINATION VARIABLE, `outA2`, gets
reassigned from it differently per branch and `baseValue` needs to survive
into the negative branch for a second use `baseValue + q - 1`) needed its
true `u16` (unsigned, 0..65535) type before the compiler stopped optimizing
away retail's real `addu $a2,$a1,$zero` copy. This is a second, DISTINCT
confirmation of the already-documented "declared width is a codegen
decision" lever, but the SPECIFIC symptom here (a missing COPY instruction
across a branch, rather than a missing re-sign-extension inside a loop) is
new and worth naming on its own.

## Round 37 (bravo) update: rebuilt (confirmed accurate), permuter searched for the first time

Round 37's designated permuter-priority item 1 -- this function has
**exact length with a mid-range word-match**, the best-posed permuter
shape in this unit's queue (no drift to void the score), and is part of
this round's overall thesis: this unit's five stalls are the single
largest never-permuter-searched block in the corpus (73/111 near-misses
project-wide already have a search logged; none of this unit's five did
before this round). Per the round's "build the inherited body before you
trust its score" instruction, this exact preserved body was re-spliced
into the live unit and rebuilt from scratch.

**All three title figures reconfirmed exactly:** `funcdiff.py` reports
**77/138 words match, file 0x1FBE8-0x1FE10** (zero out-of-range drift,
genuinely exact length), first real diff still the function's second
instruction (`move t2,a0` retail vs `move t1,a0` built).

### Permuter search

`tools/setup-permuter.sh ApplyVoicePitchBend <seed>`, seed built from this
report's preserved body (all needed struct/extern declarations already
present in the live unit above this function's position, so no renamed
local types were needed this time -- this function is not first in the
file). `--debug --stack-diffs` base score: **770** (Register Differences
50 x weight 5 = 250, Reorderings 2 x 60 = 120, Insertions 2 x 100 = 200,
Deletions 2 x 100 = 200; zero Stack and zero Branch differences) --
consistent with this report's own classification of the residue as BOTH a
near-total register-class renumbering AND one genuine extra/missing
instruction (the deferred `& 0xFFFF` mask), not a pure register-identity
case.

Real search: `permuter.py -j 6 --stop-on-zero --best-only --stack-diffs`,
bounded `timeout 900` (15 minutes), run in the background while hand work
proceeded on the unit's other four functions.

**Result: not closed in 71,309+ iterations; exit status not reliably
captured (see the process-lifetime finding below).** The search ran with
`-j 6 --stop-on-zero --best-only --stack-diffs` under a `timeout 900`
bound. The best score reached was **485** (from base 770), saved at
`permuter-work/ApplyVoicePitchBend/output-485-1/`; no candidate ever reached
zero across the whole run. Diffing that candidate against `base.c` shows
the improvement came from dropping the `& 0xFFFF` mask on the SECOND
`func_8002E038` call argument (relying on the value's already-narrow
range) plus some statement-grouping changes (a `do {...} while(0)` around
part of the positive-`threshold` branch, a merged
`byteVal = (D_8008EA18 = ...)` assignment-chain) -- none of which, on
inspection, is a translatable idiomatic C change: the dropped mask is
exactly the ALREADY-diagnosed "genuine extra instruction" residue this
report names, but removing the mask outright is a semantic change (relies
on this call's actual argument range, not proven equivalent in general),
and the grouping mutations are permuter-internal restructurings with no
natural source-level phrasing. **Confirms rather than closes the report's
own two-part diagnosis**: the register-class renumbering plateau (worth
250 of the 770 base score alone) never moved in ~71k iterations, consistent
with CLAUDE.md's own classification of this residue as a banned-to-fix
register-identity case.

**Process-lifetime finding, worth recording as a lesson for future
runners**: this search was launched with a hand-rolled
`(cmd; echo "permuter rc=$?" >> log) &` backgrounded from a synchronous
Bash tool call, on the theory that the trailing `&` alone was sufficient
to detach it. The search's own iteration counter and per-candidate output
directories (`output-*-1/`) kept growing and updating for the full
duration and were retrievable after the fact, so the SEARCH itself ran to
its natural `timeout 900` bound -- but the trailing `echo "permuter
rc=$?"` marker never appeared in the log, and by the time this was
checked the process had already exited (not present in `ps` under its
PID). The most likely explanation is that the wrapping subshell was
reaped when its parent shell session ended, before the trailing `echo`
could run, even though the `timeout`-wrapped child had already completed
on its own. **The lesson: prefer the Bash tool's own
`run_in_background: true` for a bounded permuter search rather than a
hand-rolled `cmd & ; echo rc=$?` -- it is designed to survive across tool
calls and reports the exit code through the harness itself, where a
manually-backgrounded chain's trailing marker command is not guaranteed
to run.** The search's own DATA (iteration count, best score, saved
candidates) survived regardless, so this is a bookkeeping gap, not a lost
result -- but the exact `rc` (124 vs. some other value) could not be
established with certainty for this one search, and per this round's own
instruction that ambiguity is recorded here rather than papered over with
a guessed value.

### Proposed learning

**A permuter base score in the 700s with 2 insertions AND 2 deletions (not
just register differences) is diagnostic on its own, before any search
runs**: it confirms a report's own "two independent problems" read (a
register-class shift AND a missing/extra instruction) against an
independent scorer, rather than relying solely on `asm-differ`'s prose
description. Worth checking the debug breakdown's Insertions/Deletions
counts against a report's own claimed residue shape before spending search
time -- a mismatch there would mean the seed is not scoring the residue
the report thinks it is.

## Naming

**ApplyVoicePitchBend** (was `func_8002F3E8`) -- Tier A. Body fully
evident (preserved below, exact length): matches a voice by its
(track/note/program) identity fields, then computes a curve-table-driven
bend from a 0-127 depth value centered at `0x40` (`threshold = a4 - 0x40`,
looked up in `D_8008E978`'s `bendCurveUp`/`bendCurveDown` fields depending
on sign) applied to a base value, writes the result through
`func_8002E038`, and returns `1` on match / `0` otherwise -- the return
value is what `ApplyPitchBendToAllVoices` sums into its own return.
"PitchBend" rather than a vaguer name because the `a4-0x40`-centered,
signed-threshold, curve-table shape is the textbook MIDI pitch-bend-depth
computation, not merely "some per-voice adjustment".

## Head review, round 62: the tier-A name is corroborated by the callee

The naming pass rested "PitchBend" on this function's own shape (a 0-127
depth centred at `0x40`, signed into `bendCurveUp`/`bendCurveDown`). That
reading is right, and there is a second, independent line of evidence the
pass did not cite: the callee it writes its result through,
`func_8002E038` (`src/code_179d8_l.c:183`), is a note-to-pitch converter on
its face. It computes `origA0 + 0x3C - e->unk4`, divides the result by 12,
indexes a table 16 entries per semitone, and finishes with a shift by
`q12 - 5`. `0x3C` is 60, MIDI middle C; 12 is semitones per octave; the
trailing shift is octave scaling of an SPU sample-rate value. So the value
this function bends is a PITCH, established by the arithmetic of the
function that produces it, not only by the MIDI-shaped depth encoding here.

Two independent kinds of evidence, so tier A stands. `func_8002E038` is
itself a naming candidate (a note-to-SPU-pitch converter) for whoever takes
`code_179d8_l`; it was correctly left alone this round as out of unit.

## NON_MATCHING body promoted, round 67

Placed in `src/code_179d8_m.c` under `#ifdef NON_MATCHING`, `INCLUDE_ASM`
kept in `#else`. Field references updated from the report's original
`unkC`/`unkD` to the unit's current `bendCurveUp`/`bendCurveDown` names
(mapped by OFFSET, not by the field's "Up"/"Down" label: `unkD` (+0xD, used
for `threshold > 0`) is `bendCurveDown`, and `unkC` (+0xC, used for
`threshold < 0`) is `bendCurveUp` -- the struct's own inline comments claim
the opposite polarity, which looks like a naming-pass error worth a
separate look, not something this promotion should silently paper over).
`./build-and-verify.sh` green (zero bytes changed) and
`tools/check-nonmatching.sh code_179d8_m` green.
