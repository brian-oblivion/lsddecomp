# Entity__DistanceToPeer

> Renamed from `Entity__DistanceToRegion` on 2026-09-26 (tools/rename.py). Address 0x8005d7fc.

> Renamed from `func_8005D7FC` on 2026-09-19 (tools/rename.py). Address 0x8005d7fc.

**Unit:** Entity · **Size:** 26 words · **Status:** MATCHED (26/26 words, whole-image build verified byte-exact)

## What it does

Not an `Entity`/`Entity`-pair function despite living in this unit's vtable
(`gEntityMethods` offset `+0x144`, see the class-table note in `Entity.h`) — its
SECOND argument is an unrelated type (`EntityRegionRef *`, newly named this
round: a flag plus a pointer to an array of 0x38-byte slots, element `[1]`
of which is read). Computes a "distance" between `this`'s 3D position
(`this->unk14`, an `EntityPos *`) and one such slot: `|pos.x - slot.x0| +
|pos.z - slot.z0|` — always the *sum* of the two axis deltas' absolute
values, regardless of which delta is negative (both retail's `bltz`/`j`
branch structure and this reconstruction compute the same closed form either
way, just via different instruction paths per sign).

## Derivation

```
range = (region->flag != 0) ? &region->slots[1] : NULL;    // NULL is safe here per the data, never actually dereferenced with flag==0
dx = |this->unk14->x - range->x0|
dz = this->unk14->z - range->z0
return (dz >= 0) ? (dx + dz) : (dx - dz);
```

## Final C

```c
s32 Entity__DistanceToPeer(Entity *this, EntityRegionRef *region) {
    EntityRegionSlot *range;
    EntityPos *pos;
    s32 dx;
    s32 dz;

    range = NULL;
    if (region->flag != 0) {
        range = &region->slots[1];
    }
    pos = this->unk14;
    dx = pos->x - range->x0;
    if (dx < 0) {
        dx = ~dx + 1;
    }
    dz = pos->z - range->z0;
    return (dz >= 0) ? (dx + dz) : (dx - dz);
}
```

## Attempt log

Two residues, both closed by matching retail's exact bit-level idiom rather
than the "obviously equivalent" one:

1. `if (dx < 0) { dx = -dx; }` compiled to a single `negu` (`subu $a1,$zero,$a1`)
   — retail spells the negation out as `nor $v0,$zero,$a1` /
   `addiu $a1,$v0,1` (two instructions, the textbook two's-complement-by-hand
   form). Writing `dx = ~dx + 1;` explicitly reproduced retail's two-instruction
   form exactly.
2. The final `dz >= 0 ? dx+dz : dx-dz` return needed to be an actual ternary
   expression, not an `if`/`return`/`return` — both are logically identical
   and GCC compiled the `if` form to the exact same (wrong) shape regardless
   of which arm was written first, but the ternary spelling produced
   retail's `bltz`-then-`j`-over-the-other-arm shape on the first try.

## Proposed learning

For a two-armed arithmetic return where BOTH arms are used depending on a
sign test, prefer a ternary (`cond ? a : b`) over `if (cond) return a; return
b;` when the two don't match on the first attempt — they are not
interchangeable to this compiler even though they're semantically identical
C, and swapping which arm comes first in the `if` form did not help where
switching to a ternary did.

## Naming

**Tier A.** Renamed from `func_8005D7FC` this round (tools/rename.py). A
pure leaf: `|this->unk14->x - slot.x0| + |this->unk14->z - slot.z0|`
(retail computes the sum either way regardless of which delta is negative).
A distance calculation's mechanics ARE its purpose.

## Track 4 (2026-09-26, round 88, echo)

Renamed from `Entity__DistanceToRegion`, and its argument retyped from `EntityRegionRef *` to `Class65650 *peer`. Every caller passes `this->peer` (the player), and the old view's fields were SceneNode's: `flag` (+0x0C) is `parent`, `slots` (+0x14) is `coord2`, and `slots[1]` (+0x38 into it) is `coord2->unk38`, the world position. So the body is |dx| + |dz| between this entity's translation (`coord2->tx/tz`) and the peer's world position, taken only when the peer has a parent. EntityRegionRef/EntityRegionSlot are deleted. Slot +0x144 renamed `distanceToPeer`. Tier A: the mechanics are the body.

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
