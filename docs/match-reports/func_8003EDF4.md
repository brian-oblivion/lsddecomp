# func_8003EDF4 — MATCHED

Unit: `code_2cc8c_d`. Round 14, runner delta. 19/19 words, full match.

## Signature

```c
void func_8003EDF4(Unk18Obj *self);
```

Not a `D_8006E8E4` vtable slot; called directly by symbol.

## What it does

Teardown counterpart to `func_8003ECD0`'s init (this round, stalled at
71/73 — see that report; this function is unaffected by the stall, its
own logic is independent).

```c
void func_8003EDF4(Unk18Obj *self) {
    if (self->unk70 != 0) {
        DrawSync(0);
        func_80017CFC((void *)self->unk78);
        self->unk70 = 0;
    }
}
```

## Header changes

`include/code_2cc8c.h`: new extern `DrawSync(s32 a0)` (PsyQ library,
`asm/psyq_GsLinkObject4.s`, not decompiled).
