# Entity__MoodCue50 -- MATCHED (36/36 words)

> Renamed from `func_80060CF0` on 2026-09-24 (tools/rename.py). Address 0x80060cf0.

Unit: `Entity_d` (fresh carve, round 2026-09-03). Mood-dispatch handler:
`void Entity__MoodCue50(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue50(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC < this->unk80 * 5) {
        if (this->unk84 == 0xF || this->unk84 == 0x46) {
            out->unk10 = 0;
            out->unk1C = 7;
            out->unk30 = 7;
            out->unk44 = 7;
        }
    } else {
        this->methods->slot160(this);
        this->unk44 = 1;
    }
}
```

## Derivation notes

Matched first attempt, no iteration needed. Uses the already-documented
`EntityMethods::slot160` (`void (*)(Entity *self)`, previously known from
`Entity__DetachUnk4C`/`Entity__NotifyReset`/`Entity__UpdateDeactivationState` in the earlier Entity units)
-- no header change required here.

### Proposed learning

None -- straightforward two-way dispatch, no residue.
