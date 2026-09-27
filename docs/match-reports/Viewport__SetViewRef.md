# Viewport__SetViewRef — MATCHED

> Renamed from `Unk18Obj__SetUnk20` on 2026-09-25 (tools/rename.py). Address 0x8003ebf8.

> Renamed from `func_8003EBF8` on 2026-09-23 (tools/rename.py). Address 0x8003ebf8.

Unit: `code_2cc8c`. Round 14, runner delta. 13/13 words, full match.

## Signature

```c
void Viewport__SetViewRef(Unk18Obj *self, Vec3_2cc8c *a1);
```

`Unk18ObjMethods`'s own `+0x07C` slot occupant.

## What it does

Sibling of `Viewport__SetViewPoint`: copies `a1` wholesale into `self->unk20`,
guarded by the same `self->unk10` flag.

```c
void Viewport__SetViewRef(Unk18Obj *self, Vec3_2cc8c *a1) {
    if (self->unk10 != NULL) {
        self->unk20 = *a1;
    }
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk20` (`+0x020`, `Vec3_2cc8c`,
immediately after `unk14`, no gap); `Unk18ObjMethods::slot7C` retyped from
`void *` to `Vec3_2cc8c *`.

## Naming

`Unk18Obj__SetUnk20` -- tier A. Sibling of `Viewport__SetViewPoint`: same whole-Vec3-copy shape guarded by `self->unk10`, writes `unk20`. No consumer of `unk20` was found anywhere in this unit (unlike `unk14`), so kept as a plain mechanical name rather than guessing a role.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetUnk20`. Renamed for the GsRVIEW2 member it writes: `refView.vr`, the reference point. Slot +0x07C `setViewRef`, parameter `LongVec3 *`. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
