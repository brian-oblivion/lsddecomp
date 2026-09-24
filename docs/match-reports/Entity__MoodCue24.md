# Entity__MoodCue24 -- MATCHED (59/59 words)

> Renamed from `func_8005F368` on 2026-09-24 (tools/rename.py). Address 0x8005f368.

Unit: `Entity_c`. Runner: bravo.

## Shape

```c
void Entity__MoodCue24(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % 15 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 7;
        out->unk20 = -2;
    }
    this->methods->slot48(this, 1, SCALE_DOUBLE);
    this->methods->slot44(this, 0, ROTATION_YAW_PLUS2);
    this->methods->slotC4(this, -0x200, 0);
}
```

## Notes

- `% 15` gate: same magic multiplier (0x88888889) as `Entity__MoodCue33`'s
  `% 30` gate, different shift amount in the mfhi/sra chain -- confirms the
  DECOMPILATION_LEARNINGS caution that the constant alone never identifies
  the divisor; here the reconstruction is `v0*16 -> v0*15`, shift 3 instead
  of `Entity__MoodCue33`'s shift 4.
- Three unconditional vtable calls in a row after the gate -- straight
  sequential statements, no reshaping needed.
- Extern added: `ROTATION_YAW_PLUS2` (own file-scope declaration; already declared
  in `Entity_b.c`).
- Clean of both open toolchain blockers.

Matched first attempt (1/30).
