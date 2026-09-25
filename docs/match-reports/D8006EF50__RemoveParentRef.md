# D8006EF50__RemoveParentRef -- MATCHED (28/28 words), round 82

> Renamed from `func_800424E0` on 2026-09-25 (tools/rename.py). Address 0x800424e0.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** D_8006EF50 slot +0x024 (removeParentRef override).
- **What:** +0x018 is a parentRefs cursor (a `BasicClassListNode *`); if it points at the node holding the parent being removed, step it to `next` first. Then calls BasicClass's removeParentRef. So D_8006EF50 walks its parent refs incrementally (D8006EF50__NotifyParents, its notifyParents, is the likely walker) and this keeps the cursor valid across a removal.
- **Result:** byte-exact, 28/28 words, 0 ins / 0 del, whole-image SHA1 green. First build.
- **Types:** unit-local `D_8006EF50Obj.unk18` retyped `s32` -> `BasicClassListNode *` (func_800425D8's `= 0` still matches, verified in the same build). No shared header touched.

## Source

```c
void D8006EF50__RemoveParentRef(D_8006EF50Obj *self, BasicClass *parent) {
    if (self->unk18 != NULL && parent == self->unk18->value) {
        self->unk18 = self->unk18->next;
    }
    Get_vtable_BasicClass()->removeParentRef((BasicClass *)self, parent);
}
```
