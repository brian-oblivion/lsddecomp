# Class876FC__ReleaseModelChildren -- MATCHED (12/12 words)

> Renamed from `func_80056B8C` on 2026-09-23 (tools/rename.py). Address 0x80056b8c.

Unit `class_3bb8c_s`. `self` is the owning `LinkNode`.

## Classification

Clean on all four carve-time screens. A guarded release of the fixed
2-element `arr7C` array, using the same `ReleaseBasicClassArray(void **array, s32
count)` already established in `code_8220_b.c` and reused (for the SIBLING
5-element `arr84` array) in `class_3bb8c_o.c`.

## Body

```c
void Class876FC__ReleaseModelChildren(LinkNode *self) {
    if (self->unk6C != 0) {
        ReleaseBasicClassArray((void **)self->arr7C, 2);
    }
}
```

### Proposed learning

None -- a plain guarded release, no residue.
