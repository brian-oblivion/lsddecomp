# Viewport__AttachViewChild — MATCHED

> Renamed from `Unk18Obj__AttachViewChild` on 2026-09-25 (tools/rename.py). Address 0x8003eacc.

> Renamed from `func_8003EACC` on 2026-09-23 (tools/rename.py). Address 0x8003eacc.

Unit: `code_2cc8c_d`. Round 14, runner delta. 46/46 words, full match (3
real attempts).

## Signature

```c
void Viewport__AttachViewChild(Unk18Obj *self, void *a1, void *a2, void *a3, void *arg5);
```

`Unk18ObjMethods`'s own `+0x044` slot occupant. `arg5` is the 5th
argument, passed on the incoming stack (o32 ABI, beyond the 4 register
args).

## What it does

One-time init, guarded by `self->unk10` (already typed `GenericObj *` from
round 13): registers `a1` as a child via the inherited BasicClass
"addChild" slot (`slot10`), dispatches `slot78`/`slot7C` with `a2`/`a3`,
dispatches `slot80` with `arg5` (or a default global, `gDefaultViewTwist`, when
`arg5` is `NULL`), then hands `&self->unk14` to `func_8003F2AC` (the next,
still-uncarved slice). Does nothing once `self->unk10` is already set.

```c
void Viewport__AttachViewChild(Unk18Obj *self, void *a1, void *a2, void *a3, void *arg5) {
    Unk18ObjMethods *m = self->methods;

    if (self->unk10 != NULL) {
        return;
    }
    m->slot10(self, a1);
    m->slot78(self, a2);
    m->slot7C(self, a3);
    m->slot80(self, arg5 != NULL ? arg5 : gDefaultViewTwist);
    func_8003F2AC(self->unk14);
}
```

## Two things the first two attempts got wrong

1. **`self->methods` needs to be cached in a local before the four
   dispatches, not re-read at each call site.** Retail loads `self->methods`
   into `$s1` once, before even the `self->unk10` guard check, and reuses
   it across all four vtable calls. A first attempt calling
   `self->methods->slotXX(...)` directly at each site re-read the field
   fresh after every intervening `jalr` (each call is opaque to the
   optimizer, so it can't assume the field is unchanged), one word short
   per re-read. This is the OPPOSITE of DECOMPILATION_LEARNINGS' existing
   "do not cache a `this->field` across an intervening vtable call" entry
   (`Entity__MoodCue34`) — that entry is about a case where caching was WRONG;
   here it's required. Both are real, and only the disassembly (does the
   register hold across the call, or get reloaded?) tells you which one a
   given function needs.
2. **The `arg5`-or-default fallback needs to be a plain ternary passed
   directly to the call, not an assignment that reuses the parameter.**
   Writing `if (arg5 == NULL) { arg5 = gDefaultViewTwist; } m->slot80(self,
   arg5);` made GCC promote the STACK-passed `arg5` into an extra
   callee-saved register (`$s2`), loaded eagerly at function entry — one
   register more than retail uses. Retail re-reads `arg5` from its own
   stack slot only once, right at the point of use, needing no register
   for it at all. `m->slot80(self, arg5 != NULL ? arg5 : gDefaultViewTwist);`
   (no reassignment of the parameter itself) reproduces this.

## Header changes

`include/code_2cc8c.h`:
- `Unk18Obj` gains `unk14` (`+0x014`, opaque `u8[0x030-0x014]` span,
  address-only — `func_8003F2AC` isn't decompiled in this project).
- `Unk18ObjMethods` gains `slot78`/`slot7C`/`slot80` (all `void (*)
  (Unk18Obj*, void*)`), splitting the old `pad078` span. Occupants (this
  unit): `Viewport__SetViewPoint`/`Viewport__SetViewRef` (both still queued as of this
  report) and `Viewport__SetTwist` (the documented `gp_rel` blocker, not
  decompiled).
- New externs `gDefaultViewTwist` (`asm/data/7B008.sdata.s`, address-only) and
  `func_8003F2AC` (`asm/code_2cc8c_e.s`, the next uncarved slice).

## Proposed learning

Confirms the register-caching question ("does `this->field` need to
survive across an intervening call, or does re-reading it match?") is
genuinely per-function and has to be read off the disassembly each time —
this function needed caching where an earlier-documented one (round 12's
`Entity__MoodCue34`) needed the opposite. Also: **a parameter used exactly
once, defaulted via `if (param == NULL) param = X;` immediately before its
one use, can still get promoted into an extra callee-saved register if the
compiler can't prove no intervening call needs it preserved — spelling the
default as a ternary passed directly into the call, without reassigning
the parameter, avoids the promotion.** Same family as this round's
`SceneNode__ComposeAndApplyRotation` (declaration-order-driven register hoisting) but a
different lever (expression form, not declaration order).

## Naming

`Unk18Obj__AttachViewChild` -- tier B. One-time init guarded by `self->unk10`: registers `a1` through the inherited `addChild` slot (which, per `Viewport__AddChild` in `code_2cc8c_c.c`, sets `self->unk10` itself when `a1`'s dynamic-class tag is 4), forwards `a2`/`a3` to `slot78`/`slot7C` (this unit's own `Viewport__SetViewPoint`/`Viewport__SetViewRef`), then hands `&self->unk14` to Sony's `GsSetRefView2`. "View" is inferred from that GsSetRefView2 hand-off, not proven for the field itself -- tier B, not A, per round 72's rule on asserting what data means.

## Proposed field names

Not applied -- `unk10` is shared with `code_2cc8c_c.c` (`Viewport__AddChild`/
`Viewport__RemoveChild`/`Viewport__Viewport`/`Viewport__Finalize` all touch
it), so this unit does not own it per track 3's ownership rule.

- `unk10` -> `viewChild` (tier B). Set by `Viewport__AddChild` (sibling unit)
  when a `GenericObj` child's dynamic-class tag is 4; every guard in THIS
  unit that reads it (`Viewport__SetViewPoint`, `Viewport__SetViewRef`,
  `Viewport__DetachViewChild`, `Viewport__AttachViewChild` itself) gates on
  whether a "view" is attached. Posted to the round-73 broadcast for the
  head to apply by type scope.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__AttachViewChild`. Slot +0x070 `attachViewChild(self, node, vp, vr, twist)`: the tier-B "view" reading is now settled, since &self+0x014 is the argument of GsSetRefView2, i.e. a GsRVIEW2 (`refView`), and the three setters write its vp, vr and rz. The default twist gDefaultViewTwist is a Ratio16 {0, 1}. TaskCore__OnInit passes a zero LongVec3 (D_8006E86C) as both points. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.

## Sony's headers (round 95, alpha, polish pass)

src/code_2cc8c_d.c now includes `<libgte.h>`, `<libgpu.h>` and `<libgs.h>` and its local prototypes of Sony functions are gone; every call takes Sony's own declaration, byte-identical. Interim casts at this function's call sites, until include/Viewport.h's ViewportOt/ViewportRefView become Sony's GsOT/GsRVIEW2: `GsSetRefView2((GsRVIEW2 *)&self->refView)`: `ViewportRefView` is Sony's GsRVIEW2 under a local name (include/Viewport.h).

The comments that sat on the deleted prototypes, moved here verbatim:

```c
/* GsSetRefView2 is Sony's (`libgs/gs_131.o`, linked from the SDK object).
   Declared LOCALLY rather than in include/code_2cc8c.h, which six units
   include: the real `LIBGS.H` prototype for this name will collide there.
   This is only the shape THIS unit's call sites use -- the real one takes a
   GsRVIEW2*. */
extern void GsSetRefView2(void *arg0);
```
