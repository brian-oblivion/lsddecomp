# TitleMenu__LoadFromCard -- MATCHED 27/27, round 43

> Renamed from `TitleMenu__UpdateMemcardSaveStatus` on 2026-09-26 (tools/rename.py). Address 0x8004e1c4.

> Renamed from `Class86B60__UpdateMemcardSaveStatus` on 2026-09-26 (tools/rename.py). Address 0x8004e1c4.

> Renamed from `func_8004E1C4` on 2026-09-24 (tools/rename.py). Address 0x8004e1c4.

Unit `class_3bb8c_d`, class `TitleMenu`. **REOPENED -- ASSIGNABLE** from round
42's `gp_rel` resolution (`--gp-symbols`/`--no-nop-mflo-mfhi`, see CLAUDE.md
"Open toolchain blockers"). The round-14 stub report recorded 2 `gp_rel` hits
and no derivation; this round wrote and matched the function from scratch.

## Body

```c
void TitleMenu__LoadFromCard(TitleMenu *self)
{
    self->methods->slot128(self);
    self->unkAC->methods->slot74(self->unkAC, D_8008AA10, D_8008AA18,
                                  self->unkBC, self->unkC0);
}
```

Byte-exact on the first build: `./build-and-verify.sh` -> `OK: build matches
retail SLPS_015.56`; `funcdiff.py TitleMenu__LoadFromCard` -> `27/27 words match (file
0x3E9C4-0x3EA30)`.

## Derivation

Two calls:

1. `self->methods->slot128(self)` -- `lw $v0,0x0($s0); lw $v0,0x128($v0); jalr`.
   Offset 0x128 fell inside `TitleMenuMethods`' existing `pad128[0x12C-0x128]`
   gap between `slot124` (+0x124) and `slot12C` (+0x12C) -- a brand-new slot,
   added to `include/class_3bb8c.h` as `void (*slot128)(TitleMenu *self);`.
   No other function in this unit reaches it.

2. `self->unkAC->methods->slot74(self->unkAC, D_8008AA10, D_8008AA18,
   self->unkBC, self->unkC0)` -- five arguments (`self`, two globals'
   VALUES, `self->unkBC`, `self->unkC0`), the fifth going to the stack at
   `0x10($sp)` exactly as a 5-argument call requires. `unkAC`'s methods
   table already had `release` (+0x004) and `slot70` (+0x070, from
   `TitleMenu__EndCardAccess`); this call reaches +0x074 immediately after `slot70`
   with no gap, so `slot74` was appended there.

   The two `%gp_rel` loads (`D_8008AA10`, `D_8008AA18`) read the globals'
   own VALUES, not their addresses (`lw $a1, %gp_rel(D_8008AA10)($gp)`,
   plain `lw`, no `la`) -- unlike every other `D_...` reference in this
   unit's header so far, which are all address-of. Traced both values:
   `D_8008AA10` = `0x80011464`, `D_8008AA18` = `0x8001149C`
   (`asm/data/7B12C.sdata.s`). Both land inside the still-uncarved rodata
   block `D_80011434` (`asm/data/1C34.rodata.s`, a mixed string/SJIS-glyph
   table spanning 0x80011434-0x800114DC) at offsets 0x30 and 0x68
   respectively -- NEITHER has its own `dlabel`, so neither can be spelled
   by name. Declared `D_8008AA10`/`D_8008AA18` as `extern void *` (the
   globals themselves, not what they point to) and forwarded their values
   opaquely, matching how `slot74`'s two middle parameters are used
   (arg1/arg2, untyped beyond "pointer-shaped value forwarded verbatim").

`self->unkBC` and `self->unkC0` were already established `s32` fields from
`TitleMenu__TitleMenu`'s report (return value / output-buffer word of
`DreamSysView_3bb8c_c::slot1B0`); this function forwards both by VALUE
(register `$a3`, stack word), consistent with the existing types -- no
retype needed.

## Header changes

`include/class_3bb8c.h`, both additive (no existing declaration touched):

- `TitleMenuMethods`: `pad128[0x12C-0x128]` (4 bytes) replaced by
  `void (*slot128)(TitleMenu *self);` at the same offset/size.
- `TitleMenuUnkACObjMethods_3bb8c_d`: `void (*slot74)(TitleMenuUnkACObj_3bb8c_d
  *self, void *arg1, void *arg2, s32 arg3, s32 arg4);` appended immediately
  after `slot70` (+0x070, 4 bytes), landing exactly at +0x074 with no gap.
- New externs `D_8008AA10`, `D_8008AA18` (both `void *`), documented as
  VALUE-of (not address-of) globals holding pointers into unowned rodata.

### Proposed learning

A `%gp_rel(sym)($gp)` load feeding a register argument is not automatically
"take the address of `sym`" -- check whether the instruction is a plain `lw`
(loads the global's own stored VALUE) versus an `la`/address computation.
This unit's header had, until this function, only ever seen the address-of
case; the value-of case surfaces when the "address" being forwarded was
itself precomputed into a `.sdata` pointer variable by someone else's static
initializer, and the pointee (here, an offset into an unowned rodata blob
with no `dlabel` of its own) may not even be nameable.

## Naming (round 77, naming runner delta)

Renamed `func_8004E1C4` -> `TitleMenu__LoadFromCard`. **Tier B, lower confidence**: Calls `slot128` then forwards `self->unkBC`/`unkC0` (no icon handle, no literal flags) through `unkAC`'s `slot74` -- the simpler sibling of `TitleMenu__SaveToCard` and the dispatch target for `TitleMenu__Tick`'s case-3. Purpose beyond "the icon-less variant of the two unkAC dispatch calls" is not established.

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). slot128 is this class's beginMemcardSave; unkBC/unkC0 are saveBlock/saveBlockSize, saveBlock cast to s32 for the TaskObjF view's s32 parameter (no code). Byte-identical (whole image green, 0 new warnings, nonmatching green).
