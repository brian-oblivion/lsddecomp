# func_8003EA7C — MATCHED

Unit: `code_2cc8c_d`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void func_8003EA7C(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x060` slot occupant.

## What it does

One-instruction field setter.

```c
void func_8003EA7C(Unk18Obj *self, s32 a1) {
    self->unk54 = a1;
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk54` (`+0x054`).
