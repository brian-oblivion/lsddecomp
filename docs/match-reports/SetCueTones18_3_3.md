# SetCueTones18_3_3

> Renamed from `func_80063CAC` on 2026-09-25 (tools/rename.py). Address 0x80063cac.

**Unit:** Entity_f · **Size:** 7 words · **Status:** MATCHED (7/7 words)

## What it does

Sibling of `SetCueTones7_7_7`: another one-parameter `out`-only setter,
`void SetCueTones18_3_3(EntityMoodHandlerArg *out)`: `out->unk1C=0x12;
out->unk10=0; out->unk30=3; out->unk44=3;`.

## Derivation

First transcription wrote `out->unk44 = 0x12` (matching the value used for
`unk1C` a few lines above, an easy visual slip) instead of the actual
stored value, `3` (the SAME register value already computed for
`out->unk30` two lines earlier, reused rather than reloaded — confirmed
against the raw hex, `sw $v0, 0x44($a0)` with `$v0` unchanged since the
`ori $v0, $zero, 0x3` two instructions prior). One-word fix.

## Proposed learning

**A tiny multi-field setter reusing one register across several stores is
exactly the shape where "the value looks like the one above it" produces
a plausible-but-wrong guess** — verify each `sw`'s SOURCE register's most
recent assignment individually rather than pattern-matching against a
neighboring store's literal.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `SetCueTones18_3_3` | B | the whole body; callers `Entity__MoodCue85` (x2) and `Entity__MoodCue86` |

Free function, only argument the `SoundCueSet`. Attenuation 0, voice 0
tone 18 (0x12), voices 1 and 2 tone 3, pitch offsets untouched (left at
ServiceSoundCueSet's per-tick 0). Tone numbers in decimal, like the MoodCue
row numbers. `Entity__MoodCue82` and `Entity__MoodCue89` write the same
voice 0/1 pair inline without voice 2, so this is not a unique
"the" cue; purpose not established, tier B.
