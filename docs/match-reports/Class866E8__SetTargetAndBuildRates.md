# Class866E8__SetTargetAndBuildRates

> Renamed from `func_8004B38C` on 2026-09-24 (tools/rename.py). Address 0x8004b38c.

**Unit:** class_3bb8c · **Size:** 35 words · **Status:** MATCHED.

## Result

```c
s32 Class866E8__SetTargetAndBuildRates(Obj866E8 *self, void *arg1, Unk6CObj *arg2, Descriptor10 *arg3) {
    s32 stackBuf[3];
    s32 ret;

    self->unk6C = arg2;
    self->unkBC = *arg3;
    ret = ComputeCellWorldOffsets(arg1, stackBuf, self->unk68, &self->unk54, arg3);
    return self->methods->slotF8(self, ret, stackBuf, &D_80086904);
}
```

## Derivation

First-pass reading of the raw asm badly mis-transcribed this function -- it
looked like a single call to `ComputeCellWorldOffsets` whose result was the return
value. It is actually **two calls**: `ComputeCellWorldOffsets` first, whose `s32`
result is immediately forwarded (via `$a1`) as the **second argument** to a
*second* dispatch, `self->methods->slotF8(self, ret, &stackBuf, &D_80086904)`
-- and it's `slotF8`'s return that the function actually returns. Caught only
by tracing every register from the `jal` through to the epilogue instead of
stopping at the first call.

`self->unkBC = *arg3;` (whole-struct copy) is the interesting byte-level
piece: retail compiles it as two UNALIGNED `lwl`/`lwr` word loads plus a
plain `sh` for the trailing halfword -- not the aligned `lw`/`sw` a naive
struct-of-{2 words, 1 half} type would produce. This only reproduces if the
struct's own natural alignment is **less than 4**, which needs every member
to be `s8`/`s16` (no `s32`) -- confirmed by cross-referencing
`ComputeCellWorldOffsets`'s OWN read of the same 10-byte pointee (individual signed
bytes at `+0x0..+0x3`, signed halfwords at `+0x4`,`+0x6`,`+0x8`), giving a
mixed byte/short struct (`Descriptor10`) whose alignment is 2. This is the
same "no s32 member forces the unaligned-block-copy shape" idiom already
documented for `FlashbackRotation` in `include/DreamSys.h` -- confirmed as a
second, independent instance.

`self->unk6C = arg2;` (raw pointer store, no dereference in this function)
combined with `Class866E8__GetTargetDescriptor`'s later dereference of the SAME field (`+0x014`,
see that report) is what pinned `unk6C`'s type to `Unk6CObj *` rather than
leaving it `void *`/`s32`.

The 5th argument to `ComputeCellWorldOffsets` is the caller's OWN `arg3` pointer passed
straight through (not `&self->unkBC`, even though `self->unkBC` was *just*
populated from `*arg3` on the previous line) -- confirmed by the stack spill
(`sw a3, 0x10(sp)`) using the original `$a3`, never reloaded from
`self->unkBC`.

### Proposed learning

**Trace every register from a `jal` through to the function's own epilogue
before concluding the call's return value IS the function's return value.**
A call's result can be immediately handed to a SECOND call as an argument,
with the second call's own result being what actually gets returned. Confirmed
here after an initial mis-transcription assumed the first call's `$v0` value
survived untouched to the epilogue.
