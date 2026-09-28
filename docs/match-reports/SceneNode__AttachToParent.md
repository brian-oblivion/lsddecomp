# SceneNode__AttachToParent

> Renamed from `Class6B5CC__AttachToParent` on 2026-09-26 (tools/rename.py). Address 0x8001d0ec.

> Renamed from `func_8001D0EC` on 2026-09-23 (tools/rename.py). Address 0x8001d0ec.

**Unit:** SceneNode · **Size:** 46 words · **Status:** MATCHED (46/46 words)

## What it does

`SceneNode` vtable slot `+0x04C`, the "attach" half of an attach/detach
pair with `SceneNode__DetachFromParent` (`+0x050`, this unit's own report). If
`self->unkC` is already set, does nothing and returns `self` unchanged
(already attached). Otherwise: stores `obj` into `self->unkC`, copies
`obj->unk14` into `self->unk14->unk48`, calls `obj->methods->slot10(obj,
self)` (the owner's own "register a child" hook), then either copies an
optional 3rd argument (`vec`) into `self->unk14->unk18/unk1C/unk20`, or
zeroes those three fields if `vec` is `NULL`, and finally clears
`self->unk14->unk0`.

## The C

```c
SceneNodeObj *SceneNode__AttachToParent(SceneNodeObj *self, UnkOwner_d294 *obj, LongVec3 *vec) {
    SceneNodeSub14 *sub;

    if (self->unkC == NULL) {
        self->unkC = obj;
        sub = self->unk14;
        sub->unk48 = obj->unk14;
        obj->methods->slot10(obj, self);
        sub = self->unk14;
        if (vec != NULL) {
            sub->unk18 = vec->x;
            sub->unk1C = vec->y;
            sub->unk20 = vec->z;
        } else {
            sub->unk18 = 0;
            sub->unk1C = 0;
            sub->unk20 = 0;
        }
        self->unk14->unk0 = 0;
    }
    return self;
}
```

## Two residues, two fixes -- worth generalizing together

**1. Cache a re-read field ONLY across the span with no intervening call;
reload after each call.** First attempt used `self->unk14->unk44 = ...`
etc. directly (no local), by analogy with `SceneNode__SceneNode`'s round-1
lesson ("don't cache a re-read field across a call, re-derive it fresh
each time"). That produced THREE separate reloads of `self->unk14`
(retail has three too, but only where a call actually intervenes) and, in
one case, extra reload instructions retail doesn't have. The fix was
caching `self->unk14` into a local `sub` and REUSING it across the
`unk18`/`unk1C`/`unk20` triple (no call between those three stores), while
still reloading `sub = self->unk14` fresh right after the ONE call that
does intervene (`obj->methods->slot10`). **This is not a contradiction of
`SceneNode__SceneNode`'s lesson -- it's the same rule read in both directions:**
cache within a call-free span, reload after a call. `SceneNode__SceneNode`'s
residue happened to need re-deriving because a call sat between the two
uses; this one needed caching because no call did. Read the disassembly's
own reload points as the ground truth for where the SOURCE re-mentions
the value, rather than applying "always reload" or "always cache" as a
blanket rule.

**2. A guarded early `return self;` that duplicates a later, identical
`return self;` costs an instruction if the compiler can't share the
epilogue.** First attempt wrote `if (self->unkC != NULL) { return self; }`
followed later by the real `return self;` at the function's end. Retail
computes `v0 = self` exactly ONCE, at the single label both the early-exit
branch and the normal fall-through path land on. My two-early-return
version computed it twice (once per return site) because GCC 2.6.3 didn't
merge the two identical tail sequences on its own. The fix: wrap the
entire guarded body in `if (self->unkC == NULL) { ... }` and leave exactly
ONE `return self;` at the end, so there is only ever one place that
produces the return value for GCC to schedule. Suspect this generalizes:
prefer a single trailing return over an early return whose value is
identical to a later one, when a residue looks like one redundant `move`
right before an epilogue.

## Provenance

round 11 (2026-09-03), runner charlie, unit SceneNode, second pass. 3 build iterations (initial
attempt, then one fix per residue above). Established `UnkOwner_d294`/
`UnkOwnerMethods_d294` (the attach/detach target class, `+0x010`
ctor-like/`+0x014` dtor-like slots) and `SceneNodeSub14`'s
`unk18`/`unk1C`/`unk20` Vec3 fields, and retyped `SceneNodeObj::unkC`
from an untyped `void *` to `UnkOwner_d294 *` (see header comment on that
field for the cross-check with `SceneNode__DetachFromParent`).

## Naming

Round 71 (alpha). `func_8001D0EC` -> `SceneNode__AttachToParent`, **tier A**. Table slot +0x04C. Only when not already attached: stores the parent in self->unkC, sets coord2->super to the parent's coordinate, calls parent->addChild(self) (BasicClass slot +0x010), copies the optional translation into coord2 (coord.t, +0x18..+0x20) or zeroes it, flg = 0. class_3bb8c_s independently calls this slot `attachToParent`.

## Proposed field names

For the head to apply by type scope. Each one fails to compile in another unit when renamed in the definition, so this unit did not apply it.

- `SceneNodeObj.unkC` -> `parent` (tier A): AttachToParent stores the object it then calls addChild on; DetachFromParent calls removeChild on it and clears it. Accessors: SceneNode, code_d294_b, code_d294_c.
- `SceneNodeSub14.unk18/unk1C/unk20` -> `tx/ty/tz` (tier A): GsCOORDINATE2.coord.t[0..2] (+0x04 + 0x14), written from AttachToParent's translation argument. Accessors: SceneNode, code_d294_c.

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.

## Round 101 (delta): track 7

Step 3 (locals and parameters): `obj` -> `parent`, `vec` -> `offset` (it becomes coord2->coord.t, the offset from the parent), `sub` -> `coord2`. Byte-identical.

Step 5 (comments): Function comment added; `MATCHING:` on the `coord2` local reloaded after addChild, measured this pass (writing `self->coord2->...` throughout breaks the build).
