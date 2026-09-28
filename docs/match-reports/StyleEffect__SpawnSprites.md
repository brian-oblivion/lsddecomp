# StyleEffect__SpawnSprites -- MATCHED (54/54 words)

> Renamed from `Class876FC__SpawnSprites` on 2026-09-26 (tools/rename.py). Address 0x80056d18.

> Renamed from `func_80056D18` on 2026-09-23 (tools/rename.py). Address 0x80056d18.

**Unit:** class_3bb8c_s · **Round:** 44 (2026-09-15)

## Provenance

Round-42's "REOPENED -- ASSIGNABLE" banner applies: stub-stalled as
`gp_rel`-blocked on `gStyleEffectTim`, already present in
`config/gp-symbols.txt`. Matched byte-exact this round, but only after
finding and undoing a self-inflicted whole-image size regression (below).

## What it is

Populates all 5 slots of `self->arr84` (already known from
`class_3bb8c_o.c`'s `LinkOwnerObj::arr84`) with freshly-allocated nodes via
`New_VariantSprite`, wires each one to `self` through `slot4C`, forwards
`self->unk74` through `slotB8`, and — only when the caller passed a non-NULL
`tbl` — also calls each new node's `slot48` with it.

## C

```c
void StyleEffect__SpawnSprites(void *self, s32 a1, s32 a2, void *tbl) {
    LinkNode **p = (LinkNode **)((u8 *)self + 0x84);
    LinkNode *node;
    s32 i;

    for (i = 0; i < 5; i++, p++) {
        node = (LinkNode *)New_VariantSprite(a2, 0, gStyleEffectTim);
        *p = node;
        node->methods->slot4C(node, self, 0);
        (*p)->methods->slotB8(*p, ((LinkNode *)self)->unk74);
        if (tbl != 0) {
            (*p)->methods->slot48(*p, 1, tbl);
        }
    }
}
```

`self`'s type (`void *`) and the unused `a1` parameter match the
already-established forward declaration at the top of this file (used by
`StyleEffect__BuildRandomSprites`'s existing call, `StyleEffect__SpawnSprites(self, 0, 0, tblOrNull)`), so
no signature change was needed anywhere else.

## A whole-image SHA1 failure with no compile error and no in-range diff

The FIRST attempt at this function used `LinkNode *sn = self;` at the top
(to avoid repeating `((LinkNode *)self)->unk74` inline) and passed `sn->unk74`
to `slotB8`. That version compiled cleanly, `funcdiff.py` reported large,
scattered residues in `StyleEffect__InitByKind` (5 words) and `StyleEffect__UpdateByKind` (4
words) — TWO OTHER, already-believed-correct functions in this same unit —
and `StyleEffect__SpawnSprites` ITSELF showed near-total misalignment starting from word
0 (`addiu $sp,$sp,-0x28` vs `-0x30`, a bigger stack frame).

Per CLAUDE.md's own recipe for exactly this signature (`cmp -l` +
`build/lsdde.map`):

```
cmp -l build/SLPS_015.56 disk/SLPS_015.56 | head
#  8073 304 270   (1-based; vram = (8073-1) - 0x800 + 0x80010000 = 0x80011788)
grep -n 80011788 build/lsdde.map
#  .rodata  0x80011788  0xe4  build/src/DreamSys.c.o
```

The build was also 12 bytes (505868 vs 505856) LARGER than retail overall.
Since `.rodata` links before `.text` project-wide, a 12-byte GROWTH anywhere
early enough shifts every subsequent address, including unrelated units'
`.rodata` — which is exactly what the map showed, even though nothing in
this session touched `DreamSys.c.o` or its rodata. The actual cause was
local: `LinkNode *sn = self;` added ONE extra callee-saved register to
`StyleEffect__SpawnSprites`'s own prologue (bigger `-0x30` frame vs retail's `-0x28`),
growing THIS function by exactly 12 bytes/3 words and cascading forward
through the whole link. Removing the alias and casting `self` inline at
each use dropped the frame back to `-0x28` and the function, and the whole
image, matched byte-exact on the next build.

### Proposed learning

**This is the THIRD independent confirmation of MATCHING-GUIDE.md's
"adding or removing any local variable can renumber every saved register"
finding** (previously: `StyleEffect__DriftModelChildren`'s own report, this unit; a second,
unrelated function elsewhere) — but with a new twist worth recording
explicitly: the growth doesn't have to just *renumber* registers, it can
add a WHOLE EXTRA saved register/stack slot, which changes the function's
BYTE LENGTH, not just its register identities. A length change is a strictly
worse failure signature than a same-length residue, because it shows up as
scattered, plausible-looking diffs in *other, unrelated, already-matched*
functions rather than as an obvious diff in the function that actually
grew — exactly the "conflicted merge"/"struct edit" class of failure
CLAUDE.md documents, except the trigger here was a local variable choice
inside a single function, not a struct edit or a merge. The tell was the
same as CLAUDE.md's own recipe: a whole-image SHA1 failure with a clean
compile and no useful signal from `funcdiff.py`'s own per-function warning
alone — `cmp -l` plus the map turned it into an exact byte-length culprit in
under a minute. Recommend generalizing the existing hazard note (currently
scoped to struct edits) to also cover "a from-scratch local variable that
merely aliases an existing pointer/value for readability."

## Naming

Round 70 (alpha). `func_80056D18` -> `StyleEffect__SpawnSprites`, **tier B**.

Two callers: StyleEffect__BuildRandomSprites (tbl = gSpriteScaleHalf or NULL)
and class_3bb8c_o.c's StyleEffect__SpawnPlainSprites (tbl = NULL, kind 3). Body:
five `New_VariantSprite(a2, 0, gStyleEffectTim)` into +0x084, each attachToParent(self,
no offset), slotB8(self->color), and updateScale(1, tbl) when tbl != NULL.

Why "sprites": in D800879C4's table (tools/classtable.py gVariantSpriteMethods) slots
+0x060/+0x064/+0x068 set bits 31/30/28-29 of a word at +0x064 (GsDOFF,
GsALON, semitrans rate: LIBGS.H:303-308), slot +0x0B8 (Sprite__SetColor)
copies three bytes to +0x078..+0x07A and slot +0x044 writes +0x084. Those
are GsSPRITE's attribute, r/g/b and rotate offsets (LIBGS.H:111-122) for a
GsSPRITE embedded at +0x064; StyleEffect__RandomizeSprites' `angle` at +0x084
is the same rotate. The D800879C4 class itself is still unnamed, so B.

## Track 4 (2026-09-26, round 87)

The sprites' class is unified as VariantSprite (`include/VariantSprite.h`,
formerly `D_800879C4` / `New_D800879C4`). This unit's local
`extern void *New_D800879C4(void *, void *, void *)` was a view of it and
is gone; the header's `VariantSprite *New_VariantSprite(s32 variant, void *arg2,
void *texture)` is used instead, so `a2` is passed as the variant without
the `(void *)` cast and the result is cast to this unit's `LinkNode *`
(pointer casts, no code; byte-identical).

## Track 4 (2026-09-26, round 88, charlie)

class_3bb8c_s.c's `LinkNode` view (owner and children under one type) is gone: the owner is `StyleEffect` (include/StyleEffect.h), `modelChildren` are `Actor *`, `sprites` are `VariantSprite *`, and the local `Vec3S` is `LongVec3`. Accessor renames: `kind` is Actor's `pendingExtra` (+0x054, where the ctor stores it); `offset`/`rotation`/`scale`/`modelChildLayout`/`tableIndex`/`color`/`altColor` are `params.*`; slot `slotB8` is Actor's `setTranslation` on the owner and model children and Sprite's `setColor` on sprites; `*coord2 = 0` is `coord2->flg = 0`. Image byte-identical.

## Track 6 (round 93, bravo)

D_8008ACA8 -> gStyleEffectTim (rename.py): ObjM's ETC.TIM, stored by
SetStyleEffectSources and handed to New_VariantSprite as its image.

## Naming (track 7, round 101)

- Locals: `p` -> `slot`, `node` -> `sprite`. Loop bound `5` is
  `ARRAY_COUNT(((StyleEffect *)self)->sprites)`.
