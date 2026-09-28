# SceneNode__NotifyWithHull — MATCHED

> Renamed from `SceneNode__NotifyIfUnk20Active` on 2026-09-26 (tools/rename.py). Address 0x8001d568.

> Renamed from `Class6B5CC__NotifyIfUnk20Active` on 2026-09-26 (tools/rename.py). Address 0x8001d568.

> Renamed from `func_8001D568` on 2026-09-18 (tools/rename.py). Address 0x8001d568.

Unit: `SceneNode`. Round 13, runner delta. 38/38 words, full match.

## Signature

```c
void SceneNode__NotifyWithHull(SceneNodeObj *self, s32 a1);
```

## What it does

Gated on `2 <= a1 < 4`, `self->unk20 != NULL`, and `TmdModel__GetBoundsCount(self->unk20)`
being true: fills a local stack buffer via this class's own `+0x8C` vtable
slot (`SceneNode__GetModelHull`, already matched in this unit — forwards to
`TmdModel__GetHull(self->unk20, dest)`), then forwards that same buffer into
`+0x90` (`SceneNode__TransformAndNotifyParents`, also already matched), with the original `a1`
passed through as `SceneNode__TransformAndNotifyParents`'s own `a2`.

```c
void SceneNode__NotifyWithHull(SceneNodeObj *self, s32 a1) {
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
    if (!TmdModel__GetBoundsCount(self->unk20)) {
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
   (`SceneNode__TransformAndNotifyParents`) only reads its own `a1` argument's `+0x0`/`+0x4`, so
   `GenericCountList_d294`'s minimal 8-byte shape is sufficient for THAT
   call to compile and match. But the SAME buffer is also the `dest` argument
   to `SceneNode__GetModelHull` → `TmdModel__GetHull` (PsyQ, `asm/psyq_GsLinkObject4.s`,
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

`include/scene_node.h`:

- `SceneNodeMethods`: typed `+0x08C` (`slot8C`, `void (*)(SceneNodeObj*,
  void*)`, occupant `SceneNode__GetModelHull`) and `+0x090` (`slot90`, `void (*)
  (SceneNodeObj*, GenericCountList_d294*, s32)`, occupant `SceneNode__TransformAndNotifyParents`).
  Both already-matched functions in this unit; confirmed occupants via
  `tools/classtable.py gSceneNodeMethods`. Split out of the `pad060[0x0A0-0x060]`
  span that previously covered them (that pad's own comment claimed nothing
  dispatched through it — no longer true once this call site was written).
- New extern `TmdModel__GetBoundsCount(void *arg0)` returning `s32`, PsyQ library
  (`asm/psyq_GsLinkObject4.s`), same opaque `self->unk20` shape as
  `TmdModel__GetHull`/`SceneNode__GetModelHull`'s own declarations.

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
currently writing reads back out of it.** `SceneNode__NotifyWithHull` itself never reads
`buf`'s contents; it only forwards the pointer twice. The size that makes
the frame match came from PsyQ's `TmdModel__GetHull`, three calls away. When a
"send a same buffer to two vtable slots" shape scores an in-range match but
funcdiff's outside-range byte count is huge, check what the buffer's
producer (not just its declared field-access pattern) writes through it
before assuming the type/size you inferred from the READER slot is
complete.

## Naming (round 54, bravo, track 3)

Renamed from `func_8001D568` via `tools/rename.py`. **Tier B** -- gates
on `2 <= a1 < 4` and `self->unk20 != NULL && TmdModel__GetBoundsCount(self->unk20)`,
then chains `SceneNode__GetModelHull` (slot `+0x08C`) into
`SceneNode__TransformAndNotifyParents` (slot `+0x090`). Name describes
the gate-then-forward mechanics; purpose of the `a1` range or the
underlying notification is not established. Purely local to this unit
+ its header.

## Track 6 (round 91, echo): named `SceneNode__NotifyWithHull`, tier B

For `2 <= event < 4`, when a model with bounds is linked, fills a TmdHull through getModelHull and hands it to transformAndNotifyParents with the event. Mechanics only: what events 2 and 3 mean is not established. Overridden by Actor__NotifyMove, DreamSys__NotifyLinkAttempt, StageMap__OnSlotEvent. Was `NotifyIfUnk20Active`. Slot +0x088 kept as `notifyIfUnk20Active` (callers in six units outside this job); `notifyWithHull` proposed. The class was renamed Class6B5CC -> SceneNode in the same pass (include/scene_node.h's banner has the evidence).

## Round 100 (delta): track 7

Parameter `a1` -> `event`. The local `u8 buf[0x38]` is a `TmdHull hull`
(0x34 bytes): the frame is unchanged, because 0x34 rounds up to the same
0x58 frame; the inner comment below claimed 0x38 was needed for it, and the
whole-image build shows it is not. The `(TmdHull *)` cast went with it.
Events 2 and 3 stay literals: what they mean is not established (see the
naming note above).

### History: the comments in src/code_d294_b.c before this pass, verbatim

```c
/* a1 gates a small range (2 <= a1 < 4). When self->model is set and
 * TmdModel__GetBoundsCount(self->model) reports true, fills a stack buffer through
 * this class's own +0x8C slot (SceneNode__GetModelHull, already matched in this
 * unit -- fills it via TmdModel__GetHull(self->model, dest)) then forwards
 * that same buffer, retyped as a TmdHull, into +0x90
 * (SceneNode__TransformAndNotifyParents, also already matched in this unit), with the original
 * a1 passed through as SceneNode__TransformAndNotifyParents's own a2. */

/* Sized to reproduce retail's own frame (0x58): SceneNode__GetModelHull's own
     * target (TmdModel__GetHull, TmdModel) writes a TmdHull
     * (include/TmdModel.h: a count word and eight 6-byte corners, 0x34
     * bytes) into its `dest`, so the true destination struct is bigger than
     * the 8 bytes a count and one corner would reserve. */
```
