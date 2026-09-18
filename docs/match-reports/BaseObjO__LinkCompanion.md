> Renamed from `func_800570B4` on 2026-09-18 (tools/rename.py). Address 0x800570b4.

# BaseObjO__LinkCompanion -- MATCHED (31/31 words)

Unit: `class_3bb8c_o` (round 17). `BaseObjOMethods::slot10` -- the "link"
half of a buddy-object pair, already named `slot10`
(`code_55dd4.h`/`D800878D4Methods`) and `vtable_DreamSys::slot10`
(`DreamSys.h`) by two sibling units, both citing this exact function
address. Chains to a fixed base handler, then classifies `arg` by its own
vtable header word and records it into one of two companion-pointer
fields.

## Final source

```c
typedef struct TagWordMethodsO {
    s32 header; /* +0x000 */
} TagWordMethodsO;
typedef struct TagWordObjO {
    TagWordMethodsO *methods;
} TagWordObjO;

void BaseObjO__LinkCompanion(BaseObjO *self, TagWordObjO *arg) {
    s32 tag;

    GetClass6B5CCMethods()->slot10(self, arg);
    tag = arg->methods->header;
    if ((tag & 0xFFF) == 0x114) {
        self->unk4C = arg;
    } else if ((tag & 0xF) == 5) {
        self->unk50 = arg;
    }
}
```

## Derivation

- **Order matters: chain to the fixed table FIRST, tag-check SECOND.**
  Retail calls `GetClass6B5CCMethods()->slot10(self, arg)` before ever reading
  `arg`'s own vtable header, then re-derives `arg->methods->header` fresh
  afterward. Reversing the order (tag-check first) does not reproduce this
  and was not needed -- writing the statements in retail's own order was
  sufficient first try.
- **`GetClass6B5CCMethods()` called with implicit `self` in `$a0`.** At the call
  site nothing has touched `$a0` yet (it still holds this function's own
  first parameter), matching the project's established "MEASURED:
  `GetClass6B5CCMethods` takes no real arguments" 0-argument declaration -- no
  spurious `move` is emitted.
- **Tag classification reads a full WORD (`arg->methods->header`), masked
  `& 0xFFF` / `& 0xF`, not a byte.** This is a DIFFERENT tag check than
  `DispatchObjO__func_57320`'s/`BaseObjO__func_571f8`'s (which read just the low byte via
  `lbu`) -- kept as a separate local type, `TagWordObjO`, rather than
  conflating the two shapes.
- **`self->unk4C = arg;` / `self->unk50 = arg;` are companion-object
  pointers**, mirroring `code_55dd4.h`'s already-documented `Class65650`
  field `unk50` ("companion-object pointer, unlinked via slot14") at the
  exact same offset in a sibling class built on the same base -- this
  function is the "link" (sets the pointer); `BaseObjO__UnlinkCompanion` right after
  it in ROM order is the "unlink" (clears it), matching the header's own
  cross-reference between the two.

### Proposed learning

- **A companion-pointer link/unlink pair's tag classification is a
  reusable fingerprint across sibling classes.** `(header & 0xFFF) ==
  0x114` picks out one companion KIND (`0x114` is `D_800866E8`'s own
  header word, confirmed via `tools/classtable.py --scan`) and
  `(header & 0xF) == 5` picks out a second, broader kind (any table whose
  header's low nibble is 5). Worth remembering the exact masks if another
  class's link/unlink pair turns up with the same shape.

## Naming

**`BaseObjO__LinkCompanion` -- tier A.** Mechanics ARE the purpose:
classifies `arg` by its own vtable header tag (`(header&0xFFF)==0x114` or
`(header&0xF)==5`) and stores it into the matching one of two companion
pointer fields (`companion1`/`companion2`) -- a "link" operation exactly
as the existing term "companion-object pointer" (already used by
`code_55dd4.h` for the identical field pattern at the same shared slot) is
already established for. `func_800570B4` is the "link" half of the
`slot10`/`slot14` pair; `BaseObjO__UnlinkCompanion` (`slot14`) is the other
half, right after it in ROM order.
