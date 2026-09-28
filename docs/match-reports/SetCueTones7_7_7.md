# SetCueTones7_7_7

> Renamed from `func_80063C84` on 2026-09-25 (tools/rename.py). Address 0x80063c84.

**Unit:** Entity · **Size:** 10 words · **Status:** MATCHED (10/10 words)

## What it does

A trivial one-parameter setter, `void SetCueTones7_7_7(EntityMoodHandlerArg
*out)`: `out->unk10=0; out->unk1C=7; out->unk20=-2; out->unk30=7;
out->unk34=-2; out->unk44=7; out->unk48=-2;`. Called from several other
functions in this unit as a shared "reset to state 7" helper — the ONLY
argument is `out` (`EntityMoodHandlerArg*`), unlike this unit's usual
`(Entity *this, EntityMoodHandlerArg *out)` mood-handler signature; no
`this` dereference appears anywhere in the disassembly.

## Derivation

Direct transcription; the single-parameter signature is unambiguous from
the raw bytes (every store is `sw $vN, OFFSET($a0)`, never touching `$a1`).
No residue.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `SetCueTones7_7_7` | B | the whole body; callers `Entity__MoodCue85` (x3) only |

Free function (no `this`; the only argument is the `SoundCueSet`), so
`VerbNoun`. Read with the `SoundCueSet` field readings (`Entity__MoodCue07.md`
`## Proposed field names`; `ServiceSoundCueSet`): attenuation 0, and voices
0, 1 and 2 each request tone 7 with pitch offset -2. The name records the
tone triple, which is what the body does; the -2 pitch is left out of it for
length and is written here. What the chord is for is not established, hence
tier B. Replaces a report line that called it a "reset to state 7" helper:
nothing in it touches a state field.
