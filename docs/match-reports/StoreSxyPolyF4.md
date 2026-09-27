# StoreSxyPolyF4

> Renamed from `func_80019724` on 2026-09-17 (tools/rename.py). Address 0x80019724.

**Unit:** TmdRenderer · **Size:** 10 instructions · **Status:** MATCHED (10/10 words)

Conditional variant of the GTE-store family (see `StoreSxyPolyF3.md` for the
family overview). `flag` selects between a full 3-register store (same
stride-4 layout as `StoreSxyPolyF3`) and a single-register store of just IR3
to a different, unrelated struct field.

## What it does

```c
void StoreSxyPolyF4(void *dst, s32 flag)
{
    char *p = (char *)dst + 0x14;

    if (flag) {
        __asm__ volatile (
            "swc2 $12, 0x8(%0)\n\t"
            "swc2 $13, 0xc(%0)\n\t"
            "swc2 $14, 0x10(%0)"
            : : "r" (dst) : "memory");
    } else {
        __asm__ volatile (
            "swc2 $14, 0x0(%0)"
            : : "r" (p) : "memory");
    }
}
```

## The one non-obvious part: `p` must be a real, unconditionally-computed pointer

A first attempt just wrote the else-branch as
`__asm__("swc2 $14, 0x14(%0)" : : "r"(dst))` — functionally identical (same
final address, `dst + 0x14`), and it reproduced every instruction **except**
the delay slot right after the `beqz`. Retail's delay slot holds
`addiu $v0, $a0, 0x14` (computing the else-branch's target address early,
scheduled into the branch's delay slot because it has no dependency on the
branch condition), and the else-branch itself is `swc2 $14, 0x0($v0)` — base
register `$v0`, offset `0`. The direct-offset version instead folds the
`+0x14` straight into the `swc2` immediate and leaves the delay slot a
`nop`, which is a different (if same-effect) instruction sequence — same
address computed, wrong bytes.

Declaring `char *p = (char *)dst + 0x14;` as an ordinary C local unconditionally,
then feeding `p` (not `dst`) to the else-branch's inline asm as its `"r"`
operand, reproduces retail exactly: GCC computes `p` regardless of `flag`
(nothing stops it, and the DFA scheduler has a free delay slot to put it in)
and the else branch dereferences the already-computed pointer at offset 0.
Verified byte-exact (including the delay-slot instruction) via the
CLAUDE.md reproducer pipeline before writing to `src/`.

## Provenance

round 12 (2026-09-03), runner charlie, unit code_8220_b (fresh carve).
Matched on the second construction (see above); first construction was
functionally correct but byte-wrong in the delay slot, caught by comparing
against the reproducer's disassembly rather than trusting "same address".

### Proposed learning

When an `INCLUDE_ASM` body computes an address into a saved/temp register
in a branch delay slot **before** the branch that decides whether it's
used, don't fold that offset into the eventual load/store's own immediate
field even if the resulting effective address is identical either way —
write the address computation as an explicit, unconditional C pointer local
before the `if`, so GCC's scheduler has the same free instruction to hoist
into the same delay slot. Two byte-identical *addresses* reached via two
different *instruction sequences* is exactly the kind of thing "close
enough" reasoning misses; only the reproducer pipeline or `funcdiff`/
asm-differ catches it.

## Naming (round 51, bravo)

`func_80019724` -> `StoreSxyPolyF4`, parameter `flag` -> `storeFirst3`.
**Tier A**; family evidence in `docs/match-reports/StoreSxyPolyF3.md`.

1. **Layout.** The true branch stores `+0x8`/`+0xC`/`+0x10` (POLY_F4's
   `xy0`/`xy1`/`xy2`, the same as POLY_F3's, since a POLY_F4 is a POLY_F3
   with a fourth vertex appended) and the false branch stores a single SXY2
   at `+0x14`, which is POLY_F4's `xy3`.
2. **Call site.** The two cases that pass it to `ProjectQuadFace`
   (0x80018C88 and 0x80018D3C) write `len = 5`, `code = 0x28` -- POLY_F4
   exactly.

`flag` -> `storeFirst3` because `ProjectQuadFace` calls the callback twice,
`1` before the fourth vertex's own `gte_rtps()` and `0` after: the flag
selects "the three vertices the shared transform produced" versus "the
fourth vertex's result", not an on/off.

## Round 91 polish (delta, track 7)

`p` is now `short *xy3 = &((POLY_F4 *)dst)->x3`, still computed
unconditionally at the top (byte-identical).
