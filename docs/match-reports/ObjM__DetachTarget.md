# ObjM__DetachTarget

> Renamed from `func_80052EBC` on 2026-09-24 (tools/rename.py). Address 0x80052ebc.

**Unit:** class_3bb8c_l · **Size:** 21 words (0x54 bytes) ·
**Status: MATCHED 21/21**, whole-image SHA1 green.

## What it does

```c
void ObjM__DetachTarget(Obj87034_3bb8c_l *self) {
    self->methods->slot14(self, self->unk3C);
    GetClass86668Methods()->slot48(self);
}
```

Straight-line: dispatch through `self`'s own vtable slot `+0x14`, then
through the shared BasicClass-family base table's slot `+0x48` (via
`GetClass86668Methods()`, the same accessor `ObjM__AttachTarget` uses at a different
slot). No branches, matched on the first correctly-typed attempt.

## Notes

- `BaseMethods87034_3bb8c_l::slot48` (the base accessor's `+0x48`) is a
  DIFFERENT table from `Obj87034Methods_3bb8c_l::slot48` (self's own
  `+0x48`, used by `ObjM__TeardownStyle` on a sibling object) — same numeric
  offset, unrelated classes, no naming collision since they're separate
  struct types.
