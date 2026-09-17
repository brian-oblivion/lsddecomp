> Renamed from `func_800196FC` on 2026-09-17 (tools/rename.py). Address 0x800196fc.

# StoreSxyPolyFT3

**Unit:** code_8220_b · **Size:** 5 instructions · **Status:** MATCHED (5/5 words)

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
