# Class86F88_3bb8c_j__AddChild -- MATCHED (33/33 words)

> Renamed from `func_80051D1C` on 2026-09-24 (tools/rename.py). Address 0x80051d1c.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86ED0`. This is the "add
child" method: registers `arg1` with the inherited `BasicClass::addChild`
and additionally caches it into one of two single-slot fields depending
on a tag read off `arg1`'s own vtable header word.

## Body

```c
void Class86F88_3bb8c_j__AddChild(Class86ED0 *self, void *arg1)
{
    s32 tag;

    if (arg1) {
        Get_vtable_BasicClass()->addChild(self, arg1);
        tag = **(s32 **)arg1 & 0xF;
        if (tag == 2) {
            self->unk34 = arg1;
        } else if (tag == 5) {
            self->unk38 = arg1;
        }
    }
}
```

`tag` is `*(s32 *)(*(void **)arg1) & 0xF` -- the low nibble of the header
word at `arg1`'s own vtable (`arg1->methods->header`, same convention as
the project's established `GenericTagMethods_3bb8c_c`, except this is a
FULL WORD read (`lw`) rather than a byte read, hence the raw double
pointer-deref instead of reusing that byte-typed struct). Matched first
try; note the `addChild` call happens BEFORE the tag check here, but
AFTER it in the sibling `Class86F88_3bb8c_j__RemoveChild` -- read each function's own
disassembly rather than assuming a shared order.
