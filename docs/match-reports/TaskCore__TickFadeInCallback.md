# TaskCore__TickFadeInCallback — MATCH (27/27 words)

> Renamed from `TaskCore__TickFadeCallback` on 2026-09-28 (tools/rename.py). Address 0x8003cbc0.

> Renamed from `Obj86B60__TickFadeCallback` on 2026-09-25 (tools/rename.py). Address 0x8003cbc0.

> Renamed from `func_8003CBC0` on 2026-09-24 (tools/rename.py). Address 0x8003cbc0.

**Unit:** Task · **Size:** 27 instructions

## What it does

```c
s32 TaskCore__TickFadeInCallback(Obj86B60 *self)
{
    s32 result;

    result = 1;
    if (self->unk88 != NULL) {
        result = self->unk88(self);
    }
    if (result != 0) {
        self->methods->slot60(self, 5);
    }
    return result;
}
```

The consumer of `self->unk88` (set by `TaskCore__SetFadeInCallbackEnabled`). Exactly the
"default value, then conditionally overwritten" idiom already documented
in DECOMPILATION_LEARNINGS.md: the `beqz`'s delay slot sets `result = 1`
UNCONDITIONALLY, then the taken call overwrites it. This IS this class's
own vtable slot `+0x0AC` (per `classtable.py`).

**Addendum after `TaskCore__Update` was matched (its only caller in this
unit):** originally typed `(Obj86B60*, s32 a1)`, assuming the caller
forwarded a genuine (if unused) `a1` argument, per CLAUDE.md's "unused
parameter in the callee" idiom. `TaskCore__Update`'s own residue proved this
wrong: its `jalr` to this slot sets up ONLY `a0=self`, and `$a1` at that
call site is a caller-saved register left over from an EARLIER, unrelated
call (`slot60(self, 6)`) a few instructions before -- not a value the
source is deliberately forwarding. Retyped to `s32 (*)(Obj86B60*)`
(one argument). This function's OWN compiled bytes are unaffected by the
retype (the parameter was never read in its body either way); the earlier
27/27 match stands unchanged. See `TaskCore__Update`'s report for the full
account and the generalized lesson (an argument register surviving an
intervening CALL, not just the next instruction, is not reliably a real
argument).

## Struct knowledge established

- `Obj86B60Methods::slotAC` (+0x0AC) -- IS this function; `s32
  (*)(Obj86B60*)`, one argument (retyped, see addendum above).

## Provenance

round 2026-09-02, runner echo, unit Task. 1 attempt (plus a
same-round retype with no rebuild-affecting change, see addendum).

## Naming (round 78, delta)

**Tier B.** `func_8003CBC0` -> `Obj86B60__TickFadeCallback`. Body: if
`self->unk88` is set, calls it (`result = self->unk88(self)`, i.e. invokes
`TaskCore__TickFadeIn` when the fade is enabled -- see
`SetFadeCallbackEnabled`); if the result is nonzero, transitions to state 5
via `SetState`. Occupies slotAC. This is the "run the enabled fade tick, and
advance state when it signals done" half of the `unk88` pair; the meaning of
state 5 itself is not established.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__TickFadeCallback (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 98, alpha)

`result` -> `done`; setState(5) -> TASKCORE_STATE_ACTIVE. Byte-identical.
