# Entity__MoodCue65 -- MATCHED (71/71 words)

> Renamed from `func_80061F30` on 2026-09-24 (tools/rename.py). Address 0x80061f30.

Unit: `Entity_e` (round 13). Ignores its `out` argument entirely (same shape
as `Entity__MoodCue67`/`Entity__MoodCue70` in this unit): a one-shot 1-in-3 dice
roll on the first tick fires three vtable calls and sets `unk44 = 0xB`,
then a second block guarded by that flag fires an `unkFC`-threshold call
and an unconditional `slotC4`.
`void Entity__MoodCue65(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue65(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        if (rand() % 3 == 0) {
            this->methods->slot48(this, 1, SCALE_HALF);
            this->methods->slotCC(this, -0x12C, 0);
            this->methods->slot44(this, 1, ROTATION_YAW_PLUS90);
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0xB) {
        if (this->unkFC == 0x7D0) {
            this->methods->slot44(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->slotC4(this, -0x14, 0);
    }
}
```

## Derivation notes

Same "1-in-3" `rand() % 3 == 0` idiom as `Entity__MoodCue67` in this unit
(magic `0x55555556`, no post-`mfhi` shift, `sll 1`/`addu` reconstructing
`q*3`, compared directly against the dividend). This one matched clean on
the first pass, unlike `func_80062570`'s residue -- the difference is that
here the modulo result is never multiplied by anything afterward, it is
only compared to the dividend inline; `func_80062570`'s stall was specific
to scaling the remainder by a further constant.

`slot48` (already `s32`-returning), `slotCC` (`s32`-returning), `slot44`
(`void`, `(self, s32, void*)`), and `slotC4` (`void`) are all pre-existing
vtable slot types from earlier work in this unit and `Entity_d`; every
call here discards its return value, consistent with those slots' existing
types. `ROTATION_YAW_PLUS90` already has an extern/callsite later in this same file
(`func_80062660`); `SCALE_HALF` and `ROTATION_YAW_MINUS90` are new per-unit `extern
u8 [];` data-table externs, same convention as the rest of this file.

No new struct or vtable-slot knowledge.
