# GameApplication__RunTitleMenu

> Renamed from `GameApplication__PollGraphRoomStatus` on 2026-09-27 (tools/rename.py). Address 0x80026410.

> Renamed from `Class6D3C8__PollGraphRoomStatus` on 2026-09-26 (tools/rename.py). Address 0x80026410.

> Renamed from `func_80026410` on 2026-09-24 (tools/rename.py). Address 0x80026410.

**Unit:** game_shell · **Size:** 66 instructions (0x108 bytes) · **Status:** MATCHED (66/66 words, whole-image SHA1 green). Took roughly a dozen iterations — the most attempt-expensive function of this round.

## What it does

`GameApplicationMethods` slot `+0x058`. Gated by `self->arg->unk10 != 0` (a third
sibling gate on the ctor argument, alongside `GameApplication__ShowIntroLogos`'s `unk0C` and
`GameApplication__PlayOpeningMovie`'s `unk08`). Checks the owned `DreamSys`'s own status slot
(`+0x1A0`); if it isn't already `1` and `self->unk24` hasn't latched, kicks
off one `PollTask` (`New_GraphRoom`) and, if *that* reports `2`, runs
`GameApplication__PlaySpecialDayMovies`. Then polls a second `PollTask` (`New_TitleMenu`) in a loop,
restarting the first `PollTask` each time it reports `2`, until it reports
anything else; clears `self->unk24` and returns `0` or `2` depending on
whether that final status was below `1` (unsigned).

## Final C

```c
s32 GameApplication__RunTitleMenu(GameApplication *self) {
    s32 status;
    s32 pollDone;

    if (self->arg->unk10 != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);

        status = self->dreamSys->vt->DreamSys__GetCurrentDayAndYear(self->dreamSys, 0);
        if (status != 1) {
            if (self->unk24 == 0) {
                status = GameApplication__RunTask(New_GraphRoom, self->dreamSys, self->unk1C);
                if (status == 2) {
                    GameApplication__PlaySpecialDayMovies(self);
                }
            }
        }

        pollDone = 2;
    retry:
        status = GameApplication__RunTask(New_TitleMenu, self->dreamSys, self->unk1C);
        if (status == pollDone) {
            GameApplication__RunTask(New_GraphRoom, self->dreamSys, self->unk1C);
            goto retry;
        }

        self->unk24 = 0;
        return ((u32)status < 1) << 1;
    }
    return 2;
}
```

## The path to byte-exact, in order (this is the expensive one — read it if
you hit a similar shape)

**Attempt 1 (5/66): plain early return.**
`if (self->arg->unk10 == 0) return 2;` followed by the main body. This
produced a completely different top: `bnez` (inverted condition) skipping an
inline `j <epilogue>; li v0,2` pair, instead of retail's forward `beqz`
straight to a `li v0,2` that falls into the shared epilogue. **The
HEAD-BROADCAST goto/return lever (from `New_Pad`) does NOT transfer
here unmodified** — swapping `return 2;` for `goto fail; ... fail: return
2;` (tried next, still 5/66, byte-identical output) changed *nothing*, because
that lever is about a *simple* function with one early check and one normal
exit; this function's normal path ends in a *loop*, and GCC's block-layout
decision for the early check depends on the whole function's shape, not just
the local goto/return spelling.

**Attempt 3 (16/66, then 24/66 after fixing the `if (status == 2)` register
question): invert the whole thing.** Wrap the ENTIRE body in
`if (self->arg->unk10 != 0) { ...; return computed; }` followed by a
trailing `return 2;` for the fall-through case — i.e., **the early-exit
value belongs at the END of the function as plain sequential code after the
if-block, not as an early return before it.** This alone fixed the top-level
block placement (retail's forward `beqz` straight to the tail's `li v0,2`).
The remaining residue after this fix was a `result` variable I'd introduced
to share one register between the two `status == 2` comparisons *and* the
final shift — this over-shared: retail's final `sll` targets a fresh `v0`,
not the persistent register, because the final computed value and the loop
poll-constant are unrelated across the function.

**Attempt 5 (29/66): a named `pollDone = 2` local, used only by the retry
loop's check, not the two shift and confirm.** This got every instruction
right except one: my code's *middle* `if (status == 2)` check (right after
the inner `unk24 == 0` block, unrelated to the loop) compiled to reuse the
persistent register too, because `pollDone = 2` was assigned *before* that
middle check in program order, and GCC's value numbering saw the same
literal already sitting in a register and reused it rather than
re-materializing a fresh one — even though the *middle* check has nothing to
do with the loop.

**Final fix (66/66): move `pollDone = 2` to occur textually AFTER the middle
check, immediately before the `retry:` label.** With no live "2" value
in scope yet at the middle check, GCC materializes a fresh temporary there
(matching retail's `li v1,0x2`), and only allocates the persistent
callee-saved register for `pollDone` starting from its actual declaration
point — which retail's own instruction scheduler then hoists earlier still
(into the delay slot of the unrelated first `beq`, an empty slot it happened
to fill), without that hoisting reaching back far enough to touch the middle
check.

## Proposed learning

**A named local used only for a LOOP's exit/retry comparison must be
initialized textually AFTER any earlier, unrelated comparison against the
same literal value, even if the earlier comparison is never touched by the
loop.** GCC 2.6.3's value numbering will reuse an already-materialized
constant for a later identical-literal comparison purely because it's
sitting in a live register at that program point, regardless of whether the
two comparisons are semantically related — the fix is to narrow the
variable's C-level live range to start only where its *own* control flow
(the loop) begins, not extend it to cover every comparison against the same
number. This is a companion finding to `Pad__DispatchEvents`'s "prologue store
order is unreachable from C": here, by contrast, the register-allocation
CHOICE (not just instruction order) *is* reachable from C — through where a
variable's assignment sits in program order — just not the reachable lever
you'd first reach for (writing the comparisons to literally match retail's
instruction-level register reuse).

**The goto/return early-exit lever from `New_Pad` is shape-specific,
not universal.** It fixed a function with one early check and a single
normal-path return. Here, with a loop on the normal path, the fix was the
opposite shape at the source level: wrap the *entire* normal path in the
positive condition and let the early-exit value fall out as trailing
sequential code, not an early return.

## HEAD BROADCAST cross-check (this round's two levers)

- **goto/return early-exit lever:** tried directly, no effect (see above);
  the fix that actually worked was restructuring which branch holds the
  fall-through code, not swapping `return` for `goto`.
- **hand-hoisted loop invariant lever:** not directly applicable (no array
  loop here), but the underlying idea — let a value's C-level liveness match
  where the compiler's own analysis would create it, rather than forcing it
  by hand — is exactly what closed this residue too.

## Naming history (before round 100)
**`GameApplication__RunTitleMenu` -- tier B.** Mechanics: gated by
`arg->unk10`, checks the owned `DreamSys`'s own status accessor, then loops
`GameApplication__RunTask(New_TitleMenu, ...)`, restarting
`GameApplication__RunTask(New_GraphRoom, ...)` on every "2" report, until
the second poll task reports something else. `New_GraphRoom` is an
established, evidence-backed name from another unit
(`src/class_3bb8c_t.c:315`, `GraphRoom__GraphRoom`), so "GraphRoom" is
real vocabulary, not a guess -- but `New_TitleMenu`'s own class is still
unnamed, and this function's ultimate purpose (what "graph room" readiness
gates) is not established here. The name describes the poll/retry mechanics
around the one named PollTask class involved.

## Track 7 polish (round 100, echo)

### Naming

**`GameApplication__RunTitleMenu` -- tier B** (renamed from `GameApplication__PollGraphRoomStatus` with tools/rename.py). Evidence: nothing is polled: it runs the day's GraphRoom (unless day 1 or skipGraphRoomPoll), PlaySpecialDayMovies when GraphRoom scored, then the TitleMenu, and GraphRoom again each time the menu returns TITLEMENU_RESULT_GRAPH; its return tells Application__RunMainLoop to run a day (GAMEAPPLICATION_LOOP_DAY, the menu's result 0) or go back to the opening movie. Tier B: the name leads with the menu, and the GraphRoom prelude is part of the hook too.

Body changes, all byte-identical: pollDone -> graphResult (MATCHING: it must stay a named local set after the GraphRoom check, see the derivation); literals 2 -> GRAPHROOM_RESULT_SCORED, TITLEMENU_RESULT_GRAPH, GAMEAPPLICATION_LOOP_DAY.

### History: code_1677c.c comments before the round-100 polish

Moved here from the source, verbatim (names as they stood then, where the tools had not already rewritten them).

```c
/* Gated by self->config->pollGraphRoom. Checks the DreamSys's own status slot
 * (+0x1A0); if it isn't already "1" and self->skipGraphRoomPoll hasn't latched, kicks
 * off one PollTask (New_GraphRoom) and, if THAT reports "2", runs
 * GameApplication__StartGraphRoomStreamTask. Then polls a second PollTask (New_TitleMenu) in a loop,
 * restarting the first PollTask each time it reports "2", until it
 * reports anything else; clears self->skipGraphRoomPoll and returns 0 or 2 depending
 * on whether that final status was below 1. */
```
