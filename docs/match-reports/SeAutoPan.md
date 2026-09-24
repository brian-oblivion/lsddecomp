# SeAutoPan -- MATCHED (116/116 words)

> Renamed from `BeginVoiceFade` on 2026-09-24 (tools/rename.py). Address 0x8002e874.

> Renamed from `func_8002E874` on 2026-09-20 (tools/rename.py). Address 0x8002e874.

Unit: `src/code_179d8_m.c`. Round 24, runner bravo.

## Result

Byte-exact. `./build-and-verify.sh` green (`build exit=0`, whole-image SHA1
matches). `funcdiff.py`: `116/116 words match (file 0x1F074-0x1F244)`, no
out-of-range drift.

## Final C

```c
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34Half;
extern Rec34Half gVoiceFadeActive[]; /* "interpolating" flag */
extern Rec34Half gVoiceFadeStep[]; /* "interpolating" flag (companion pair) */
extern Rec34Half gVoiceFadeInterval[]; /* step/quotient */
extern Rec34Half gVoiceFadeCountdown[]; /* step/quotient (companion pair) */
extern Rec34Half gVoiceFadeAccum[]; /* saved start value */
extern Rec34Half gVoiceFadeLimit[]; /* saved end value */

void SeAutoPan(s16 a0, s16 a1, s16 a2, s16 a3) {
    s16 q;

    if (a1 == a2) {
        return;
    }
    gVoiceFadeActive[a0].unk0 = 1;
    gVoiceFadeAccum[a0].unk0 = a1;
    gVoiceFadeLimit[a0].unk0 = a2;
    if ((a1 - a2 < 0 ? a2 - a1 : a1 - a2) < a3) {
        q = a3 / (a1 - a2);
        gVoiceFadeStep[a0].unk0 = 1;
        gVoiceFadeInterval[a0].unk0 = q;
        gVoiceFadeCountdown[a0].unk0 = q;
    } else {
        q = (a1 - a2) / a3;
        gVoiceFadeInterval[a0].unk0 = 0;
        gVoiceFadeStep[a0].unk0 = q;
    }
}
```

## Sibling note (see the unit header and the round prompt)

This function IS the "near-identical sibling of `SeAutoVol` in
`code_179d8_l`" the unit header calls out: same prologue shape (`addu
t3,a0,zero` / `addu t0,a1,zero` / `addu t1,a2,zero`), same `s16`
argument-narrowing (`sll #16`/`sra #16` on `a1`, `a2` before the compare),
same early-out branch comparing the two narrowed arguments (`beq a1,a2,
RET`). Confirmed by hand, matching the header's verdict: **this is register
pressure from four `s16` parameters kept live across a `div`-heavy body,
not a BIOS trampoline** -- there is no `jr $t2` anywhere in this function,
and the body is ordinary arithmetic (two variable divisions with the
standard div-by-zero/`INT_MIN`-overflow `break` guards), not a jump
through a table. `code_179d8_l`'s runner (charlie, this round) is deriving
the same opening shape from the sibling side; this confirms it.

`gVoiceFadeActive`..`gVoiceFadeLimit` are six 2-bytes-apart symbols in the SAME
0x34-byte-stride channel-configuration record family `code_179d8_j.c`
documents (`Rec34D994`, `Rec34Byte`) -- this unit's own local
`Rec34Half` type (already declared for `ClearNoiseVoices`'s `D_8008D98C`,
hoisted above this function since it is the earlier ROM-order user).

## Shape

Given an index `a0` and three `s16` values `a1` (start), `a2` (end), `a3`
(a rate/threshold): if `a1 == a2` there is nothing to interpolate, return
immediately. Otherwise, UNCONDITIONALLY record the start/end pair and set
an "interpolating" flag (`gVoiceFadeActive[a0]=1`, `gVoiceFadeAccum[a0]=a1`,
`gVoiceFadeLimit[a0]=a2`) -- this happens regardless of which branch below is
taken, a fact the disassembly states plainly but that is easy to
misattribute as "only in the close case" on a first read (see below).
Then branch on whether `abs(a1 - a2) < a3`:

- **"Close" (the gap is smaller than the rate):** compute
  `a3 / (a1 - a2)` (note: the actual signed difference, NOT its absolute
  value -- so this quotient can be negative), and store it into BOTH
  `gVoiceFadeInterval[a0]` and `gVoiceFadeCountdown[a0]`, with `gVoiceFadeStep[a0]` set to the
  constant `1`.
- **"Far" (the gap is at least the rate):** compute `(a1 - a2) / a3`
  (dividend and divisor swapped from the close case), store it into
  `gVoiceFadeStep[a0]`, and clear `gVoiceFadeInterval[a0]` to `0`.

Both divisions are variable (non-constant) `s16`/`s16` divisions, which
this compiler expands to the standard MIPS `div` idiom with explicit
divide-by-zero and `INT_MIN / -1` overflow traps (`break 7`, `break 6`) --
ordinary C, no special handling needed.

## Two wrong turns before the right CFG

1. **First attempt: literally duplicated the close/far bodies inside
   BOTH arms of a `diff < 0` / `diff >= 0` outer if, on the theory that
   retail's single shared close-tail and shared far-tail (each reached
   from two different preceding branches) meant the SOURCE had two
   textually-identical bodies that GCC's own cross-jump/tail-merging pass
   would collapse into one.** Wrong on two counts: this compiler
   generation does not reliably cross-jump merge that way at this
   optimization level, AND -- more importantly -- **I had misread which
   instructions were inside the conditional at all.** Building this gave a
   completely different, much-shorter control-flow shape with the
   `gVoiceFadeActive`/`B8`/`BA` writes structured as part of a merged early
   computation the compiler produced from the duplicated bodies, nothing
   like retail. Effectively unusable as a diagnostic (5/116, gross
   structural mismatch, not a near-miss).

2. **Second attempt: introduced an explicit `s16 absdiff = ... ? ... :
   ...;` local and a single `if (absdiff < a3) {...} else {...}`, still
   with the `gVoiceFadeActive`/`B8`/`BA` writes placed INSIDE the close branch.**
   This was based on the same misreading carried over: I had read the
   disassembly's `gVoiceFadeActive`/`B8`/`BA` block as belonging to the "close"
   path because it is followed (many instructions later) by the actual
   close/far branch, without checking whether a `beqz`/`bnez` stood
   between the writes and the entry point. It does not -- **the writes are
   between the early-return `beq` and the FIRST diff computation,
   unconditional.** This attempt scored WORSE (2/116) because it now also
   duplicated the sign check incorrectly (moved the abs-value ternary to
   before the unconditional writes, which retail does not do either).

3. **What worked: re-read the `.s` file instruction-by-instruction instead
   of trusting my own earlier paraphrase**, confirming with the actual
   listing that `gVoiceFadeActive`/`B8`/`BA` sit STRICTLY BETWEEN the `beq
   .L8002EA3C` (early return) and the first `subu` that computes `a1 -
   a2` -- i.e., unconditional -- and that the close/far split is a
   SEPARATE, later `if` using the abs-value ternary. Moving those three
   stores out of both branches, to run unconditionally right after the
   early return, matched first try at this structure.

### Proposed learning

**When a block of stores in the disassembly PRECEDES a branch by many
instructions, re-verify there is no conditional branch anywhere between
the block and the point you believe gates it, by reading the `.s` file's
actual branch/label structure rather than trusting an earlier paraphrase
of it.** A block that looks like it "belongs to" a later branch because it
logically supplies that branch's data (here: recording start/end values
before deciding how to interpolate between them) can be entirely
UNCONDITIONAL, executed on both branches (or before any branch at all). My
own first read of this function moved straight from "these fields look
related to the close case" to "these fields are written only in the close
case" without checking for a branch boundary in between, and that
misreading cost two full-mismatch attempts before a plain re-read of the
listing (not a new derivation, just closer reading) resolved it. The tell
here specifically: `grep -n 'label\|beqz\|bnez\|beq\|bne\|blez\|bgez\|bltz'`
between the point you think gates a block and the block itself -- if it's
empty, the block runs on every path through that point.

## Naming

**SeAutoPan** (was `func_8002E874`) -- Tier B. Mechanics fully evident
from the body: given a voice index and a from/to/rate triple, it
unconditionally records the from/to pair and the "fading" flag into the
`gVoiceFade*` arrays, then computes a per-tick step (and, in the
"close" branch, a throttle interval) so that `StepVoiceFade` can advance
the value later. What in the game actually gets faded this way (a MIDI
CC-driven volume slide? an automatic release curve?) is not established --
only that the destination of the ramp feeds into a stereo-volume
computation (see StepVoiceFade's own report), which is why "Fade" rather
than a more specific term. See the unit header comment in
`src/code_179d8_m.c` for the cross-function picture.
