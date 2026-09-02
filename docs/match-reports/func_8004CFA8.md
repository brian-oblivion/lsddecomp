# func_8004CFA8 — MATCHED (2/2 words)

Two-instruction leaf: `jr $ra` / `addiu $v0, $a0, 0x1CC`. Not a vtable slot
(not in `D_800866E8`, per `tools/classtable.py`), and no caller found
anywhere in the executable (only reference is its own `.s` file) — an
unused or not-yet-carved accessor.

## Disassembly

```
jr   $ra
 addiu $v0, $a0, 0x1CC     ; return &self->unk1CC (address-of only, no load)
```

## Final C

```c
void *func_8004CFA8(Obj866E8 *self) {
    return &self->unk1CC;
}
```

## New struct knowledge (`include/class_3bb8c.h`)

- `Obj866E8::unk1CC` (`s32`, +0x1CC) — real type unknown, only its address
  is ever taken here. No caller exists in this executable to cross-check
  against, so the field type is a placeholder (matches the project's
  convention for address-of-only fields, e.g. `Class866E8::unk1C0` in
  `include/class_3ac78.h`).

## Attempts

1 (matched on first attempt — pure address arithmetic, no ambiguity).

### Proposed learning

None new.
