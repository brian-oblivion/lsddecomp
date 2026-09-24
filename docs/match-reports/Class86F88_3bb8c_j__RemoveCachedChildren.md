# Class86F88_3bb8c_j__RemoveCachedChildren -- MATCHED (22/22 words)

> Renamed from `func_8005217C` on 2026-09-24 (tools/rename.py). Address 0x8005217c.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86ED0`.

## Body

```c
void Class86F88_3bb8c_j__RemoveCachedChildren(Class86ED0 *self)
{
    self->methods->slot14(self, self->unk34);
    self->methods->slot14(self, self->unk38);
    self->unk3C = 0;
}
```

Calls `Class86ED0Methods::slot14` (established this round from
`Class86F88_3bb8c_j__AddChild`'s asm, self+one pointer arg) twice, once per tagged-child
cache, then clears `self->unk3C` (also established this round, from
`Class86F88_3bb8c_j__AddChildAndSetState`). Matched first try.
