# StoreSxyPolyFT4 — MATCHED (10/10 words)

> Renamed from `func_80019774` on 2026-09-24 (tools/rename.py). Address 0x80019774.

Unit: `src/code_8220_c.c` (carved round 13, third slice of `code_8220`).
Same COP2-store-leaf class already established in `tmd_renderer`
(`StoreSxyPolyF3`, `StoreSxyPolyF4`, etc. — see
`docs/DECOMPILATION_LEARNINGS.md`'s GTE-store-leaf entries): a `flag`-gated
choice of which COP2 data registers to write into `dst`, no plain-C form
possible.

## Final source

```c
void StoreSxyPolyFT4(void *dst, s32 flag)
{
    if (flag) {
        __asm__ volatile (
            "swc2 $12, 0x8(%0)\n\t"
            "swc2 $13, 0x10(%0)\n\t"
            "swc2 $14, 0x18(%0)"
            : : "r" (dst) : "memory");
    } else {
        char *p = (char *)dst + 0x20;

        __asm__ volatile (
            "swc2 $14, 0x0(%0)"
            : : "r" (p) : "memory");
    }
}
```

## Notes

Byte-exact on the first attempt — a direct transcription of `tmd_renderer`'s
`StoreSxyPolyF4`/`StoreSxyPolyG4` pattern (`if (flag) { three swc2 at fixed
offsets } else { one swc2 through dst+N }`) with this function's own offsets
(`0x8`/`0x10`/`0x18` vs. else-branch `0x20`). No new residue, no new lever.

## Naming (round 77, alpha)

`func_80019774` -> `StoreSxyPolyFT4`, parameter `flag` -> `storeFirst3`.
**Tier A.** Sixth and final member of the StoreSxyPoly** family split
across tmd_renderer (StoreSxyPolyF3/G3/FT3/GT3/F4/G4) and this unit.
StoreSxyPolyG4.md already named this function's call-site discriminator
when it named its own sibling: the POLY_FT4 cases in SortTmdObject
(`len = 9`, `code = 0x2C`) pass this function; the POLY_GT4 cases
(`len = 12`, `code = 0x3C`) pass `func_8001979C` (now StoreSxyPolyGT4,
below). Offsets `0x8`/`0x10`/`0x18` (true branch) and `0x20` (false
branch) match POLY_FT4's xy0..xy3 layout exactly as StoreSxyPolyG4's for
POLY_G4. `storeFirst3` matches the already-established parameter name on
StoreSxyPolyF4/StoreSxyPolyG4.

## Round 91 polish (bravo)

`dst` is now `POLY_FT4 *prim` (libgpu.h) and the else branch is
`gte_stsxy2(&prim->x3)`: the `char *p = dst + 0x20` local was not needed
(byte-identical without it). Parameter names unchanged (`storeFirst3`).
