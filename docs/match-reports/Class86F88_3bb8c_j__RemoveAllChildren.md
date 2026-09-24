# Class86F88_3bb8c_j__RemoveAllChildren -- MATCHED (17/17 words)

> Renamed from `func_80051E20` on 2026-09-24 (tools/rename.py). Address 0x80051e20.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86F88_3bb8c_j`. Full reset: clears
both tagged caches and `unk50` (same three fields as `Class86F88_3bb8c_j__ClearCachedRefs`),
then chains to the inherited `BasicClass::removeAllChildren`.

## Body

```c
void Class86F88_3bb8c_j__RemoveAllChildren(Class86F88_3bb8c_j *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk50 = NULL;
    Get_vtable_BasicClass()->removeAllChildren(self);
}
```

Matched first try.

## Naming

- `Class86F88_3bb8c_j__RemoveAllChildren` -- tier A. The removeAllChildren occupant (classtable.py D_80086F88 +0x018): clears unk34/unk38/unk50 then chains the base removeAllChildren. Matches the BasicClass convention.
