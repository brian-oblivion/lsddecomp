# Class6B5CC__NotifyIfUnk20Active — MATCHED

> Renamed from `func_8001D568` on 2026-09-18 (tools/rename.py). Address 0x8001d568.

Unit: `code_d294_b`. Round 13, runner delta. 38/38 words, full match.

## Signature

```c
void Class6B5CC__NotifyIfUnk20Active(Class6B5CCObj *self, s32 a1);
```

## What it does

Gated on `2 <= a1 < 4`, `self->unk20 != NULL`, and `func_8001F3A4(self->unk20)`
being true: fills a local stack buffer via this class's own `+0x8C` vtable
slot (`Class6B5CC__ReadUnk20Data`, already matched in this unit — forwards to
`TmdModel__GetHull(self->unk20, dest)`), then forwards that same buffer into
`+0x90` (`Class6B5CC__TransformAndNotifyParents`, also already matched), with the original `a1`
passed through as `Class6B5CC__TransformAndNotifyParents`'s own `a2`.

```c
void Class6B5CC__NotifyIfUnk20Active(Class6B5CCObj *self, s32 a1) {
    u8 buf[0x38];

    if (a1 >= 4) {
        return;
    }
    if (a1 < 2) {
        return;
    }
    if (self->unk20 == NULL) {
        return;
    }
    if (!func_8001F3A4(self->unk20)) {
        return;
    }
    self->methods->slot8C(self, buf);
    self->methods->slot90(self, (GenericCountList_d294 *)buf, a1);
}
```

## Two things that had to be found, in order

1. **`a1 < 4 && a1 >= 2` in one `if` condition gets optimized into a range
   check.** Writing the two guards as one logical `&&` expression made GCC
   2.6.3 fold `2 <= a1 < 4` into `(unsigned)(a1 - 2) < 2` (`addiu`+`sltiu`),
   a single comparison — not retail's two independent `slti` branches.
   Splitting the same two conditions into two separate early-return `if`
   statements reproduced retail's two-branch shape exactly. Not yet in
   DECOMPILATION_LEARNINGS as its own entry; see Proposed learning below.

2. **The stack buffer has to be sized 0x38 bytes, not
   `sizeof(GenericCountList_d294)` (8 bytes).** `self->methods->slot90`
   (`Class6B5CC__TransformAndNotifyParents`) only reads its own `a1` argument's `+0x0`/`+0x4`, so
   `GenericCountList_d294`'s minimal 8-byte shape is sufficient for THAT
   call to compile and match. But the SAME buffer is also the `dest` argument
   to `Class6B5CC__ReadUnk20Data` → `TmdModel__GetHull` (PsyQ, `asm/psyq_GsLinkObject4.s`,
   not decompiled), whose own disassembly stores through offsets out past
   `+0x32` of `dest`. An 8-byte local buffer compiles fine (nothing checks
   bounds) but reserves too little stack, giving a frame 0x30 bytes
   *smaller* than retail's 0x58 — a silent, byte-identical-looking-at-the-
   call-site bug that only shows up as "everything after this instruction
   differs" in funcdiff (first attempt: 6/38, with the 284855-byte outside-
   range warning). Declared the local as a raw `u8 buf[0x38]` sized purely
   to reproduce retail's frame; the true field layout of PsyQ's `dest`
   struct is out of scope for this project. `self->methods->slot8C(self,
   buf)` takes it as `void *` with no cast needed; `slot90` needs an
   explicit `(GenericCountList_d294 *)` cast since it's a different pointer
   type than `u8[]`.

## Header changes

`include/code_d294.h`:

- `Class6B5CCMethods`: typed `+0x08C` (`slot8C`, `void (*)(Class6B5CCObj*,
  void*)`, occupant `Class6B5CC__ReadUnk20Data`) and `+0x090` (`slot90`, `void (*)
  (Class6B5CCObj*, GenericCountList_d294*, s32)`, occupant `Class6B5CC__TransformAndNotifyParents`).
  Both already-matched functions in this unit; confirmed occupants via
  `tools/classtable.py gClass6B5CCMethods`. Split out of the `pad060[0x0A0-0x060]`
  span that previously covered them (that pad's own comment claimed nothing
  dispatched through it — no longer true once this call site was written).
- New extern `func_8001F3A4(void *arg0)` returning `s32`, PsyQ library
  (`asm/psyq_GsLinkObject4.s`), same opaque `self->unk20` shape as
  `TmdModel__GetHull`/`Class6B5CC__ReadUnk20Data`'s own declarations.

## Proposed learning

**A range check written as one `&&` expression (`lo <= x && x < hi`) can get
folded by GCC 2.6.3 into a single unsigned-subtract comparison
(`(unsigned)(x - lo) < (hi - lo)`), which retail does not always do.** When
the disassembly shows two INDEPENDENT `slti`/branch pairs testing the same
variable against two different bounds (not a combined `sltiu` off an
`addiu`-adjusted value), write the two bounds as two separate statements —
guard-clause early returns worked here — rather than one compound boolean
expression. This is the same family as the already-recorded "branch TARGETS
disagree" discriminator: read the actual comparison instructions before
trusting that a natural-looking `&&` will reproduce them.

**A vtable call's argument buffer can need to be sized for what a
DIFFERENT, non-decompiled callee (reached transitively through another
already-matched slot) writes into it, not for what the function you're
currently writing reads back out of it.** `Class6B5CC__NotifyIfUnk20Active` itself never reads
`buf`'s contents; it only forwards the pointer twice. The size that makes
the frame match came from PsyQ's `TmdModel__GetHull`, three calls away. When a
"send a same buffer to two vtable slots" shape scores an in-range match but
funcdiff's outside-range byte count is huge, check what the buffer's
producer (not just its declared field-access pattern) writes through it
before assuming the type/size you inferred from the READER slot is
complete.

## Naming (round 54, bravo, track 3)

Renamed from `func_8001D568` via `tools/rename.py`. **Tier B** -- gates
on `2 <= a1 < 4` and `self->unk20 != NULL && func_8001F3A4(self->unk20)`,
then chains `Class6B5CC__ReadUnk20Data` (slot `+0x08C`) into
`Class6B5CC__TransformAndNotifyParents` (slot `+0x090`). Name describes
the gate-then-forward mechanics; purpose of the `a1` range or the
underlying notification is not established. Purely local to this unit
+ its header.
