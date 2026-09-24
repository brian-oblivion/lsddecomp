# Class86F88_3bb8c_j__NotifyChild -- MATCHED (44/44 words)

> Renamed from `func_80051E64` on 2026-09-24 (tools/rename.py). Address 0x80051e64.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86ED0`.

## Body

```c
void Class86F88_3bb8c_j__NotifyChild(Class86ED0 *self, void *arg1, s32 arg2)
{
    s32 tag;

    Get_vtable_BasicClass()->slot38(self, arg1, arg2);
    tag = **(s32 **)arg1 & 0xF;
    if (tag == 2) {
        self->methods->slot5C(self, arg1, arg2);
    } else if (tag == 5) {
        self->methods->slot58(self, arg1, arg2);
    }
}
```

Calls the inherited `BasicClass::slot38` (already declared in
`include/class_3bb8c.h`'s `BasicMethods866E8F`, established by
class_3bb8c_f) unconditionally first, then dispatches through Class86ED0's
OWN vtable (`slot5C`/`slot58`) based on the same tag-nibble convention as
`Class86F88_3bb8c_j__AddChild`/`Class86F88_3bb8c_j__RemoveChild`. Established `Class86ED0Methods::slot5C`
(+0x05C, "tag==2") and `slot58` (+0x058, "tag==5") from this function.
Matched first try.
