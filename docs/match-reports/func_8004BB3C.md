# func_8004BB3C -- STALL (14/105 words on the last diff; structurally 104/105 instructions, missing ONE)

Unit: `class_3bb8c`. Slot `Obj866E8Methods::slotFC` (verified against
`tools/classtable.py 0x800866E8`). Not toolchain-blocked: no `gp_rel` hit,
no `addiu $at,$at,%lo` hit in `asm/nonmatchings/class_3bb8c/func_8004BB3C.s`,
and no dense-`switch`/`jr $v0` table dispatch either.

## What it does

`void func_8004BB3C(Obj866E8 *self, SetupEntry866E8 *arr1, s32 count)`:
iterates `arr1[0..count)` (a 0xC-byte-strided array). Per entry:

1. `e = self->methods->slot118(self, arr1[i].id)` (resolve an `Elem` by
   index/key).
2. `self->methods->slot88(self, 6, e, i)` -- the SAME already-documented
   slot88 (dispatches to `func_8004AA6C`, outside this unit) that
   `func_8004BD14` also calls, there with a literal `7` instead of `6`. No
   header change needed for this slot, it already existed.
3. If `arr1[i].ptr0 != 0`: conditionally call `slot108(self, e)` (new slot,
   guarded by `e->unk4->unk2C != 0`), copy `arr1[i].rate` into
   `e->unk4->unk30`, call `e->unk4->methods->slot78(e->unk4, arr1[i].ptr0)`
   (new `ElemTargetMethods` slot -- `ptr0` forwarded verbatim, never itself
   dereferenced in this unit), set `e->flag = 1` and `self->unk1B0 = 1`.
4. Else: the SAME `slot108` guard, then if `e->unk4->unk2A != 0`, call
   `e->unk4->methods->slot74(e->unk4)` (new `ElemTargetMethods` slot,
   self-only) and clear `e->flag`.

After the loop: `self->unk1B4 = func_8004BCE0(self)` (already matched, a
plain count of `self->arr[i].flag != 0`).

New header additions (all committed, additive): `Obj866E8Methods::slotFC`
(func_8004BB3C's OWN identity slot, verified via classtable -- signature
`(self, SetupEntry866E8 *arr1, s32 count)`; later corrected once
func_8004B700 needed to CALL this slot and the earlier draft signature here
turned out to have been copy-pasted from slot88's shape by mistake -- see
that function's own report) and `::slot108` (split out of the
`pad0FC`/`pad108` padding gaps -- `pad0FC` was actually TWO slots, `0xFC`
and `0x100`; `slot104` sits between them at `0x104`);
`ElemTargetMethods::slot74`/`slot78` (split out of what was
`pad000[0x7C]`, now `pad000[0x74]` + the two new slots + existing
`slot7C`); `SetupEntry866E8` (new type, `void *ptr0` @0x0, `s16 rate`
@0x4, `s32 id` @0x8, size 0xC).

## Best body reached

```c
void func_8004BB3C(Obj866E8 *self, SetupEntry866E8 *arr1, s32 count) {
    s32 i;
    Elem *e;

    for (i = 0; i < count; i++) {
        e = self->methods->slot118(self, arr1[i].id);
        self->methods->slot88(self, 6, e, i);
        if (arr1[i].ptr0 != 0) {
            if (e->unk4->unk2C != 0) {
                self->methods->slot108(self, e);
            }
            e->unk4->unk30 = arr1[i].rate;
            e->unk4->methods->slot78(e->unk4, arr1[i].ptr0);
            e->flag = 1;
            self->unk1B0 = 1;
        } else {
            if (e->unk4->unk2C != 0) {
                self->methods->slot108(self, e);
            }
            if (e->unk4->unk2A != 0) {
                e->unk4->methods->slot74(e->unk4);
                e->flag = 0;
            }
        }
    }
    self->unk1B4 = func_8004BCE0(self);
}
```
(Correction from an earlier draft of this report: step 2 dispatches
`slot88`, an ALREADY-DOCUMENTED slot shared with `func_8004BD14`
-- func_8004BB3C's own identity is `slotFC` at `+0xFC`, confirmed via
`tools/classtable.py`, and is dispatched INTO from `func_8004B700`
elsewhere in this unit, not from within this function's own body. An
earlier draft of this report conflated the two and, worse, propagated
`slot88`'s parameter shape onto the `slotFC` header entry itself -- that
header mistake was caught and fixed while deriving func_8004B700's call
site, which needed `slotFC`'s REAL signature, `(self, SetupEntry866E8*,
s32 count)`, to compile. If re-deriving this function, `self->methods`
offset `0xFC` is `slotFC` = func_8004BB3C itself; do not use that name for
the offset-`0x88` call above.)

**This body is 104/105 instructions structurally IDENTICAL to retail** --
every single instruction from the prologue through the epilogue matches
retail's OPCODE and OPERAND SHAPE one-for-one, differing only in WHICH
callee-saved register (`$s0`-`$s6`) holds which value (a consistent,
total renaming, not a partial one), **except for exactly one missing
instruction**: retail has an extra `addiu $s4,$s4,0xc` right before the
loop-continue branch (at `3CA4`, in the delay slot of the `bnez` back to the
loop head) that this build's compiled output does not emit anywhere. That
single missing 4-byte instruction is what shifts everything after it and
produces the LARGE-looking "14/105" and the drift warning -- the underlying
match is much closer than that score suggests.

## Residue: retail walks the array with TWO independently-incrementing
## pointers; every source shape tried here produces only ONE

Retail keeps TWO separate live pointers through the loop, `$s3` (= `arr1+4`,
walking the `rate`/`id` pair) and `$s4` (= `arr1`, walking `ptr0`), each
incremented by `0xC` **independently** every iteration (`$s3`'s increment is
folded into an earlier `jalr`'s delay slot; `$s4`'s is the extra
instruction). This build's compiled output, however it's written, only
ever derives ONE `$s`-register induction variable and reads `ptr0`/`rate`/
`id` all relative to it -- functionally identical, one instruction shorter,
does not match.

Tried and rejected (all against this exact function, in this order):

1. Plain `arr1[i].field` for everything (id/ptr0/rate) -- 14/105, one
   induction variable, missing the extra `addiu`.
2. Explicit intermediate element pointer, `SetupEntry866E8 *entry =
   &arr1[i];`, then `entry->field` throughout (the documented
   "intermediate element pointer" idiom from DECOMPILATION_LEARNINGS,
   normally a strong lever for array loops in this codebase) -- IDENTICAL
   14/105, no change at all.
3. A nested sub-struct (`SetupEntry866E8 { void *ptr0; struct { s16 rate;
   s32 id; } sub; }`), accessed as `arr1[i].sub.field` -- IDENTICAL 14/105.
4. The same nested sub-struct, but with an explicit `SetupSub866E8 *sub =
   &arr1[i].sub;` computed once per iteration (mixing indexed `ptr0` access
   with a sub-struct pointer for the rest) -- IDENTICAL 14/105.
5. Fully manual pointer walking with NO array indexing at all: two
   independent locals (`void **ptrWalk`, `SetupSub866E8 *subWalk`) seeded
   once before the loop and advanced via explicit `(u8 *)p + 0xC` casts at
   the bottom of the loop body -- REGRESSED to 4/105 (extra pointer-
   arithmetic instructions the cast-based increment needs, that retail
   doesn't have).
6. The same two-pointer idea using NATURAL (non-cast) pointer arithmetic
   (`sub++`/`entry++` on properly-typed, correctly-strided pointers) --
   not fully evaluated; discovered mid-attempt that giving `SetupSub866E8`
   a padded size of `0xC` (so `sub++` advances by the right amount) would
   corrupt `sizeof(SetupEntry866E8)` if the padded type were embedded in
   it, and unpicking that cleanly was not finished this round. This is the
   most promising untried direction -- a `SetupSub866E8` sized 0xC used
   ONLY as a local walking-pointer's target type, kept entirely separate
   from the real (unpadded, 0xC via 3 flat fields) `SetupEntry866E8`.

GCC 2.6.3's strength reduction appears to reliably PROVE that `arr1[i].ptr0`
and `arr1[i].rate`/`arr1[i].id` (or any sub-expression derived from indexing
the SAME array with the SAME index) share one linear family and collapses
them to one induction variable, REGARDLESS of how the C groups or aliases
the fields (attempts 1-4 all landed on the identical instruction count and
register set). Only fully severing the provable relationship (attempt 5)
produced two variables, but at the cost of extra addressing instructions
that made the total WORSE, not better. Attempt 6 (natural pointer
arithmetic instead of cast-based) is the untried lever most likely to
resolve this without the cast overhead, but needs the padding kept local
rather than baked into the shared struct.

## Attempts

6 manual structural variants (`arr1[i].field` plain / `entry->field]` /
nested-sub-indexed / nested-sub-with-per-iteration-pointer / manual
cast-based dual pointer walk / started-but-unfinished natural dual pointer
walk). Best and current-best: attempt 1 (also 2, 3, 4 -- all tied), 14/105
apparent, structurally 104/105 real instructions matching, one missing
`addiu`.

### Proposed learning

**GCC 2.6.3's strength reduction does not appear controllable from source
once it can PROVE two array accesses share a base+index** -- every
C-level regrouping of `arr1[i].fieldA` vs `arr1[i].fieldB` (plain indexing,
an intermediate element pointer, a nested sub-struct, a per-iteration
sub-pointer) produced the IDENTICAL single-induction-variable output in
this case, even though the documented "explicit intermediate element
pointer" idiom (`DECOMPILATION_LEARNINGS`, confirmed twice elsewhere in this
project) normally changes register allocation for array loops. **The
lever that finally splits an array walk into TWO independent pointers, if
there is one, is not a grouping change but a genuine SEVERING of the
provable relationship** -- e.g. two loop-local pointers whose relationship
to a shared base is established once, OUTSIDE the loop, and never
recomputed from `arr1[i]` inside it (closer to attempt 5/6 above) --
and even that needs the INCREMENT to be free (natural typed `ptr++`, sized
so the type's own stride equals the real array stride) rather than an
explicit byte-cast `+0xC`, which visibly costs extra instructions. Screen
for "two independently-incrementing registers walking what is provably one
array" early (compare the FIRST diverging address's instruction to see if
it's a lone extra `addiu $sN,$sN,<stride>` right at the loop-continue
branch) -- it is cheap to misdiagnose as a big structural bug when 104 of
105 instructions already agree.
