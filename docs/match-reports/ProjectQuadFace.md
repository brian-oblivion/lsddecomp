# ProjectQuadFace — MATCHED (82/82 words)

> Renamed from `func_800194A4` on 2026-09-17 (tools/rename.py). Address 0x800194a4.

Unit: `src/TmdRenderer.c`. Quad submission routine — the four-vertex sibling
of `ProjectTriFace` (triangle submission, same unit). Computes four vertex
pointers (vs. three), transforms the first three as a triangle through
`TransformAndCullPoly` (shared with `ProjectTriFace`), then does a SEPARATE single-
vertex `rtps` transform for the fourth vertex, writes four Z outputs (vs.
three) via `swc2 $16`-`$19` into `prim->0x94/0x98/0x9c/0xa0`, stores the
fourth vertex's transformed screen XY into `prim->0x6c`, and calls
`FlagLargePolyForDivide(prim, 4)` (code `4` = "quad", vs. `ProjectTriFace`'s `3` =
"triangle"). Also confirms `callback`'s real signature: this function makes
TWO calls to it with explicit different second arguments (`callback(self,
1)` before the fourth-vertex work, `callback(self, 0)` after) — so it is
`void (*)(void *, s32)`, not the single-arg guess `ProjectTriFace`'s lone
call site couldn't distinguish from a genuinely-unused second parameter.

## Final source

```c
s32 ProjectQuadFace(void *arg0, u8 *prim, u16 idx0, u16 idx1, u16 idx2, u16 idx3, void (*callback)(void *, s32))
{
    u8 *vtxSlot = prim + 0xa4;

    *(void **)(prim + 0xa4) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx0 * 8;
    *(void **)(prim + 0xa8) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx1 * 8;
    *(void **)(prim + 0xac) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx2 * 8;
    *(void **)(prim + 0xb0) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx3 * 8;

    __asm__ volatile (
        "lwc2 $0, 0x0(%0)\n\t"
        "lwc2 $1, 0x4(%0)\n\t"
        "lwc2 $2, 0x0(%1)\n\t"
        "lwc2 $3, 0x4(%1)\n\t"
        "lwc2 $4, 0x0(%2)\n\t"
        "lwc2 $5, 0x4(%2)\n\t"
        :
        : "r" (*(void **)(vtxSlot + 0x0)), "r" (*(void **)(vtxSlot + 0x4)), "r" (*(void **)(vtxSlot + 0x8)));

    if (TransformAndCullPoly(arg0, prim) != 0) {
        goto fail;
    }

    callback(arg0, 1);

    __asm__ volatile (
        "lwc2 $0, 0x0(%0)\n\t"
        "lwc2 $1, 0x4(%0)\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4A180001\n\t"     /* rtps */
        :
        : "r" (*(void **)(vtxSlot + 0xc)));

    {
        u8 *p0 = *(u8 **)(prim + 0x94) + 0x14;
        u8 *p1 = *(u8 **)(prim + 0x98) + 0x14;
        u8 *p2 = *(u8 **)(prim + 0x9c) + 0x14;
        u8 *p3 = *(u8 **)(prim + 0xa0) + 0x14;

        __asm__ volatile (
            "swc2 $16, 0x0(%0)\n\t"
            "swc2 $17, 0x0(%1)\n\t"
            "swc2 $18, 0x0(%2)\n\t"
            "swc2 $19, 0x0(%3)\n\t"
            :
            : "r" (p0), "r" (p1), "r" (p2), "r" (p3)
            : "memory");
    }

    callback(arg0, 0);

    __asm__ volatile (
        "swc2 $14, 0x0(%0)\n\t"
        :
        : "r" (prim + 0x6c)
        : "memory");

    FlagLargePolyForDivide(prim, 4);
    return 0;
fail:
    return 1;
}
```

`rtps` (single-vertex perspective transform) is, like `rtpt`/`nclip`/`avsz3`
in `TransformAndCullPoly`, unsupported as a mnemonic by this pinned binutils —
emitted as `.word 0x4A180001` (its raw retail bytes).

## How it was built (2 real attempts)

Went in almost directly on the strength of `ProjectTriFace`'s lessons (explicit
intermediate element pointer for the offset-`0x14` stores, `goto`+labeled
`return` for the differing-value early exit, no bogus GPR clobbers on the
`lwc2`/`swc2`/`rtps` blocks, reload struct fields rather than caching), with
one new wrinkle this function surfaced:

- **First attempt (9/82, badly drifted):** dereferenced `prim + 0xa4` /
  `0xa8` / `0xac` / `0xb0` directly (absolute offsets) everywhere, exactly
  as `ProjectTriFace` did successfully for ITS three vertex slots. This
  function's frame differs from `ProjectTriFace`'s by ONE more callee-saved
  register in retail (`$s0`-`$s3`, not `$s0`-`$s2`) precisely because retail
  computes `s1 = prim + 0xA4` ONCE and reuses that pointer — both for the
  post-transform reload (`lw $t0, 0x0($s1)` / `0x4($s1)` / `0x8($s1)`, SMALL
  offsets) and, later, to fetch the fourth vertex pointer
  (`s1 = *(s1 + 0xC)`, reusing the same register for a new purpose once its
  first job is done). Dereferencing the struct at ABSOLUTE offsets each time
  cannot reproduce this — it's not just a register-choice difference, the
  actual immediate encoded in the `lw` differs (`0xa4` vs `0x0` relative to a
  base), so no amount of register-allocation coaxing can close it.
- **Fix:** introduce `u8 *vtxSlot = prim + 0xa4;` once, and address the later
  reloads as `vtxSlot + 0x0/0x4/0x8/0xc` instead of `prim + 0xa4/0xa8/0xac/
  0xb0`. GCC promoted `vtxSlot` to its own callee-saved register (matching
  retail's 4-register frame) since it's live across the `TransformAndCullPoly`
  call. 9/82 -> 82/82 in one step.

### Proposed learning

**When a sibling function in the same unit already matched using absolute
struct-offset dereferences, and the new function's retail frame uses ONE
MORE callee-saved register than the working sibling's, look for a spot
where retail computes an intermediate pointer INTO the struct and reuses it
with small offsets — not just once, but potentially across a live range
that spans a function call and gets repurposed afterward for something
else.** The tell isn't only the extra saved register in the prologue; it's
that the `lw`/`sw` immediates in the reload sequence are small
(`0x0`/`0x4`/`0x8`/`0xc`) rather than matching the struct's real field
offsets (`0xa4`/`0xa8`/`0xac`/`0xb0`) — an absolute-offset dereference
cannot produce that encoding no matter how the surrounding C is reshaped,
because the immediate itself is wrong, not just which register holds the
address. (`ProjectQuadFace`, 9/82 -> 82/82 from introducing one intermediate
pointer local.)

## Naming (round 51, bravo)

`func_800194A4` -> `ProjectQuadFace`, `arg0` -> `prim`, `prim` -> `ctx`,
`callback` -> `storeSxy`. **Tier B**, same evidence and the same parameter
swap as `docs/match-reports/ProjectTriFace.md` -- read that one first; only
what is specific to the four-vertex form is below.

- The first three vertices go through the identical shared
  `TransformAndCullPoly`; the fourth is transformed on its own afterwards
  with a single `gte_rtps()`. That is why `storeSxy` takes a second
  argument and is called twice, `1` before the fourth vertex's transform
  (store the first three screen XYs) and `0` after (store the fourth).
  Hence the callback parameter name `storeFirst3` on the two quad store
  leaves, `StoreSxyPolyF4` and `StoreSxyPolyG4`.
- `FlagLargePolyForDivide(ctx, 4)` -- again a vertex count, not a kind code.
- This function is what already established `storeSxy`'s real two-argument
  signature; the triangle form's single call site could not distinguish it
  from an unused second parameter.

## Round 91 polish (delta, track 7)

As ProjectTriFace: `PolyDrawCtx` fields, the four slot stores through the
existing plain `vtx` pointer (struct stores cost 70 bytes), sort Zs to
`&divVtx4[N]->sz`, the fourth SXY to `&ctx->sxy3`.
