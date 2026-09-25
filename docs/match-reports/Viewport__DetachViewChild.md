# Viewport__DetachViewChild — MATCHED

> Renamed from `Unk18Obj__DetachViewChild` on 2026-09-25 (tools/rename.py). Address 0x8003eb84.

> Renamed from `func_8003EB84` on 2026-09-23 (tools/rename.py). Address 0x8003eb84.

Unit: `code_2cc8c_d`. Round 14, runner delta. 16/16 words, full match.

## Signature

```c
void Viewport__DetachViewChild(Unk18Obj *self);
```

`Unk18ObjMethods`'s own `+0x074` slot occupant.

## What it does

Teardown counterpart to `Viewport__AttachViewChild`'s init (this unit, this round):
removes `self->unk10` as a child via the inherited BasicClass
"removeChild" (`slot14`), only if it was ever set.

```c
void Viewport__DetachViewChild(Unk18Obj *self) {
    if (self->unk10 != NULL) {
        self->methods->slot14(self, self->unk10);
    }
}
```

## Header changes

None beyond the prototype — `slot14` was already typed from round 13.

## Naming

`Unk18Obj__DetachViewChild` -- tier B. Teardown counterpart of `Viewport__AttachViewChild`: removes `self->unk10` as a child through the inherited `removeChild` slot, only if it was ever set. Same tier-B caveat on "view" as the attach side.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__DetachViewChild`. Slot +0x074 `detachViewChild`; removes `viewNode`. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
