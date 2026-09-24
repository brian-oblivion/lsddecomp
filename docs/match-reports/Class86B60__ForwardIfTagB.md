# Class86B60__ForwardIfTagB -- MATCH

> Renamed from `func_8004D788` on 2026-09-24 (tools/rename.py). Address 0x8004d788.

Unit `class_3bb8c_d`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86B60__ForwardIfTagB`: 35/35 words match.

## Source

```c
void Class86B60__ForwardIfTagB(Class86B60 *self, GenericHeaderObj_3bb8c_d *arg1, s32 arg2)
{
    Get_vtable_TaskCore()->slot38(self, arg1, arg2);
    if ((arg1->methods->header & 0xF) == 0xB) {
        self->methods->slot138(self, arg1, arg2);
    }
}
```

## Derivation

Two calls: an unconditional forward of all three parameters to the shared
base class table's `slot38`, then a runtime-type-id gate on `arg1` (its
vtable's header word, low nibble compared against the literal `0xB`) that
conditionally forwards the same three parameters to `self`'s own `slot138`.
First attempt, byte-exact -- the single-`if` shape matched the disassembly's
branch structure directly with no residue.

`self`'s concrete type is inferred as `Class86B60 *` by unit continuity
(every other function in this file is a `Class86B60` method) rather than
proven by any raw field access in this specific function -- it is reached
purely through vtable dispatch here, so the byte match constrains only the
POINTER's identity/arity, not which named struct it is. Worth flagging for
whoever next resolves a caller of this function.

## Struct changes (additive, `include/class_3bb8c.h`)

- New type `GenericHeaderObj_3bb8c_d` / `GenericHeaderMethods_3bb8c_d` --
  reads a vtable's header word as a full `s32` (`lw` then `andi`), NOT the
  existing `GenericTagInst_3bb8c_c`/`GenericTagMethods_3bb8c_c` (which reads
  only the low BYTE via `lbu`, established by `Class86AA0__ForwardIfTag34` in
  `class_3bb8c_c`). Reusing the byte-typed struct here would have emitted
  the wrong load width.
- `BaseTaskCtorTable_3bb8c_c::slot38` -- new slot, `void (*)(void *self,
  void *arg1, s32 arg2)`.
- `Class86B60Methods::slot138` -- new slot, same 3-argument signature.

### Proposed learning

None new. This is a clean instance of the already-documented
"`arg->methods->header & 0xF` is a runtime type ID" shape (round 13), just
the first time this unit's own `slot38`/`slot138` pair exercises it.

## Naming (round 77, naming runner delta)

Renamed `func_8004D788` -> `Class86B60__ForwardIfTagB`. **Tier B**: Forwards to the base class's `slot38` unconditionally, then to its own `slot138` only when `arg1`'s vtable header low nibble == 0xB -- the same runtime-type-id-gated forward shape already named `Class86AA0__ForwardIfTag34` in `class_3bb8c_c.c`. Purpose of tag 0xB itself not established.
