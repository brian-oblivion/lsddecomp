# func_8004FFF4 -- MATCH

Unit `class_3bb8c_g`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py func_8004FFF4`: 16/16 words match.

## Source

```c
void func_8004FFF4(Class86E00_3bb8c_g *self, s32 arg1)
{
    if (self->unk6C != NULL) {
        self->unk6C->methods->slot80(self->unk6C, arg1, 0x7F, 0x7F);
    }
}
```

First attempt, byte-exact. `arg1` is forwarded straight through with no
local setup of its own -- retail's own delay slot for the call sets only
the two literal `0x7F`s, and `$a1` is never touched between the function's
entry and the `jalr`, which is what proves this function's own 2nd
parameter is the call's `arg1` rather than an unused register.

## Struct changes (a within-unit retype, caught before any cross-unit
exposure)

- `Class86E00_3bb8c_g::unk6C` **retyped** from `s32` to the new
  `Class86E00Unk6CObj_3bb8c_g *`. This function is the first to
  dereference it (through its own `+0x080` vtable slot); the earlier
  `s32` reading came only from seeing it forwarded opaquely as
  `Class86E00SubObj_3bb8c_g::slot4C`'s 3rd argument while surveying the
  unit (see `func_8004FF40`'s report), which is equally consistent with a
  scalar or a pointer. Same size, no layout change.
- `Class86E00SubObjMethods_3bb8c_g::slot4C`'s 3rd parameter retyped to
  match (`Class86E00Unk6CObj_3bb8c_g *` instead of `s32`) -- this slot has
  no other known caller yet in this unit, so the retype is fully
  contained.
- New type `Class86E00Unk6CObj_3bb8c_g` / `Class86E00Unk6CObjMethods_3bb8c_g`
  (`slot80` the only reached slot).

Both retyped declarations were introduced by this unit's own earlier
functions THIS round (not inherited from `main`), so there is nothing to
flag for `class_3bb8c_e`/`class_3bb8c_f` here -- the correction is fully
internal to `class_3bb8c_g`'s own work.

### Proposed learning

None new.
