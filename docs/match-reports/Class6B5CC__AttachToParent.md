# Class6B5CC__AttachToParent

> Renamed from `func_8001D0EC` on 2026-09-23 (tools/rename.py). Address 0x8001d0ec.

**Unit:** code_d294 · **Size:** 46 words · **Status:** MATCHED (46/46 words)

## What it does

`Class6B5CC` vtable slot `+0x04C`, the "attach" half of an attach/detach
pair with `Class6B5CC__DetachFromParent` (`+0x050`, this unit's own report). If
`self->unkC` is already set, does nothing and returns `self` unchanged
(already attached). Otherwise: stores `obj` into `self->unkC`, copies
`obj->unk14` into `self->unk14->unk48`, calls `obj->methods->slot10(obj,
self)` (the owner's own "register a child" hook), then either copies an
optional 3rd argument (`vec`) into `self->unk14->unk18/unk1C/unk20`, or
zeroes those three fields if `vec` is `NULL`, and finally clears
`self->unk14->unk0`.

## The C

```c
Class6B5CCObj *Class6B5CC__AttachToParent(Class6B5CCObj *self, UnkOwner_d294 *obj, Vec3_d294 *vec) {
    Class6B5CCSub14 *sub;

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
etc. directly (no local), by analogy with `Class6B5CC__Class6B5CC`'s round-1
lesson ("don't cache a re-read field across a call, re-derive it fresh
each time"). That produced THREE separate reloads of `self->unk14`
(retail has three too, but only where a call actually intervenes) and, in
one case, extra reload instructions retail doesn't have. The fix was
caching `self->unk14` into a local `sub` and REUSING it across the
`unk18`/`unk1C`/`unk20` triple (no call between those three stores), while
still reloading `sub = self->unk14` fresh right after the ONE call that
does intervene (`obj->methods->slot10`). **This is not a contradiction of
`Class6B5CC__Class6B5CC`'s lesson -- it's the same rule read in both directions:**
cache within a call-free span, reload after a call. `Class6B5CC__Class6B5CC`'s
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

round 11 (2026-09-03), runner charlie, unit code_d294, second pass. 3 build iterations (initial
attempt, then one fix per residue above). Established `UnkOwner_d294`/
`UnkOwnerMethods_d294` (the attach/detach target class, `+0x010`
ctor-like/`+0x014` dtor-like slots) and `Class6B5CCSub14`'s
`unk18`/`unk1C`/`unk20` Vec3 fields, and retyped `Class6B5CCObj::unkC`
from an untyped `void *` to `UnkOwner_d294 *` (see header comment on that
field for the cross-check with `Class6B5CC__DetachFromParent`).
