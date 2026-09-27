# SceneNode__NoOpSlotB0 -- MATCHED (trivial)

> Renamed from `func_8001E49C` on 2026-09-26 (tools/rename.py). Address 0x8001e49c.

Round 54 (bravo, track 3). `code_d294_b`.

## What it does

Slot `+0x0B0` occupant (`tools/classtable.py gSceneNodeMethods`), between the
documented `SceneNode__RaycastHullAgainstFaces` (slot `+0x0AC`) and `SceneNode__AddToActorParents` (the
table's own last slot, `+0x0B4`) -- no caller in this project dispatches
through it by name or by a documented vtable comment. Whole body is
`{}` (`jr $ra; nop`, confirmed via `objdump` on the built object) --
a no-op override, generated as a byte-exact match by splat itself with no
decompilation work involved (CLAUDE.md: "Not every matched function was
work"). No report existed for this function before this round; created
now so track 3's naming pass has somewhere to record the tier decision.

## Naming

**Kept as `SceneNode__NoOpSlotB0` -- Tier C.** Same shape and same disposition as the
already-established no-op-stub precedent in this exact class,
`SceneNode__NoOpSlot5C` (`code_d294.c`, matched, never renamed): a vtable
override whose entire behavior is "do nothing." Renaming a no-op to
anything more specific than its offset would assert a purpose ("this
class disables feature X here") that the empty body cannot support --
exactly the "wrong tier-A name is worse than `func_`" case. What IS
known (that this is a deliberate no-op override, not missing code) is
recorded here.

## Track 6 (round 91, echo): named `SceneNode__NoOpSlotB0`, tier C

Empty. Slot +0x0B0 has no caller; Actor and its subclasses null it. `NoOpSlotNN` precedent. Was `func_8001E49C`. The class was renamed Class6B5CC -> SceneNode in the same pass (include/SceneNode.h's banner has the evidence).
