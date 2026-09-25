# Actor__RemoveAllChildren -- MATCHED (16/16 words)

> Renamed from `BaseObjO__ClearCompanions` on 2026-09-25 (tools/rename.py). Address 0x800571a8.

> Renamed from `func_800571A8` on 2026-09-18 (tools/rename.py). Address 0x800571a8.

Unit: `class_3bb8c_o` (round 17). `BaseObjOMethods::slot18` -- an
unconditional full teardown of both companion pointers (no tag check),
already named `unk18`/`Actor__RemoveAllChildren` in `code_55dd4.h`'s
`D800878D4Methods`.

## Final source

```c
void Actor__RemoveAllChildren(BaseObjO *self) {
    self->unk4C = NULL;
    self->unk50 = NULL;
    GetClass6B5CCMethods()->slot18(self);
}
```

## Derivation

`sw $zero,0x4C($s0)` / `sw $zero,0x50($s0)` (both unconditional, no tag
read at all) then chain to the fixed table's `slot18(self)`. Straight
transcription, matched first try -- no tag classification is present here
because, unlike `slot10`/`slot14`, this slot takes no companion-object
argument to classify; it simply clears both fields regardless of which one
(if either) was set.

### Proposed learning

None -- confirms rather than extends the link/unlink pair's field naming
already established by `Actor__AddChild`/`Actor__RemoveChild`.

## Naming

**`Actor__RemoveAllChildren` -- tier A.** Mechanics ARE the purpose:
unconditionally clears BOTH companion fields (no tag check, unlike
`LinkCompanion`/`UnlinkCompanion`) and chains to the base table's own
`slot18`. `slot18` has no other established occupant name in any sibling
header (`code_55dd4.h` still calls the field `unk18`), so this name is
this unit's own contribution.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `BaseObjO__ClearCompanions`. Override of +0x018 (removeAllChildren), named for its slot: clears both companions, then chains Class6B5CC's. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a Class6B5CC subclass and the base of Class65650/Entity, DreamSys and Class876FC. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_o.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
