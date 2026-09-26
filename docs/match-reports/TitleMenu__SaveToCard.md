# TitleMenu__SaveToCard -- MATCHED 56/56, round 43

> Renamed from `TitleMenu__UpdateMemcardSaveWithIcon` on 2026-09-26 (tools/rename.py). Address 0x8004e0e4.

> Renamed from `Class86B60__UpdateMemcardSaveWithIcon` on 2026-09-26 (tools/rename.py). Address 0x8004e0e4.

> Renamed from `func_8004E0E4` on 2026-09-24 (tools/rename.py). Address 0x8004e0e4.

Unit `class_3bb8c_d`, class `TitleMenu`. **REOPENED -- ASSIGNABLE** from
round 42's `gp_rel` resolution. The round-14 stub recorded 3 `gp_rel` hits
and no derivation; this round wrote and matched the function from scratch.

## Body

```c
void TitleMenu__SaveToCard(TitleMenu *self)
{
    s32 buf;

    buf = self->unk60->unk14;
    self->unkA4->methods->slot19C(self->unkA4, &buf);
    self->methods->slot128(self);
    if (self->unkA4->methods->slot1AC(self->unkA4)) {
        *(u8 *)D_8008AA10 = 0;
    }
    self->unkAC->methods->slot78(self->unkAC, D_8008AA10, D_8008AA18, 0xD, 3,
                                  self->iconHandle, self->unkBC, self->unkC0);
}
```

Byte-exact on the first build: `funcdiff.py TitleMenu__SaveToCard` -> `56/56 words
match (file 0x3E8E4-0x3E9C4)`; whole-image `OK: build matches retail
SLPS_015.56`.

## Derivation

- `buf = self->unk60->unk14; self->unkA4->methods->slot19C(self->unkA4,
  &buf);` -- the IDENTICAL idiom already matched in this unit's own
  `TitleMenu__RefreshViewValue` (stack-buffer-out-parameter call on the same
  `DreamSysView_3bb8c_c::slot19C`), just without that function's own
  leading `Get_vtable_TaskCore()->slot94(self)` base-class call.
- `self->methods->slot128(self)` -- the slot this round's `TitleMenu__UpdateMemcardSaveStatus`
  established (single-argument, `self` only).
- `self->unkA4->methods->slot1AC(self->unkA4)` -- a NEW slot on
  `DreamSysViewMethods_3bb8c_c`, at the offset immediately after
  `slot1A8` (+0x1A8) and before the already-known `slot1B0` (+0x1B0) --
  the `pad1AC[0x1B0-0x1AC]` gap was exactly 4 bytes and is now this one
  slot. Return type `s32`: the caller's own `beqz $v0` tests it directly,
  so a `void` return would be observably wrong.
- The nonzero-return branch writes a single zero byte through
  `D_8008AA10` (`*(u8 *)D_8008AA10 = 0;`) -- the same `void *` global
  `TitleMenu__UpdateMemcardSaveStatus` (this round) established as holding a precomputed
  pointer into unowned rodata (a `%gp_rel` load of the global's own
  VALUE, reloaded here with an identical `lw`).
- The final call, `self->unkAC->methods->slot78(...)`, is an 8-argument
  dispatch (four in registers, four on the stack at `0x10`-`0x1C($sp)`):
  `self->unkAC`, `D_8008AA10`, `D_8008AA18`, the literal `0xD`, the
  literal `3`, `self->iconHandle`, `self->unkBC`, `self->unkC0`. The offset
  (+0x078) falls immediately after this round's `TitleMenu__UpdateMemcardSaveStatus`-derived
  `slot74` (+0x074, 4 bytes) with no gap, so it was appended there.
  `self->iconHandle` is forwarded opaquely (this slot never dereferences it),
  so its parameter stays `void *` rather than the fuller
  `GenericReleaseObj_3bb8c_d *` the header already has for that field.

## Header changes

`include/class_3bb8c.h`, both additive:

- `DreamSysViewMethods_3bb8c_c`: `pad1AC[0x1B0-0x1AC]` (4 bytes) replaced
  by `s32 (*slot1AC)(DreamSysView_3bb8c_c *self);` at the same
  offset/size.
- `TitleMenuUnkACObjMethods_3bb8c_d`: appended `slot78` (8 args) directly
  after this round's `slot74`, at +0x078 with no gap.

No existing declaration was retyped or resized.

### Proposed learning

None beyond what round 43's earlier two reports (`TitleMenu__UpdateMemcardSaveStatus`,
`TaskObjF__TaskObjF`) already recorded for this unit -- this function's own
derivation was routine once those two slots (`slot128`, `D_8008AA10`)
were on file, and it re-confirmed `slot74`'s exact byte offset by landing
its own new slot immediately after it.

## Naming (round 77, naming runner delta)

Renamed `func_8004E0E4` -> `TitleMenu__SaveToCard`. **Tier B, lower confidence**: Refreshes the view (same idiom as `TitleMenu__RefreshViewValue`), calls `slot128`, conditionally clears `D_8008AA10` behind the same `self->unkA4->methods->slot1AC()` gate `TitleMenu__CreateNameField` also tests, then forwards `self->iconHandle` plus literal flags (0xD, 3) through `unkAC`'s `slot78`. It is `TitleMenu__Tick`'s case-2 dispatch target. Purpose beyond "the icon-carrying variant of the two unkAC dispatch calls" is not established.

## Proposed field names

**Head, round 77: NOT APPLIED.** `saveInfoWord`/`saveInfoBuf` restate the types (a word, a buffer) without saying what they hold; kept `unkBC`/`unkC0` until a reader establishes it.

`TitleMenu::unkBC`/`unkC0` both have real accessors outside this unit
(`src/class_3bb8c_c.c`'s `TitleMenu__TitleMenu` sets both from
`dreamSysView->methods->slot1B0`), so per the compiler-ownership rule
these are PROPOSALS, not renames. Also posted to the round-77 broadcast.

- **`unkBC` -> `saveInfoWord`, tier B.** The `s32` return value of
  `dreamSysView->methods->slot1B0`, forwarded verbatim by this function
  (and `TitleMenu__UpdateMemcardSaveStatus`) into `unkAC`'s dispatch
  calls alongside the memcard-icon/name buffers. Exact meaning of the
  word not established.
- **`unkC0` -> `saveInfoBuf`, tier B.** The output-buffer word `slot1B0`
  fills by reference (same call as above); forwarded alongside
  `saveInfoWord` in the same two call sites. Paired with `saveInfoWord` by
  construction, not independently confirmed.

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). slot128 is this class's beginMemcardSave; `unk60->unk14` is TaskCore's slotCounts[5]; unkBC/unkC0 are saveBlock/saveBlockSize, saveBlock cast to s32 for the TaskObjF view's s32 parameter (no code). Byte-identical (whole image green, 0 new warnings, nonmatching green).
