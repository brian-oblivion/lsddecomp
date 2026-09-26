# Viewport__GetFadeBox — MATCHED

> Renamed from `Viewport__GetSubHandle` on 2026-09-26 (tools/rename.py). Address 0x8003f230.

> Renamed from `Unk18Obj__GetSubHandle` on 2026-09-25 (tools/rename.py). Address 0x8003f230.

> Renamed from `func_8003F230` on 2026-09-23 (tools/rename.py). Address 0x8003f230.

Unit: `code_2cc8c_d`. Round 14, runner delta. 3/3 words, full match.

## Signature

```c
SubHandleObj *Viewport__GetFadeBox(Unk18Obj *self);
```

`Unk18ObjMethods`'s own `+0x0AC` slot occupant.

## What it does

A plain getter for the already-typed `unkB0` field (established by alpha's
round-13 `Viewport__Viewport`).

```c
SubHandleObj *Viewport__GetFadeBox(Unk18Obj *self) {
    return self->unkB0;
}
```

## Header changes

None beyond the prototype — `unkB0` was already typed `SubHandleObj *`.

## Naming

`Unk18Obj__GetSubHandle` -- tier A. Plain no-guard getter, `return self->unkB0;` -- the pure-leaf-getter case the tiering rule names explicitly as tier A by definition.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__GetSubHandle`. Slot +0x0AC `getSubHandle`, returns `SceneNode *`. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
