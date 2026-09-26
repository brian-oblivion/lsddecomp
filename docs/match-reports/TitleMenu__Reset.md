# TitleMenu__Reset -- MATCH

> Renamed from `Class86B60__Reset` on 2026-09-26 (tools/rename.py). Address 0x8004d814.

> Renamed from `TitleMenu__ShowTitleIcon` on 2026-09-26 (tools/rename.py). Address 0x8004d814.

> Renamed from `func_8004D814` on 2026-09-24 (tools/rename.py). Address 0x8004d814.

Unit `class_3bb8c_d`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TitleMenu__Reset`: 33/33 words match.

## Source

```c
void TitleMenu__Reset(TitleMenu *self)
{
    self->unk34 = 0;
    self->unk2C = 0x190;
    self->methods->slotD4(self, &D_800114E8, 0);
    self->methods->slot6C(self, 0xA);
    self->unkA4->methods->slotF0(self->unkA4, 0, 0);
}
```

First attempt, byte-exact. Retail's instruction order (two field stores,
then three vtable calls in this exact order) transcribed directly with no
reshaping needed.

## Struct changes (additive, `include/class_3bb8c.h`)

- `TitleMenu::unk2C` (s32, `= 0x190`), `TitleMenu::unk34` (s32, `= 0`) --
  new fields carved out of the existing `pad004` gap.
- `TitleMenuMethods::slot6C` (`self, s32 arg1`), `::slotD4` (`self, void
  *arg1, s32 arg2`) -- new slots.
- `TitleMenu::unkA4` **retyped** from `void *` to `DreamSysView_3bb8c_c *`
  -- this is the first function to dereference it through its own vtable
  (`slotF0`) rather than only forwarding it opaquely (as
  `TitleMenu__TitleMenu`/`FormatNumberIntoBuffer`, both already matched in
  `class_3bb8c_c.c`, do). Same size (4 bytes), so no layout change; the
  existing assignment `self->unkA4 = dreamSys;` in `TitleMenu__TitleMenu` (where
  `dreamSys` is a `void *` parameter) still compiles under ordinary C
  pointer-conversion rules -- confirmed by the full rebuild going green.
  Added a forward `typedef struct DreamSysView_3bb8c_c
  DreamSysView_3bb8c_c;` ahead of `TitleMenu`'s own definition so the
  pointer type is visible there; the struct's full body stays at its
  original location further down the file.
- `DreamSysViewMethods_3bb8c_c::slotF0` (`self, s32 arg1, s32 arg2`, both
  call-site arguments literal `0`) -- new slot. Distinct from
  `TitleMenuMethods::slotF0` (see `TitleMenu__SetState`'s report) -- same
  offset number, two unrelated tables, no conflict.
- New `extern s32 D_800114E8;` (address-of only, placeholder type, same
  convention as the neighbouring `D_80086D44`/`D_800114DC`).

### Proposed learning

None new.

## Naming (round 77, naming runner delta)

Renamed `func_8004D814` -> `TitleMenu__Reset`. **Tier B**: Passes `&D_800114E8` (a real dlabel string, "ETC\\TITLE.TIM" -- `asm/data/1C34.rodata.s`) to `slotD4`, sets `slot6C(self, 0xA)`, and resets `unk34`/`unk2C`. Named from the one concrete piece of evidence available (the TIM filename); the rest of the sequence's purpose is not established.

## Proposed field names

**Head, round 77:** `unkA4 -> dreamSysView` APPLIED by type scope.

`TitleMenu::unkA4` has a real accessor outside this unit
(`src/class_3bb8c_c.c`'s `TitleMenu__TitleMenu` sets it from its own
`dreamSys` parameter), so per the compiler-ownership rule this is a
PROPOSAL, not a rename. Also posted to the round-77 broadcast.

- **`unkA4` -> `dreamSysView`, tier A.** Already typed
  `DreamSysView_3bb8c_c *`; this function is the first to dereference it
  through its own vtable (`slotF0`). The name just mirrors the existing,
  already-confirmed type name -- a pure rename, no new claim.

## Track 4 (2026-09-26, round 88, bravo)

Renamed `TitleMenu__ShowTitleIcon` -> `TitleMenu__Reset` (tools/rename.py): it
is the occupant of gTitleMenuMethods +0x040, IntermediateBase's
`resetCounters` slot (the ctor's last call), the same override GraphRoom has
(GraphRoom__Reset: fadeRate, unk2C 0x190, setSubHandle("ETC\HGRAPH.TIM"),
setFrameBound(10)). This one clears unk34, sets unk2C 0x190, then
setSubHandle("ETC\TITLE.TIM", NULL), setFrameBound(10) and the DreamSys's
getSetFlashbackSession(0, 0); no up-call to TaskCore__Reset. The TIM path,
the old name's only evidence, is the sub-handle setSubHandle loads, which is
TaskCore's mechanism, not something this function shows. Slot names at each
call now TaskCore's (setSubHandle, setFrameBound) instead of slotD4/slot6C.
