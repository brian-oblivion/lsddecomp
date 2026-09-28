# TaskCore__OnInit

> Renamed from `TaskCoreObj__func_8003C238` on 2026-09-25 (tools/rename.py). Address 0x8003c238.

> Renamed from `func_8003C238` on 2026-09-23 (tools/rename.py). Address 0x8003c238.

> **Type rename note (round 73, track 3):** every `TaskCoreObjMethods` /
> `TaskCoreObj` below (the type this function's own body split off from
> `TaskCoreMethods`) was renamed to `StreamTaskUnk18Methods` /
> `StreamTaskUnk18Obj` in `include/Task.h` -- `TaskCoreObj` was doing
> double duty for two unrelated classes (this one, and New_TaskCore's own
> 0xA4-byte allocation), and freeing the name for the latter matches the
> `StreamTaskUnkNNObj` convention already used for this class's other private
> sub-objects. Pure type rename, no compiled bytes changed; not done through
> `tools/rename.py` since C type names carry no address for it to resolve.
> The text below is left as originally written for the history.

**Unit:** Task · **Size:** 102 words · **Status:** MATCHED (102/102)

This occupies `TaskCoreMethods`'s (`gTaskCoreMethods`) own slot `+0x04C` (per
`classtable.py gTaskCoreMethods`, and confirmed by `StreamTask__OnInit`'s own
single-argument call into this exact function). It is the unit's largest
queued function and the round's last one; the head flagged it as likely to
carry a surviving residue. It matched byte-exact on the first real attempt
instead, after a header-only correction pass caught two real modeling
mistakes before any bytes were compared.

## Summary

```c
void TaskCore__OnInit(StreamTaskObj *self) {
    TaskCoreObj *unk18;
    TaskCoreObjMethods *core;

    unk18 = self->unk18;
    core = unk18->methods;
    self->methods->slotE0(self, self->unk14);
    self->unk78->methods->slot4C(self->unk78, self->unk14, 0);
    if (self->unk88 != 0) {
        self->methods->slotE4(self, &self->unk90);
        self->unk78->methods->slotB8(self->unk78, 1, &self->unk90);
    }
    if (self->unk74 == 0) {
        self->unkC->unk0->methods->slot78(self->unkC->unk0, &self->unk90, sDefaultMovieFrame);
    }
    self->unkC->unk0->methods->slot78(self->unkC->unk0, &self->unk90, 0);
    core->slot48(unk18, self->unk28);
    core->slot4C(unk18, self->unk2C);
    core->slot50(unk18, self->unk30);
    core->slot70(unk18, self->unk14, sTaskCoreViewOrigin, sTaskCoreViewOrigin, 0);
    core->slot8C(unk18);
    self->unk38 = 0;
}
```

## Two real modeling corrections, found by reading arity conflicts before writing any C

Both corrections were forced by genuine arity mismatches at struct offsets
already given a type in earlier rounds -- reading the raw disassembly first,
before touching the header, is what surfaced them; neither was visible from
`funcdiff` (the whole build stayed green throughout, since the corrections
only retyped existing fields/slots, never changed a compiled instruction).

**1. `self->unk18` is NOT an instance of `TaskCoreMethods`'s class.**
`TaskCore__OnDeinit` (earlier this round) modeled `self->unk18->methods` as
sharing `TaskCoreMethods` (`gTaskCoreMethods`'s own vtable shape), on the strength
of `gTaskCoreMethods` happening to have non-null entries at the two offsets
(`+0x074`, `+0x090`) that function called. This function calls THREE MORE
slots on `self->unk18` -- `+0x048`, `+0x04C`, `+0x050`, all 2-argument
setters. `gTaskCoreMethods`'s own `+0x04C` is `TaskCore__OnInit` -- this very
function -- and it is unambiguously single-argument (confirmed by
`StreamTask__OnInit`'s already byte-exact call). A vtable slot's signature has to
agree across every instance of one class; a genuine arity conflict at a
shared offset is proof `self->unk18` is a sibling class, not the same one.
Split into a new `TaskCoreObjMethods` type (`include/Task.h`), moving
`slot74`/`slot90` out of `TaskCoreMethods` into it and adding the three new
slots plus `+0x08C`. **Updated `TaskCore__OnDeinit.md`'s "New structure
discovered" section with a correction note** rather than rewriting it.

**2. `self->unk78` is NOT the same class as `StreamTaskUnkB4Obj`.**
`TaskCore__Finalize` (this round, function before this one) unified
`self->unk78` into `StreamTaskUnkB4Obj` on 5-way evidence: five sibling
fields all torn down identically via a 1-argument `slot04`. This function
calls `self->unk78`'s own slot `+0x04C` with **3 arguments**
(`self->unk78->methods->slot4C(self->unk78, self->unk14, 0)`), where
`StreamTaskUnkB4Methods::slot4C` (from the already-matched `StreamTask__Exit`,
called on `self->unkB4`) is fixed at 1 argument. Same reasoning as above:
real arity conflict at a shared offset means these are sibling classes that
happen to agree at `slot04` (likely via a shared base), not one class.
Reverted `self->unk78` to its own `StreamTaskUnk78Obj/Methods` type, now
with three known slots (`04`, `4C` here, `50` from `TaskCore__OnDeinit`, `B8`
here). **Updated `TaskCore__Finalize.md` with a correction note** rather than
rewriting it, and left `unk48`/`unk74`/`unk7C`/`unk80` unified with
`StreamTaskUnkB4Obj` as-is -- no function has yet called a conflicting slot
on any of those four, so there is no counter-evidence to act on there.

## New structure discovered

- `StreamTaskObjMethods` gained `slotE0(self, s32 a1)` and
  `slotE4(self, u8 *a1)`, both void, both this function's own forward
  targets.
- New fields: `unk14` (`+0x014`, `s32`), `unk88` (`+0x088`, `s32`, boolean
  guard), `unk90` (`+0x090`, `u8`, address-only -- 3 bytes before `unk93`,
  same buffer).
- `TaskTextMethods::slot78`'s 3rd parameter widened from `s32 flag` to
  `u8 *flag`: `TaskCore__OnDeinit`'s call passes literal `0` (valid for either
  type, unchanged bytes there), this function's own call passes
  `sDefaultMovieFrame`, a real rodata address.
- Two new rodata externs, address-only: `sDefaultMovieFrame`, and `sTaskCoreViewOrigin`
  (passed **twice, same address**, as `TaskCoreObjMethods::slot70`'s 3rd
  AND 4th arguments -- an odd but unambiguous call-site fact, left
  unexplained).

## Third-learning check (per head's request)

**Needed, and in the least obvious form yet: a value computed at the TOP of
the function, before ANY call, still needed a local because it survives
MANY intervening calls to the very end.** `self->unk18->methods` (`core` in
the C above) is loaded in the first few instructions -- before even the
first `jalr` -- and is not used again until the function's last five
statements, after roughly a dozen intervening calls. Declaring `unk18` and
`core` as the function's first two C89 block-top declarations, each
initialized from the previous, reproduced retail's own instruction order
exactly (`self->unk18` loaded, then immediately dereferenced into `core`,
both before `slotE0`'s call) with zero reshaping needed afterward. The
generalization: this lever is not about "reuse near the point of use" --
it's about the value's LIFETIME crossing any call at all, however far
ahead of its next use that call is.

## Proposed learning

**An arity conflict at a shared vtable offset between two fields
UN-unifies them, and it is real counter-evidence, not noise.** This round
produced two: `self->unk18` vs. `TaskCoreMethods`, and `self->unk78` vs.
`StreamTaskUnkB4Obj`. Both were caught by writing out the FULL argument
list for every `jalr` in the raw disassembly before touching the header --
an arity that doesn't match an already-established slot's signature is a
hard stop, not a detail to gloss over. Where the round's earlier unification
lessons (`TaskCore__Finalize`) taught "same slot number + same access pattern +
same caller is strong positive evidence," this function's residue-free match
teaches the complementary negative: a single conflicting arity at that same
slot number is enough to override it, because a class's vtable ABI must be
uniform across every instance and override.

## Naming

**TaskCoreObj__func_8003C238** -- tier C. Occupies `gTaskCoreMethods` slot
`+0x04C`, the unit's largest function: commits several previously-configured
fields into `self->unk18` (`StreamTaskUnk18Obj`, this same round's type
rename -- see the note above) and drives the `TaskText` sub-object twice.
Every individual call is understood (see "Summary" above), but no single
verb covers what the function accomplishes as a whole, so left
`Class__func_xxxxx`.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from TaskCoreObj__func_8003C238 (tools/rename.py). Occupant of +0x04C (`onInit`, the call IntermediateBase__Init makes after adding the children), named for the slot. IntermediateBase types the slot (self, s32, s32, s32) from that call; this occupant takes self alone. The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-25, round 85, bravo)

The viewport is cast to `Viewport *` (include/Viewport.h, round 85) instead of the local StreamTaskUnk18Obj view, and its slots are called by name: +0x048 setOtLength (unk28), +0x04C setUnk44 (unk2C), +0x050 setUnk48 (unk30), +0x070 attachViewChild (unk14, &sTaskCoreViewOrigin twice, NULL twist), +0x08C initOt. sTaskCoreViewOrigin is a zero LongVec3. Byte-identical.

## Track 4 (2026-09-26, round 88, alpha)

bgLayer is a `BgLayer *` (include/BgLayer.h): the StreamTaskUnk78Obj casts are gone, and the two calls are by slot name, `attachToParent(bgLayer, (SceneNode *)unk14, NULL)` (+0x04C, SceneNode's; returns SceneNode *, discarded, where the view said void) and `setColor(bgLayer, 1, (BgLayerRgb *)baseColor)` (+0x0B8). Byte-identical.

## Track 7 (2026-09-27, round 99, alpha)

- `TaskTextObj`/`TaskTextMethods`, this unit's local view of
  `initArgs->drawSystem`, is retired: its only slot, `+0x078 slot78`, is
  `DrawSystem::clearImage(self, color, rect)` (include/DrawSystem.h), which
  TitleMenu__OnDeinit already calls through `DrawSystem *`. The calls now cast
  to `DrawSystem *`; byte-identical. The two clears are therefore: the
  default movie frame (`&sDefaultMovieFrame`, a DrawRect
  {640, 0, 320, 240}) to baseColor when there is no sub handle, then the whole
  screen (NULL rect) to baseColor.
- `unk28` -> `otLength` (tier A: onInit passes it to the viewport's
  setOtLength) and `unk30` -> `packetSize` (tier B, see TaskCore__Reset).
  Accessor set from the compiler's error list: this unit only.
- `D_8006E86C` -> `sTaskCoreViewOrigin` (tier A: a zero LongVec3 passed as
  both attachViewChild's viewpoint and reference point; only this unit
  reaches it).

### History moved from include/code_2c054.h

- Round 2 (this function's match): the slot's third parameter was widened
  from `s32` to a pointer, because TaskCore__OnDeinit's call passes a literal
  0 (valid for either) but this function's passes
  `&gDefaultStreamTaskInitData`, a real address. `gDefaultStreamTaskInitData`
  and `sTaskCoreViewOrigin` were reached only by address and typed as a byte
  array and a LongVec3.
- The viewport's local view here (`StreamTaskUnk18Obj`) was merged into
  include/Viewport.h in round 85; StreamTask::player's (`StreamTaskUnkB4Obj`)
  into include/MoviePlayer.h in round 89.

## Proposed field names

Accessors outside Task (the compiler's error list), so for the head
to apply by type scope:

- IntermediateBase `+0x014 unk14` -> `lightRig`: its own comment says it is
  `initArgs->lightRig` or init's `New_LightRig()`; this function hangs the
  BgLayer and the slot widgets under it and attaches the view to it.
  Accessors: Task.c, TitleMenuTaskObjF.c, ObjMStyleActor.c, class_3bb8c_m.c.
- TaskCore `+0x02C unk2C` -> `packetCount` (tier B, TaskCore__Reset's
  report): handed to the viewport's `setUnk44`. Accessors: Task.c,
  TitleMenuTaskObjF.c (TitleMenu__Reset), ObjMStyleActor.c (GraphRoom__Reset).
  Viewport's own `unk44`/`unk48` and `setUnk44`/`setUnk48` would follow as
  `packetCount`/`packetSize`.


## Track 6 (2026-09-27, round 99, runner bravo)

`gDefaultStreamTaskInitData` is renamed `sDefaultMovieFrame` (tier B, see GetDefaultMovieFrame.md): the rect this function clears when there is no sub handle is the frame StreamTask builds its MoviePlayer with. That the clear is FOR the movie area is not established; the name records where else the rect goes.

## Track 10 (2026-09-28, round 104, echo)

TaskCore fields renamed (include/TaskCore.h): `unk2C` -> `maxPackets` (the value onInit passes to the viewport's setMaxPackets), `unk34` -> `clearOnDeinit` (onDeinit clears the screen only while it is nonzero), `unk93` -> `clearColor` (setColors' `clear` argument, the colour onDeinit clears to); TaskCoreTarget `unk8` -> `initialSlot` (setState(ACTIVE)'s setActiveSlot argument). Byte-identical (whole image green). The 300/400 packet counts stay literal: they are per-class tuning values beside the field that names them, like fadeRate and otLength.
