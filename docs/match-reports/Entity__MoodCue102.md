# Entity__MoodCue102 — MATCH (148/148 words)

> Renamed from `func_800646D8` on 2026-09-24 (tools/rename.py). Address 0x800646d8.

**Unit:** Entity_g · **Size:** 148 instructions

## Blocker screen

```
grep -nE 'gp_rel|addiu *\$at, *\$at, *%lo|nop_mflo_mfhi' asm/nonmatchings/Entity_g/Entity__MoodCue102.s
```

No hits.

## What it does

Another `gEntityMoodHandlerTable` mood-handler row. Standard `out->unk10 = this->methods->
slot148(this);` opener, the established `this->unk80 / 2` signed-halving
idiom compared against `this->unk84`, a threshold gate on `this->unkFC`
picking one of five `D_8008xxxx` row pointers via a cascading `>=` chain,
`this->methods->slot48(this, 1, a2)`, and two more state-code gates on
`this->unkFC`/`this->unk44` at the tail.

## The C

```c
void Entity__MoodCue102(Entity *this, EntityMoodHandlerArg *out) {
    void *a2;

    if (this->unkFC == 0) {
        this->methods->slotCC(this, -0x200, 0);
    }
    out->unk10 = this->methods->slot148(this);
    if (this->unk84 == this->unk80 / 2) {
        out->unk1C = 7;
        out->unk20 = -2;
        out->unk30 = 3;
        out->unk34 = -2;
    }
    if (this->unkFC >= 0x33) {
        this->methods->slot44(this, 0, D_80089CAC);
    }
    if (this->unkFC >= 0x30D) {
        SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
        if (this->unkFC >= 0x790) {
            a2 = D_80089E14;
        } else if (this->unkFC >= 0x78B) {
            a2 = D_80089DE4;
        } else if (this->unkFC >= 0x786) {
            a2 = SCALE_HALF;
        } else if (this->unkFC >= 0x781) {
            a2 = SCALE_QUARTER;
        } else {
            a2 = SCALE_EIGHTH;
        }
        this->methods->slot48(this, 1, a2);
        if (this->unkFC < 0x7D0) {
            this->methods->slotC4(this, -0x40, 0);
        } else {
            this->unk44 = 1;
        }
    } else {
        this->methods->slotC4(this, -0x100, 0);
    }
    if (this->unkF4 != 0 && this->unk44 == 0) {
        this->unk44 = 0xC;
        this->unk94->methods->slot130(this->unk94, 1);
        this->methods->slot30(this, 0xA);
    }
    if (this->unk44 == 0xC) {
        this->unk94->methods->slotC4(this->unk94, 0x100, 0);
    }
}
```

## Residue chased: two swapped row pointers in the cascading chain

First attempt built and scored 146/148, differing only at the LAST two
`addiu %lo(...)` immediates -- `SCALE_EIGHTH` and `SCALE_QUARTER` swapped. Tracing
the `bnez`/delay-slot-recompute chain by hand for the last two arms (each
delay slot recomputes `$v0` for the NEXT comparison down the chain, and the
final arm sets `a2 = SCALE_EIGHTH` BEFORE testing whether to overwrite it with
`SCALE_QUARTER`, i.e. the "set first, conditionally overwrite" idiom applied to
which POINTER a2 ends up holding, not a value) showed the two low-end arms
were reversed from my first reading. Fixed by swapping which arm gets which
row pointer; closed to 148/148 on the second build.

## Struct/table knowledge established

- No new fields; corroborates every field this unit's first function
  (`Entity__MoodCue98`) and the existing `Entity_b.c` siblings already
  established (`unkFC`, `unk44`, `unk80`, `unk84`, `unkF4`, `unk94`).

### Proposed learning

A chain of `bnez`/delay-slot-recompute range comparisons that ends by
setting a default value UNCONDITIONALLY and then conditionally overwriting
it (rather than the more common "test, then branch to one of two
assignments") is easy to get backwards when reading top-to-bottom, because
the LAST arm's default assignment sits BEFORE its own guarding branch in
program order. Trace which value survives the fallthrough vs which
overwrites it explicitly, rather than pattern-matching "assignment then
jump" against every arm uniformly -- the last arm in a cascading chain is
often shaped differently from the ones before it.

## Head-broadcast levers: applicability

Neither lever (negation idiom; dual-based-type array walkers) applies --
this function has no negation and no array walk, only field reads/writes
and vtable dispatch.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g. 2 attempts (one residue,
two swapped row-pointer arms in a cascading range chain).


## Naming

Why `MoodCue102`: the function's address sits in `gEntityMoodHandlerTable`
row 102 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity_b through Entity_f), so the
names sort in table order.

## Data constants decoded this round

`SCALE_EIGHTH` (0x80089E20) and `SCALE_QUARTER` (0x80089DCC), both cascade
arms in this function's `moodTimer`-threshold chain, decoded directly from
`disk/SLPS_015.56` as four s16 `{num,den}` pairs (X/Y(yaw)/Z/W, matching
`SCALE_HALF`/`SCALE_SIX`'s own layout):

- `SCALE_EIGHTH`: `(1,8, 1,8, 1,8, 1,8)` -- uniform X=Y=Z=1/8, the same
  unit-fraction-word convention as `SCALE_HALF` (1/2).
- `SCALE_QUARTER`: `(1,4, 1,4, 1,4, 1,2)` -- uniform X=Y=Z=1/4, W=1/2
  (ignored per the established `SCALE_HALF`/`SCALE_SIX`/
  `ROTATION_YAW_MINUS120` precedent that the 4th pair is never reflected
  in the name).

Also used by `Entity__MoodCue104`/`Entity__MoodCue121` (`SCALE_QUARTER`)
elsewhere in this unit -- same symbol, not redecoded per call site.

## Three constants left unnamed this round

- `D_80089CAC` (`updateRotation` arg, `moodTimer >= 0x33` branch): s16-pair
  decoded `(0,1, -1,3, 0,1, 0,1)` -- only Y nonzero, but -1/3 degree is not
  a whole number, so it does not fit the established `ROTATION_YAW_PLUS2`/
  `ROTATION_YAW_PLUS9`/`ROTATION_YAW_PLUS1` whole-degree convention. No
  fractional-degree rotation constant has a name anywhere in the project
  yet, so inventing one here (`ROTATION_YAW_MINUS_THIRD`-style) would be a
  new naming style, not an application of an existing one.
- `D_80089E14` (`updateScale` arg, `moodTimer >= 0x790` arm): decoded
  `(1,1, 1,1, 1,1, 1,8)` -- uniform X=Y=Z=1/1, i.e. the same VALUE as the
  existing (structurally distinct, different address, 0xC-byte/3-entry)
  `SCALE_ONE` used by `SceneNode__UpdateScale`. Not renamed to `SCALE_ONE`
  or a variant: two differently-addressed, differently-shaped symbols
  sharing one implied meaning is confusing, not clarifying.
- `D_80089DE4` (`updateScale` arg, `moodTimer >= 0x78B` arm): decoded
  `(4,5, 6,5, 5,5, 2,1)` -- X=4/5, Y=6/5, Z=1, not uniform, so it is not
  one of this project's single-ratio `SCALE_*` names.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
