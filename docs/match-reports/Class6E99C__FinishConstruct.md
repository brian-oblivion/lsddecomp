> Renamed from `func_8003FED8` on 2026-09-20 (tools/rename.py). Address 0x8003fed8.

# Class6E99C__FinishConstruct -- MATCH (27/27 words, first attempt)

Unit `code_2cc8c_e`, carved round 14. `Class6E99CMethods::finishConstruct` (`+0x040`),
dispatched by the class's own ctor (`Class6E99C__Class6E99C`) right after it installs
`self->methods`.

```c
void Class6E99C__FinishConstruct(Class6E99CObj *self, s32 a1) {
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

`slot60`/`slot64` resolve to `func_800406E4`/`func_80040714`
(code_2cc8c_f, bravo's own functions), dispatched purely through the
vtable -- no extern needed.

## Two-arg slot, not four

The ctor's call site only sets up `a1` before dispatching `self->methods->
finishConstruct(self, a2)`; `a2`/`a3` registers still hold leftover values from the
PRECEDING `Obj6EAC0__GetBaseMethods()->ctor(...)` call and are never intentionally set.
Since this occupant's own body never reads a 3rd/4th argument, the slot's
type is `(Class6E99CObj *self, s32 a1)` -- two args, matching CLAUDE.md's
"the converse does NOT hold" caution (a `jalr` with no visible extra setup
is not proof of a 1-arg call; here it IS 2-arg on the strength of the
occupant's own body, and 2 is also all the CALL SITE bothers to configure).

## Naming (round 61, track 3)

**`Class6E99C__FinishConstruct`** -- tier A. `Class6E99CMethods::finishConstruct`
(`+0x040`), dispatched by `Class6E99C__Class6E99C` immediately after
installing `self->methods`. Named to match the architecturally identical
slot in the same class hierarchy: `Obj6EAC0Methods::slot40` (bravo's own
unrenamed slot name, `code_2cc8c_f`) is already
named `Obj6EAC0__FinishConstruct` (round 54, `code_2cc8c_f`/this header),
and `ClassEAC0Methods::finishConstruct` (this unit, `ClassEAC0__FinishConstruct`)
occupies the SAME offset one level up the same chain, dispatched the same
way (right after a ctor installs the vtable). Three independent occupants
at the identical offset, all doing "one-time post-construction setup",
is strong cross-checked evidence for the slot's role, not a guess from
this one function alone.
