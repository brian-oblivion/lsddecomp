# Entity__MoodCue98 — MATCH (48/48 words)

> Renamed from `func_80064618` on 2026-09-24 (tools/rename.py). Address 0x80064618.

**Unit:** Entity_g · **Size:** 48 instructions

## Blocker screen (mandatory)

```
grep -nE 'gp_rel|addiu *\$at, *\$at, *%lo|nop_mflo_mfhi' asm/nonmatchings/Entity_g/Entity__MoodCue98.s
```

No hits. Consistent with the coordinator's measured all-clear for this unit
(all three blocker screens return zero across all 37 functions of the
original `Entity_f`/`Entity_g` segment).

## What it does

One of the mood-dispatch handler rows of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`,
row-aligned at file offset `0x8008A4D0`, `(0x8008A4D0 - 0x80089EB0) / 16 =
98`, confirming this unit's functions occupy that SAME table
`include/Entity.h`'s own comment already documents for `Entity__MoodCue05`/
`Entity__MoodCue10`/etc). Takes the standard `(Entity *this, EntityMoodHandlerArg
*out)` handler signature but never reads `out` in its body -- a genuinely
unused parameter, not a derivation gap (same "declare it, never reference
it" shape CLAUDE.md's own guidance describes for an ignored callee
parameter).

Gated on `this->unkF4 != 0`: if `Entity__GetOrCreateFadeBox(this, NULL, 0, 0xA, 0)`
(the cache-or-create accessor, already matched in `Entity.c`) returns
non-null, dispatches `this->unk100->methods->slotD4` with a literal `7`
(same slot `Entity__MoodCue57` already established with a literal `4`),
`this->methods->slot160`, and a NEW slot on `this->unk94`'s own table
(`+0x21C`, self-only). Unconditionally, at the end: `this->methods->slotC4`
with `(-0x1E, 1)`.

## The C

```c
void Entity__MoodCue98(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkF4 != 0) {
        if (Entity__GetOrCreateFadeBox(this, NULL, 0, 0xA, 0) != 0) {
            this->unk100->methods->slotD4(this->unk100, this->unk50, 7, 0);
            this->methods->slot160(this);
            this->unk94->methods->slot21C(this->unk94);
        }
    }
    this->methods->slotC4(this, -0x1E, 1);
}
```

Matched on the first build.

## Struct/table knowledge established

- `Unk94Methods`: added `slot21C` (`void (*)(Unk94Obj *self)`), a new slot
  past the previous highest (`slot200`).

### Proposed learning

The `gEntityMoodHandlerTable` mood-handler table has (at least) 99 rows -- this unit's
first function alone lands at row index 98, well past the ~20 rows Entity/
Entity_b/Entity_c/Entity_d/Entity_e have matched so far. Every row's first
word is a function taking `(Entity *this, EntityMoodHandlerArg *out)`, but
`out` is not always read -- do not treat an unused `out` parameter as a
signature-derivation problem; declare it anyway for consistency with the
sibling handlers and move on. The row-alignment check
(`(fileoffset - 0x80089EB0) % 16 == 0` against the table's own base in
`asm/data/79528.data.s`) is a fast, cheap confirmation that a queued
function really is one of these handlers before assuming the signature.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g. Matched on the first
build.


## Naming

Why `MoodCue98`: the function's address sits in `gEntityMoodHandlerTable`
row 98 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity_b through Entity_f), so the
names sort in table order.

## Track 4 (2026-09-26, round 87, echo)

`this->unk100` is a `FadeBox *` (include/FadeBox.h); the slot
call through its +0x0D4 is now `startFadeDown` (FadeBox__StartFadeDown), with `companion2`, an
`s32` in Entity.h, cast `(BasicClass *)` as the fade's source (FadeBox's
configure adds it as a child; no code). Image byte-identical.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
