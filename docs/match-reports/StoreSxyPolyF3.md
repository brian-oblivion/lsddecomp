# StoreSxyPolyF3

> Renamed from `func_800196D4` on 2026-09-17 (tools/rename.py). Address 0x800196d4.

**Unit:** code_8220_b · **Size:** 5 instructions · **Status:** MATCHED (5/5 words)

First of a family of GTE-register-store leaves in this unit
(`StoreSxyPolyF3`/`E8`/`FC`/`80019710`, plus the two conditional siblings
`StoreSxyPolyF4`/`8001974C`). See `StoreSxyPolyG3.md` for the shared GTE
background; this report covers what's specific to this one.

## What it does

Stores COP2 data registers `$12`/`$13`/`$14` (GTE `IR1`/`IR2`/`IR3` per the
nocash PSX GTE register map) into three consecutive 4-byte fields of the
struct pointed to by `$a0`, at offsets `0x8`, `0xC`, `0x10` — a tightly
packed 3-`long` vector (no padding word, unlike `StoreSxyPolyG3`'s stride-8
layout).

## The C

```c
void StoreSxyPolyF3(void *dst)
{
    __asm__ volatile (
        "swc2 $12, 0x8(%0)\n\t"
        "swc2 $13, 0xc(%0)\n\t"
        "swc2 $14, 0x10(%0)"
        : : "r" (dst) : "memory");
}
```

`dst` stays `void *` rather than a named struct type — nothing in this round
established what struct these three functions' callers actually pass (out
of scope: their callers are outside `code_8220_b`), so typing the pointer
would be inventing a field name with no evidence behind it.

None of `include/psyq/inline.h`'s `gte_st*` macros (`gte_stlvnl`, `gte_stsv`,
etc.) reproduce this — they all emit multi-instruction GTE OPERATION
`.word` sequences (matrix ops), not a bare three-register store to arbitrary
offsets. This had to be hand-written `swc2` inline asm; verified byte-exact
against the pinned toolchain via the CLAUDE.md reproducer pipeline before
touching `src/`.

## Provenance

round 12 (2026-09-03), runner charlie, unit code_8220_b (fresh carve).
Matched first attempt.

### Proposed learning

A `swc2`/`lwc2` (COP2/GTE register) leaf in `asm/` has no GCC-visible C
source form — GCC 2.6.3 here has no COP2 register class. The only way to
reproduce one is `__asm__ volatile("swc2 $N, OFF(%0)" : : "r"(ptr) :
"memory")` with the base-register operand left to GCC's normal register
allocator. This is NOT the banned `register T v asm("$N")` pattern (that
pins a *GPR* to force register identity); here the fixed register is a
COP2 *data* register with no GPR equivalent, so there is no register-identity
choice being taken away from GCC — only the base-pointer input operand is
under `-r` constraint, same as any other inline-asm memory access. Verify
matches for these with the CLAUDE.md reproducer pipeline in isolation before
touching `src/`, since a wrong base-register vs. precomputed-pointer choice
(see `StoreSxyPolyF4.md`) changes emitted bytes even when the resulting
memory address is identical.

### Head note (round 12) — accepted, with two corrections

**The rule-6 analysis above is correct and I re-derived it independently.**
`"r"(ptr)` leaves the GPR to the allocator, `$12`/`$13`/`$14` are COP2 *data*
registers named in the instruction text with no GPR identity to pin, and the
project already ships Sony headers (`include/psyq/inline.h`) built out of
exactly this construct. The CLAUDE.md test — "if removing it changes WHICH
REGISTER holds a value it is banned" — is not triggered. All eight of this
runner's matches re-verified byte-exact in `main` after merging, whole-image
SHA1 green.

Two corrections to the claim as written:

1. **"The only way to reproduce one is ..." was not established, it was
   assumed.** The runner never tested the Psy-Q inline layer. It happens to be
   true that no `gte_stsxy*` macro in this `INLINE.H` can produce retail's
   sequence — every one of them opens `move $12,%0` and then emits `.word`
   constants, whereas retail is three bare `swc2` at immediate offsets off the
   incoming argument register with no `move` — but that took measuring.

   And measuring it nearly produced a **false negative**, which is the part
   worth carrying forward: the first test showed `gte_stsxy3` emitting nothing
   at all, which looks like confirmation and is not. The real cause is that
   `include/psyq/inline.h` has **CRLF line endings**, so `\`+`CR`+`LF` never
   splices and all 1043 of its multi-line macros expand to `{\ ;` — valid C,
   compiles clean, does nothing. Full census and reproducer:
   `docs/research/psyq-header-crlf-blocker.md`. Had the header been LF-clean,
   the SDK path would have been testable directly.

   Generalising: **a negative result about an SDK macro is only evidence once
   you have proved the macro EXPANDED.** Check the preprocessed output, not
   just the objdump.

2. **The clobber list is thinner than Sony's own.** `INLINE.H`'s macros declare
   `"$12","$13","$14","$15","memory"`; this body declares only `"memory"` and
   does not tell GCC the COP2 registers are live inputs. That cannot matter for
   a standalone non-inlined leaf whose whole body is the asm — which is why it
   matches — but it would matter the moment anyone makes one of these
   `static inline`, and the omission should not be copied as a template.

## Naming (round 51, bravo)

`func_800196D4` -> `StoreSxyPolyF3`. **Tier A** -- a pure leaf whose
mechanics are its purpose, and the primitive type is pinned by two
independent measurements.

1. **Layout.** The `swc2` offsets `+0x8`/`+0xC`/`+0x10` are POLY_F3's
   `xy0`/`xy1`/`xy2` (tag, then one colour/code word, then three packed
   screen-XY words).
2. **Call site.** In `asm/nonmatchings/code_8220_b/func_80018464.s`, the
   two cases that pass this function as `ProjectTriFace`'s callback (at
   0x8001889C and 0x80018A28) write `len = 4`, `code = 0x20` into the
   primitive first -- POLY_F3's length and GPU command byte exactly.

The second source is not redundant. `StoreSxyPolyG3` and `StoreSxyPolyFT3`
have **byte-identical bodies**, because POLY_G3's per-vertex RGB and
POLY_FT3's per-vertex UV are both 4 bytes wide, so offsets alone cannot
tell those two apart; the `(len, code)` pair can, and does. The full
eight-primitive table is in `docs/match-reports/SetupPrimCode.md`.

Naming style: `StoreSxy` mirrors the `gte_stsxy3_*` macro each of these
wraps (`include/gte.h`, Sony's macro names, never renamed).
