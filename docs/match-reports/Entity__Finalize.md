# Entity__Finalize

> Renamed from `Entity__Destructor` on 2026-09-26 (tools/rename.py). Address 0x8005d1ec.

> Renamed from `func_8005D1EC` on 2026-09-19 (tools/rename.py). Address 0x8005d1ec.

**Unit:** Entity · **Size:** 35 words · **Status:** MATCHED (35/35 words, whole-image build verified byte-exact)

## What it does

Entity's destructor-side teardown: tears down `this->unk100` and
`this->unk104` (each a `Unk100Obj *`, see `Entity__GetOrCreateUnk100.md`) by calling
their own `slot04` (a per-object dtor-like slot, not `EntityMethods`'), then
calls the shared "BasicClass" ancestor's own `dtor` slot,
`Get_vtable_Class65650()->dtor(this)` (offset `+0x00C` in the shared table — see the
big comment in `Entity.h`).

## Final C

```c
void Entity__Finalize(Entity *this) {
    if (this->unk100 != NULL) {
        this->unk100->methods->slot04(this->unk100);
    }
    if (this->unk104 != NULL) {
        this->unk104->methods->slot04(this->unk104);
    }
    Get_vtable_Class65650()->dtor(this);
}
```

## Attempt log

Matched on the first attempt — a straightforward transliteration of the
disassembly (and of m2c's own output, which was already correct here).

## Proposed learning

None new.

## Naming

**Tier A.** Renamed from `func_8005D1EC` this round (tools/rename.py).
Occupies `EntityMethods` dtor slot +0x00C (`tools/classtable.py`), the
direct counterpart to `Entity__Entity`'s ctor slot +0x008. A destructor's
mechanics (tear down the two cached sub-objects, then the shared ancestor's
own `dtor`) are its purpose.

## Track 4 (2026-09-26, round 87, echo)

`unk100` is a `Class6E99C *` and `unk104` a `BasicClass *` (Entity.h);
both calls through their +0x004 are now `release` (BasicClass__Release in
every table). `unk104` is only ever released in Entity code, so it is typed
no further than the slot it reaches. Image byte-identical.

## Track 4 (2026-09-26, round 88, echo)

Renamed from `Entity__Destructor`: the occupant of +0x00C, Class6B5CC's `finalize`. Body: release `unk100` and `unk104`, then Class65650's finalize -- what the slot says. Tier A: an override named for its slot (FINISHING-PLAN track 4 step 6).

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
