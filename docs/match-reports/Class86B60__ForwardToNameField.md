# Class86B60__ForwardToNameField -- MATCH

> Renamed from `func_8004DC64` on 2026-09-24 (tools/rename.py). Address 0x8004dc64.

Unit `class_3bb8c_d`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86B60__ForwardToNameField`: 27/27 words match.

## Source

```c
void Class86B60__ForwardToNameField(Class86B60 *self, s32 arg1)
{
    Get_vtable_TaskCore()->slotE0(self, arg1);
    self->nameField->methods->slot4C(self->nameField, arg1, &D_8008A9B4);
}
```

First attempt, byte-exact. Two calls: base table forward, then a second
slot on `self->nameField` (the sub-object `Class86B60__DestroyNameField` releases).

## Struct changes (additive, `include/class_3bb8c.h`)

- `Class86B60::nameField` **retyped** from the generic
  `GenericReleaseObj_3bb8c_d *` (set by `Class86B60__DestroyNameField`'s report) to a new
  dedicated `Class86B60UnkB0Obj_3bb8c_d *`, which carries BOTH the shared
  `release` slot at `+0x004` (same signature as before, so
  `Class86B60__DestroyNameField`'s call site is unaffected) and this function's own
  `+0x04C` slot. Done because `iconHandle`/`unkAC` never reach a second slot
  and there is no evidence they share this fuller shape -- keeping
  `nameField` on its own local view avoids projecting one instance's richer
  interface onto the other two.
- `BaseTaskCtorTable_3bb8c_c::slotE0` -- new slot, `void (*)(void *self,
  s32 arg1)`.
- New `extern s32 D_8008A9B4;` (address-of only, placeholder type).

### Proposed learning

None new.

## Naming (round 77, naming runner delta)

Renamed `func_8004DC64` -> `Class86B60__ForwardToNameField`. **Tier B**: Forwards `arg1` to the base class's `slotE0`, then dispatches `arg1` and a fixed global (`&D_8008A9B4`) through `self->nameField`'s own `slot4C`. Purpose of the forwarded value/event not established.
