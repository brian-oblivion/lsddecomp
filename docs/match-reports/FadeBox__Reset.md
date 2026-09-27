# FadeBox__Reset -- MATCH (27/27 words, first attempt)

> Renamed from `Class6E99C__Reset` on 2026-09-26 (tools/rename.py). Address 0x8003fed8.

> Renamed from `FadeBox__FinishConstruct` on 2026-09-26 (tools/rename.py). Address 0x8003fed8.

> Renamed from `func_8003FED8` on 2026-09-20 (tools/rename.py). Address 0x8003fed8.

Unit `code_2cc8c_e`, carved round 14. `FadeBoxMethods::finishConstruct` (`+0x040`),
dispatched by the class's own ctor (`FadeBox__FadeBox`) right after it installs
`self->methods`.

```c
void FadeBox__Reset(FadeBoxObj *self, s32 a1) {
    self->unk70 = a1;
    self->state = 0;
    self->step = 0xA;
    self->unk78 = 0;
    self->unk7C = 0;
    self->methods->slot60(self, 0);
    self->methods->slot64(self, 0);
    self->altMode = 0;
}
```

`slot60`/`slot64` resolve to `BoxFill__SetDisplay`/`BoxFill__SetSemiTrans`
(code_2cc8c_e, bravo's own functions), dispatched purely through the
vtable -- no extern needed.

## Two-arg slot, not four

The ctor's call site only sets up `a1` before dispatching `self->methods->
finishConstruct(self, a2)`; `a2`/`a3` registers still hold leftover values from the
PRECEDING `GetBoxFillMethods()->ctor(...)` call and are never intentionally set.
Since this occupant's own body never reads a 3rd/4th argument, the slot's
type is `(FadeBoxObj *self, s32 a1)` -- two args, matching CLAUDE.md's
"the converse does NOT hold" caution (a `jalr` with no visible extra setup
is not proof of a 1-arg call; here it IS 2-arg on the strength of the
occupant's own body, and 2 is also all the CALL SITE bothers to configure).

## Naming (round 61, track 3)

**`FadeBox__Reset`** -- tier A. `FadeBoxMethods::finishConstruct`
(`+0x040`), dispatched by `FadeBox__FadeBox` immediately after
installing `self->methods`. Named to match the architecturally identical
slot in the same class hierarchy: `Obj6EAC0Methods::slot40` (bravo's own
unrenamed slot name, `code_2cc8c_e`) is already
named `TextRow__Reset` (round 54, `code_2cc8c_e`/this header),
and `ClassEAC0Methods::finishConstruct` (this unit, `BoxFill__Reset`)
occupies the SAME offset one level up the same chain, dispatched the same
way (right after a ctor installs the vtable). Three independent occupants
at the identical offset, all doing "one-time post-construction setup",
is strong cross-checked evidence for the slot's role, not a guess from
this one function alone.

## Track 4 (2026-09-26, round 87, echo)

Renamed from `FadeBox__FinishConstruct` to `FadeBox__Reset`: it is the
override of the inherited +0x040 slot, which SceneNode names `reset` and
whose BoxFill occupant is `BoxFill__Reset` (`tools/classtable.py gFadeBoxMethods
--vs gBoxFillMethods`: +0x040 OVERRIDDEN, `BoxFill__Reset` ->
this function). FINISHING-PLAN track 4 step 6 names an override for its
slot, and the body does no more than a reset: it stores the ctor's
channel mask as `defaultChannels`, zeroes `state`, `channels` and `unk7C`,
sets `step` to 10, `altMode` to 0, and turns the box's display and
semi-transparency off (the inherited `setDisplay`/`setSemiTrans`). Its
parameter list `(self, channels)` differs from the slot's `(self)`, so the
slot keeps the inherited type and the ctor calls it through
`FadeBoxResetFn` (include/FadeBox.h). Tier A.

## Track 6 (2026-09-26, round 93, charlie)

Renamed with the class: `Class6E99C` is now `FadeBox`
(`python3 tools/renametype.py Class6E99C FadeBox`, table
`python3 tools/rename.py D_8006E99C gFadeBoxMethods`), tier A; the evidence
is in New_FadeBox.md's Track 6 section. The method's own name was kept: it
already says what the body does. renametype.py rewrote the old class name in
this report's earlier history too (known, pending an operator decision).

