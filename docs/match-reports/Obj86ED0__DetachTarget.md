# Obj86ED0__DetachTarget -- MATCHED (22/22 words)

> Renamed from `func_80051270` on 2026-09-24 (tools/rename.py). Address 0x80051270.

Unit `class_3bb8c_i`, carved round 14.

`Obj86ED0`'s vtable slot 0x050: detaches both typed child slots via
`self->methods->removeChild` (the class's own override, `Obj86ED0__RemoveChild`),
then zeroes `unk3C`. Note `unk34`/`unk38` themselves are NOT zeroed here
(only `unk3C` is) -- transcribed as observed, not assumed symmetric with
`Obj86ED0__ClearChildRefs`/`Obj86ED0__RemoveAllChildren`.

```c
void Obj86ED0__DetachTarget(Obj86ED0 *self)
{
    self->methods->removeChild(self, self->unk34);
    self->methods->removeChild(self, self->unk38);
    self->unk3C = NULL;
}
```

First attempt, matched immediately once the earlier in-unit drift was
resolved.

## Naming

- `Obj86ED0__DetachTarget` -- tier A. gObj86ED0Methods +0x050 (classtable.py). Symmetric teardown of Obj86ED0__AttachTarget: removes childType2/childType5 via self->methods->removeChild, clears target.
