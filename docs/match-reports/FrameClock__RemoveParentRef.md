# FrameClock__RemoveParentRef -- MATCHED (28/28 words), round 82

> Renamed from `D8006EF50__RemoveParentRef` on 2026-09-26 (tools/rename.py). Address 0x800424e0.

> Renamed from `func_800424E0` on 2026-09-25 (tools/rename.py). Address 0x800424e0.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** gFrameClockMethods slot +0x024 (removeParentRef override).
- **What:** +0x018 is a parentRefs cursor (a `BasicClassListNode *`); if it points at the node holding the parent being removed, step it to `next` first. Then calls BasicClass's removeParentRef. So FrameClock walks its parent refs incrementally (FrameClock__NotifyParents, its notifyParents, is the likely walker) and this keeps the cursor valid across a removal.
- **Result:** byte-exact, 28/28 words, 0 ins / 0 del, whole-image SHA1 green. First build.
- **Types:** unit-local `D_8006EF50Obj.parentCursor` retyped `s32` -> `BasicClassListNode *` (FrameClock__Reset's `= 0` still matches, verified in the same build). No shared header touched.

## Source

```c
void FrameClock__RemoveParentRef(D_8006EF50Obj *self, BasicClass *parent) {
    if (self->parentCursor != NULL && parent == self->parentCursor->value) {
        self->parentCursor = self->parentCursor->next;
    }
    Get_vtable_BasicClass()->removeParentRef((BasicClass *)self, parent);
}
```

## Naming

- `FrameClock__RemoveParentRef` -- tier A. Slot +0x024: steps the parentCursor past the parent being removed (so a live iteration in NotifyParents/Tick doesn't dereference a freed entry) then calls the base BasicClass removeParentRef. Round-82 broadcast: "a parentRefs cursor ... steps past a removed parent".

## Track 4 (2026-09-26, round 88, delta)

Renamed from `D8006EF50__RemoveParentRef`: override of +0x024, named for its slot. The class (id 0x5, table `gFrameClockMethods`, formerly `D_8006EF50`) is unified as `FrameClock` in `include/FrameClock.h`, whose banner holds the evidence for the class name: its tick (+0x044) is called by IntermediateBase__OnTag1Notify on the DrawSystem's per-VSync event 2, counts one frame and tells its parents event 2, or 3 while paused, or 4 while flag14 is set. Fields renamed: `count` -> `frameCount`, `flag10` -> `paused`. Any source block above is the pre-unification spelling (`D_8006EF50Obj`); the live body in `src/code_322b4.c` takes `FrameClock *`, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
