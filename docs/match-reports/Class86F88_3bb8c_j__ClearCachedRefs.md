# Class86F88_3bb8c_j__ClearCachedRefs -- MATCHED (4/4 words)

> Renamed from `func_80051C74` on 2026-09-24 (tools/rename.py). Address 0x80051c74.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86ED0`.

## Body

```c
void Class86F88_3bb8c_j__ClearCachedRefs(Class86ED0 *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk50 = NULL;
}
```

Trivial three-field reset (the two "tagged child" caches plus `unk50`).
Matched first try -- the trailing store naturally lands in the `jr $ra`
delay slot.
