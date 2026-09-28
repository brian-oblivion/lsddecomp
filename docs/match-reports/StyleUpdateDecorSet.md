# StyleUpdateDecorSet -- MATCHED, 93/93 words, insertions 0 / deletions 0, whole-image SHA1 green

> Renamed from `func_800549A8` on 2026-09-23 (tools/rename.py). Address 0x800549a8.

REVISITED, round 61: STALL (24/93, ~4 words short, two rounds' "same
register-pressure residue as `StyleBuildDecorSet`") -> **MATCHED on the first
attempt**; names/types not relevant (unit has not passed track 3).

## What closed it

One construct, carried over from `StyleBuildDecorSet` earlier in this same session:
**the pair of adjacent globals is copied into the local pair by a WHOLE-STRUCT
assignment, not field by field.**

```c
pos = *(PairXY *) &sStyleDecorPosX;      /* NOT pos.x = sStyleDecorPosX; pos.y = sStyleDecorPosY; */
```

A struct assignment is a BLKmode `set`, and gcc 2.6.3's `cse.c` answers a
BLKmode destination by calling `invalidate_memory()` -- it discards **every**
cached memory value rather than the ones that might overlap. That is the
entire reason retail reloads `sStyleDecorVariant` for its `== 2` test (`lw v1,
%gp_rel(sStyleDecorVariant)` at 0x45220, after the guard already read it at 0x451A8)
and reloads `pos.y` from the stack immediately after writing it (`lw v0,
0x1C(sp)` at 0x45230). Rounds 46 and 47 both classified those two reloads as
*"register-pressure-driven, not something a source rewrite obviously
controls"* and stalled the function on them. They were one source-shape
difference and cost one build to fix.

Full derivation of the mechanism, and the measurements that established it,
are in `docs/match-reports/StyleBuildDecorSet.md` (round 61). This function is the
confirmation: the lever was derived on a sibling and transferred unchanged.

Three smaller corrections to round 46's body went in at the same time, all
read off the raw `.s` rather than guessed:

| correction | evidence |
| --- | --- |
| the array is walked with a pointer (`wp++`), not indexed by the loop counter (`arr[i]`) | `addiu s0,s0,4` in the `bnez`'s delay slot at 0x452CC; `lw a0,0(s0)` twice per iteration |
| the two per-iteration dispatch receivers are two separate `*wp` reads | `lw a0,0(s0)` at 0x45274 **and again** at 0x45294 -- the intervening call clobbers memory, so both are real loads, not a CSE failure |
| the output buffer is 8 bytes, not 3 or 4 | it occupies sp+0x10..0x17, because `pos` sits at sp+0x18. `AdjustRgbByDelta` writes only `[0..2]`, so the SIZE is inferred from the stack layout and nothing else -- see the comment at the definition. |

Round 46's `s3` name is kept as `shift` and `s1` as `srcOfs`; `srcOfs` walks a
3-byte-stride table (the same stride `StyleFillEffectKind3` uses on `sStyleKind3Colors`),
which is consistent with `AdjustRgbByDelta`'s three byte writes.

## What round 46 got right, and is worth keeping

The magic-number division. `(delta / 600) * 3`, with `0x1B4E81B5` and
`sra 6`, was found by brute-forcing `x/N` for `N` in `2..2000` through the
pinned pipeline and grepping the objdump for the constant. That derivation was
correct and went straight into the matching body unchanged. The learning it
proposed -- brute-force the divisor rather than reverse the reciprocal math --
stands.

The struct-and-flow recovery (call targets, method slots +0xB8/+0xBC/+0x64,
`sStyleSceneRefs + 0xC` chased one field further, the loop bounds) was also correct
throughout. **What was wrong was only the VERDICT**, and specifically the part
of it that named an unfalsifiable cause. "Register pressure" identifies no
construct, suggests no experiment, and ends the investigation; two rounds
stopped there.

## The matched body

```c
void StyleUpdateDecorSet(void) {
    ObjAC7CSub *self;
    s32 delta;
    s32 shift;
    u8 rgb[8];
    PairXY pos;
    s32 srcOfs;
    s32 i;
    void **wp;
    ObjSlotB8B8 *obj;

    if (sStyleDecorVariant == 0) {
        return;
    }
    self = *(ObjAC7CSub **) (sStyleSceneRefs + 0xC);
    delta = self->field18 - self->field24;
    shift = (delta / 600) * 3;
    if (shift <= 0) {
        return;
    }
    pos = *(PairXY *) &sStyleDecorPosX;
    i = 0;
    if (sStyleDecorVariant == 2) {
        pos.y += 0x1E;
    }
    wp = sStyleDecorSlots;
    srcOfs = 0;
    pos.y += shift * 3;
    do {
        AdjustRgbByDelta(rgb, (u8 *) (srcOfs + sStyleDecorColors), shift);
        obj = (ObjSlotB8B8 *) *wp;
        obj->methods->slotB8(obj, 1, rgb);
        obj = (ObjSlotB8B8 *) *wp;
        i++;
        srcOfs += 3;
        obj->methods->slotBC(obj, &pos);
        pos.y += 3;
        wp++;
    } while (i < 0x12);
    AdjustRgbByDelta(rgb, (u8 *) sStyleClearColor, shift);
    self->methods->slot64(self, rgb);
}
```

`PairXY` is declared in the unit just above `StyleBuildDecorSet`, with the reason
it must be a struct written next to it.

### Proposed learning

**A stall whose stated cause is "register pressure" is an unfinished
diagnosis, and should be re-opened by default.** Both this function and its
sibling carried that verdict across two rounds; in both cases the real cause
was a single named C construct that a one-line change tests. The
discriminator to reach for first: **an unexplained reload -- of a global
already read, or of a stack slot just written, with no call in between -- is
evidence of a BLKmode (struct or array) assignment upstream**, because
`cse.c` handles a BLKmode set by throwing away every cached memory value.

The transfer is the point. The lever was derived on `StyleBuildDecorSet`, where it
took the body from 6 words short to 1 and did not close it; it closed this
one outright, first attempt. **Sibling functions share source idioms, so a
lever found on one is worth spending a build on for every sibling before
anything else is tried** -- and the sibling relationship was already recorded
in both reports ("structurally the sibling of `StyleBuildDecorSet`"), which is what
made this cheap.

## Naming

**`StyleUpdateDecorSet`, tier B.**

Sibling of `StyleBuildDecorSet` (same `sStyleDecorVariant` guard, same
`sStyleDecorSlots` array, same `paramA`/`paramB`-shaped position pair).
Computes a time-based `shift` from an `ObjAC7CSub` object's `field18`/
`field24` delta, then per-element recolors (`AdjustRgbByDelta`) and
repositions (`slotB8`/`slotBC`) all 18 objects every frame. MATCHED,
93/93.

## Round 93 polish (delta, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `600` | `STYLE_DECOR_FADE_HEIGHT` | B | divisor of the viewport's `refView.vp.y - refView.vr.y` (view-point y less reference-point y; which way is up in this world is not established here) giving the fade step. |

The local views were Viewport (`+0x018`/`+0x024` are `refView.vp.y`/`refView.vr.y`; `+0x064` is setClearColor) and BoxFill (`+0x0B8` setColor, `+0x0BC` setPosition). Locals: `height`, `fade`, `colorOfs`, `slot`, `band`.

### Comments moved here from src/world/ObjMStyleActor.c

Verbatim as they stood before the round-93 comment pass (identifiers already carry this round's renames).

```c
/* MATCHED round 61 (bravo), first attempt, after the BLKmode-struct-copy
 * lever found on StyleBuildDecorSet -- see docs/match-reports/StyleUpdateDecorSet.md.
 * `rgb` is written only at [0..2] (by AdjustRgbByDelta); its declared size of 8
 * is inferred from the STACK LAYOUT (it occupies sp+0x10..0x17, with `pos`
 * at sp+0x18), not from any access. */
```
