# Class86F88_3bb8c_j__ReleaseResources -- MATCHED (28/28 words)

> Renamed from `func_800520A0` on 2026-09-24 (tools/rename.py). Address 0x800520a0.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86ED0`.

## Body

```c
void Class86F88_3bb8c_j__ReleaseResources(Class86ED0 *self)
{
    if (self->unk50) {
        self->methods->slot90(self);
        self->unk50 = self->unk50->methods->slot4(self->unk50);
    }
}
```

Established `self->unk50`'s type as `Class86ED0Handle *` (already
introduced this round from `Class86F88_3bb8c_j__LoadResources`'s evidence) and
`Class86ED0Methods::slot90` (+0x090, self-only). `Class86ED0HandleMethods::
slot4` (+0x004, self-only, returns a handle-typed pointer stored back into
`self->unk50` -- a "release, returns the successor/NULL" shape) was
already declared for `Class86F88_3bb8c_j__LoadResources`'s use; this is the second confirming
call site. Matched first try.
