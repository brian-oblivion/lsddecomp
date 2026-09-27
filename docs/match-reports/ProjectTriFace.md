# ProjectTriFace — MATCHED (57/57 words)

> Renamed from `func_800193C0` on 2026-09-17 (tools/rename.py). Address 0x800193c0.

Unit: `src/TmdRenderer.c`. Triangle submission routine: computes three
vertex-array pointers from a shared base and three `u16` indices, stores
them into the primitive/context struct (`prim`, same struct
`TransformAndCullPoly` operates on — see that report), loads them into the GTE
via `lwc2`, calls `TransformAndCullPoly` to transform/clip/OT-bucket, and — on
success — writes the resulting screen Z into three separate output arrays
(`prim->0x88/0x8c/0x90`, each element's `+0x14` field) via `swc2`, invokes
a caller-supplied callback, and calls `FlagLargePolyForDivide(prim, 3)` (the literal
`3` looks like a "this is a triangle" primitive-kind tag, paired with `4` in
`ProjectQuadFace`'s equivalent call).

## Final source

```c
s32 ProjectTriFace(void *arg0, u8 *prim, u16 idx0, u16 idx1, u16 idx2, void (*callback)(void *))
{
    *(void **)(prim + 0xa4) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx0 * 8;
    *(void **)(prim + 0xa8) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx1 * 8;
    *(void **)(prim + 0xac) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx2 * 8;

    __asm__ volatile (
        "lwc2 $0, 0x0(%0)\n\t"
        "lwc2 $1, 0x4(%0)\n\t"
        "lwc2 $2, 0x0(%1)\n\t"
        "lwc2 $3, 0x4(%1)\n\t"
        "lwc2 $4, 0x0(%2)\n\t"
        "lwc2 $5, 0x4(%2)\n\t"
        :
        : "r" (*(void **)(prim + 0xa4)), "r" (*(void **)(prim + 0xa8)), "r" (*(void **)(prim + 0xac)));

    if (TransformAndCullPoly(arg0, prim) != 0) {
        goto fail;
    }

    {
        u8 *p0 = *(u8 **)(prim + 0x88) + 0x14;
        u8 *p1 = *(u8 **)(prim + 0x8c) + 0x14;
        u8 *p2 = *(u8 **)(prim + 0x90) + 0x14;

        __asm__ volatile (
            "swc2 $17, 0x0(%0)\n\t"
            "swc2 $18, 0x0(%1)\n\t"
            "swc2 $19, 0x0(%2)\n\t"
            :
            : "r" (p0), "r" (p1), "r" (p2)
            : "memory");
    }
    callback(arg0);
    FlagLargePolyForDivide(prim, 3);
    return 0;
fail:
    return 1;
}
```

Types: `idx0`/`idx1`/`idx2` are `u16` (retail's `andi $rN,$rN,0xffff` on the
register-passed ones is the truncation GCC emits for a 16-bit parameter, and
the stack-passed third index needs no such mask because `lhu` already
zero-extends on load). `prim`'s real struct type is unknown outside this
file (see `TransformAndCullPoly`'s report for the fields established so far);
treated as `u8 *` with raw offsets, matching that function's approach.

## How it was built (7 attempts to full match)

1. First cut (8/57): a `void *base = *(void**)(prim+0xc); ... base + idxN*8`
   local, `void *prim0` cast to a separate `u8 *prim`, and both `lwc2`/`swc2`
   asm blocks over-clobbering `"$2","$3","$4","$5"`. Wrong on three
   independent axes at once.
2. Fixed the `u8 *prim` parameter typing (dropped the redundant `void*`
   local that GCC wasn't coalescing away, producing a spurious `move`) —
   still 8/57, revealing the OTHER problems underneath.
3. Stopped caching `*(void**)(prim+0xc)` in a `base` local and re-read it
   fresh for each of the three vertex-address computations (retail's own
   `lw $v1, 0xc($s0)` appears three times, once per vertex, not once cached)
   — 24/57. This is the same "reload, don't cache" idiom already documented
   for `GetNextBasicClass`/`SetupPrimCode` in this unit, now confirmed a third
   time on a different construct (a struct field read feeding three
   independent computations, not a linked-list walk or a same-byte bit op).
4. Tried several variants of how the third vertex pointer reaches the `lwc2`
   asm operand (a bare local var computed once and reused directly, three
   named locals with no reload at all, reordering the asm operand list) —
   all SCORED WORSE (7-22/57) than step 3's plain re-dereference, and
   distorted the FIRST TWO vertices' register choices too even though those
   statements were untouched. Reverted to step 3's shape.
5. **The actual bug, found by re-reading the asm block's own clobber list**:
   the `lwc2` asm's `: "$2", "$3", "$4", "$5"` clobber was WRONG — `lwc2`
   loads into COP2 *data* registers `$0`-`$5` (the register numbers named in
   the instruction text), not the GPRs `$2`-`$5` (`v0`-`a1`). Declaring GPRs
   `v0`-`a1` clobbered forced GCC to copy the operand values OUT of whatever
   register they were already computed in and INTO fresh temp registers
   before the asm, because the "already correct" register happened to be one
   of the falsely-clobbered ones. Dropping the bogus clobber list entirely
   made the `lwc2` block match retail exactly in one step — including the
   detail that the third vertex pointer needed no `move`/reload at all,
   because with nothing telling GCC otherwise, it simply left the value
   where the preceding `addu` had already put it (`$v0`), same as retail.
   24/57 -> 33/57.
6. The second (`swc2`) asm block had a similar but more subtle issue: it
   used `0x14(%N)` addressing directly (offsetting inside the instruction),
   where retail instead computes an explicit `ptr + 0x14` via `addiu` and
   stores at offset `0`. Both are semantically identical but encode to
   different bytes and, because retail's version needs 3 MORE live values
   (the incremented pointers) held across the store, changes which
   registers get chosen for everything downstream too. Introducing named
   `u8 *p0/p1/p2 = *(u8**)(...) + 0x14;` locals and storing at offset `0`
   reproduced retail's exact register choices (`a0`, `v1`, `v0`) — matching
   the project's established "take an explicit intermediate element pointer"
   idiom, just applied to raw offsets rather than an array element. 33/57
   -> 45/57.
7. Final residue: the early-exit `if (TransformAndCullPoly(...) != 0) { return 1;
   }` compiled to a `bnez` with an EMPTY delay slot and a separate `li
   $v0,1` later, one word longer than retail's version (which puts `ori
   $v0,$zero,1` directly in the branch's delay slot). Retyping the early
   exit as `goto fail; ... fail: return 1;` (rather than an inline `return
   1;`) moved the return-value materialization into the delay slot, matching
   retail exactly — the same `goto`-vs-`return` non-interchangeability
   already documented for `New_Pad` in DECOMPILATION_LEARNINGS, now
   confirmed on an early exit whose value DIFFERS from a later same-`return`
   -type success path (`0` vs `1`), not just on the "different value on the
   `goto` path" shape it was originally found on. 45/57 -> 57/57.

### Proposed learning

**A wrong clobber in a raw-register (`$N`-named) asm block is not
inert — it actively pessimizes the register allocation of the C AROUND
it, and the direction of the damage looks exactly like an unrelated
register-identity residue.** Concretely: naming GPRs in an asm block's
clobber list when the instructions inside actually target a DIFFERENT
register file (here: COP2 data registers, whose mnemonic operands `$0`-`$31`
share GPR-looking syntax but are a disjoint numbering) tells GCC those GPRs
are unusable across the asm, so any operand value already sitting in one of
them gets evicted to a fresh temp register first — producing a spurious
extra `move`/reload that doesn't exist in retail, and shifting every
register choice made afterward in ways that look, from a funcdiff score
alone, like an unrelated structural mismatch. When a `swc2`/`lwc2`/`cfc2`
asm block's residue looks like "half my registers are just off by a
constant offset, and there's one extra instruction I can't place," check
the clobber list for GPR names before reshaping the surrounding C — the
project's own store-leaf precedents (`StoreSxyPolyF3` etc., same unit)
clobber ONLY `"memory"`, never a GPR number, and that omission is
load-bearing, not incidental style. (`ProjectTriFace`, 8/57 -> 33/57 from
this one fix alone.)

Also reconfirms two entries already in DECOMPILATION_LEARNINGS on new
constructs: "reload from memory rather than caching" now covers a struct
field feeding three independent downstream computations (not just a linked-
list walk), and "`goto`+labeled `return` vs. inline `return`" now covers an
early exit whose return value differs from the function's normal-path
value, not only the shape the entry was originally derived from.

## Naming (round 51, bravo)

`func_800193C0` -> `ProjectTriFace`, `arg0` -> `prim`, `prim` -> `ctx`,
`callback` -> `storeSxy`. **Tier B** -- the mechanics are fully established
and the caller's use is clear, but "Face" is read off the caller's data
shape rather than from a name in the game.

Evidence for the name: the function takes three `u16` indices into the
8-byte vertex array the context holds at `+0x0C`, loads those vertices into
the GTE, delegates the transform and the cull decision to
`TransformAndCullPoly`, writes each vertex's screen Z into a sort slot, has
the caller's callback write the screen XY into the primitive, computes the
screen bounding box, and returns 0 drawn / 1 culled. That is "project one
triangle and say whether it survived". Its caller `SortTmdObject` iterates
a list of records each carrying three or four vertex indices, which is a
face list.

**The parameter names were backwards and that is worth stating plainly.**
The round-13 signature called the second parameter `prim`. It is not the
primitive -- it is the per-object draw context (`ctx+0x0C` vertex array,
`ctx+0x88..0x90` per-vertex sort records, `ctx+0xA4..0xAC` vertex slots).
The GPU primitive is `arg0`, the argument that was unnamed: it is the same
pointer `SetupPrimCode` stamps the `(len, code)` pair into, and it is what
`storeSxy` writes screen XYs into at that primitive type's own POLY_xx
offsets. See `docs/match-reports/SetupPrimCode.md` for the eight-for-eight
`(len, code)` identification. Zero bytes changed; the swap is a naming fix,
not a semantic one.

`FlagLargePolyForDivide(ctx, 3)`'s literal `3` is a VERTEX COUNT, not a
"primitive kind" tag -- that function walks `count` screen-XY pairs and
computes their 2D bounding box (`docs/match-reports/FlagLargePolyForDivide.md`
derives the body). The old "3 = triangle, 4 = quad" gloss in this report
and in `include/code_8220.h` had the right numbers for the wrong reason
and is corrected in both places.

## Round 91 polish (delta, track 7)

`ctx` is the unit's `PolyDrawCtx`: `faceVtx[0..2] = &vertices[idxN]`, the sort
Zs to `&divVtx3[N]->sz` (Sony's `RVECTOR.sz`, +0x14). **Measured:** writing
the slots as `ctx->faceVtx[i] = ...` (struct stores) moved the third store
and its `lw` reload ahead of the `lhu` of the stack argument, 67 bytes off;
writing them through a plain `SVECTOR **vtx = ctx->faceVtx` (scalar stores,
as the old `ctx + 0xa4` form was) is byte-exact. The source carries a
MATCHING line.
