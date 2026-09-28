# Entity__StartSoundCue -- MATCHED (36/36 words)

> Renamed from `func_8005DAFC` on 2026-09-19 (tools/rename.py). Address 0x8005dafc.

**Unit:** Entity · Runner: charlie, round 23.

## What it does

Lazily-flavoured "start" call: initializes a slot table via `InitSoundCueSet`
(matched in `PlacementGridVabSound.c`), passing that unit's own `this->unk58`, the
address of `this->unk9C` as the object to init, `this->moodIndex + 1` as
`arg2`, `this` itself as `arg3`, and the current mood row's dispatch-handler
function pointer (`gEntityMoodHandlerTable[this->moodIndex].handler`, first word of the
16-byte `gEntityMoodHandlerTable` row) as `arg4`. Then calls two more self-only vtable
slots and resets a pair of counters.

## Final C

```c
typedef struct EntityMoodHandlerRow EntityMoodHandlerRow;
struct EntityMoodHandlerRow {
    void *handler; /* +0x00 */
    u8 pad04[0x10 - 0x04];
};
extern EntityMoodHandlerRow gEntityMoodHandlerTable[];
extern void InitSoundCueSet(s32 arg0, void *arg1, s32 arg2, Entity *arg3, void *arg4);

void Entity__StartSoundCue(Entity *this) {
    InitSoundCueSet(this->unk58, &this->unk9C, this->moodIndex + 1, this,
                  gEntityMoodHandlerTable[this->moodIndex].handler);
    this->methods->slot12C(this);
    this->methods->slot110(this);
    this->unkFC = 0;
    this->unkF8 = 1;
}
```

Byte-exact on the first attempt, whole-image build verified.

## Notes

`InitSoundCueSet`'s own match report (in `PlacementGridVabSound.c`) already established
its real signature; DreamSys.c's own extern (`InitSoundCueSet(s32, void *,
s32, DreamSys *, void *)`) is the same function called with a different
unit's own local `arg3` type -- per project convention, this file's own
extern types `arg3` as `Entity *` instead, matching the "multiple
independent local views" rule for cross-unit prototypes (kept local to this
`.c`, not added to `Entity.h`, since `InitSoundCueSet` is defined in a
different unit).

`gEntityMoodHandlerTable` is the mood-index-selected event-dispatch table (16-byte rows:
handler fn ptr + 3 data words) already referenced by name in several match
reports for `Entity`'s handler functions (e.g. `Entity__MoodCue05`), but this
is the first place any unit indexes the RAW TABLE itself in C rather than
just being one of its handler bodies. Declared a minimal
`EntityMoodHandlerRow` (only the first word named) directly in `Entity.c`,
not `Entity.h` -- the only field this function needs is the handler pointer,
treated opaquely (passed straight through to `InitSoundCueSet` as a `void *`,
never called here). Whoever carves `Entity__UpdateTargetProximity` (the actual table
dispatcher, still in the uncarved `Entity`) should check whether this
minimal row type is enough or needs the data words added, and should
probably promote it to `Entity.h` at that point since it would then have
two real users.

Two new `EntityMethods` vtable slots added, both self-only `void`:
`slot110` (this function's second call) and `slot12C` was already
documented. `slot110`'s only known caller is this function.

### Proposed learning

None beyond what's already documented -- this one was a clean, mechanical
translation with no residue. The round-23 head broadcast's three levers
(s16-local widening, sltiu-implies-unsigned-counter, subscript-vs-pointer
addressing) do not apply here: no `s16` locals, no loop, no `&arr[i+j]`
shape.

## Naming

**Tier B.** Renamed from `func_8005DAFC` this round (tools/rename.py).
Calls the already-matched, already-named `InitSoundCueSet` (PlacementGridVabSound.c)
on the `(this->soundCueChannel, &this->soundCueSet)` pair (both renamed
this round), then two self-only slot calls, then resets `this->unkFC` and
sets `this->unkF8 = 1`. Pairs with `Entity__StopSoundCue`.

## Proposed field names

- `Entity::unkF8` -> `soundCueActive` -- **tier B.** Set here, cleared by
  `Entity__StopSoundCue`; read directly by Entity.c
  (`grep -rn -- '->unkF8\b' src/world/Entity.c`). CROSS-UNIT, proposed rather
  than applied.
- `EntityMethods::slot168` -> `startSoundCue` -- **tier B.** `tools/
  classtable.py` resolves +0x168 to this very function (self-referential
  dispatch). CROSS-UNIT: called by `Entity__UpdateSoundCueStart` (Entity.c), whose own
  gate (`this->unkF8 == 0`, i.e. sound cue not yet active) is exactly
  consistent with "start the sound cue when a proximity condition fires."
  Proposed rather than applied.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Polish (round 96, bravo, track 7)

- Step 2: The flat mood-row "tables" this body read are columns of gEntityMoodTable's
16-byte row (their symbols are the row base 0x80089EA4 plus the column
offset: gEntityUnlockKindTable +0x02, gEntityLinkStageTable +0x07,
gEntityEventVideoTable +0x08, gEntityProximityThresholdTable +0x0A,
gEntityMoodHandlerTable +0x0C), now EntityMoodRow fields; byte-identical.
Entity.h's old claim that they were "SEPARATE global arrays (own base
symbols, own lui/addiu) ... not sub-fields of the gEntityMoodTable row" was
wrong: GCC spells a constant-offset field of a global array as
%hi/%lo(sym + off), which splat labels as its own symbol.

- Step 2: the local EntityMoodHandlerRow view (a handler word padded to 16 bytes, over gEntityMoodHandlerTable) is deleted: the handler is EntityMoodRow::handler (+0x0C, SoundCueCallbackFn). The local InitSoundCueSet extern now spells PlacementGridVabSound.c's definition (VabStreamObj *, SoundCueSet *, SoundCueCallbackFn, s32 return); byte-identical.
