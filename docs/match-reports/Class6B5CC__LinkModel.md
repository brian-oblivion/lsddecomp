> Renamed from `func_8001E770` on 2026-09-17 (tools/rename.py). Address 0x8001e770.

# Class6B5CC__LinkModel -- MATCHED (16/16 words)

Unit: `code_d294_c` (round 14). Called by `func_8001CC48` (`code_d294.c`)
as its conditional forward target when `other`'s vtable-header tag is 9.
Stores `other` into `self->unk20`, copies one field out of it, then calls
the Psy-Q `GsLinkObject4` through a pointer computed off `other->unkC`.
`void Class6B5CC__LinkModel(Class6B5CCObj *self, GenericObj_d294 *other)`.

## Final source

```c
void Class6B5CC__LinkModel(Class6B5CCObj *self, GenericObj_d294 *other) {
    self->unk20 = other;
    self->unk18 = other->unk10;
    GsLinkObject4((u8 *)((GenericObj_d294 *)self->unk20)->unkC + 0xC, &self->unk10, 0);
}
```

## New struct/extern knowledge (`include/code_d294.h`, additive)

- **`GenericObj_d294` gains `unk10`** (`s32`), immediately following the
  already-known `unkC` (`void *`) with no gap -- read here into
  `self->unk18`.
- **New extern for `GsLinkObject4`** (`void(void*, void*, s32)`), the raw
  Psy-Q symbol (`config/symbols.slps01556.lsdde.txt`, `0x8001EF70`,
  segment `psyq_GsLinkObject4` immediately after this carve) -- not
  previously given its own prototype (only its wrapper `func_8001F51C`
  was declared). Declared with only the shape this one call site needs.

No existing field was retyped or renamed; `Class6B5CC__UnlinkModel`'s comment in the
header was also updated (round 14) to drop "still uncarved" now that both
functions have moved into this unit -- prototype/type unchanged.

## Derivation notes

- **Residue: reading `other->unkC` directly (rather than through
  `self->unk20`, which was just assigned `other` one statement earlier)
  scored 2/16, with the whole function drifting to the wrong length.**
  Retail reloads `self->unk20` (a `lw a0, 0x20($v0)` right after the
  store) instead of reusing the register that already held `other` --
  the classic "retail re-reads a value it just wrote instead of keeping
  the argument register live" shape. Rewriting the `GsLinkObject4` call's
  first argument to go through `self->unk20` (cast back to
  `GenericObj_d294 *`) instead of the `other` parameter directly matched
  clean, register swap included: retail's `$v0`/`$v1` roles (self in
  `$v0`, the loaded `unk10` value in `$v1`) fell out naturally once the
  source read through the field instead of the parameter.
- The pointer arithmetic `(u8 *)(...)->unkC + 0xC` matches retail's
  `lw a0, 0xC($a0); addiu a0, a0, 0xC` -- read `other->unkC`, then advance
  it 12 bytes; `GsLinkObject4`'s own body is out of scope (Psy-Q library).

### Proposed learning

**When a function stores a parameter into a field and then immediately
uses that SAME value again, check whether retail re-reads it through the
field rather than reusing the parameter's own register.** This is a
sibling of the already-promoted "direct calls beat caching a resolved
pointer" learning from `func_8001CD60`: GCC 2.6.3 does not always keep a
just-stored value live in its original register when a memory read of the
same value is available and the source re-mentions the field rather than
the parameter -- write the SOURCE the way retail's disassembly reads
(through the field), not the way that looks most natural (through the
still-in-scope parameter), when the two are interchangeable in value but
not in registers.
