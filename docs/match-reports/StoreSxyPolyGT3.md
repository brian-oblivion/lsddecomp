# StoreSxyPolyGT3

> Renamed from `func_80019710` on 2026-09-17 (tools/rename.py). Address 0x80019710.

**Unit:** code_8220_b · **Size:** 5 instructions · **Status:** MATCHED (5/5 words)

Third distinct offset pattern in the GTE-store family (see
`StoreSxyPolyF3.md` for the family overview and reproducer methodology):
stride-12 this time (`0x8`, `0x14`, `0x20`).

## The C

```c
void StoreSxyPolyGT3(void *dst)
{
    __asm__ volatile (
        "swc2 $12, 0x8(%0)\n\t"
        "swc2 $13, 0x14(%0)\n\t"
        "swc2 $14, 0x20(%0)"
        : : "r" (dst) : "memory");
}
```

## Provenance

round 12 (2026-09-03), runner charlie, unit code_8220_b (fresh carve).
Matched first attempt; verified byte-exact in isolation via the CLAUDE.md
reproducer pipeline before writing to `src/`.

## Naming (round 51, bravo)

`func_80019710` -> `StoreSxyPolyGT3`. **Tier A**; family evidence in
`docs/match-reports/StoreSxyPolyF3.md`.

1. **Layout.** `+0x8`/`+0x14`/`+0x20` -- stride 12, POLY_GT3's `xy0`/`xy1`/
   `xy2`, with an RGB word and a UV word between each pair.
2. **Call site.** The case that passes it (0x80019058) writes `len = 9`,
   `code = 0x34` -- POLY_GT3 exactly.

This is the only tri-flavoured store with a stride-12 layout, so here the
two sources agree with nothing else competing.
