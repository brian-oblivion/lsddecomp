# func_8003FED8 -- MATCH (27/27 words, first attempt)

Unit `code_2cc8c_e`, carved round 14. `Class6E99CMethods::slot40` (`+0x040`),
dispatched by the class's own ctor (`func_8003FE2C`) right after it installs
`self->methods`.

```c
void func_8003FED8(Class6E99CObj *self, s32 a1) {
    self->unk70 = a1;
    self->unk6C = 0;
    self->unk74 = 0xA;
    self->unk78 = 0;
    self->unk7C = 0;
    self->methods->slot60(self, 0);
    self->methods->slot64(self, 0);
    self->unk98 = 0;
}
```

`slot60`/`slot64` resolve to `func_800406E4`/`func_80040714`
(code_2cc8c_f, bravo's own functions), dispatched purely through the
vtable -- no extern needed.

## Two-arg slot, not four

The ctor's call site only sets up `a1` before dispatching `self->methods->
slot40(self, a2)`; `a2`/`a3` registers still hold leftover values from the
PRECEDING `Obj6EAC0__GetBaseMethods()->ctor(...)` call and are never intentionally set.
Since this occupant's own body never reads a 3rd/4th argument, the slot's
type is `(Class6E99CObj *self, s32 a1)` -- two args, matching CLAUDE.md's
"the converse does NOT hold" caution (a `jalr` with no visible extra setup
is not proof of a 1-arg call; here it IS 2-arg on the strength of the
occupant's own body, and 2 is also all the CALL SITE bothers to configure).
