# Class86F88_3bb8c_j__ResetCounters -- MATCHED (4/4 words)

> Renamed from `func_80051F14` on 2026-09-24 (tools/rename.py). Address 0x80051f14.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86ED0`.

## Body

```c
void Class86F88_3bb8c_j__ResetCounters(Class86ED0 *self)
{
    self->unk20 = 0;
    self->unk24 = 0;
    self->unk28 = 0;
}
```

Trivial three-field reset, an unrelated field group from `Class86F88_3bb8c_j__ClearCachedRefs`'s
(`unk34`/`unk38`/`unk50`). Matched first try.
