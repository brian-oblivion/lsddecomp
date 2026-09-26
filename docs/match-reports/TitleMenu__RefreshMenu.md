# TitleMenu__RefreshMenu -- MATCHED 87/87, round 43

> Renamed from `TitleMenu__CommitNameEntry` on 2026-09-26 (tools/rename.py). Address 0x8004de08.

> Renamed from `Class86B60__CommitNameEntry` on 2026-09-26 (tools/rename.py). Address 0x8004de08.

> Renamed from `func_8004DE08` on 2026-09-24 (tools/rename.py). Address 0x8004de08.

Unit `class_3bb8c_d`, class `TitleMenu`. **REOPENED -- ASSIGNABLE** from
round 42's `gp_rel` resolution. The round-14 stub recorded 1 `gp_rel` hit
and no derivation; this round wrote and matched the function from scratch.

## Body

```c
extern void CheckSaveScoreFlag(void *arg0, void *arg1, void *arg2);

void TitleMenu__RefreshMenu(TitleMenu *self)
{
    s32 size;
    s32 origState;
    void *buf1;
    s32 buf2;

    size = self->nameField->unkA9;
    origState = self->state;
    buf1 = BMemPMgrAlloc(size);
    DecodeFullWidthSjis(buf1, D_8008AA18);
    self->nameField->methods->slotCC(self->nameField, buf1);
    BMemPMgrFree(buf1);
    CheckSaveScoreFlag(self, self->unk4C, self->unkA4);
    self->methods->slotE0(self, self->unk14);
    self->unkA4->methods->slot19C(self->unkA4, &buf2);
    self->state = 5;
    self->methods->slot60(self, 0xB);
    self->methods->slot11C(self, buf2, 1);
    self->methods->slot60(self, 0xF);
    self->methods->slotF0(self, (void *)origState, 0);
    self->unkA4->methods->slot19C(self->unkA4, &buf2);
}
```

Byte-exact on the first build: `funcdiff.py TitleMenu__RefreshMenu` -> `87/87 words
match (file 0x3E608-0x3E764)`; whole-image `OK: build matches retail
SLPS_015.56`. (Measured with the unit's other stall, `TitleMenu__TickNameFieldCursor`,
temporarily restored to `INCLUDE_ASM` so its own known length residue could
not contaminate this function's out-of-range read -- see that function's
own report. `TitleMenu__TickNameFieldCursor` is back in C in the committed tree; re-verify
the whole image after any further edit there.)

## Derivation

- `size = self->nameField->unkA9;` -- a NEW `u8` field at `TitleMenuUnkB0Obj_
  3bb8c_d`'s +0x0A9 (immediately before this round's `TitleMenu__CreateSaveTitle`-
  established `unkAA`/`unkAB`/`unkAC`), read unsigned and used directly as
  an allocation size.
- `origState = self->state;` -- a snapshot of the CURRENT `state` value,
  taken before this function overwrites it with the literal `5` later.
  Read at function entry (before the allocator call) in the disassembly,
  which is why the C statement is placed there too rather than immediately
  before its one use at the very end.
- `buf1 = BMemPMgrAlloc(size); DecodeFullWidthSjis(buf1, D_8008AA18); self->
  nameField->methods->slotCC(self->nameField, buf1); BMemPMgrFree(buf1);` -- the
  identical allocate/fill/consume/free idiom already matched in this unit's
  own `TitleMenu__CreateSaveTitle` (this round), just simpler (no `strcpy`/`strlen`
  sizing step here -- the size comes straight from `unkA9`).
  `slotCC` is a NEW slot on `TitleMenuUnkB0ObjMethods_3bb8c_d`, landing at
  +0x0CC, 0x10 bytes after this round's `slotB8` (+0x0B8) with an
  intervening pad.
- `CheckSaveScoreFlag(self, self->unk4C, self->unkA4);` -- `CheckSaveScoreFlag` is
  ALREADY MATCHED, in a DIFFERENT unit (`src/class_3bb8c_c.c`), as a
  genuinely 2-parameter function (`Ctx678_3bb8c_c *ctx, Result678_3bb8c_c
  *out`). This call site sets up a THIRD argument (`self->unkA4` in `$a2`)
  that unit's own signature never receives -- the same independent-arities
  situation already on file for `Get_vtable_TaskCore`/`BaseTaskCtorTable_
  3bb8c_c` vs. `TaskCoreMethods` (`include/code_2c054.h`). This unit's own
  local 3-argument extern matches what THIS call site actually needs;
  class_3bb8c_c.c's 2-argument declaration is untouched. (`include/class_
  3bb8c.h`'s own comment on `Ctx678_3bb8c_c`, written when this call site
  was still uncarved asm, already named `TitleMenu__RefreshMenu` as CheckSaveScoreFlag's
  "one caller" -- now confirmed and closed.)
- `self->methods->slotE0(self, self->unk14);` -- a NEW slot at +0x0E0 on
  `TitleMenuMethods` (inside the previous `pad0DC[0xF0-0xDC]` gap),
  forwarding the already-established `unk14` (`void *`, from this round's
  `TitleMenu__BeginCardAccess`) opaquely.
- `self->unkA4->methods->slot19C(self->unkA4, &buf2);` -- the SAME slot
  already established (`TitleMenu__RefreshViewValue`), but used here as an OUT
  parameter: `buf2` is uninitialized before the call and its value is read
  afterward (`self->methods->slot11C(self, buf2, 1)`), unlike every
  earlier call site which supplied a value IN. The signature
  (`DreamSysView_3bb8c_c *self, s32 *arg1`) needs no change -- a pointer
  argument works identically whether the callee reads or writes through
  it.
- `self->state = 5;` then two calls to a NEW slot `slot60` (+0x060 on
  `TitleMenuMethods`, inside the previous `pad044[0x06C-0x044]` gap) with
  literals `0xB` and `0xF`.
- `self->methods->slot11C(self, buf2, 1);` -- a NEW slot at +0x11C (inside
  the previous `pad0F4[0x124-0xF4]` gap), taking the value `slot19C` just
  filled.
- `self->methods->slotF0(self, (void *)origState, 0);` -- reuses the
  ALREADY-established `slotF0` (`void *arg1, s32 arg2`, from
  `TitleMenu__SetState`), but this call forwards an `s32` (`origState`) through
  the `void *` parameter. Cast at the call site rather than retyping the
  shared slot -- the bit pattern is identical either way (both are one
  register-width value), and `TitleMenu__SetState`'s own already-matched call
  keeps the pointer type.
- The function ends with a SECOND, identical `slot19C` out-call, result
  discarded (the function returns immediately after).

## Header changes

`include/class_3bb8c.h`, all additive:

- `TitleMenuUnkB0Obj_3bb8c_d`: new `unkA9` (`u8`) at +0x0A9, splitting the
  existing pad.
- `TitleMenuUnkB0ObjMethods_3bb8c_d`: new `slotCC` at +0x0CC, after a new
  `pad0BC[0xCC-0xBC]` gap following this round's `slotB8`.
- `TitleMenuMethods`: three new slots, each splitting an existing pad --
  `slot60` (+0x060, `pad044` split), `slotE0` (+0x0E0, `pad0DC` split),
  `slot11C` (+0x11C, `pad0F4` split).

`src/class_3bb8c_d.c`: local (not shared-header) 3-argument extern for
`CheckSaveScoreFlag`, matching this call site; `class_3bb8c_c.c`'s own
2-argument declaration for the same real function is untouched.

No existing declaration was retyped or resized; `slotF0`'s existing `void
*` parameter type is reused via an explicit cast at this call site rather
than changed.

### Proposed learning

**A stalled sibling function's own drift can hide a clean match on the
function next to it in the SAME unit.** `TitleMenu__RefreshMenu` scored 6/87 with a
128324-byte out-of-range warning on the first build -- not because of
anything wrong in `TitleMenu__RefreshMenu` itself, but because `TitleMenu__TickNameFieldCursor`
(this unit's OTHER round-43 target, still an unresolved 2-word-short
residue at the time) sits immediately before it in ROM order and was
shifting every address after it. Restoring `TitleMenu__TickNameFieldCursor` to
`INCLUDE_ASM` in isolation revealed `TitleMenu__RefreshMenu` was byte-exact all
along. When a function's own diff looks structurally wrong immediately
after editing an UNRELATED, EARLIER function in the same unit, check
whether that earlier function is still drifting before assuming the
one under the cursor has a real residue -- this is the same
attribution hazard CLAUDE.md's "Address drift" section documents for
round 20, just triggered by a same-unit sibling instead of a forgotten
`#if 0` wrapper.

## Naming (round 77, naming runner delta)

Renamed `func_8004DE08` -> `TitleMenu__RefreshMenu`. **Tier B**: Allocates/fills/frees a temp buffer via `BMemPMgrAlloc`/`DecodeFullWidthSjis`/`BMemPMgrFree` and pushes it through `self->nameField->methods->slotCC`, then calls `CheckSaveScoreFlag`, sets `state = 5`, and runs two `TitleMenu__SetState` calls (0xB then 0xF) bracketing a refresh of the DreamSysView. Read as committing the entered name-field text and advancing state; the two literal state values are not otherwise established.

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). `nameField->unkA9` is TextRow's cellCount and +0x0CC its setText; `state` is TaskCore's activeSlot; slotE0 updateSlotElements, slot60 setState, slot11C setSlotCursor, slotF0 setActiveSlot (origSlot is s32 now, no cast); `unk4C` is TaskCore's `target`. Byte-identical (whole image green, 0 new warnings, nonmatching green).
