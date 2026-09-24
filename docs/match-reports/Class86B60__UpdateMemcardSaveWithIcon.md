# Class86B60__UpdateMemcardSaveWithIcon -- MATCHED 56/56, round 43

> Renamed from `func_8004E0E4` on 2026-09-24 (tools/rename.py). Address 0x8004e0e4.

Unit `class_3bb8c_d`, class `Class86B60`. **REOPENED -- ASSIGNABLE** from
round 42's `gp_rel` resolution. The round-14 stub recorded 3 `gp_rel` hits
and no derivation; this round wrote and matched the function from scratch.

## Body

```c
void Class86B60__UpdateMemcardSaveWithIcon(Class86B60 *self)
{
    s32 buf;

    buf = self->unk60->unk14;
    self->unkA4->methods->slot19C(self->unkA4, &buf);
    self->methods->slot128(self);
    if (self->unkA4->methods->slot1AC(self->unkA4)) {
        *(u8 *)D_8008AA10 = 0;
    }
    self->unkAC->methods->slot78(self->unkAC, D_8008AA10, D_8008AA18, 0xD, 3,
                                  self->unkA8, self->unkBC, self->unkC0);
}
```

Byte-exact on the first build: `funcdiff.py Class86B60__UpdateMemcardSaveWithIcon` -> `56/56 words
match (file 0x3E8E4-0x3E9C4)`; whole-image `OK: build matches retail
SLPS_015.56`.

## Derivation

- `buf = self->unk60->unk14; self->unkA4->methods->slot19C(self->unkA4,
  &buf);` -- the IDENTICAL idiom already matched in this unit's own
  `Class86B60__RefreshViewValue` (stack-buffer-out-parameter call on the same
  `DreamSysView_3bb8c_c::slot19C`), just without that function's own
  leading `Get_vtable_TaskCore()->slot94(self)` base-class call.
- `self->methods->slot128(self)` -- the slot this round's `Class86B60__UpdateMemcardSaveStatus`
  established (single-argument, `self` only).
- `self->unkA4->methods->slot1AC(self->unkA4)` -- a NEW slot on
  `DreamSysViewMethods_3bb8c_c`, at the offset immediately after
  `slot1A8` (+0x1A8) and before the already-known `slot1B0` (+0x1B0) --
  the `pad1AC[0x1B0-0x1AC]` gap was exactly 4 bytes and is now this one
  slot. Return type `s32`: the caller's own `beqz $v0` tests it directly,
  so a `void` return would be observably wrong.
- The nonzero-return branch writes a single zero byte through
  `D_8008AA10` (`*(u8 *)D_8008AA10 = 0;`) -- the same `void *` global
  `Class86B60__UpdateMemcardSaveStatus` (this round) established as holding a precomputed
  pointer into unowned rodata (a `%gp_rel` load of the global's own
  VALUE, reloaded here with an identical `lw`).
- The final call, `self->unkAC->methods->slot78(...)`, is an 8-argument
  dispatch (four in registers, four on the stack at `0x10`-`0x1C($sp)`):
  `self->unkAC`, `D_8008AA10`, `D_8008AA18`, the literal `0xD`, the
  literal `3`, `self->unkA8`, `self->unkBC`, `self->unkC0`. The offset
  (+0x078) falls immediately after this round's `Class86B60__UpdateMemcardSaveStatus`-derived
  `slot74` (+0x074, 4 bytes) with no gap, so it was appended there.
  `self->unkA8` is forwarded opaquely (this slot never dereferences it),
  so its parameter stays `void *` rather than the fuller
  `GenericReleaseObj_3bb8c_d *` the header already has for that field.

## Header changes

`include/class_3bb8c.h`, both additive:

- `DreamSysViewMethods_3bb8c_c`: `pad1AC[0x1B0-0x1AC]` (4 bytes) replaced
  by `s32 (*slot1AC)(DreamSysView_3bb8c_c *self);` at the same
  offset/size.
- `Class86B60UnkACObjMethods_3bb8c_d`: appended `slot78` (8 args) directly
  after this round's `slot74`, at +0x078 with no gap.

No existing declaration was retyped or resized.

### Proposed learning

None beyond what round 43's earlier two reports (`Class86B60__UpdateMemcardSaveStatus`,
`TaskObjF__TaskObjF`) already recorded for this unit -- this function's own
derivation was routine once those two slots (`slot128`, `D_8008AA10`)
were on file, and it re-confirmed `slot74`'s exact byte offset by landing
its own new slot immediately after it.
