# Class866E8__GetTargetDescriptor

> Renamed from `func_8004C158` on 2026-09-24 (tools/rename.py). Address 0x8004c158.

**Unit:** class_3bb8c · **Size:** 26 words · **Status:** MATCHED (first attempt).

## Result

```c
Descriptor10 *Class866E8__GetTargetDescriptor(Obj866E8 *self, s32 arg1, void **out) {
    void *v1;

    v1 = (u8 *)self->unk6C->unk14 + 0x18;
    if (out != 0) {
        *out = v1;
    }
    if (arg1 != 0) {
        if (self->methods->slot110(self, arg1, v1) != 0) {
            return 0;
        }
    }
    return &self->unkBC;
}
```

## Derivation

Two independent gates, both falling through to the same `return
&self->unkBC;` (the `unkBC` `Descriptor10` established by `Class866E8__SetTargetAndBuildRates`
this same round):

- `arg1 == 0`: skip the `slot110` dispatch entirely, go straight to the
  fallback return.
- `arg1 != 0`: call `self->methods->slot110(self, arg1, v1)`; only a
  **nonzero** result short-circuits to `return 0;` (in the delay slot of the
  call's `bnez`, unconditionally overwritten to 0, which is what makes this
  read backwards at a glance -- the call's actual return value is discarded,
  a `bnez`+`li v0,0` pair encodes "return 0 iff call succeeded").

`self->unk6C` turned out to be a pointer (`Unk6CObj`), not the raw `s32` an
earlier guess might suggest -- `Class866E8__SetTargetAndBuildRates` (matched later in the same
round) only ever stores its own `arg2` there raw, never dereferencing it, so
nothing in that function alone would have caught the mistake; this function's
own `+0x014` dereference is what pins the type down. `unk6C->unk14` is itself
only ever used for address-of-plus-offset arithmetic (`+0x018`), never
dereferenced further here, so it stays `void *`.

### Proposed learning

**A `bnez`-then-unconditional-`li`-zero pair in a delay slot inverts the
obvious reading of a call's result.** The call's own return value is
discarded; the pair is testing "was it nonzero" purely to decide whether the
*caller's own* return value should become 0. Skimming the raw instructions
suggests "save the call result", but it's "gate the caller's own return on
the call result's zero-ness". Traced correctly here by working out, for each
of the two fall-through targets (`L8004C1A8` vs `L8004C1AC`), what value `$v0`
actually holds *at the target*, not at the branch -- the same discipline
broadcast #3 (delay-slot-belongs-to-the-target) states more generally.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004C158` | `Class866E8__GetTargetDescriptor` | B | Occupant of `D_800866E8` +0x10C. Resolves `self->unk6C`'s own position substruct, optionally hands it to `slot110` (`Class866E8__ComputeFootprintDescriptor`) to fill a caller-supplied `Descriptor10Ext`, and always returns `&self->unkBC` -- this object's own current footprint descriptor. "Get...Descriptor" names the return value's role; "Target" reflects `self->unk6C`'s established role as the stored position source (`Class866E8__SetTargetAndBuildRates` sets it). |
