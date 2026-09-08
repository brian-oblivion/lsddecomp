# func_8002DDBC — STALL

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
