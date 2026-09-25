# Entity__MoodCue94

> Renamed from `func_80064294` on 2026-09-25 (tools/rename.py). Address 0x80064294.

**Unit:** Entity_f · **Size:** 111 words · **Status:** MATCHED (111/111 words)

## What it does

On `this->unkFC == 0`: `if (this->unk94->methods->slot200(this->unk94) !=
7) { this->unk44 = 0xB; }`. Unconditionally: `out->unk10 =
this->methods->slot148(this); if (out->unk4 % 10 == 0) { out->unk1C =
0xE; }`. On `this->unkFC == this->unk80`: `slot128(this,1)`; if
`this->unk44 != 0`, on a `rand()&1` miss calls `slot48(this,1,SCALE_SIX)`
and `slotCC(this,0x800,0)`; independently, on `rand()%3==0`, calls
`slot44(this,0,ROTATION_YAW_PLUS180)`. Finally, `if (this->unk7C != 0) {
slotC4(this,-0x80,1); }`.

## Derivation

Direct transcription. No residue.
