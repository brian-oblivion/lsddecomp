# Viewport__DeinitOt — MATCHED

> Renamed from `Unk18Obj__DeinitOt` on 2026-09-25 (tools/rename.py). Address 0x8003edf4.

> Renamed from `func_8003EDF4` on 2026-09-23 (tools/rename.py). Address 0x8003edf4.

Unit: `code_2cc8c_d`. Round 14, runner delta. 19/19 words, full match.

## Signature

```c
void Viewport__DeinitOt(Unk18Obj *self);
```

Not a `gViewportMethods` vtable slot; called directly by symbol.

## What it does

Teardown counterpart to `Viewport__InitOt`'s init (this round, stalled at
71/73 — see that report; this function is unaffected by the stall, its
own logic is independent).

```c
void Viewport__DeinitOt(Unk18Obj *self) {
    if (self->unk70 != 0) {
        DrawSync(0);
        BMemPMgrFree((void *)self->unk78);
        self->unk70 = 0;
    }
}
```

## Header changes

`include/code_2cc8c.h`: new extern `DrawSync(s32 a0)` (PsyQ library,
`asm/psyq_GsLinkObject4.s`, not decompiled).

## Naming

`Unk18Obj__DeinitOt` -- tier A. Teardown counterpart of `Viewport__InitOt`, guarded by the same `unk70` latch: calls Sony's `DrawSync(0)`, frees the `unk78` buffer, clears `unk70`.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__DeinitOt`. Slot +0x090 `deinitOt`; frees `ot[0]`, the start of InitOt's allocation. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
