# Entity__MoodCue86

> Renamed from `func_80063BC0` on 2026-09-25 (tools/rename.py). Address 0x80063bc0.

**Unit:** Entity_f · **Size:** 49 words · **Status:** MATCHED (49/49 words)

## What it does

`if (this->unkFC < 0xA) { slot130 } else if (this->unkFC == 0xA) {
slot12C }` (sharing a merged `jalr` tail across both call targets, as
retail already produces naturally). Independently, `if (this->unk84 ==
0xA) { func_80063CAC(out); }`. Independently, `if (this->unkFC ==
this->unk80 + 0xA) { slot16C; this->unk44 = 1; }`.

## Derivation

Straightforward transcription; calls the not-yet-defined-at-this-point
`func_80063CAC` (defined later in this unit, ROM order), matching this
unit's forward-declaration convention. No residue.
