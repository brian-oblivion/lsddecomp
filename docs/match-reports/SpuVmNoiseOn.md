# SpuVmNoiseOn -- MATCHED (32/32 words)

> Renamed from `PlayFixedSound` on 2026-09-23 (tools/rename.py). Address 0x8002f368.

> Renamed from `func_8002F368` on 2026-09-20 (tools/rename.py). Address 0x8002f368.

Unit: `src/psyq/libsnd_vmanager.c`. Round 24, runner bravo.

## Result

Byte-exact. `./build-and-verify.sh` green (`build exit=0`, whole-image SHA1
matches). `funcdiff.py`: `32/32 words match (file 0x1FB68-0x1FBE8)`, no
out-of-range drift.

## Final C

```c
extern u16 D_8008EA26;
extern u8 spuVmMaxVoice;
extern u8 D_8008EA1B;

extern s32 SpuVmAlloc(s32 a0);
extern void vmNoiseOn2(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4);

void SpuVmNoiseOn(s32 a0, s32 a1) {
    s32 v0;

    D_8008EA1B = 0x7F;
    v0 = SpuVmAlloc(0xFF) & 0xFF;
    D_8008EA26 = v0;
    if (v0 < spuVmMaxVoice) {
        vmNoiseOn2(*(u8 *)&D_8008EA26, a0 & 0xFFFF, a1 & 0xFFFF, 0x80FF, 0x5FC8);
    }
}
```

## Shape

Straight-line leaf, one conditional call:

1. Force flag byte `D_8008EA1B = 0x7F` unconditionally.
2. Call `SpuVmAlloc(0xFF)` (a `libsnd_vmanager`/charlie function, still
   `INCLUDE_ASM` there), mask the result to a byte, store it into
   `D_8008EA26` (a 16-bit store -- the value is always 0..0xFF here, so the
   upper byte written is always 0).
3. Compare that masked value against `spuVmMaxVoice` (loop-bound/threshold byte,
   documented in `libsnd_vmanager.c`). If it is `< spuVmMaxVoice`, call
   `vmNoiseOn2` (also owned by `libsnd_vmanager`) with five arguments: the
   LOW BYTE re-read from `D_8008EA26` (register `$a0`), this function's own
   two arguments narrowed to `u16` (`$a1`, `$a2`), the constant `0x80FF`
   (`$a3`), and the constant `0x5FC8` passed on the stack (5th argument,
   `0x10($sp)`).

`SpuVmAlloc` and `vmNoiseOn2` are declared here as unit-local `extern`
prototypes only (per CLAUDE.md's rule on prototypes for functions another
unit defines) -- they are not added to any shared header.

## The one real snag: `sh`-then-`lbu` on the same symbol

Retail stores `D_8008EA26` with a 16-bit store (`sh`) and later re-reads it
with an 8-bit load (`lbu`) at the SAME address -- a genuinely mixed-width
access to one symbol, not two overlapping symbols. Getting this byte-exact
took three attempts:

1. **First attempt: plain `(u8)D_8008EA26` cast on a `u16` variable.**
   Compiled to `lhu` + `andi 0xff` (load the full halfword, then mask) --
   one extra instruction versus retail's direct `lbu`, and it also disturbed
   delay-slot scheduling: retail hoists `li $v0, 0x5fc8` (the 5th argument's
   constant) into the `beqz`'s delay slot, but the extra `andi` displaced it,
   landing `sw $v0, 0x10($sp)` (the same constant) as a separate instruction
   before the `jal` instead of in its delay slot. **17/32 words, drift.**

2. **Second attempt: `*(volatile u8 *)&D_8008EA26`, storage also declared
   `volatile u16`.** Volatile correctly stopped the *scalar-truncation*
   optimization, but a volatile pointer-to-byte over an address-of
   expression stopped GCC from folding the address computation into the
   load's own displacement at all: it emitted `lui`/`addiu`/`lbu` (build the
   pointer, then dereference) instead of retail's `lui`/`lbu %lo(...)`
   two-instruction symbolic-offset form. Worse than attempt 1: **17/32
   words, more drift** (4 extra instructions instead of 1, and volatility
   also blocked the delay-slot hoist of the branch that guards the whole
   block, since a volatile load can no longer be speculated above the
   branch that decides whether it runs).

3. **What worked: `*(u8 *)&D_8008EA26`, storage a plain (non-volatile)
   `u16`.** Without `volatile`, GCC 2.6.3's pre-strict-aliasing analysis
   still forces a genuine reload through the pointer-to-byte cast (it does
   not fold the two accesses into a cached register value), AND it still
   folds the address computation into the load displacement, because it is
   no longer fighting a volatile qualifier over the pointer arithmetic. This
   reproduces retail's exact `lui`/`lbu %lo(sym)(...)` two-instruction read
   and lets the branch's delay slot get the constant-hoist it needs.
   **32/32, byte-exact.**

### Proposed learning

A symbol accessed at TWO DIFFERENT WIDTHS by the same function (a `u16`
store followed later by a byte re-read at the same address, or vice versa)
is not itself exotic -- it reproduces with an ordinary pointer-cast
dereference (`*(u8 *)&sym`), NOT with a scalar truncation (`(u8)sym`), which
this compiler widens back out into a full-width load plus a mask. But do
NOT reach for `volatile` to "help" force the reload: on this pointer-cast
form, volatile actively hurts -- it stops the compiler from folding the
address computation into the load's own `%lo` displacement, AND it blocks
delay-slot hoisting of an unrelated constant load across the branch that
guards the access. Plain (non-volatile) `u16` storage plus a plain (also
non-volatile) `u8 *` cast on the read reproduced retail exactly; this
compiler generation still forces the reload through the pointer without any
volatile qualifier at all. Worth confirming on a second instance before
promoting further, but both of the wrong turns here are worth knowing before
trying either.

## Naming

**Superseded, round 71 (track 2):** the track-3 game name `PlayFixedSound`
is replaced by Sony's own name -- fingerprint EXACT masked 1.00 vs
libsnd/vmanager `SpuVmNoiseOn` (discs 3.3/3.5; `libsnd/vm_noise` on 3.6).
This is Sony's SDK code, not decompiled game logic; track 2 names those
functions and moves them out of tracks 1/1b/3.

**SpuVmNoiseOn** (was `func_8002F368`) -- Tier B. Identical shape to
SpuVmNoiseOnWithAdsr (allocate a free voice, key it on) but with the trailing two
`vmNoiseOn2` parameters replaced by the constants `0x80FF`/`0x5FC8`
rather than forwarded from the caller -- i.e. a specific always-the-same
sound rather than a general playback entry point. What that fixed sound
IS (a UI cue? a fixed sample?) is not established from this function's
body; "Fixed" names the mechanical distinction from SpuVmNoiseOnWithAdsr, not the
sound's identity.
