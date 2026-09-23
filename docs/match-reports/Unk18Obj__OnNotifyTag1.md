# Unk18Obj__OnNotifyTag1 — MATCHED

> Renamed from `func_8003EE88` on 2026-09-23 (tools/rename.py). Address 0x8003ee88.

Unit: `code_2cc8c_d`. Round 14, runner delta. 14/14 words, full match.

## Signature

```c
void Unk18Obj__OnNotifyTag1(Unk18Obj *self, GenericObj *arg1, s32 arg2);
```

`Unk18ObjMethods`'s own `+0x098` slot occupant (`slot98`, dispatched by
`Unk18Obj__OnNotify`, this round).

## What it does

```c
void Unk18Obj__OnNotifyTag1(Unk18Obj *self, GenericObj *arg1, s32 arg2) {
    if (arg2 == 2) {
        self->methods->slotA4(self);
    }
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18ObjMethods` gains `slotA4` (`+0x0A4`,
`void (*)(Unk18Obj*)`), splitting the pad between `slot9C` and the
already-typed `slotA8`.
