# Entity__MoodCue21 -- MATCHED (52/52 words)

> Renamed from `func_8005F0D8` on 2026-09-24 (tools/rename.py). Address 0x8005f0d8.

Unit: `Entity_c`. Runner: bravo.

## Shape

```c
void Entity__MoodCue21(Entity *this, EntityMoodHandlerArg *out) {
    s32 half;
    s32 rem;

    out->unk10 = this->methods->slot148(this);
    half = this->unk80 / 2;
    rem = out->unk4 % half;
    if (rem == 0) {
        out->unk1C = 0xA;
    } else if (rem == 3) {
        out->unk30 = 0xD;
    }
    this->methods->slotC4(this, -0x1E, 1);
}
```

## Notes

- `this->unk80 / 2` is the confirmed signed-halving idiom already documented
  on the field in `include/Entity.h` (`(x + (unsigned)x>>31) >> 1`), shared
  with `Entity__MoodCue09`/`Entity__MoodCue11` in `Entity_b.c`.
- The remainder of `out->unk4 % half` is computed once and tested against two
  literals (`0`, `3`) with different effects -- an ordinary `if`/`else if`
  reproduced the branch structure exactly; both branches converge before the
  unconditional `slotC4` call, matching retail's single merge label.
- Clean of both open toolchain blockers.

Matched first attempt (1/30).
