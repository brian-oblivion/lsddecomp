# TaskObjF__ClearLinks — MATCH (6/6 words)

> Renamed from `TaskObjF__ClearResourceSlots` on 2026-09-27 (tools/rename.py). Address 0x8004e3f4.

> Renamed from `func_8004E3F4` on 2026-09-24 (tools/rename.py). Address 0x8004e3f4.

**Unit:** title_menu (round 14, first slice of the new `Node3bb8cE` class --
see `include`-local declarations at the top of `src/ui/title_menu.c`; this
class is unrelated to `Obj866E8`/`gStageMapMethods` in `include/class_3bb8c.h`).

## What it does

`void TaskObjF__ClearLinks(Node3bb8cE *self)`. Zeroes the object's five typed
resource-slot pointers (`unk60`, `unk64`, `unk68`, `unk78`, `unk7C`) — the
same five fields `TaskObjF__RemoveAllChildren` zeroes before calling the base class's
`removeAllChildren`. Almost certainly a constructor/init helper for this
class's own resource-slot state.

## Result

Matched on the first attempt — a straight sequence of five word stores, no
branches, no register-identity ambiguity possible.

```c
void TaskObjF__ClearLinks(Node3bb8cE *self)
{
    self->unk60 = NULL;
    self->unk64 = NULL;
    self->unk68 = NULL;
    self->unk78 = NULL;
    self->unk7C = NULL;
}
```

### Proposed learning

None beyond what's already recorded — a pure straight-line store sequence
needs nothing beyond a direct transcription.

## Naming (round 78, track 3)

`func_8004E3F4` -> `TaskObjF__ClearLinks`. **Tier B.** NOT a `gTaskObjFMethods` entry (checked against the full table) -- called once, directly, from `TaskObjF__TaskObjF`'s own ctor (title_menu.c), before `self->methods->slot40(self, arg2)` (`TaskObjF__SetCardSlot`). Zeroes the same five resource-slot pointers (`res02`/`res05`/`unk68`/`res10`/`res20`) that `TaskObjF__RemoveAllChildren` zeroes before chaining to the base class. Mechanics are exact (construction-time init of the resource slots); tier B because it is one line of evidence (a single call site) rather than two independent callers agreeing.

## Naming (round 98, track 7)

`TaskObjF__ClearResourceSlots` -> `TaskObjF__ClearLinks` (tools/rename.py).
**Tier A.** It NULLs TaskObjF's five object pointers -- `inputSource`,
`tickSource`, `spriteParent`, `textEntry`, `itemList` -- which are links to
other objects (four of them children `TaskObjF__AddChild` files by class id),
not resources in the `FileResource` sense the old name suggested.
`TaskObjF__RemoveAllChildren` clears the same five before chaining to
BasicClass. One caller, the ctor.

## Unit banner history (moved from src/class_3bb8c_e.c, round 98)

The unit's banner used to carry its history, now here because this is the
unit's first function: class_3bb8c_e was carved in round 14 from the same
class_3bb8c remainder segment as class_3bb8c_b/_c/_d/_f, its class was named
TaskObjF in round 78 (track 3), and it moved onto include/TaskObjF.h in
round 89 (track 4). Until round 89 the unit read the object through its own
view, `Node3bb8cE` (round 78 confirmed it was TaskObjF): its
res02/res05/res10/res20 are TaskObjF's inputSource/tickSource/textEntry/
itemList, filed by AddChild on the child's class id, and its zero-only
`unk68` is spriteParent. The banner also listed the 13 `gTaskObjFMethods`
entries by offset; include/TaskObjF.h's method table is now the place for
that.
