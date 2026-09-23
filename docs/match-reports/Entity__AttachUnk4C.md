# Entity__AttachUnk4C -- MATCHED (65/65 words)

> Renamed from `func_8005D314` on 2026-09-19 (tools/rename.py). Address 0x8005d314.

**Unit:** Entity · Runner: charlie, round 23.

## What it does

Gated by `this->unk0C` (the same shared-ancestor "gate flag" `Entity__DetachUnk4C`
reads), forwards all four of its own arguments straight through to a new
`BasicClassMethods` slot (`slot4C`), stashes `arg3` into `this->unk4C` (the
`Unk4CObj *` field), then runs two independent early-out checks against two
new mood-row byte tables (`D_80089EA7`, `D_80089EAF`, both immediately
adjacent to already-declared tables of the same family) gating two more
self-only vtable calls.

## Final C

```c
void Entity__AttachUnk4C(Entity *this, s32 arg1, s32 arg2, Unk4CObj *arg3, s32 arg4) {
    if (this->unk0C != 0) {
        return;
    }
    func_80066818()->slot4C(this, arg1, arg2, arg3, arg4);
    this->unk4C = arg3;
    if (D_80089EA7[this->moodIndex * 0x10] != 0) {
        return;
    }
    this->methods->slot15C(this);
    if (D_80089EAF[this->moodIndex * 0x10] != 0) {
        return;
    }
    this->methods->slot168(this);
}
```

Byte-exact on the first attempt, whole-image build verified.

## Notes

Two new vtable slots, both already partly informed by the batch's earlier
functions: `BasicClassMethods::slot4C` (5 args: `self, s32, s32, void*,
s32` -- the last real gap in the low offsets of that shared-ancestor table
this unit has needed so far) and `EntityMethods::slot15C`/`slot168` were
already documented (called by `Entity__UpdateActivationState`/`Entity__UpdateSoundCueStart`
respectively) -- this is just their second known caller.

`D_80089EA7` and `D_80089EAF` are two more single-byte, `moodIndex*0x10`-
strided tables in the same family as the already-declared `gEntityUnlockKindTable`
("GetUnlockEffect"), `gEntityLinkStageTable` ("GetLinkStage"), `gEntityEventVideoTable`
("GetEventVideo") -- confirmed via `asm/data/79528.data.s` as their own
`dlabel`s (own relocations), not sub-fields of `gEntityMoodTable`. Declared
`extern s8 D_80089EA7[]`/`extern s8 D_80089EAF[]` in `Entity.h` next to
their siblings, matching the `lb` (signed) instruction at both sites.

`arg3` is stored into `this->unk4C` right after being forwarded to
`slot4C`, which is what pins its type to `Unk4CObj *` here (the shared
ancestor slot's own parameter is typed generically `void *arg3` in
`BasicClassMethods`, since it's an inherited slot other units may call
with unrelated pointer types).

### Proposed learning

None new -- this one built clean on the first attempt with no residue,
likely because the round's earlier three stalls (`Entity__IsNearTarget`,
`Entity__NotifyLinkStage`) had already surfaced the two live traps in this unit
(parameter-vs-fresh-local register identity, `~x+1` vs `-x`) and this
function's shape simply didn't trigger either: every argument here is
used exactly once, in the order retail computes it, with no shared local
threaded across a branch.

Round-23 head broadcast's three levers do not apply (no `s16` locals, no
loop, no `&arr[i+j]` shape) -- reported per the "reply with the negative
answer too" instruction.

## Naming

**Tier B.** Renamed from `func_8005D314` this round (tools/rename.py). The
mechanic is a leaf-simple field set (`this->unk4C = arg3`) past a gate, so
the name says exactly that -- "Attach" for the store, "Unk4C" because the
field's own real-world purpose is still open (its only other known reader,
`Entity__MoodCue12`, dereferences it as a vtable-holding object but that alone
doesn't say what it IS). Pairs with `Entity__DetachUnk4C` below under the
same `this->unk0C` gate.
