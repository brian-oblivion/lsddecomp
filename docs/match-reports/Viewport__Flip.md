# Viewport__Flip — MATCHED

> Renamed from `Unk18Obj__Flip` on 2026-09-25 (tools/rename.py). Address 0x8003f04c.

> Renamed from `func_8003F04C` on 2026-09-23 (tools/rename.py). Address 0x8003f04c.

Unit: `code_2cc8c`. Round 14, runner delta. 87/87 words, full match (2
real attempts).

## Signature

```c
void Viewport__Flip(Unk18Obj *self);
```

`Unk18ObjMethods`'s own `+0x0A4` slot occupant (`slotA4`, dispatched by
`Viewport__OnNotifyTag1`, this round).

## What it does

Recomputes `self->unk74` from `self->unkC->methods->slot54`. If
`self->unkB8` is set, resets the graphics context (PsyQ `ResetGraph`) and
notifies `self->unkC->methods->slot50` — once unconditionally, plus once
more if `unk74` is still 0 — then forwards the current `unk74`-indexed
slot to two rendering helpers. If `self->unkB4` is also set, notifies
`slot50` a further time when `unk74` is still 0. Finally collapses
`self->unk74` to a plain boolean: 1 if it's 0, else 0.

```c
void Viewport__Flip(Unk18Obj *self) {
    s32 idx;
    u8 *rawBytes;

    if (self->unk70 == 0) {
        return;
    }

    self->unk74 = self->unkC->methods->slot54(self->unkC);
    if (self->unkB8 == 0) {
        goto tail_check;
    }

    ResetGraph(1);
    self->unkC->methods->slot50(self->unkC);

    if (self->unkB4 != 0) {
        if (self->unk74 == 0) {
            self->unkC->methods->slot50(self->unkC);
        }
    }

    idx = self->unk74;
    rawBytes = (u8 *)&self->unk58;
    GsSortClear(rawBytes[0], rawBytes[1], rawBytes[2],
                  *(s32 *)((u8 *)self + 0x78 + idx * 4));

    idx = self->unk74;
    func_8003FBF4(*(s32 *)((u8 *)self + 0x78 + idx * 4));

    if (self->unkB4 != 0 && self->unk74 == 0) {
        self->unkC->methods->slot50(self->unkC);
    }

tail_check:
    self->unk74 = (self->unk74 == 0);
}
```

## What the first attempt got wrong

Misread the tail's control flow as an early-exit special case (`if
(self->unk74 != 0) { self->unk74 = 0; return; }`) instead of recognizing
it as the SAME "only dispatch `slot50` when `unk74 == 0`" guard already
used earlier in the function, both converging on the identical final
`self->unk74 = (self->unk74 == 0);` computation. Retail's own `bnez
v0,.L8003F190` jumps INTO the shared tail computation (with the boolean
already computed in the branch's delay slot), not to a separate inline
epilogue — writing the guard as `if (self->unkB4 != 0 && self->unk74 ==
0) { dispatch }` followed by one shared tail statement reproduces this
exactly, and is simpler than the first attempt besides.

## Header changes

`include/code_2cc8c.h`:
- `GenericObjMethods` gains `slot50` (`void`, return unused) and `slot54`
  (`s32`, return stored into `self->unk74`) — real occupants unknown
  (`self->unkC`'s dynamic class isn't otherwise identified in this unit).
- New externs: `ResetGraph` (PsyQ, `LIBGPU.H`'s own declared signature —
  declared locally rather than including the whole SDK header, matching
  this unit's existing style for PsyQ calls), `GsSortClear` (PsyQ,
  reads `self->unk58`'s bytes UNSIGNED — same "writer reads signed, this
  reader reads unsigned" situation as `unk5B`/`Viewport__Update`, this
  round), `func_8003FBF4` (next slice, uncarved).

## Naming

`Unk18Obj__Flip` -- tier A. The `slotA4` occupant: recomputes `unk74` (a 0/1 double-buffer index) from a child object's own method, optionally resets the graphics context, clears+draws the CURRENT OT half (`GsSortClear`+`GsDrawOt`, both indexed by `unk74`), then collapses `unk74` to the other of 0/1 for next call -- the standard double-buffer flip shape (recompute index, drain current buffer, toggle for next frame).

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__Flip`. Slot +0x0A4 `flip`. `unkC` is `drawSystem` (the class-1 child, gDrawSystemMethods), whose +0x054 (DrawSystem__GetActiveBuffer) gives otIndex and +0x050 (DrawSystem__SwapBuffers) swaps; reached through code_2cc8c.h's GenericObj view with a cast, since DrawSystem has no header. `unkB8` is `drawEnabled`; the OT reads are `ot[idx]`. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.

## Sony's headers (round 95, alpha, polish pass)

src/code_2cc8c.c now includes `<libgte.h>`, `<libgpu.h>` and `<libgs.h>` and its local prototypes of Sony functions are gone; every call takes Sony's own declaration, byte-identical. Interim casts at this function's call sites, until include/Viewport.h's ViewportOt/ViewportRefView become Sony's GsOT/GsRVIEW2: `GsSortClear(..., (GsOT *)self->ot[idx])` and `GsDrawOt((GsOT *)self->ot[idx])`.

The comments that sat on the deleted prototypes, moved here verbatim:

```c
/* Sony's `GsDrawOt` (libgs/gs_111, linked from the SDK object since round
 * 34; was func_8003FBF4, and was declared in include/code_2cc8c.h until this
 * round). Local for the same collision reason as the three above. Sony's own
 * argument is a `GsOT *`; this call site passes the same otIndex-indexed slot
 * it hands GsClearOt, as a plain word, and is left that way.
 * gs_111 and gs_112 are byte-identical objects defining GsDrawOt and
 * GsDrawOtIO at this one address -- gs_111/GsDrawOt is what the build links. */
extern void GsDrawOt(ViewportOt *ot);

/* Sony's `GsSortClear` (libgs/gs_001, fingerprint exact vs the disc corpus,
 * not yet linked from an SDK object). Local for the same collision reason as
 * the three above: LIBGS.H's own prototype is `void GsSortClear(u_char r,
 * u_char g, u_char b, GsOT *ot);`. This call site reads self->clearColor's own
 * three bytes UNSIGNED (same "writer reads signed, this reader reads
 * unsigned" situation as farColor/Viewport__Update) and passes the fourth as a
 * plain word, same as GsClearOt/GsDrawOt above. */
extern void GsSortClear(u8 a0, u8 a1, u8 a2, ViewportOt *ot);
```


## Track 7 (round 95, alpha, polish pass)

The clear colour is passed as `self->clearColor.r/.g/.b` (GsSortClear's own `unsigned char` parameters make the load `lbu`), no `u8 *` over the struct; the slot-offset notes (+0x050, +0x054) are gone from the comment, the slot names say it. Byte-identical.

Round 96 (alpha, track 6). include/Viewport.h's local `ViewportOt` (a
0x14-byte view of the GsOT header: length, org, pad) is deleted: `ot[2]` is
Sony's `GsOT *`, `otTags[2]` Sony's `GsOT_TAG *` (each header's `org`) and
`workBase[2]` Sony's `PACKET *` (GsSetWorkBase's argument), and every unit
including Viewport.h takes Sony's headers after common.h. The `(GsOT *)`
casts at GsClearOt, GsSortClear, GsDrawOt and drawNode's five sort calls and
Update's `(PACKET *)` cast are gone. Byte-identical (whole image green).


## Track 7 (round 100, echo, polish pass)

Field `unkB4` is now `extraSwap` (see Viewport__SetExtraSwap.md).
