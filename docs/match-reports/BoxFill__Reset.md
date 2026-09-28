# BoxFill__Reset -- MATCH (37/37 words, first attempt)

> Renamed from `ClassEAC0__FinishConstruct` on 2026-09-25 (tools/rename.py). Address 0x800405d0.

> Renamed from `func_800405D0` on 2026-09-20 (tools/rename.py). Address 0x800405d0.

Unit `ScreenWidgets`, carved round 14. `ClassEAC0Methods::finishConstruct` (`+0x040`),
dispatched by the class's own ctor (`BoxFill__BoxFill`).

```c
void BoxFill__Reset(ClassEAC0Obj *self, BoxFillSize *a1, void *a2, s32 a3) {
    ClassEAC0Methods *methods;

    self->unk44 = a3;
    self->unk48 = 1;
    self->unk4C = 0;
    self->unk58 = 0;
    self->unk5C = 0;
    self->unk5E = 0;
    self->unk60 = a1->x;
    self->unk62 = a1->y;
    methods = self->methods;
    if (a2 == NULL) {
        a2 = sBoxFillDefaultColor;
    }
    methods->slotB8(self, 1, a2);
    self->methods->slotCC(self, 0xD);
}
```

**Correction: `self->methods` needs to be hoisted into a local BEFORE the
`a2` null-check**, not read fresh at the `slotB8` call site. An earlier
version of this report read `self->methods` inline at the call and claimed
37/37 -- taken during the stale-build window described in
`New_FadeBox.md`. Genuinely rebuilt, that inline form regressed
(16/37): after `FadeBoxObj::unk60`/`unk62` were retyped `s16` -> `u16`
(see `FadeBox__PushPosition.md`), GCC stopped loading `self->methods` early and
deferred it past the `a2` check instead, where retail loads it right after
storing `unk62`. Hoisting it into a named local restores retail's early
load. Worth noting as a SHARED-FIELD side effect: a field TYPE change in
`include/Task.h` regressed an unrelated, already-matched function's
SCHEDULING (not its correctness) purely by changing surrounding register
pressure -- a milder cousin of the project's documented "any struct edit
is potentially non-local" caution, this time via codegen shape rather than
a byte-offset shift.

## Misread caught before committing

First reading treated `a2` as a MODE flag (0 vs nonzero) selecting between
two fixed tables (`sBoxFillDefaultColor`/`sFadeBoxBlackColors`). Re-reading the raw
instructions closely: retail's `bnez $a2, .L80040628` branches on `a2`
ITSELF being nonzero, and when `a2 == 0` the fallthrough OVERWRITES `a2`
with `&sBoxFillDefaultColor` before the shared call -- i.e. `a2` isn't a flag at
all, it IS the `tableEntry` pointer, with `sBoxFillDefaultColor` only as the
default when the caller passes `NULL`. Same family as the project's
established "a discarded/defaulted value is not evidence of the wrong
type" caution, just for a parameter rather than a return.

## Naming (round 61, track 3)

**`BoxFill__Reset`** -- tier A. `ClassEAC0Methods::finishConstruct`
(`+0x040`), dispatched by `BoxFill__BoxFill` immediately after
installing `self->methods` -- the identical architectural role, at the
identical offset, as `FadeBox__Reset` one level up and
`TextRow__Reset` (round 54, this same table family) one level
further down. See `FadeBox__Reset.md`'s naming note for the
full three-occupant cross-check.

## Track 4 (2026-09-25, round 85, charlie)

Class 0x64 (was D_8006EAC0) is unified as BoxFill in include/BoxFill.h: Viewport__DrawNode draws a node whose class-id low byte is 0x64 with GsSortBoxFill over the GsBOXF at +0x058 (pri +0x044, `relative` +0x048, x/y +0x050/+0x054). The body now takes `BoxFill *`; zero bytes changed. Renamed from `ClassEAC0__FinishConstruct`: the +0x040 occupant, SceneNode's `reset` slot, named for its slot (FINISHING-PLAN track 4 step 6; tier A for the slot, the body initialises every BoxFill field: pri from the ctor's third argument, relative = 1, GsBOXF attribute/x/y = 0, w/h from the size pair, colour (default sBoxFillDefaultColor) through setColor, mask through setMask(13)). Its parameter list differs from the slot's (self only); the slot keeps SceneNode's type and the ctor casts to BoxFillResetFn. Unlike SceneNode__Reset it does not reset the coordinate; it never calls the base.

## Track 6 (round 99, alpha)

`SkipShort2` (`s16 x; u8 pad2[2]; s16 y;`, named for its layout) is now
`BoxFillSize` (`tools/renametype.py SkipShort2 BoxFillSize --any-stem`), and
its fields are what every caller passes: two words, `s32 w; s32 h;`. Storing
an s32 field into the u16 `boxW`/`boxH` still reads only the low halfword
(`lhu` at +0x000/+0x004): measured byte-exact with the whole image, so the
halfword-and-gap view was never load-bearing. Accessors `size->x`/`->y`
became `size->w`/`->h` in BoxFill__Reset, FadeBox__PushPosition and
BoxFill__SetSize (which still casts: the setSize slot keeps `s32 *`, since
its caller in Task passes an `s32 size[2]`).

## Track 7 (round 100, charlie)

### Naming

- **`sBoxFillDefaultColor`** (was `D_8008A924`) -- tier A. An sdata word
  `80 80 80 00`: the colour Reset passes to setColor when its `color`
  argument is NULL, and its only reader. Named for that one use.
