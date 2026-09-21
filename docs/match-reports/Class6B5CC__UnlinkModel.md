# Class6B5CC__UnlinkModel -- MATCHED (3/3 words)

> Renamed from `func_8001E7B0` on 2026-09-17 (tools/rename.py). Address 0x8001e7b0.

Unit: `code_d294_c` (round 14, first slice-3 carve). Called by
`func_8001CD20` (`code_d294.c`) as its forward target for
`Class6B5CCMethods::slot18`. `void Class6B5CC__UnlinkModel(Class6B5CCObj *self)`.

## Final source

```c
void Class6B5CC__UnlinkModel(Class6B5CCObj *self) {
    self->unk18 = 0;
    self->unk20 = 0;
}
```

## Derivation notes

Already MEASURED and documented verbatim in `include/code_d294.h`'s own
standing comment before this carve existed ("Class6B5CC__UnlinkModel's whole body is
`self->unk18 = 0; self->unk20 = 0;`") -- this round only had to move the
body from documentation into `src/code_d294_c.c` and confirm it still
matches now that the function has its own real address. No residue.
Comment in the header updated (round 14) to say "now carved" instead of
"still uncarved" since the function moved out of `asm/` into this unit;
no type or prototype changed.

No new struct or vtable-slot knowledge.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001E7B0` -> `Class6B5CC__UnlinkModel`. Tier B.** It zeroes
  exactly the two fields `Class6B5CC__LinkModel` sets, and only those: the
  GsDOBJ2's `tmd` pointer at +0x18 and the source object at +0x20. The name
  is the inverse of its pair BY CONSTRUCTION, not because any Sony call
  pins it -- there is no GS call here at all, which is why this is B where
  `Class6B5CC__LinkModel` is A.
- Its one dispatch path supports the pairing: `func_8001CD20` (code_d294.c)
  forwards `Class6B5CCMethods::slot18` straight into it, while
  `func_8001CC48` -- the +0x010 slot -- is what forwards into
  `Class6B5CC__LinkModel`.
