# Class866E8__GetUnk1CC — MATCHED (2/2 words)

> Renamed from `func_8004CFA8` on 2026-09-24 (tools/rename.py). Address 0x8004cfa8.

Two-instruction leaf: `jr $ra` / `addiu $v0, $a0, 0x1CC`. Not a vtable slot
(not in `gClass866E8Methods`, per `tools/classtable.py`), and no caller found
anywhere in the executable (only reference is its own `.s` file) — an
unused or not-yet-carved accessor.

## Disassembly

```
jr   $ra
 addiu $v0, $a0, 0x1CC     ; return &self->unk1CC (address-of only, no load)
```

## Final C

```c
void *Class866E8__GetUnk1CC(Obj866E8 *self) {
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

## Naming

**Tier A**, pure getter -- `return &self->unk1CC;`, two instructions, no
caller found anywhere in the executable. `unk1CC` itself is left
unrenamed: `Class866E8__GetUnk1CC` only ever takes its address, never
reads through it, so its real type (and therefore any real name) is not
established from this unit -- class_3ac78's own independent view of the
same offset (`Class866E8__Reset`: "set to -1") does not clarify it
either. Following the "GetSetUnk10Field0"-style precedent for a field
whose meaning is unknown but whose offset is fixed.
