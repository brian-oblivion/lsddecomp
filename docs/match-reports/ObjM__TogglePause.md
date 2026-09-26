# ObjM__TogglePause

> Renamed from `ObjM__DispatchActiveState` on 2026-09-26 (tools/rename.py). Address 0x80053458.

> Renamed from `func_80053458` on 2026-09-24 (tools/rename.py). Address 0x80053458.

**Unit:** class_3bb8c_l · **Size:** 26 words (0x68 bytes) ·
**Status: MATCHED 26/26**, whole-image SHA1 green.

## What it does

```c
void ObjM__TogglePause(Obj87034_3bb8c_l *self) {
    Obj87034Methods_3bb8c_l *m = self->methods;

    if (self->unk80 != 0) {
        m->slotC4(self);
        m->slotD4(self);
    } else {
        m->slotD0(self);
    }
}
```

## Residue: register identity, fixed by removing a `void (*fn)(...)` local

The first form used a `void (*fn)(Obj87034_3bb8c_l *)` local, set inside
each branch and called once at the end (mirroring `ObjM__Update`'s
matched shape, which uses exactly that pattern one function earlier in
this same unit):

```c
/* did NOT match: self/methods swapped between $s0/$s1 vs retail */
Obj87034Methods_3bb8c_l *m = self->methods;
void (*fn)(Obj87034_3bb8c_l *);
if (self->unk80 != 0) {
    m->slotC4(self);
    fn = m->slotD4;
} else {
    fn = m->slotD0;
}
fn(self);
```

This compiled with `self` and `m` (`self->methods`) holding EACH OTHER's
retail registers (`$s0`/`$s1` swapped) — same instructions, same values,
wrong register identity throughout the whole function. Per CLAUDE.md this
is a stall class, not something to fix with a register pin. Reordering the
two local declarations (`fn` before `m`) made it WORSE (introduced a
fourth diff). The fix that worked was removing the `fn` indirection
entirely and calling directly through `m->slotXX(self)` in each branch —
semantically identical (the deferred-call-through-a-pointer pattern was
only ever called once per branch anyway) but it changed which two values
GCC decided were worth keeping in the two available saved registers, and
picked the same allocation retail did.

### Proposed learning

**A `void (*fn)(...)` local deferred-call pattern that matched cleanly in
one function of a unit is not safe to reuse by analogy in a sibling
function of the same unit** if that sibling has a DIFFERENT number of live
values competing for saved registers. Here it cost a register-identity
swap between `self` and `self->methods` that reordering declarations could
not fix; removing the indirection (call directly in each branch) did. This
is the same lever DECOMPILATION_LEARNINGS already documents for a
crossjump-mergeable shared tail ("write the full call statement out in
each branch rather than deferring through a function-pointer local"), just
reached via a register-identity residue instead of a content-tail residue.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80053458` | `ObjM__TogglePause` | B | see below |

**Evidence.** vtable slot +0x074. Gated on `self->unk80`: dispatches `slotC4` then `slotD4` when set, else `slotD0` alone -- structurally similar to `ObjM__Update`'s own `unk80` branch but a distinct slot pair.


## Track 4 (2026-09-26, round 89, echo)

Renamed from `ObjM__DispatchActiveState` (rename.py). The two branches are the pause overlay's: with `pauseSetupStep` (+0x080) nonzero it runs clearCloseReadyFlag (+0x0C4) and teardownPauseOverlay (+0x0D4), else advancePauseSetup (+0x0D0), which builds the "Pause" TextRow. It fills Class86668's `slot74`, which `ObjM__DispatchPadEvent` runs for pad code 0x21. Tier B (a toggle of the pause overlay; which button 0x21 is, is not established).
