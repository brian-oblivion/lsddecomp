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
    return self->methods->slotF8(self, ret, stackBuf, &sDefaultTargetSpecs);
}
```

## Derivation

First-pass reading of the raw asm badly mis-transcribed this function -- it
looked like a single call to `ComputeCellWorldOffsets` whose result was the return
value. It is actually **two calls**: `ComputeCellWorldOffsets` first, whose `s32`
result is immediately forwarded (via `$a1`) as the **second argument** to a
*second* dispatch, `self->methods->slotF8(self, ret, &stackBuf, &sDefaultTargetSpecs)`
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

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B38C` | `Class866E8__SetTargetAndBuildRates` | B | Occupant of `D_800866E8` +0x0E4 (`tools/classtable.py 0x800866E8`), class prefix confirmed. Stores its own `arg2`/`arg3` into `self->unk6C`/`self->unkBC` (the tracked position source and current footprint descriptor), then computes offsets via `ComputeCellWorldOffsets` and forwards the result through its own vtable slot `slotF8`, which is `Class866E8__BuildRateEntries`'s identity slot. Mechanics only -- "sets the target/descriptor, then builds rates from it" describes what the body does, not why. |

## Proposed field names

Both below are read/written by `Obj866E8`-typed code OUTSIDE this unit
(`src/class_3bb8c_b.c`), confirmed by actually attempting the rename: the
field definition was changed, the whole-image oracle re-run, and
`class_3bb8c_b.c`'s own `Class866E8__ComputeFootprintFromRotation`
(`self->unk6C->unk14->unk44`) failed to compile with no matching member.
Reverted per track 3 step 3's cross-unit rule; proposing here for the head
to apply by type scope.

| field | proposed name | tier | evidence |
| --- | --- | --- | --- |
| `Obj866E8::unk6C` | `posSource` | B | Stored raw by `Class866E8__SetTargetAndBuildRates` (`self->unk6C = arg2`, never dereferenced there); dereferenced by `Class866E8__GetTargetDescriptor` as `self->unk6C->unk14 + 0x18` to obtain the `QueryPos866E8` (world-position triple) fed to `Class866E8__ComputeFootprintDescriptor`; `class_3bb8c_b.c`'s `Class866E8__ComputeFootprintFromRotation` independently reaches the SAME `->unk14->unk44` chain. "posSource" names the mechanic (an object whose `unk14` substruct supplies positions), not an asserted game identity. |
| `Obj866E8::unkBC` | `descriptor` | B | Whole-struct-copied from a caller `Descriptor10*` in `Class866E8__SetTargetAndBuildRates`; overwritten wholesale (44 bytes, past the declared field into trailing padding -- see `Class866E8__UpdateFootprintTracking.md`'s own derivation) and read back (`Class866E8__GetTargetDescriptor` returns `&self->unkBC` directly) as "the object's current footprint descriptor". Not tested for cross-unit accessors this round (only `unk6C` was); the head should re-run the same compiler check before applying. |
