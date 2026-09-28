# Entity__MoodCue123 — MATCH (77/77 words)

> Renamed from `func_80065238` on 2026-09-24 (tools/rename.py). Address 0x80065238.

**Unit:** Entity_g · **Size:** 77 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. `rand() % 5 == 0` gate on `unk44`
when `unkFC==0`; `slot130`/`slotC4(0x64,0)` unconditionally;
`slot16C`/`unk44=1` on `unkFC==0x3E8`; and, when `unk44==0xB &&
unkFC>=0x12D`, two calls to a NEW `Unk94Obj` slot (`+0x94`).

## The C

```c
void Entity__MoodCue123(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        if (rand() % 5 == 0) {
            this->unk44 = 0xB;
        }
    }
    this->methods->slot130(this);
    this->methods->slotC4(this, 0x64, 0);
    if (this->unkFC == 0x3E8) {
        this->methods->slot16C(this);
        this->unk44 = 1;
    }
    if (this->unk44 == 0xB) {
        if (this->unkFC >= 0x12D) {
            this->unk94->methods->slot94(this->unk94, 0, 2);
            this->unk94->methods->slot94(this->unk94, 0, 7);
        }
    }
}
```

Matched on the first build.

## Struct/table knowledge established

- `Unk94Methods`: added `slot94` (`(Unk94Obj *self, s32 arg1, s32 arg2)`),
  called twice here with the same `arg1=0`, different `arg2` (2, 7).

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g. Matched on the first
build.


## Naming

Why `MoodCue123`: the function's address sits in `gEntityMoodHandlerTable`
row 123 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity through Entity_f), so the
names sort in table order.

**This handler also occupies row 126** of `gEntityMoodHandlerTable` (same
`handler` word at both `0x80089EB0+0x10*123` and `0x80089EB0+0x10*126`;
the row's other three words -- data0/data1/data2 -- differ between the two
rows, so it is one function shared by two distinct mood-row
configurations, not a naming collision). Named for its lower/first row per
the existing convention (Entity__MoodCue81, Entity_e round 59/77); not a
second name.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-26, round 94, alpha)

The two `onPadEvent(peer, 0, 2)` / `(0, 7)` calls carry a comment: in
`DreamSys__OnPadEvent` (`src/DreamSys.c`) event 2 sets
`MOVE_COMMAND_FORWARD` and event 7 switches to `MOVE_MODE_RUN` while moving
forward. No pad-event enum exists; one in `include/DreamSys.h` would also
change `DreamSys.c`, so it is proposed rather than added.
