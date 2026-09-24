# Entity__MoodCue75 -- MATCHED (39/39 words)

> Renamed from `func_800628D4` on 2026-09-24 (tools/rename.py). Address 0x800628d4.

Unit: `Entity_e` (round 12). Calls `Class6B5CC__FaceTarget` with the same
`(this, this->unk94, 1, 0, 0)` argument shape already established in
`Entity_d.c`'s handlers, plus a same-value dispatch through
`EntityMethods::slot130`/`slot30`.
`void Entity__MoodCue75(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue75(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 == 0) {
        out->unk10 = 0;
        out->unk1C = 0x19;
        out->unk30 = 0x19;
        out->unk44 = 0x19;
        Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
    }
    if (this->unkFC == this->unk80) {
        this->methods->slot130(this);
        this->methods->slot30(this, 0xA);
    }
}
```

## Derivation notes

Matched first attempt (once written after `Entity__MoodCue73`'s lesson about
delay-slot placement was fresh -- double-checked here that `sw zero,
0x10(a1)` really is straight-line code following the `bnez`, not the
branch's delay slot, before nesting it inside the `if`; in this function it
genuinely is guarded, unlike `Entity__MoodCue73`). No new struct or vtable
knowledge -- `slot130`, `slot30`, and `Class6B5CC__FaceTarget`'s signature were all
already established.

### Proposed learning

None new; this one is a clean confirmation that the delay-slot-placement
check from `Entity__MoodCue73`'s report generalizes correctly rather than
over-correcting into "always hoist stores out of `if` bodies".
