# StoreSxyPolyG3

> Renamed from `func_800196E8` on 2026-09-17 (tools/rename.py). Address 0x800196e8.

**Unit:** TmdRenderer · **Size:** 5 instructions · **Status:** MATCHED (5/5 words)

Sibling of `StoreSxyPolyF3` (see that report for the GTE-store family
overview and the reproducer methodology). This one and `StoreSxyPolyFT3`
(next function, byte-identical body) store to a stride-8 layout instead of
the tightly-packed stride-4 layout.

## GTE background

`$12`/`$13`/`$14` are COP2 data registers `IR1`/`IR2`/`IR3` (nocash PSX GTE
register map). `swc2 $N, OFF($a0)` stores one 32-bit sign-extended IR value
to memory — no C-level representation exists for a COP2 register in this
GCC 2.6.3 build (no `-mgte`, no register class), so this is inline asm, not
translated arithmetic.

## What it does

```c
void StoreSxyPolyG3(void *dst)
{
    __asm__ volatile (
        "swc2 $12, 0x8(%0)\n\t"
        "swc2 $13, 0x10(%0)\n\t"
        "swc2 $14, 0x18(%0)"
        : : "r" (dst) : "memory");
}
```

Offsets `0x8`, `0x10`, `0x18` — 8 bytes apart, not 4. Plausibly the X (or
first-long) component of three separate 8-byte fields further out in a
struct this unit doesn't otherwise touch; not resolved further since no
caller lives in this unit.

## Provenance

round 12 (2026-09-03), runner charlie, unit code_8220_b (fresh carve).
Matched first attempt; verified byte-exact in isolation via the CLAUDE.md
reproducer pipeline before writing to `src/`.

## Naming (round 51, bravo)

`func_800196E8` -> `StoreSxyPolyG3`. **Tier A**; the family's evidence and
the two-independent-sources argument are in
`docs/match-reports/StoreSxyPolyF3.md`.

Specific to this one:

1. **Layout.** `+0x8`/`+0x10`/`+0x18` -- stride 8, POLY_G3's `xy0`/`xy1`/
   `xy2` with a per-vertex RGB word between each.
2. **Call site.** `SortTmdObject` hoists this function's address into `$s6`
   once at 0x80018758 and passes it as the callback for the two cases that
   write `len = 6`, `code = 0x30` -- POLY_G3 exactly.

Point 2 is what separates this function from `StoreSxyPolyFT3`, whose body
is byte-identical.
