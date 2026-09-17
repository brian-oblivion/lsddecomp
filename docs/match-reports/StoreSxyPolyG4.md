> Renamed from `func_8001974C` on 2026-09-17 (tools/rename.py). Address 0x8001974c.

# StoreSxyPolyG4

**Unit:** code_8220_b · **Size:** 10 instructions · **Status:** MATCHED (10/10 words)

Sibling of `StoreSxyPolyF4` — same conditional shape (full 3-register store
vs. single IR3 store), different offsets: full store uses the stride-8
layout (`0x8`/`0x10`/`0x18`, same as `StoreSxyPolyG3`/`StoreSxyPolyFT3`) and
the single-register fallback targets offset `0x20` instead of `0x14`. See
`StoreSxyPolyF4.md` for why the fallback path needs an unconditionally
precomputed pointer local rather than a folded immediate offset.

## The C

```c
void StoreSxyPolyG4(void *dst, s32 flag)
{
    char *p = (char *)dst + 0x20;

    if (flag) {
        __asm__ volatile (
            "swc2 $12, 0x8(%0)\n\t"
            "swc2 $13, 0x10(%0)\n\t"
            "swc2 $14, 0x18(%0)"
            : : "r" (dst) : "memory");
    } else {
        __asm__ volatile (
            "swc2 $14, 0x0(%0)"
            : : "r" (p) : "memory");
    }
}
```

## Provenance

round 12 (2026-09-03), runner charlie, unit code_8220_b (fresh carve).
Matched first attempt (same construction as `StoreSxyPolyF4`, applied with
this function's own offsets and verified independently).
