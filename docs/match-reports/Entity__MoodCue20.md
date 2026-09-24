# Entity__MoodCue20 -- MATCHED (57/57 words)

> Renamed from `func_8005EFF4` on 2026-09-24 (tools/rename.py). Address 0x8005eff4.

Unit: `Entity_c`. Runner: bravo.

## Shape

```c
void Entity__MoodCue20(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0 && rand() % 7 == 0) {
        this->methods->slot48(this, 1, SCALE_Y2);
    }
    if ((out->unk4 & 3) == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 0x1C;
    }
    this->methods->slotC4(this, -0x64, 0);
}
```

## Notes

- The first gate is `rand() % 7 == 0`, guarded by `this->unkFC == 0` -- the
  usual "one-shot random effect on entry" shape already seen in
  `Entity__MoodCue16`/`Entity__MoodCue12` in `src/Entity_b.c`.
- **`out->unk4 & 3`, not `% 4`.** Retail emits a bare `andi $v0,$v0,0x3` with
  no sign-correction shift, unlike every modulus-by-non-power-of-2 gate in
  this unit (which all carry the mult/mfhi/sra/subu chain). Writing `% 4`
  here would risk GCC inserting a sign-fix for the general signed-modulus
  case; the plain bitwise `&` reproduces the bare `andi` byte-for-byte on the
  first attempt, so no sign correction was needed in practice -- worth
  remembering as a candidate discriminator: a bare `andi` with no correction
  present in the asm is a tell that the source used `&`, not `%`, even where
  a modulus by a power of two would read equally naturally.
- Extern added: `SCALE_Y2` (opaque row pointer, same convention as the
  other `D_80089Dxx`/`D_80089Cxx` rows already declared in `Entity_b.c`).
- Clean of both open toolchain blockers.

Matched first attempt (1/30).

### Proposed learning

A bare `andi $v0, $v0, N` (power-of-2 mask) with **no** accompanying sign-fix
sequence is a source-level `& (N-1)`, not `% N` -- GCC 2.6.3 still inserts a
correction for a genuinely signed `%` by a power of two, so its absence is a
real discriminator, not just an equally-valid alternate spelling.
