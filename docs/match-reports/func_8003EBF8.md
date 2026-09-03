# func_8003EBF8 — MATCHED

Unit: `code_2cc8c_d`. Round 14, runner delta. 13/13 words, full match.

## Signature

```c
void func_8003EBF8(Unk18Obj *self, Vec3_2cc8c *a1);
```

`Unk18ObjMethods`'s own `+0x07C` slot occupant.

## What it does

Sibling of `func_8003EBC4`: copies `a1` wholesale into `self->unk20`,
guarded by the same `self->unk10` flag.

```c
void func_8003EBF8(Unk18Obj *self, Vec3_2cc8c *a1) {
    if (self->unk10 != NULL) {
        self->unk20 = *a1;
    }
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk20` (`+0x020`, `Vec3_2cc8c`,
immediately after `unk14`, no gap); `Unk18ObjMethods::slot7C` retyped from
`void *` to `Vec3_2cc8c *`.
