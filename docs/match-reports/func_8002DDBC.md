# func_8002DDBC — STALL (round 33 update: 98/112 -> 108/112, 4 words short, down from 14 — see below)

**14 words short** (best derivation compiled to 98 words against retail's
112); raw word-match is not meaningful under that drift (funcdiff reports
2/112 windowed, which is drift ripple, not real residue count — see
`docs/DECOMPILATION_LEARNINGS.md`'s "one instruction short, everything after
it shifted" class, except here the gap is larger and has more than one
cause). **First real diff is at the very first instruction**, vram
`0x8002DDBC` / file `0x1E5BC`: retail's `move $a3,$a0` has no counterpart at
all in the derived body — confirmed with `tools/asm-differ/diff.py
func_8002DDBC`, not inferred.

`code_179d8_l`, vram `0x8002DDBC`, file offset `0x1E5BC`, 112 words
(0x1C0 bytes). No `nop_mflo_mfhi` or `gp_rel` hits — this is a "clean"
function per the carve census; the residue is purely codegen-shape, not a
toolchain blocker.

## What it computes

SPU voice-channel setup: `D_8006DAD4` holds `0x1F801C00`, the PS1 SPU base
hardware address (confirmed by reading `asm/data/57070.data.s`), and the
function writes to `+0x194`/`+0x196` from it — the SPU key-on-low/key-on-high
registers. `a0` is the voice-channel number (0-31, masked to a byte), `a1`
and `a2` are per-voice values written into two 16-byte-stride parameter
tables (`D_8008D7F0`, `D_8008D7F2`); `D_8008D970[chan]` is a per-voice flag
byte OR'd with `3`; a loop over all `D_8008E9D0` (a voice count) entries of a
52-byte-stride array at `D_8008D9A3` masks each entry's low bit; the target
channel's own 52-byte-stride slot gets three fields set (`D_8008D98C+2`←10
[before the loop, if the earlier `sll v0,a0,4` load offset arithmetic is
read as `D_8008D98C` base itself rather than a sub-field — not fully
resolved, see below], `D_8008D9A3+0`←2, `D_8008D98A+0`←0); and finally the
computed `lowBit`/`highBit` (a 32-bit voice-enable mask split across two
16-bit halves by whether `chan<16`) get OR'd into `D_8008E228`/`D_8008E22C`
and AND-NOT'd into `D_80090C60`/`D_80090C64` (an enable/mute pair), then
written to the two SPU key-on registers.

## Best-derived body (98/112 words, preserved for the next attempt)

```c
extern u8 D_8008D7F0[];
extern u8 D_8008D7F2[];
extern u8 D_8008D970[];
extern u8 D_8008E9D0;

extern u8 D_8008D98A[];
extern u8 D_8008D98C[];
extern u8 D_8008D9A3[];

extern u16 D_8008E228;
extern u16 D_8008E22C;
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 *D_8006DAD4;

void func_8002DDBC(s32 a0, s32 a1, s32 a2) {
    s32 a3;
    s32 off16;
    s32 v1;
    s32 lowBit;
    s32 highBit;
    s32 idx52;
    s32 li;
    s32 i;
    u16 e228;
    u16 e22c;
    u16 c60;
    u16 c64;

    a3 = a0;
    a0 = (u8)a0;
    off16 = a0 << 4;
    *(u16 *)(D_8008D7F2 + off16) = a2;
    __asm__("");
    v1 = D_8008D970[a0];
    *(u16 *)(D_8008D7F0 + off16) = a1;
    v1 |= 3;
    D_8008D970[a0] = v1;
    if (a0 < 16) {
        lowBit = 1 << a0;
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (a0 - 16);
    }

    idx52 = (u8)a3 * 52;
    *(u16 *)(D_8008D98C + idx52) = 10;
    if (D_8008E9D0 != 0) {
        i = 0;
        do {
            li = i * 52;
            *(u8 *)(D_8008D9A3 + li) = *(u8 *)(D_8008D9A3 + li) & 1;
            i++;
        } while ((u16)i < D_8008E9D0);
    }
    idx52 = (u8)a3 * 52;
    *(u8 *)(D_8008D9A3 + idx52) = 2;

    e228 = D_8008E228;
    e22c = D_8008E22C;
    *(u16 *)(D_8008D98A + idx52) = 0;
    c60 = D_80090C60;
    e228 = lowBit | e228;
    e22c = highBit | e22c;
    D_8008E228 = e228;
    c60 = c60 & ~e228;
    D_8008E22C = e22c;
    c64 = D_80090C64;
    D_80090C60 = c60;
    c64 = c64 & ~e22c;
    D_80090C64 = c64;
    D_8006DAD4[0xCA] = lowBit;
    D_8006DAD4[0xCB] = highBit;
}
```

## Residues, in the order they'd need fixing

1. **A missing `move $a3,$a0` and a genuine register-identity swap in the
   whole first block.** Retail preserves the original unmasked `a0` in
   `$a3` and computes the masked channel INTO `$a0` (in place); the compiled
   body above does the reverse (`$a3` ends up holding the masked value,
   `$a0` the original), even when the C is written to mirror retail's
   exact roles (`a3 = a0; a0 = (u8)a0;`, i.e. explicitly mutating the
   parameter register in place). **Variable naming in the C source does not
   control which physical register the allocator picks** — this was tested
   directly: swapping which C identifier holds which value changes nothing
   about the compiled register assignment. This is the class CLAUDE.md
   calls out as unfixable-from-C ("if reshaping does not move it, it is a
   stall") and it is NOT the `register T v asm("$N")` situation (that
   remains banned regardless).
2. **A scheduling reorder, already found and fixed once already.** In
   isolation, GCC hoists the `D_8008D970[chan]` read ahead of BOTH the
   `D_8008D7F2`/`D_8008D7F0` stores it sits between in source, defeating the
   `chan<<4` register reuse retail shows (one `sll` shared by both stride-16
   stores). A bare `__asm__("")` placed right after the first store restores
   source order and the shared shift — confirmed working in isolation and
   applied in the body above. **Proposed learning**: an unrelated LOAD
   between two STORES to different arrays can get hoisted past the first
   store by GCC 2.6.3's scheduler even with no aliasing justification; a
   scheduling barrier is the documented-permitted fix (order-only, not
   register-identity) per CLAUDE.md's test.
3. **A 14-word length gap, isolated to the `D_8008E9D0`-bounded loop over
   `D_8008D9A3`.** `tools/asm-differ/diff.py` shows retail recomputing the
   full 5-instruction `i*52` shift-add sequence (`sll,addu,sll,addu,sll`)
   on EVERY iteration, while the natural `do { li = i*52; ...; i++; }
   while(...)` loop in the body above gets **loop-strength-reduced** by
   GCC into a running accumulator (`+= 52` per iteration) — fewer
   instructions than retail. This is the same "compiler proves a cheaper
   equivalent" family as the `func_8002E038` `sra`→`srl` lever (round 24),
   but on loop induction rather than a division; it was not chased further
   here for lack of budget. A LIKELY lever, untried: some property of the
   loop that defeats strength-reduction in retail's original — e.g. the
   accumulator variable escaping the loop as a live value afterward, or an
   extra store inside the loop body that the optimizer treats as aliasing
   the induction expression. Worth trying: add a second read/write inside
   the loop body that plausibly aliases `D_8008D9A3` non-triviially (though
   the source here has none by construction), or try `for` instead of
   `do/while` on the chance it changes which pass runs first.
4. **Frame allocation.** Retail brackets the function in `addiu $sp,$sp,-8`
   / `+8` with no visible spill; the derived body above has none. Given the
   register-identity issue in (1) is already unresolved, it wasn't worth
   chasing whether resolving it would also produce the frame (plausible,
   per the "allocated-but-unused frame reproduces automatically with enough
   live locals" lever — the extra live range needed for a REAL `$a3`
   register might be exactly what pushes it over).

## Attempts

Roughly 15 build/pipeline iterations (within the 30-attempt cap), spanning:
struct-array-typed indexing (reverted in favor of manual pointer arithmetic
to force the shared `chan*16`/`chan*52` register reuse retail shows),
declaration-order permutations for the `a0`/`a3` swap (no effect — see
residue 1), and the scheduling-barrier fix for residue 2 (successful).
Restored to `INCLUDE_ASM` per the hard rule — no score short of byte-exact
may stay in `src/`.

### Proposed learnings

- Variable identifier choice in C has **no** influence on which physical
  register GCC 2.6.3's allocator assigns — only confirmed by direct test
  here, but worth stating explicitly since it is tempting (and wrong) to
  "rename to nudge" a register-identity residue.
- Loop-strength-reduction (multiply-by-constant induction variable replaced
  with repeated addition) is a real GCC 2.6.3 -O2 optimization this project
  will keep running into on any `for`/`while` loop over a stride-N array
  with a small N; when retail does NOT show the reduced form, that is a
  structural tell worth naming (parallel to the sra/srl and copy-elision
  levers already documented for `func_8002E038`), not yet reduced to a
  known trigger.

## Round 33 update (runner alpha): the loop-strength-reduction lever WAS known — this unit's own `func_8002CF18` report already named the exact fix

Re-verified the inherited 98/112 body first — reproduces exactly (compiled
length 0x188 = 98 words), residues 1 and 2 confirmed at the exact positions
this report already names.

**The "not yet reduced to a known trigger" note above undersold it: this
same unit's `func_8002CF18` report (also round 26) had already identified
the discriminator — a `(u8)`/narrow cast on an array index defeats GCC
2.6.3's strength reduction of `idx*stride` into `idx+=stride`.** The
`do`/`while` loop's index `i` was declared and used as a plain `s32` with
no narrowing cast anywhere in the multiply
(`li = i * 52;`). Changing it to `li = (u8)i * 52;` reproduced retail's
full 5-instruction recompute (`sll`/`addu`/`sll`/`addu`/`sll`) every
iteration and moved the function from **98/112 (14 short) to 108/112 (4
short)** in one change — a 10-word swing, the largest single fix of this
round across all five of this unit's queued functions.

**A second, smaller fix found by reading the realigned diff further:**
with the `(u8)` cast, one instruction differed in MASK WIDTH — retail's
loop-condition/index-use sequence masks with `0xffff` (`andi v1,a1,0xffff`)
at a specific point the `(u8)`-cast version instead masked with `0xff`.
Changing the cast in the multiply from `(u8)i` to `(u16)i` (the loop bound
comparison `while ((u16)i < D_8008E9D0)` already used `u16`; the multiply
itself had not) reproduced the `0xffff` mask exactly, with the compiled
length unchanged (still 108/112 — a byte-correctness fix, not a
word-count one). **Lesson: when applying the "narrow-cast defeats strength
reduction" idiom, match the cast WIDTH to what the surrounding code
already uses for the same index, not just any narrower-than-`s32` type —
`(u8)` and `(u16)` both suppress the reduction, but only one of them
reproduces the correct mask instruction.**

**What remains (read off the realigned diff, not inferred) is entirely
downstream of this report's already-documented, already-tried-three-ways
register-identity residue (1):** the opening `move a3,a0` retail has and
the compiled body doesn't (retail preserves the raw channel in `$a3` and
narrows `$a0` in place; the compiled body does the reverse), plus a
missing `addiu sp,sp,-8`/`+8` frame retail allocates (residue 4 from the
original report) and several downstream register-choice differences in
the tail (`D_8008E228`/`D_8008E22C`/`D_80090C60`/`D_80090C64` update
sequence) that read as cascading consequences of the same root register
allocation difference rather than independent residues — the tail's LOAD/
OP/STORE statement order in the preserved C already matches retail's
interleaving one-for-one; only the specific physical registers chosen
differ, which HARD RULE 6 explicitly puts out of reach of C-level
reshaping. Not re-attempted this round, consistent with the original
report's own conclusion after direct testing ("variable identifier choice
in C has no influence on which physical register GCC 2.6.3's allocator
assigns").

**Disposition: restored to `INCLUDE_ASM`.** This is a real 10-word
improvement banked for the next attempt; the remaining 4-word gap is
believed entirely attributable to the pre-existing register-identity
stall, not a new independent residue.

```c
extern u8 D_8008D7F0[];
extern u8 D_8008D7F2[];
extern u8 D_8008D970[];
extern u8 D_8008E9D0;

extern u8 D_8008D98A[];
extern u8 D_8008D98C[];
extern u8 D_8008D9A3[];

extern u16 D_8008E228;
extern u16 D_8008E22C;
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 *D_8006DAD4;

void func_8002DDBC(s32 a0, s32 a1, s32 a2) {
    s32 a3;
    s32 off16;
    s32 v1;
    s32 lowBit;
    s32 highBit;
    s32 idx52;
    s32 li;
    s32 i;
    u16 e228;
    u16 e22c;
    u16 c60;
    u16 c64;

    a3 = a0;
    a0 = (u8)a0;
    off16 = a0 << 4;
    *(u16 *)(D_8008D7F2 + off16) = a2;
    __asm__("");
    v1 = D_8008D970[a0];
    *(u16 *)(D_8008D7F0 + off16) = a1;
    v1 |= 3;
    D_8008D970[a0] = v1;
    if (a0 < 16) {
        lowBit = 1 << a0;
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (a0 - 16);
    }

    idx52 = (u8)a3 * 52;
    *(u16 *)(D_8008D98C + idx52) = 10;
    if (D_8008E9D0 != 0) {
        i = 0;
        do {
            li = (u16)i * 52;
            *(u8 *)(D_8008D9A3 + li) = *(u8 *)(D_8008D9A3 + li) & 1;
            i++;
        } while ((u16)i < D_8008E9D0);
    }
    idx52 = (u8)a3 * 52;
    *(u8 *)(D_8008D9A3 + idx52) = 2;

    e228 = D_8008E228;
    e22c = D_8008E22C;
    *(u16 *)(D_8008D98A + idx52) = 0;
    c60 = D_80090C60;
    e228 = lowBit | e228;
    e22c = highBit | e22c;
    D_8008E228 = e228;
    c60 = c60 & ~e228;
    D_8008E22C = e22c;
    c64 = D_80090C64;
    D_80090C60 = c60;
    c64 = c64 & ~e22c;
    D_80090C64 = c64;
    D_8006DAD4[0xCA] = lowBit;
    D_8006DAD4[0xCB] = highBit;
}
```

### Proposed learning (round 33)

**The "narrow-cast defeats loop strength reduction" idiom
(`func_8002CF18`'s report) is unit-wide, not local to the function that
found it** — it closed 10 of 14 missing words here, in a different
function, the very next round. Any function in `code_179d8_l`/`_m` with a
"my loop is N words short and retail recomputes a multiply I don't" shape
should try this FIRST, before barriers or permuter time, and should match
the cast width to whatever the surrounding comparison already uses for
the same variable rather than picking the narrowest type that compiles.

## Round 35 update (runner bravo): inherited body re-verified real; frame-allocation lever tried, negative; confirmed the remaining gap is entirely the register-identity residue

Re-verified the inherited 108/112 body first: `objdump -t` on
`build/src/code_179d8_l.c.o` confirms `func_8002DDBC` compiles to `0x1b0`
bytes = 108 words, matching round 33's figure exactly. Realigned
`asm-differ` diff confirms residues 1 (the `a3`/`a0` register swap) and 4
(the missing `addiu sp,sp,-8`/`+8` frame) at the exact positions this
report already names.

**Read the raw `.s` directly to characterize the frame precisely, since the
original report only described it as "no visible spill":** the
`addiu $sp,$sp,-0x8` sits in the DELAY SLOT of the function's first branch
(`beqz $v0,.L8002DE28`), which means it executes unconditionally
regardless of which arm is taken — it is not a per-branch allocation, it
is the function's one and only frame adjustment, just scheduled into a
delay slot instead of a leading prologue. There is no `sw`/`lw` to any
`$sp`-relative address anywhere in the whole function — the 8 bytes are
allocated and freed (`addiu $sp,$sp,0x8` right before the trailing
`jr $ra`) without ever being touched. This confirms residue 4 is a pure
"GCC decided this function's estimated register pressure needs a stack
frame" artifact, unrelated to any actual spill.

**Tried: forcing a frame via a dummy local (`s32 dummy;`, unused).**
Theory: if GCC's frame-size heuristic responds to live-range pressure
rather than the register-identity swap itself, adding one more local might
tip it into allocating the frame independent of the swap. Result: **no
change, still 108/112** — GCC 2.6.3 dead-code-eliminates an unused local
entirely at `-O2` before the register allocator ever sees it, so it can't
influence frame-size decisions. Confirms this specific lever needs a local
that is actually LIVE across some span, not merely declared; not attempted
further since any live dummy would need a plausible reason to exist in the
transcription (this project's C is meant to read as an honest
reconstruction of what the retail source did, not an obfuscated lever), and
the original report's own conclusion — that the frame is downstream of the
register-identity residue, not an independent lever — still holds. Reverted
(`git diff --stat` clean against the pre-session state).

**Disposition: unchanged at 108/112 (4 short).** No new lever moved this
function this round. The remaining gap continues to be attributable
entirely to the register-identity swap this report already tried three
ways in round 26 and confirmed unmovable — consistent with HARD RULE 6
("if reshaping does not move it, it is a stall") — plus the frame it likely
drags along once that swap resolves, which is not itself independently
reachable from C.

### Proposed learning (round 35)

**A dummy/unused local cannot be used to nudge GCC 2.6.3's frame-allocation
decision** — the dead-store elimination pass at `-O2` removes it before
register allocation runs, so "add an inert local to change register
pressure" is not a viable lever in this toolchain (as opposed to a
`volatile` local, which IS live but changes the mechanism entirely by
forcing a real memory access — a different, already-documented-elsewhere
trade-off with its own regressions). Worth noting alongside the existing
"variable identifier choice has no influence on register assignment"
learning: neither naming nor adding-and-not-using a local is a technique
that works on this compiler's allocator.

## Round 44 update (runner delta): inherited body re-verified real; first permuter search, negative

Re-verified the inherited 108/112 body first: `objdump -t` on
`build/src/code_179d8_l.c.o` confirms `func_8002DDBC` compiles to `0x1b0`
bytes = 108 words, matching rounds 26/33/35's figure exactly. Realigned
`asm-differ` confirms the same residues (the `a3`/`a0` register swap, the
missing `addiu sp,sp,-8`/`+8` frame) at the exact positions already named.

**First-ever permuter search on this function: bounded to 900s, `-j 6`,
base score 3100 (`--debug --stack-diffs`).** Best score reached: **1800**,
down from 3100, at iteration count in the hundred-thousands range (this
worktree's own single search, no other concurrent search in THIS
worktree this round -- see the process-hygiene note below). Zero
`score = 0` hits.

**The best candidate is a negative result once verified directly.** Its
only structural change from the seed: wrapping the tail's
`e22c = highBit | e22c; D_8008E228 = e228; c60 = c60 & ~e228;` triple in a
`do { ... } while (0);` block. Spliced into the real unit and rebuilt:
**still 108/112 (0x1b0), no length change** -- the isolated scaffold's
score improvement (3100 -> 1800) did not translate, consistent with this
unit's other three same-round instances of this exact divergence
(`func_8002CF18`, `func_8002E138`, `func_8002D6A4`, all round 37) EXCEPT
for `func_8002CD08` this round, where the identical construct DID
translate -- confirming again that a `do-while(0)` candidate must be
verified by direct rebuild every time, in both directions, never assumed
from the isolated score alone.

**Process-hygiene note, for the record:** this search and `func_8002CD08`'s
were launched concurrently (both running for roughly 12 minutes) before
the "one search at a time" rule was caught and corrected; flagged in the
round's broadcast. Neither search's result is believed compromised by the
overlap (both ran to their own bound independently), but it is noted here
in case a future round needs to discount timing-sensitive iteration counts.

**Disposition: unchanged at 108/112 (4 short).** Consistent with rounds
26/33/35's conclusion: the remaining gap is the register-identity swap
this report already tried multiple ways and confirmed unmovable from C,
now also confirmed immune to a bounded permuter search. Restored to
`INCLUDE_ASM` (no change from the pre-session state).
