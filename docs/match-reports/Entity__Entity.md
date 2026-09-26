# Entity__Entity

**Unit:** Entity · **Size:** 40 words · **Status:** MATCHED (40/40 words, whole-image build verified byte-exact)

## What it does

Entity's constructor, reached both directly (this function) and indirectly
through the vtable's own `ctor` slot (`Get_vtable_Entity()->ctor`, see
`New_Entity`). First calls the shared base-class constructor,
`Get_vtable_Class65650()->ctor(this, arg2, arg3)` — `Get_vtable_Class65650()` (matched in
`code_55dd4.c`) returns the SAME shared "BasicClass" ancestor vtable that
`Class65650` also derives from (see the big comment at the top of
`include/Entity.h`). Only on success does it finish initializing: assigns
`this->methods` to `Get_vtable_Entity()` (Entity's OWN vtable — the base
ctor call above runs before this entity is "really" an Entity), stores
`arg1` into `moodIndex`, zeroes `unk9C`/`unk100`/`unk104`, then calls its own
(now-current) `methods->slot40(this)`.

Note `arg1` (`moodIndex`) is *not* forwarded to the base ctor — only `arg2`
and `arg3` are. `arg1` is purely an Entity-level field.

## Derivation

The base-ctor's slot lives at `+0x008` in the shared vtable, the SAME offset
`EntityMethods` uses for its own `ctor` field — consistent with the project's
established "ctor at +0x008, dtor at +0x00C universally" base-class
convention (`docs/research/class-framework.md`).

## Final C

```c
Entity *Entity__Entity(Entity *this, s32 arg1, s32 arg2, s32 arg3) {
    if (Get_vtable_Class65650()->ctor(this, arg2, arg3) != NULL) {
        this->methods = Get_vtable_Entity();
        this->moodIndex = arg1;
        this->unk9C = 0;
        this->unk100 = NULL;
        this->unk104 = NULL;
        this->methods->slot40(this);
        return this;
    }
    return NULL;
}
```

## Attempt log

First attempt (`if (ctor(...) == NULL) { return NULL; } ...rest...; return
this;`) matched retail's SIZE (0xA0) but not its bytes: retail's
`beqz $v0,END` (on ctor failure) fills its delay slot with `move $v0,zero`
and the success path ends with an explicit `move $v0,$s2` right before
falling into the shared epilogue; the early-return-first form instead
produced `nop` in that delay slot plus an extra unconditional jump to skip
over the success path's own `move $v0,$s2` — same total instruction count,
wrong shape.

**What matched:** the OPPOSITE polarity — testing for SUCCESS first (`if
(ctor(...) != NULL) { ...; return this; } return NULL;`), with the trailing
`return NULL;` as the true fallback. This flipped which branch is the
"direct" fallthrough vs. the branch target, and produced the exact
delay-slot-optimized shape retail has.

## Proposed learning

Confirms and sharpens `New_Entity.md`'s finding: for a "guard-then-init"
function with **exactly one gate check and no `else` branch of substance**,
try BOTH polarities (`if (cond) { ...; return success; } return failure;` vs.
`if (!cond) return failure; ...; return success;`) when the delay-slot/branch
shape doesn't fall out on the first try — GCC 2.6.3 at `-O2` picks a
different one of the two as the "direct" (non-branching) path depending on
which the source states first, and only one of the two matches retail's
choice.

## Naming

**Tier A.** Occupies `EntityMethods::ctor` (+0x008, `tools/classtable.py`).
A constructor's mechanics are its purpose; matches the `Class__Class`
convention already used by `Class65650__Class65650`/`DreamSys__DreamSys`.
Not renamed (already correct).

## Track 4 (2026-09-26, round 88, echo)

Parameters retyped to (moodIndex, desc, arg2), the ctor slot's CtorParams; `desc`/`arg2` go to Class65650's ctor without casts. `soundCueSet = 0` is now `soundCueSet.tag = 0` (the SoundCueSet is embedded).

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
