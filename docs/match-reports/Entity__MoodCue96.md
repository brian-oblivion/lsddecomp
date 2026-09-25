# Entity__MoodCue96

> Renamed from `func_800644E8` on 2026-09-25 (tools/rename.py). Address 0x800644e8.

**Unit:** Entity_f · **Size:** 76 words · **Status:** MATCHED (76/76 words)

## What it does

`if (this->unkFC == 0) { slot128(this, rand() % 4); return; } if
(this->unkFC % this->unk80 == 0) { slot128(this, rand() % 4); out->unk10
= slot148(this); out->unk1C=0x16; out->unk20=2; out->unk24=0x40;
out->unk28=0x20; }`.

## Derivation

`rand() % 4` compiles to the shift-based power-of-2 modulus idiom
(`(x + (x<0 ? 3 : 0)) >> 2 << 2`, subtracted from `x`) directly from the
plain `%` operator — no manual reconstruction needed. `this->unkFC %
this->unk80` (a RUNTIME divisor, not a compile-time constant) compiles to
retail's `div`/`break 7`/`break 6`/`mfhi` sequence automatically, matching
this project's documented maspsx-expansion policy for non-constant
divisors (see CLAUDE.md's "maspsx-expanded macros" residue class and
`Entity__MoodCue96`'s own use of `mfhi` for the remainder). No residue.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue96` | B | `gEntityMoodHandlerTable` row 96 |

Why `MoodCue96`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 96 (base 0x80089EB0, stride 0x10; the row's
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

What it does, in the unit's current field names: `slot128(rand() % 4)` at tick 0; every `moodDuration` ticks another random `slot128`, attenuation from proximity, and a voice-0 tone 22 request with pitch +2 and the slot's two remaining words (`unk24`/`unk28`, ServiceSoundCueSet's `word2`/`word3`, per-tick defaults 0x7F/0x40) set to 0x40/0x20.
