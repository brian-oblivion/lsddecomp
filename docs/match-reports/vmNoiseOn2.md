# vmNoiseOn2 — STALL (round 65 update: 108 -> 107 built words, now **5 words SHORT** of retail's 112; raw word-match NOT MEASURABLE — the body is length-inexact so funcdiff's fixed 112-word window is drift-contaminated and reports 1/112, which is ripple, not residue; **first real diff still at the very first instruction**, vram 0x8002DDBC / file 0x1E5BC, retail's `move $a3,$a0` having no counterpart — per tools/asm-differ/diff.py, not inferred. The whole 25-instruction tail and the lowBit/highBit registers are now byte-exact; see the round 65 section for the two residues that were NOT the register cascade)

> Renamed from `func_8002DDBC` on 2026-09-24 (tools/rename.py). Address 0x8002ddbc.

**14 words short** (best derivation compiled to 98 words against retail's
112); raw word-match is not meaningful under that drift (funcdiff reports
2/112 windowed, which is drift ripple, not real residue count — see
`docs/DECOMPILATION_LEARNINGS.md`'s "one instruction short, everything after
it shifted" class, except here the gap is larger and has more than one
cause). **First real diff is at the very first instruction**, vram
`0x8002DDBC` / file `0x1E5BC`: retail's `move $a3,$a0` has no counterpart at
all in the derived body — confirmed with `tools/asm-differ/diff.py
vmNoiseOn2`, not inferred.

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

void vmNoiseOn2(s32 a0, s32 a1, s32 a2) {
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
   equivalent" family as the `note2pitch2` `sra`→`srl` lever (round 24),
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
  levers already documented for `note2pitch2`), not yet reduced to a
  known trigger.

## Round 33 update (runner alpha): the loop-strength-reduction lever WAS known — this unit's own `SpuVmAlloc` report already named the exact fix

Re-verified the inherited 98/112 body first — reproduces exactly (compiled
length 0x188 = 98 words), residues 1 and 2 confirmed at the exact positions
this report already names.

**The "not yet reduced to a known trigger" note above undersold it: this
same unit's `SpuVmAlloc` report (also round 26) had already identified
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

void vmNoiseOn2(s32 a0, s32 a1, s32 a2) {
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
(`SpuVmAlloc`'s report) is unit-wide, not local to the function that
found it** — it closed 10 of 14 missing words here, in a different
function, the very next round. Any function in `code_179d8_l`/`_m` with a
"my loop is N words short and retail recomputes a multiply I don't" shape
should try this FIRST, before barriers or permuter time, and should match
the cast width to whatever the surrounding comparison already uses for
the same variable rather than picking the narrowest type that compiles.

## Round 35 update (runner bravo): inherited body re-verified real; frame-allocation lever tried, negative; confirmed the remaining gap is entirely the register-identity residue

Re-verified the inherited 108/112 body first: `objdump -t` on
`build/src/code_179d8_l.c.o` confirms `vmNoiseOn2` compiles to `0x1b0`
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
`build/src/code_179d8_l.c.o` confirms `vmNoiseOn2` compiles to `0x1b0`
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
(`SpuVmAlloc`, `SePitchBend`, `SpuVmDoAllocate`, all round 37) EXCEPT
for `ServiceSoundCueSet` this round, where the identical construct DID
translate -- confirming again that a `do-while(0)` candidate must be
verified by direct rebuild every time, in both directions, never assumed
from the isolated score alone.

**Process-hygiene note, for the record:** this search and `ServiceSoundCueSet`'s
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

## Round 65 update (runner bravo): REVISIT — 108 -> 107 words, the whole tail
## and the lowBit/highBit registers now byte-exact; residue reduced to ONE
## register role swap plus the frame

**REVISITED, round 65: IMPROVED (108 -> 107 built words; the tail's 25
instructions and the `t0`/`a2` lowBit/highBit identity went from scrambled to
byte-exact; asm-differ score 3635 -> 1835); still a STALL; names/types
not relevant.**

### Rebuilt-body measurement, taken BEFORE any change (mandatory bookkeeping)

Inherited round-33/35/44 body spliced back in verbatim and built through the
real oracle:

- `objdump -t build/src/code_179d8_l.c.o` -> `0x1b0` = **108 words**,
  reproducing rounds 33/35/44 exactly. 4 words short of retail's 112.
- `tools/funcdiff.py vmNoiseOn2`:
  `insertions 18 / deletions 18 (opcode-level ...; positional skeleton
  diffs 109)`, `1/112`..`3/112` words match, and a
  `WARNING: the build differs OUTSIDE this range too (241074 bytes)`.
- **That 18/18 and that 109 are NOT a Gate 3 check-3 signature and must not be
  cited as one.** funcdiff's window is retail's fixed 112-word range; a body
  that compiles to 107-108 words puts 4-5 words of the NEXT function inside
  that window, so every figure on the line is drift-contaminated. Check 3 is
  only meaningful on a length-exact body. (Recorded here because the round-65
  brief asks for the line, and the honest reading of it is "unusable", not a
  number.)

### What this revisit actually found: a TYPE residue and a LOCAL-COUNT residue, both hidden under the "register-identity cascade" verdict

Rounds 26/33/35/44 all closed with the same sentence — that everything left
is downstream of the `a3`/`a0` register-identity swap and therefore out of
reach of C. Two of the things attributed to that cascade were not part of it.

**1. `sltiu` vs `slti` — an ordinary signedness residue, unnoticed for four
rounds.** Retail's channel test is `sltiu v0,a0,0x10`; the inherited body
compiled `slti`. `if (a0 < 16)` on an `s32` is a signed compare. Writing
`if ((u32)a0 < 16)` reproduces `sltiu` exactly, length unchanged.

*Why it hid:* asm-differ marks a row `r` (register difference) when ANY
operand differs, and this row already differed in its register operand
(`a0` vs `a3`) because of the real cascade. The opcode difference on the same
row was therefore inside a row every previous reader had already classified.
**Reading a diff by its per-row colour is not the same as reading its
opcodes.**

**2. The four cached `u16` temps in the tail were the whole tail residue.**
The inherited body held `e228`, `e22c`, `c60`, `c64` as locals and wrote the
final global updates through them. Deleting all four and writing the tail as
direct compound expressions on the globals made the **entire 25-instruction
tail byte-exact, register for register** — including the `lui at` / `sh` /
`nor` / `and` interleave that round 33 had described as "cascading
consequences of the same root register allocation difference".

It also fixed register identity **four blocks earlier**: with the temps gone,
`lowBit`/`highBit` moved from `t1`/`t0` to retail's `t0`/`a2`, and
`sllv t0,v0,..` / `move a2,zero` / `sllv a2,v0,v1` became exact. So a
register-identity difference in the middle of a function WAS reachable from
C — by changing the local count at the END of it.

Two sub-levers inside that rewrite, both independently measured:

- **Operand order is observable.** Retail emits `or v1,t0,v1`
  (`lowBit | D_8008E228`). `D_8008E228 |= lowBit;` emits `or v1,v1,t0` —
  same semantics, different word. The matching spelling is
  `D_8008E228 = lowBit | D_8008E228;`.
- **Statement order of the four global updates is the LOAD order, not the
  store order.** Retail loads `E228, E22C, C60, C64`; the inherited body's
  order `E228, C60, E22C, C64` produced exactly that load order and was
  wrong. Reordering to `E228, E22C, C60, C64` closed the rest of the tail.
  (Stores stay in source order in both, so only the loads discriminate.)

**3. A third, smaller one: the `D_8008E9D0` guard read.** Retail loads
`D_8008E9D0` *before* the `D_8008D98C[idx52] = 10` store, which fills the
load-delay slot with the `li v0,0xa`. The inherited body read it inline in
the `if`, so maspsx inserted a `nop`. Reading it into a local first
(`n = D_8008E9D0;` above the store, `if (n != 0)`) removes the `nop` and
reproduces retail's order — this is why the body is 107 words, not 108: it
lost a word it should never have had. The loop's own condition still re-reads
the global each iteration (retail does too), so this is a guard-only hoist.

### What is left, and why it is 5 words

Every remaining difference in the function is now one of two things:

1. **The `a0`/`a3` role swap.** Retail keeps the RAW channel in `$a3`
   (hence the leading `move a3,a0`) and computes the masked channel INTO
   `$a0`; the built body does the reverse. Every `r`-marked row left in the
   diff is this one swap propagating.
2. **The 8-byte frame** (`addiu sp,sp,-8` in the first branch's delay slot,
   `addiu sp,sp,8` before `jr ra`), which drags two more words with it:
   because the frame adjust occupies the `beqz` delay slot, retail cannot
   share one `li v0,0x1` between the two arms and emits it twice; and because
   the `sp` restore sits between the last `sh` and `jr ra`, retail's delay
   slot is a `nop` instead of that `sh`.

That is exactly the 5-word gap: `move` + `addiu sp,-8` + duplicated
`li v0,0x1` + `addiu sp,+8` + trailing `nop`. Nothing else is missing.

### Levers tried this round and their verdicts

| lever | result |
| --- | --- |
| `(u32)` on the `< 16` compare | **POSITIVE** — `sltiu` now exact |
| delete the four tail temps, direct global expressions | **POSITIVE** — whole tail exact, +fixed `t0`/`a2` upstream |
| `lowBit \| D_X` vs `D_X \|= lowBit` | **POSITIVE** — operand order |
| reorder the four global updates to retail's load order | **POSITIVE** |
| hoist the `D_8008E9D0` guard read into a local | **POSITIVE** — drops a spurious `nop` |
| separate masked local (`ch = (u8)a0`) instead of reassigning the parameter | inert — identical 1835, same swap. Re-confirms round 26 on the NEW body |
| masked local typed `u32` / `u16` / `s32` | all three inert on the swap and on the score |
| removing the existing `__asm__("")` barrier | **NEGATIVE** — 1835 -> 2520. The barrier is still load-bearing |
| a SECOND `__asm__("")` at the top, and one before the tail | both inert (1835 unchanged) |
| long live range: cache `D_8006DAD4` in a local from the top | **REGRESSION** — 107 -> 108 words, 1835 -> 2790 |
| `D_8008D98A` store placement sweep, 5 positions | see below |
| arm-order (write the jumping arm NOT-last) | **not applicable** — retail's arm order already matches ours instruction-for-instruction; its `li v0,0x1` duplication is caused by the frame taking the delay slot, not by block layout. This is the delay-slot counter-indication the lever's own entry warns about |
| delete-a-local across a CALL | **not applicable** — this function is a leaf, zero calls, so the live range that lever needs cannot exist here |

**The `D_8008D98A` store placement sweep is worth recording as a trap.**
Five source positions were measured. The position after the `D_80090C60`
statement scores **better** on asm-differ (1675 vs 1835) than the position
kept in the body below — and it is the WRONG one. Retail's store order is
`D98A, E228, E22C, C60, C64`; at 1675 the compiler emits `D98A` *last*, which
contradicts retail, while at 1835 it emits it in the right order merely six
instructions early. **asm-differ's scalar score is an alignment heuristic,
not a structural verdict; when two variants are close, read the memory-op
ORDER, which is a fact about the source.** The body preserved below is the
1835 one, deliberately.

### Attempts

~20 build/oracle iterations, well inside the 30-attempt cap and inside the
"30 consecutive builds with no improvement" stop rule (the last improvement
landed around iteration 12).

### Permuter: DECLINED, with the reason

Gate 3 checks run: **check 1** (would a scaffold compile and score — yes) and
**check 3** (the real-build funcdiff `insertions/deletions` line — see the
bookkeeping section: it is drift-contaminated at 107 vs 112 words and
therefore cannot AGREE or MISMATCH with anything). Check 2 was not run,
because check 3 cannot produce a verdict on a length-inexact body, and
§3.5 makes agreement — not zero-ness — the discriminator.

Round 44's search on the old body is also no longer citable as a negative
about this function (§3.5: "a negative is a verdict about the BODY it was
measured on"), because this round moved the score. But the two things left
are a frame allocation and a hard-register role assignment, neither of which
is a source-text mutation the permuter searches. Declined in favour of
spending the session's one search where it can bite.

### Preserved body (107/112 words, 5 short — rebuild this FIRST next time)

Needs, in addition to the unit's existing declarations before
`SpuVmKeyOnNow` and `vmNoiseOn` (`D_8008D7F0`, `D_8008D7F2`,
`D_8008D970`, `D_8008D98C`, `D_8008D9A3`, `D_8008E228`, `D_8008E22C`,
`D_80090C60`, `D_80090C64`, `D_8008E9D0`, `D_8006DAD4`), one extra:

#if 0
extern u8 D_8008D98A[];

void vmNoiseOn2(s32 a0, s32 a1, s32 a2) {
    s32 a3;
    s32 off16;
    s32 v1;
    s32 lowBit;
    s32 highBit;
    s32 idx52;
    s32 li;
    s32 i;
    s32 n;

    a3 = a0;
    a0 = (u8)a0;
    off16 = a0 << 4;
    *(u16 *)(D_8008D7F2 + off16) = a2;
    __asm__("");
    v1 = D_8008D970[a0];
    *(u16 *)(D_8008D7F0 + off16) = a1;
    v1 |= 3;
    D_8008D970[a0] = v1;
    if ((u32)a0 < 16) {
        lowBit = 1 << a0;
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (a0 - 16);
    }

    idx52 = (u8)a3 * 52;
    n = D_8008E9D0;
    *(u16 *)(D_8008D98C + idx52) = 10;
    if (n != 0) {
        i = 0;
        do {
            li = (u16)i * 52;
            *(u8 *)(D_8008D9A3 + li) = *(u8 *)(D_8008D9A3 + li) & 1;
            i++;
        } while ((u16)i < D_8008E9D0);
    }
    idx52 = (u8)a3 * 52;
    *(u8 *)(D_8008D9A3 + idx52) = 2;

    *(u16 *)(D_8008D98A + idx52) = 0;
    D_8008E228 = lowBit | D_8008E228;
    D_8008E22C = highBit | D_8008E22C;
    D_80090C60 = D_80090C60 & ~D_8008E228;
    D_80090C64 = D_80090C64 & ~D_8008E22C;
    D_8006DAD4[0xCA] = lowBit;
    D_8006DAD4[0xCB] = highBit;
}
#endif

### Proposed learning (round 65)

**A cached local for a read-modify-write of a GLOBAL is a register-identity
lever, and it points the opposite way from the usual instinct.** The natural
transcription of `lhu / or / sh` on a global is `t = G; t = x | t; G = t;`,
because that is what the disassembly literally does. GCC 2.6.3 CSEs the
stored value, so the direct form `G = x | G;` produces the same instructions
— but with retail's register assignment, because the temp is what gave the
allocator an extra pseudo to place. Here deleting four such temps made 25
instructions exact AND fixed two registers four basic blocks upstream.

Discriminator for when to reach for it: **a tail of load/modify/store on
several distinct globals whose instructions are all present and all in the
right order, but whose registers are scrambled.** That shape reads as the
terminal symptom of an upstream register-identity stall, and here it was the
cause of one.

Two riders, both measured here:

- **Operand order survives into the word.** `G = x | G;` and `G |= x;` differ
  by the operand order of the `or`. Check which side retail puts the
  loaded value on before assuming the compound form.
- **The source order of several global updates shows up as the LOAD order,
  not the store order** — the scheduler hoists all the loads above the
  stores, but preserves their relative order. So when the stores are already
  in the right order and the loads are not, the fix is to reorder the
  statements, not to add a barrier.

**And a caution about how the previous four rounds went wrong, which is the
more transferable half.** Every one of them closed by attributing the whole
residue to the `a3`/`a0` register-identity swap. That attribution was
*partly* right — the swap is real and is still unmoved — but it absorbed a
signedness residue and a whole-tail local-count residue that had nothing to
do with it. **A cascade diagnosis is the one verdict that gets stronger every
time it is repeated and weaker every time it is checked**, because it
predicts exactly the thing you see (lots of `r` rows) without predicting
anything you could falsify. When a report says "the rest is cascade", the
cheap test is to read the surviving rows' OPCODES with the registers ignored;
two of the three findings above fall straight out of that.

## NON_MATCHING body promoted, round 69

Promoted the round-65 preserved body into `src/code_179d8_l.c` under `#ifdef NON_MATCHING` (verified build unchanged, keeps `INCLUDE_ASM`); `./build-and-verify.sh` and `tools/check-nonmatching.sh` both green.

## Naming (round 75, runner alpha, FINISHING-PLAN track 3)

Already carries its real name: identified round 74 (track 2, runner bravo)
as `libsnd/vmanager vmNoiseOn2` (shape 0.96 vs the disc-3.3 reference, 103w
reference vs our 112w; libsnd neighborhood, right after `vmNoiseOn` as its
paired helper -- matching this function's own position immediately after
`vmNoiseOn` in `src/code_179d8_l.c`). Sony symbol; this pass does not rename
it further. The function remains a STALL (5 words short, see above).

## Track 2 (round 86, 2026-09-26, alpha)

The per-field symbols this report names (`D_8008D988`..`D_8008D9BA` at a 0x34 stride, and the twelve game names over +0x1C..+0x33 that earlier rounds gave `D_8008D9A4`..`D_8008D9BA`) are ONE Sony table: libsnd/vmanager.o (disc 3.5) bss puts `_svm_voice` at +0x198 of the block anchored at 0x8008D7F0, so `_svm_voice` = 0x8008D988, 24 voices x 0x34 = 0x4E0 bytes, ending exactly at `_svm_envx_ptr`. The symbols file now carries `_svm_voice` (size:0x4E0); the record type is `include/SvmVoice.h` (fields by offset only, Sony's rule). Field map: +0x00 `unk00` (was `D_8008D988`), +0x02 `unk02` (`D_8008D98A`), +0x04 `unk04` (`D_8008D98C`), +0x06 `unk06` (`D_8008D98E`), +0x08 `unk08` (`D_8008D990`), +0x0A `unk0A` (`D_8008D992`), +0x0C `unk0C` (`D_8008D994`), +0x0E `unk0E` (`D_8008D996`), +0x10 `unk10` (`D_8008D998`), +0x12 `unk12` (`D_8008D99A`), +0x14 `unk14` (`D_8008D99C`), +0x16 `unk16` (`D_8008D99E`), +0x18 `unk18` (`D_8008D9A0`), +0x1B `unk1B` (`D_8008D9A3`), +0x1C..+0x26 `unk1C`..`unk26` (the SeAutoVol/SetAutoVol ramp: active, step, interval, countdown, accum, limit; `D_8008D9A4`..`D_8008D9AE`), +0x28..+0x32 `unk28`..`unk32` (the SeAutoPan/SetAutoPan ramp, same order; `D_8008D9B0`..`D_8008D9BA`). Preserved bodies in this report keep the per-address `D_` spellings, which still link (except `D_8008D988`, which is now `_svm_voice` itself) (splat keeps them as auto-symbols, since the table lies past the global segment's vram range and splat does not fold them into `_svm_voice`).

The NON_MATCHING body drops its `idx52`/`li` byte offsets for `_svm_voice[(u8)a3].unk04/unk1B/unk02` and `_svm_voice[(u16)i].unk1B`; compiled length 107 -> 108 of 112, mnemonic ratio vs retail 0.831 -> 0.864. Still a stall.
