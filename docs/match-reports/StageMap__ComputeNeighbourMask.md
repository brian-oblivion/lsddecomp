# StageMap__ComputeNeighbourMask

> Renamed from `StageMap__ComputeRateFlags` on 2026-09-26 (tools/rename.py). Address 0x8004b930.

> Renamed from `Class866E8__ComputeRateFlags` on 2026-09-26 (tools/rename.py). Address 0x8004b930.

> Renamed from `func_8004B930` on 2026-09-24 (tools/rename.py). Address 0x8004b930.

**Unit:** class_3bb8c · **Size:** 68 words · **Status:** MATCHED (~6 attempts).

## Result

```c
s32 StageMap__ComputeNeighbourMask(Obj866E8 *self, s32 val, s32 flag) {
    Unk68Struct *u;
    s32 divisor;
    s32 unk4;
    s32 count;
    s32 flags;
    s32 i;

    u = self->unk68;
    divisor = u->divisor;
    unk4 = u->unk4;
    count = u->count;
    if (unk4 == 0) {
        flags = (val < divisor) ? 3 : 0;
        if (val >= divisor * (count - 1)) {
            flags |= 0x60;
        }
        if (val % divisor == 0) {
            flags |= flag ? 0x25 : 4;
        }
        if ((val + 1) % divisor != 0) {
            return ~flags;
        }
        flags |= flag ? 0x10 : 0x52;
        return ~flags;
    } else {
        flags = -1;
        for (i = 0; i < count; i++) {
            flags <<= 1;
        }
        return ~flags;
    }
}
```

## Derivation

`self->unk68` (`Unk68Struct`) gets all three of its fields
(`divisor`/`count`/`unk4`) read UNCONDITIONALLY at the top, before the branch
on `unk4` -- even `divisor`, which only the `unk4 == 0` path uses. This
confirmed `Unk68Struct`'s full layout: `s16 divisor @0`, `s16 count @2`, `s32
unk4 @4` (corroborated independently by `StageMap__SplitChunkIndex` and by
`ComputeCellWorldOffsets`'s own `arg2` parameter, fed this exact pointer at its one call
site).

The `unk4 != 0` branch is a small closed form: `~(-1 << count)`, written out
as an explicit shift loop because that's what retail's own trip-count-gated
`do`/`while`-shaped loop compiles from -- `for (i = 0; i < count; i++) flags
<<= 1;` reproduced it directly, no manual unrolling needed.

## Two residues, both closed

**1. Branch polarity/layout inverted.** First attempt wrote the natural
`if (unk4 != 0) { ALT; } MAIN;` (early-return) form. Retail's actual layout
places the `unk4 == 0` case (MAIN) as the branch's FALLTHROUGH and the
`unk4 != 0` case (ALT) at a distant label reached only by a taken branch --
i.e. the source tests `unk4 == 0` (not `!= 0`) with `if (cond) A else B`,
which GCC lays out as "if `!cond` goto B; A; goto end; B: ...". Confirmed by
literally reading the branch instruction (`bnez`, not `beqz`) and target
address direction; fixed by writing the `if`/`else` with the EQUALITY
(`unk4 == 0`) as the guarding condition and MAIN as the `if`-arm, matching the
`if(cond) A else B` compilation shape rather than the intuitively-cleaner
early-return form.

**2. `s16` locals for `divisor`/`count` produced a spurious
`lhu`+`sll`/`sra` sign-extension pair that retail does not have.** Retail
loads both fields with plain sign-extending `lh` and reuses the sign-extended
register value everywhere with no further extension. Caching them into `s16`
NAMED LOCALS (rather than `s32`) caused GCC, in this case, to defer the sign
extension: load the raw 16-bit pattern via `lhu`, then manually `sll`/`sra` by
16 wherever the signed value is actually needed (once per branch that uses
it) -- extra instructions with no retail counterpart. **Fix: declare the
locals `s32`, not `s16`, when caching a signed-halfword struct field that is
subsequently used in ordinary signed arithmetic.** The struct field itself
stays `s16` (that's real, confirmed layout); only the LOCAL COPY needs the
wider type to avoid the deferred-extension codegen.

### Proposed learning

**Caching a signed 16-bit struct field into an `s16` local (rather than
`s32`) can make GCC 2.6.3 defer the field's sign extension to point-of-use
(`lhu` at load + `sll`/`sra` per use) instead of extending once at load
(`lh`) -- especially when the same local's value is live into a branch used
by more than one path.** If a residue is an unexplained `sll`/`sra`-by-16
pair immediately preceding an arithmetic op on a value you cached from a
signed-halfword field, widen the LOCAL's declared type to `s32` (leaving the
struct field itself `s16`) before looking anywhere else.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B930` | `StageMap__ComputeNeighbourMask` | B | Takes `self` as its first parameter (a method, not a free function). Computes a bitmask from `val`/`self->unk68->divisor`/`flag` (boundary tests against the divisor, remainder tests, complemented at every return) with no field write -- a pure computation, its result forwarded by `StageMap__LoadChunksAround` to `StageMap__ComputeChunkLoadEntry` as `savedResult`, tested there against `sNeighbourBits[key]`. "Compute...Flags" names the mechanic; the individual bit meanings (0x25/0x60/0x10/0x52/...) are not established. |
