# Entity__StopSoundCue

> Renamed from `func_8005DB8C` on 2026-09-19 (tools/rename.py). Address 0x8005db8c.

**Unit:** Entity · **Size:** 25 instructions · **Status:** MATCHED (25/25 words, whole-image build verified byte-exact)

## What it does

Calls a still-uncarved function, `FlushSoundCueSet(this->unk58, &this->unk9C)`
(the same two-argument shape `Entity__TickSoundCue` uses with `ServiceSoundCueSet` — see
that report), then calls this entity's own vtable slots `+0x130` and
`+0x114` (both no-argument), and clears `this->unkF8`.

**Updated in round 2026-09-01 (runner bravo, Entity 11-function pass):**
`this->unk9C` changed from a `u8[]` array to a plain `s32` (see
`Entity__TickSoundCue.md`'s update note); this call site's array-decay
`this->unk9C` became an explicit `&this->unk9C`, same compiled address.
Re-verified byte-exact.

## Derivation

```
lw   $a0, 0x58($s0)
jal  FlushSoundCueSet
 addiu $a1, $s0, 0x9C
lw   $v0, 0x0($s0)
lw   $v0, 0x130($v0)
jalr $v0                     ; this->methods->slot130(this)
 move $a0, $s0
lw   $v0, 0x0($s0)
lw   $v0, 0x114($v0)
jalr $v0                     ; this->methods->slot114(this)
 move $a0, $s0
sw   $zero, 0xF8($s0)        ; this->unkF8 = 0
```

## Final C

```c
void Entity__StopSoundCue(Entity *this) {
    FlushSoundCueSet(this->unk58, &this->unk9C);
    this->methods->slot130(this);
    this->methods->slot114(this);
    this->unkF8 = 0;
}
```

`FlushSoundCueSet` declared `extern void FlushSoundCueSet(s32 arg0, void *arg1);`
in `entity.h`, same rationale as `ServiceSoundCueSet` in `Entity__TickSoundCue.md`.
`slot130`/`slot114` typed `void (*)(Entity *self)` in `EntityMethods`.

## Attempt log

Matched on the first attempt.

## Proposed learning

None new beyond what's already in `Entity__TickSoundCue.md` about the
`ServiceSoundCueSet`/`FlushSoundCueSet` pairing.

## Naming

**Tier B.** Renamed from `func_8005DB8C` this round (tools/rename.py).
Calls `FlushSoundCueSet` on the same `(soundCueChannel, &soundCueSet)` pair,
two self-only slot calls, clears `this->unkF8`. Exact mirror of
`Entity__StartSoundCue`.

## Proposed field names

- `EntityMethods::slot16C` -> `stopSoundCue` -- **tier B.** `tools/
  classtable.py` resolves +0x16C to this very function. CROSS-UNIT: called
  by `Entity__Deactivate` (consistent: deactivating stops the sound cue),
  `Entity__UpdateSoundCueStop` (entity.c) and `Entity__MoodCue77` (Entity_e.c). Proposed
  rather than applied.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
