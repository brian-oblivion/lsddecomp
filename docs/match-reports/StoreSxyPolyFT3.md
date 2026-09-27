# StoreSxyPolyFT3

> Renamed from `func_800196FC` on 2026-09-17 (tools/rename.py). Address 0x800196fc.

**Unit:** TmdRenderer · **Size:** 5 instructions · **Status:** MATCHED (5/5 words)

Sibling of `StoreSxyPolyF3`/`StoreSxyPolyG3` (see `StoreSxyPolyF3.md` for the
GTE-store family overview). Body is byte-identical to `StoreSxyPolyG3`'s —
same offsets (`0x8`/`0x10`/`0x18`), different call site elsewhere in the
game presumably passing a different struct type or a different one of
several near-duplicate store helpers. Not unified into one symbol since
retail itself keeps them as two separate functions at two separate
addresses.

## The C

```c
void StoreSxyPolyFT3(void *dst)
{
    __asm__ volatile (
        "swc2 $12, 0x8(%0)\n\t"
        "swc2 $13, 0x10(%0)\n\t"
        "swc2 $14, 0x18(%0)"
        : : "r" (dst) : "memory");
}
```

## Provenance

round 12 (2026-09-03), runner charlie, unit code_8220_b (fresh carve).
Matched first attempt (identical construction to `StoreSxyPolyG3`, verified
independently against this function's own `.s`).

## Naming (round 51, bravo)

`func_800196FC` -> `StoreSxyPolyFT3`. **Tier A**; family evidence in
`docs/match-reports/StoreSxyPolyF3.md`.

Specific to this one:

1. **Layout.** `+0x8`/`+0x10`/`+0x18` -- identical to `StoreSxyPolyG3`'s,
   because POLY_FT3's per-vertex UV word and POLY_G3's per-vertex RGB word
   are both 4 bytes. The round-12 report noted the two bodies are
   byte-identical and left the difference unexplained; this is the
   explanation.
2. **Call site, which is the discriminator.** The two cases that pass this
   function as `ProjectTriFace`'s callback (0x80018AC0 and 0x80018BAC)
   write `len = 7`, `code = 0x24` -- POLY_FT3 exactly. Its sibling's cases
   write `(6, 0x30)`.

Retail keeps them as two functions at two addresses and so does the source;
they are not the same function under two names, they are one shape serving
two primitive types.
