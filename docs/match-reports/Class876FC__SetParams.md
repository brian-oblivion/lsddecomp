# Class876FC__SetParams -- MATCHED (20/20 words)

> Renamed from `func_800564A4` on 2026-09-23 (tools/rename.py). Address 0x800564a4.

Unit: `class_3bb8c_r` (round 17 continuation). `Class876FCMethods::slot40`
(vtable offset `+0x040` of `gClass876FCMethods`) -- a plain 0x24-byte block copy
from the caller's argument into `self+0x58`, plus a single word clear.

## Final source

```c
/* Declared as a WORD array, not a byte array, so a whole-struct
 * assignment reproduces retail's aligned 4-word-per-iteration block-move
 * codegen -- a byte array has alignment 1 and compiles the copy as a
 * generic runtime-alignment-checked memcpy loop instead. */
typedef struct Block24 {
    s32 raw[0x24 / 4];
} Block24;

void Class876FC__SetParams(Class876FC *self, Block24 *src) {
    self->block58 = *src;
    self->unk24 = 0;
}
```

## Derivation

- **This closes the identity of `BaseObjOMethods::slot40`, left as
  "occupant outside this unit" in `class_3bb8c_o.c`'s own
  `Actor__Actor` report last pass.** That function called
  `self->methods->slot40(self)` (a plain no-argument dispatch, since its
  OWN concrete class -- a different sibling -- happens to call slot40
  with no extra argument); THIS class's own ctor
  (`Class876FC__Class876FC`, this unit) calls the SAME shared slot with a real
  second argument, `slot40(self, arg2)`. Both are correct about their own
  call sites: the slot's real arity is 1 argument beyond `self`, and the
  DreamSys-family sibling in `class_3bb8c_o.c` simply never had a value
  worth passing.
- **`u8 raw[0x24]` (a byte array) miscompiled the copy into a
  runtime-alignment-checked loop** (`or`/`andi`/`beqz` testing pointer
  alignment, then `lwl`/`lwr` unaligned loads) instead of retail's fixed
  4-word-per-iteration block move -- caught immediately as a completely
  different instruction COUNT and shape, not a subtle residue. Retyping
  the field to a WORD array (`s32 raw[9]`) forced 4-byte alignment and
  fixed it in one step, matching the already-documented
  `DreamSysUnk14Tail` idiom (`DreamSys.h`) for exactly this situation.
- Field write ORDER matches retail: the block copy happens first, the
  `self->unk24 = 0` clear happens last (in the delay slot of the
  function's own `jr $ra`) -- writing the C statements in that order
  reproduces it without needing any additional lever.

### Proposed learning

- **A block-copy target field should be declared as a WORD array by
  default when the source is a whole-struct assignment, not as a byte
  array "to be safe about size."** This is not a new idiom --
  `DreamSys.h`'s `DreamSysUnk14Tail` already documents it -- but this is
  a second, independent confirmation in a different unit, and the
  failure mode (a completely different, much longer instruction
  sequence, not a near-miss) is worth remembering as the SPECIFIC
  symptom to watch for: if a struct-assignment block copy compiles to
  `or`/`andi`/`beqz` alignment-checking preamble instead of a flat
  `lw`/`sw` sequence, the target type's alignment is the first thing to
  check, not the copy logic itself.

## Naming

**Tier A.** Renamed from the report's own `Obj876FCMethods::slot40` description: the body sets the class's own param block from the caller's argument and resets the tick counter -- "SetParams" is what the body does, not a guess at why. Struct field `setParams` (this unit's `Class876FCMethods` local view) renamed to match.
