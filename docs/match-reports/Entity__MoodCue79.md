# Entity__MoodCue79 -- MATCHED (58/58 words)

> Renamed from `func_80062FAC` on 2026-09-24 (tools/rename.py). Address 0x80062fac.

Unit: `Entity_e` (round 13). Dispatches `slot148`, checks `out->unk4 % 10`,
unconditionally fires `slot44` with the same `ROTATION_YAW_PLUS2` table other units
already reference, then a one-shot `slot30`/`unk44` latch gated on
`unk44 == 0 && unkF4 != 0`.
`void Entity__MoodCue79(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue79(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 0x19;
        out->unk20 = 2;
    }
    this->methods->slot44(this, 0, ROTATION_YAW_PLUS2);
    if (this->unk44 == 0 && this->unkF4 != 0) {
        this->methods->slot30(this, 0xB);
        this->unk44 = 0xB;
    }
}
```

## Derivation notes

- `out->unk4 % 10 == 0` is the magic-constant idiom for divisor 10
  (`0x66666667`, shift 2 after `mfhi`) -- verified against the pinned `cc1`
  with `int f(int x){return x/10;}`, byte-for-byte, continuing the
  shift-fingerprints-the-divisor technique from `Entity__MoodCue69`'s report.
- The `slot44(this, 0, ROTATION_YAW_PLUS2)` call is UNCONDITIONAL: it sits between
  the `% 10` check and the `unk44`/`unkF4` gate, not inside either `if`. The
  `a0 = this` set up in the `bne`'s delay slot at the end of the `% 10`
  branch survives untouched across the branch-not-taken path because
  nothing else writes `$a0` before the call, which is why the call reads as
  reachable from both arms of the first `if` in the disassembly -- it is
  simply after it, not conditioned by it.
- `this->unk44 == 0 && this->unkF4 != 0` is a short-circuit `&&`: the first
  `bnez` skips the whole block when `unk44 != 0`, the second `beqz` skips it
  when `unkF4 == 0`, both landing on the same `.L8006307C` exit label --
  standard back-to-back guard clauses, not two independent `if`s (there is
  no code between them for a first `if` to fall through into).
- `ROTATION_YAW_PLUS2` already has externs and a callsite (`slot44(this, 0,
  ROTATION_YAW_PLUS2)`) in `Entity_b.c`/`Entity_c.c`; this unit gets its own
  per-unit `extern u8 ROTATION_YAW_PLUS2[];` per the file's existing convention
  (documented at the top of the data-table externs already in this file),
  not a shared header declaration.

No new struct or vtable-slot knowledge; `slot148`, `slot44`, `slot30`,
`unk44`, `unkF4`, and all touched `EntityMoodHandlerArg` fields were already
known.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 79 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity_d/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_d.

Reading this function's `out->` writes with the proposed `SoundCueSet` field names (`Entity__MoodCue07.md` `## Proposed field names`, tier A/B, proposal only -- `EntityMoodHandlerArg` is shared with Entity_b/Entity_d/Entity_g): `tick`, `attenuation`, `voice0Tone`, `voice0Pitch`.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, echo)

Every literal in the live body is in its base: decimal for moodTimer and cue-set ticks, TOD frames, distances, VAB programs and `state` phases (the unit has no hex literal left). notifyParents' 11 is ENTITY_EFFECT_EVENT_VIDEO. Byte-identical (whole image green).
