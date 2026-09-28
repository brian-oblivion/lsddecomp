# SceneNode__OnPadEvent -- MATCHED (trivial)

> Renamed from `func_8001D6A4` on 2026-09-26 (tools/rename.py). Address 0x8001d6a4.

Round 54 (bravo, track 3). `code_d294`.

## What it does

Slot `+0x094` occupant (`tools/classtable.py gSceneNodeMethods`) -- one of the
three `(self, GenericObj_d294 *other, s32 arg2)` slots `SceneNode__OnNotify`
(`code_d294.c`) dispatches to depending on `other->methods->header & 0xF`
(this slot fires for tag `2`). Whole body is `{}` (`jr $ra; nop`,
confirmed via `objdump` on the built object) -- a no-op override,
generated as a byte-exact match by splat itself with no
decompilation work involved (CLAUDE.md: "Not every matched function was
work"). No report existed for this function before this round; created
now so track 3's naming pass has somewhere to record the tier decision.

## Naming

**Kept as `SceneNode__OnPadEvent` -- Tier C.** Same shape and same disposition as the
already-established no-op-stub precedent in this exact class,
`SceneNode__NoOpSlot5C` (`code_d294.c`, matched, never renamed): a vtable
override whose entire behavior is "do nothing." Renaming a no-op to
anything more specific than its offset would assert a purpose ("this
class disables feature X here") that the empty body cannot support --
exactly the "wrong tier-A name is worse than `func_`" case. What IS
known (that this is a deliberate no-op override, not missing code) is
recorded here.

## Track 6 (round 91, echo): named `SceneNode__OnPadEvent`, tier A

Empty default of the onPadEvent slot (+0x094), which SceneNode__OnNotify calls for a Pad sender (class id nibble 2); DreamSys__OnPadEvent overrides it. Was `func_8001D6A4`. The class was renamed Class6B5CC -> SceneNode in the same pass (include/SceneNode.h's banner has the evidence).
