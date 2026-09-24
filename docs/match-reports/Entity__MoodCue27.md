# Entity__MoodCue27 -- MATCHED (51/51 words)

> Renamed from `func_8005F608` on 2026-09-24 (tools/rename.py). Address 0x8005f608.

Unit: `Entity_c`. Runner: bravo.

## Shape

```c
void Entity__MoodCue27(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % 70 == 0) {
        out->unk10 = 0;
        out->unk1C = 0x1B;
    }
    this->methods->slotC4(this, -0x80, 0);
    if (this->unkFC < 0x64) {
        this->methods->slotCC(this, 0x20, 0);
    } else if (this->unkFC >= 0x12D) {
        this->methods->slotCC(this, -0x20, 0);
    }
}
```

## Notes

- The `% 70` gate is the confirmed "x % N for compile-time constant N: just
  write %" idiom -- divisor 70 recovered arithmetically from the
  mult/mfhi/sll/addu/subu reconstruction chain (`v1*8 -> v1*9 -> v1*36 ->
  v1*35 -> v1*70`), not guessed.
- Three-way dispatch on `this->unkFC`: below 100 does one thing, 100..300
  does nothing, 301+ does the opposite thing -- a plain `if`/`else if` with
  no `else` body reproduces retail's middle-range no-op exactly.
- Clean of both open toolchain blockers.

Matched first attempt (1/30).
