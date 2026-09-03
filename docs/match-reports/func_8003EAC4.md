# func_8003EAC4 — MATCHED

Unit: `code_2cc8c_d`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void func_8003EAC4(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x06C` slot occupant.

## What it does

One-instruction field setter, last of the `unk3C`/`unk40`/`unk54`/`unk60`
family.

```c
void func_8003EAC4(Unk18Obj *self, s32 a1) {
    self->unk60 = a1;
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk60` (`+0x060`).
