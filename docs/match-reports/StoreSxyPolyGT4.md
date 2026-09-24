# StoreSxyPolyGT4 — MATCHED (10/10 words)

> Renamed from `func_8001979C` on 2026-09-24 (tools/rename.py). Address 0x8001979c.

Unit: `src/code_8220_c.c`. Same COP2-store-leaf class as `StoreSxyPolyFT4`
(this unit) and `code_8220_b`'s `StoreSxyPolyF4`/`StoreSxyPolyG4` family —
`flag`-gated choice of which COP2 registers to write into `dst`.

## Final source

```c
void StoreSxyPolyGT4(void *dst, s32 flag)
{
    if (flag) {
        __asm__ volatile (
            "swc2 $12, 0x8(%0)\n\t"
            "swc2 $13, 0x14(%0)\n\t"
            "swc2 $14, 0x20(%0)"
            : : "r" (dst) : "memory");
    } else {
        char *p = (char *)dst + 0x2c;

        __asm__ volatile (
            "swc2 $14, 0x0(%0)"
            : : "r" (p) : "memory");
    }
}
```

## Notes

Byte-exact on the first attempt, offsets `0x8`/`0x14`/`0x20` (if-branch) and
`0x2c` (else-branch) substituted into the same established pattern as
`StoreSxyPolyFT4`. No new residue, no new lever.

## Naming (round 77, alpha)

`func_8001979C` -> `StoreSxyPolyGT4`, parameter `flag` -> `storeFirst3`.
**Tier A.** Sibling of StoreSxyPolyFT4 (above); the POLY_GT4 discriminator
(`len = 12`, `code = 0x3C`) is already recorded in StoreSxyPolyG4.md, which
named this function ahead of time as "func_8001979C ... whenever that unit
is worked." Offsets `0x8`/`0x10`/`0x18` (true) / `0x2C` (false) match
POLY_GT4's layout.
