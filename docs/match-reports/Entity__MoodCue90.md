# Entity__MoodCue90

> Renamed from `func_80063E68` on 2026-09-25 (tools/rename.py). Address 0x80063e68.

**Unit:** Entity_f · **Size:** 27 words · **Status:** MATCHED (27/27 words)

## What it does

`out->unk10 = this->methods->slot148(this); if (out->unk4 == 0) {
out->unk1C = (rand() & 1) ? 2 : 1; out->unk20 = 2; }`.

## Derivation

Direct transcription. No residue.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue90` | B | `gEntityMoodHandlerTable` row 90 |

Why `MoodCue90`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 90 (base 0x80089EB0, stride 0x10; the row's
first word), read from `disk/SLPS_015.56` directly rather than inferred from
address order (rounds 76-77 measured that row order does not track code
address). Nothing else references it. `Entity__StartSoundCue` hands the row's
handler to `InitSoundCueSet`, and `ServiceSoundCueSet` calls it once per tick
as `callback(owner, set)`, so `out` is the `SoundCueSet` (`EntityMoodHandlerArg`
is Entity.h's local view; field readings in `Entity__MoodCue07.md`
`## Proposed field names`: `unk4` tick, `unk10` attenuation, `unk1C`/`unk30`/`unk44`
voice 0/1/2 tone request (-2 = stop), `unk20`/`unk34`/`unk48` pitch offset).
Tier B, same as every sibling `Entity__MoodCueNN` (Entity_b..Entity_g): the
row mapping is a fact of the binary, which dream object or state a row is
for is not established. Row kept decimal so names sort in table order.

What it does, in the unit's current field names: Attenuation from proximity; on set tick 0 requests voice-0 tone 1 or 2 (coin flip) at pitch +2.
