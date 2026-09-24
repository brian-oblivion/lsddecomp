# Entity__MoodCue61 -- MATCHED (10/10 words)

> Renamed from `func_80061C04` on 2026-09-24 (tools/rename.py). Address 0x80061c04.

Unit: `Entity_e` (round 12, first carve of this unit). Smallest function in
the unit's queue, a one-shot mood handler with no loop or nested branch.
`void Entity__MoodCue61(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue61(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk84 == 0x1E) {
        out->unk1C = 0x12;
        out->unk10 = 0;
        out->unk20 = -1;
    }
}
```

## Derivation notes

Matched first attempt. Straight read of the disassembly: `this->unk84`
(already a known field, compared against literals elsewhere in Entity_d)
tested against `0x1E`; on match, three stores into the `EntityMoodHandlerArg
*out` parameter, all to already-known offsets (`unk1C`, `unk10`, `unk20`).
No new struct knowledge -- everything used was already established in
`include/Entity.h` from Entity_d's work.

### Proposed learning

None beyond what's already documented; this one confirms the existing
`EntityMoodHandlerArg` field set is enough to read this unit's handlers
without further struct excavation.
