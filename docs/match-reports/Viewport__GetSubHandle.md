# Viewport__GetSubHandle — MATCHED

> Renamed from `Unk18Obj__GetSubHandle` on 2026-09-25 (tools/rename.py). Address 0x8003f230.

> Renamed from `func_8003F230` on 2026-09-23 (tools/rename.py). Address 0x8003f230.

Unit: `code_2cc8c_d`. Round 14, runner delta. 3/3 words, full match.

## Signature

```c
SubHandleObj *Viewport__GetSubHandle(Unk18Obj *self);
```

`Unk18ObjMethods`'s own `+0x0AC` slot occupant.

## What it does

A plain getter for the already-typed `unkB0` field (established by alpha's
round-13 `Viewport__Viewport`).

```c
SubHandleObj *Viewport__GetSubHandle(Unk18Obj *self) {
    return self->unkB0;
}
```

## Header changes

None beyond the prototype — `unkB0` was already typed `SubHandleObj *`.

## Naming

`Unk18Obj__GetSubHandle` -- tier A. Plain no-guard getter, `return self->unkB0;` -- the pure-leaf-getter case the tiering rule names explicitly as tier A by definition.
