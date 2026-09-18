> Renamed from `func_8001D624` on 2026-09-18 (tools/rename.py). Address 0x8001d624.

# Class6B5CC__TransformAndNotifyParents -- MATCHED (32/32 words)

Round 12, runner delta. `code_d294_b`.

## Summary

```c
void Class6B5CC__TransformAndNotifyParents(Class6B5CCObj *self, GenericCountList_d294 *a1, s32 a2) {
    ApplyMatrixToSVArray(&a1->unk4, &a1->unk4, a1->unk0 * 8, &self->unk14->unk24);
    self->unk28 = 0;
    self->unk2C = 0;
    self->unk30 = a1;
    self->methods->slot30(self, a2);
    self->unk30 = NULL;
}
```

Seeded with m2c (`--sig 'void Class6B5CC__TransformAndNotifyParents(Class6B5CCObj *self, void *a1, s32 a2)'`),
which independently produced the same shape (a CSE'd `temp = a1 + 4;` used
for both `ApplyMatrixToSVArray` args, the two zero-stores, the stash-and-clear of
`unk30`, and the vtable dispatch) -- matched first try after typing m2c's
loose `void *`/`*a1` reads into real fields.

## New types/fields

- **`GenericCountList_d294`** (new local type, `include/code_d294.h`): a
  third "just enough to dispatch" view, seen only through this call site.
  `unk0` (its own first field) is read once and multiplied by 8 to form
  `ApplyMatrixToSVArray`'s iteration count; `&unk4` (address only, never
  dereferenced here) is `ApplyMatrixToSVArray`'s src/dest base. Both `ApplyMatrixToSVArray`
  args are the SAME address -- confirmed from the disassembly
  (`addiu $a0,$s1,0x4` then `addu $a1,$a0,$zero`, not reasoned from any
  naming), which is exactly what let m2c CSE it into one `temp_a0`.
- **`Class6B5CCMethods::slot30`** (+0x030, new field): `BasicClass__NotifyParents`,
  inherited verbatim into this class's own table (already established by the
  file banner's `--vs D_8006B58C` census) -- not decompiled here, since
  BasicClass belongs to a different, still-uncarved unit. Dispatched here as
  `(self, s32 arg1)`.
- **`Class6B5CCSub14::unk24`** (renamed from `pad24`): only its address is
  taken (`&self->unk14->unk24`, forwarded to `ApplyMatrixToSVArray` as a
  write-destination base) -- stays an opaque byte span, just renamed now
  that something references it.
- **`Class6B5CCObj::unk28`/`unk2C`** (new `s32` fields, both just zeroed
  here) and **`unk30`** (new `GenericCountList_d294 *` field): set to this
  call's own `a1` only for the duration of the `slot30` dispatch, then
  cleared again right after -- reads like a "currently processing" scratch
  slot, not a durable one. (`unk28` was already added by `func_8001D6B4`,
  matched earlier this round; this function only adds `unk2C`/`unk30`.)
- **`ApplyMatrixToSVArray`** (extern, `asm/code_d294_c.s`, the next slice, still
  uncarved): declared `(void *src, void *dest, s32 count, void *out)`,
  typed only to this call site's own shape.

## Evidence

Disassembly (`asm/nonmatchings/code_d294_b/Class6B5CC__TransformAndNotifyParents.s`).

### Proposed learning

None new -- this just extends the "opaque view typed only to the one call
site that needs it" convention already established repeatedly in this file
(`GenericObj_d294`, `UnkOwner_d294`) to a third shape.
