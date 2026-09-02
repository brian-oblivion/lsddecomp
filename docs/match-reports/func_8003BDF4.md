# func_8003BDF4

**Unit:** code_2c054 · **Size:** 26 words · **Status:** MATCHED (26/26)

## Summary

An `if`/`else` selecting one of two forwarding calls based on `self->unkD4`.

```c
void func_8003BDF4(StreamTaskObj *self) {
    if (self->unkD4 != 0) {
        self->unkB4->methods->slot4C(self->unkB4);
    } else {
        self->methods->slot60(self, 7);
    }
}
```

## Evidence

- `self->unkD4` is the field set by this unit's `func_8003BE7C` (already
  established).
- `self->unkB4->methods->slot4C(self->unkB4)`: `self->unkB4` is
  `StreamTaskUnkB4Obj*` (established). Its vtable had only slot `+0x004`
  named before this function; this one dereferences `unkB4->methods` and
  calls slot `+0x04C` with `unkB4` as the sole argument and a discarded
  return, so it is typed `void (*)(StreamTaskUnkB4Obj *self)` here, no
  counter-evidence.
- `self->methods->slot60(self, 7)` reuses the slot established matching
  `func_8003BD10` in the same round (`StreamTaskObjMethods::slot60`,
  occupied by `func_8003BC14` per `classtable.py D_8006E5F8`).

## Proposed learning

None beyond what `func_8003BD10`'s report already states.
