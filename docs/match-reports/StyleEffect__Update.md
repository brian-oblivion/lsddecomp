# StyleEffect__Update -- MATCHED (11/11 words)

> Renamed from `Class876FC__Update` on 2026-09-26 (tools/rename.py). Address 0x800564f4.

> Renamed from `func_800564F4` on 2026-09-23 (tools/rename.py). Address 0x800564f4.

Unit: `class_3bb8c_r` (round 17 continuation). Increments `self->unk24`
and forwards to `StyleEffect__UpdateByKind` (a plain statement call, not a tail-call
whose return is forwarded -- the function itself is `void`).

## Final source

```c
extern void StyleEffect__UpdateByKind(StyleEffect *self);

void StyleEffect__Update(StyleEffect *self) {
    self->unk24 = self->unk24 + 1;
    StyleEffect__UpdateByKind(self);
}
```

## Derivation

`self->unk24` (the SAME field `StyleEffect__SetParams` clears to 0, confirming
it's a plain counter this class maintains) is loaded, incremented, and
stored back BEFORE the call to `StyleEffect__UpdateByKind` -- the store sits in the
`jal`'s own delay slot in the disassembly, matching a normal C statement
order where the increment is evaluated ahead of the call it feeds no
argument to. `StyleEffect__UpdateByKind` is outside this unit's range (still
`INCLUDE_ASM` elsewhere), declared as a local extern per the established
cross-unit-call convention; nothing in this call site reads `$v0`
afterward, so `void` is the (conservative, discardable-return-caveat-
noted) starting type.

### Proposed learning

None -- a plain leaf, last of the twenty functions attempted this pass.

## Round 58 (alpha, externcheck.py) -- true callee signature update

`StyleEffect__UpdateByKind` was still `INCLUDE_ASM` when this report was written, so
its real arity was unknown here. It has since matched (round 44,
`class_3bb8c_s.c`) as genuinely 2-argument:
`void StyleEffect__UpdateByKind(LinkNode *self, void *arg1)`, and `arg1` is not
dead -- it is dereferenced (`AddVec3(&local, (Vec3S *)arg1,
&self->unk58);`) and forwarded live to three further callees in that
body's switch. This call site (`StyleEffect__UpdateByKind(self);`, one argument)
therefore leaves the real `arg1` parameter reading whatever `$a1` holds
at the call, not a value this function ever sets up -- confirmed by
`tools/externcheck.py`: fixing the extern here to the real 2-arg
signature turns the build red (`too few arguments to function
'StyleEffect__UpdateByKind'`), i.e. this is a genuine caller/callee arity
disagreement, not one of the project's documented "extra dead argument"
idioms. Left as `extern void StyleEffect__UpdateByKind(StyleEffect *self);` /
`StyleEffect__UpdateByKind(self);` (unchanged) because a call-site fix is out of
scope for the extern-hygiene pass that found it -- flagged for whoever
next touches this function or its caller.

## Naming

**Tier A.** `+0x0EC`, the class's own per-frame slot per class_3bb8c_s.c's banner ("update slot (+0x0EC)"); the body ticks a counter and forwards to `StyleEffect__UpdateByKind` every call, which is what "Update" names.

## Track 4 (2026-09-26, round 88, charlie)

Occupies Actor's +0x0EC `setPendingExtra` slot; kept its name because it is the per-frame update, not a setter. Now declared `(StyleEffect *self, LongVec3 *pos)` and forwards `pos` to StyleEffect__UpdateByKind: its only caller, StyleUpdateEffectSlots (class_3bb8c_k.c), passes the position in $a1 and UpdateByKind reads it, so the "arity-ok" 1-argument call above was the same bytes spelled with the argument implicit. Byte-identical with the argument explicit.

## Track 7 (2026-09-27, round 96, charlie)

The source comment's disassembly evidence moved here: retail's `jal` to
StyleEffect__UpdateByKind at 0x80056508 sets no `$a1`, so `pos` reaches it in
the register StyleUpdateEffectSlots loaded. Zero bytes.
