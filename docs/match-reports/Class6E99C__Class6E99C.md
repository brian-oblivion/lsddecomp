# Class6E99C__Class6E99C -- MATCH (43/43 words, first attempt)

> Renamed from `func_8003FE2C` on 2026-09-20 (tools/rename.py). Address 0x8003fe2c.

Unit `code_2cc8c_e`, carved round 14. `Class6E99CObj`'s own constructor
(`Class6E99CMethods::ctor`, slot `+0x008`).

```c
void Class6E99C__Class6E99C(Class6E99CObj *self, void *a1, s32 a2, s32 a3) {
    ClassEAC0Methods *base;
    void *tableEntry;

    base = Obj6EAC0__GetBaseMethods();
    if (a2 != 0) {
        tableEntry = &D_8006EA90[a2 * 3];
    } else {
        tableEntry = D_8006EAA8;
    }
    base->ctor((ClassEAC0Obj *)self, a1, tableEntry, a3);
    self->methods = GetClass6E99CMethods();
    self->methods->finishConstruct(self, a2);
}
```

**Correction: the `Obj6EAC0__GetBaseMethods()` call must be hoisted into its own
statement BEFORE the `if`/`else`, matching retail's own evaluation order.**
An earlier version of this report called it inline as part of the
`base->ctor(...)` expression, positioned textually AFTER the `if`/`else` --
that version actually inflated the function from 43 to 46 words (a real
register-saturation regression, +3 callee-saved registers) because GCC
evaluated `Obj6EAC0__GetBaseMethods()` late instead of early. Confirmed with a full
rebuild and an address cross-check against `build/lsdde.map`; the version
above is the one that reaches the real 43/43 stated below.

Textbook "call the further-base ctor first (through a getter for its
table, not by direct name -- `Obj6EAC0__GetBaseMethods` returns `&D_8006EAC0`), THEN
overwrite `self->methods` with this class's own table, THEN dispatch
through it immediately" idiom (DECOMPILATION_LEARNINGS' `Class86B60__Class86B60`
entry). See `include/code_2cc8c.h`'s header comment above
`struct ClassEAC0Obj` for the full class-hierarchy discovery writeup this
function anchors.

An explicit cast (`(ClassEAC0Obj *)self`) is needed at the base-ctor call:
`self` really is the SAME memory, but C has no notion that `Class6E99CObj`
and `ClassEAC0Obj` are related (they are two independent flat local views,
per this project's convention -- not a real C `struct` embedding).

`a2` is a raw index/mode value (0, or a small count), not a pointer -- this
function is the one that converts it into a `tableEntry` address before
forwarding to the next ctor down the chain, whose OWN `a2` really is
already a pointer.

## Naming (round 61, track 3)

**`Class6E99C__Class6E99C`** -- tier A. `Class6E99CMethods::ctor`
(`+0x008`). Named per the project's `Class__Class` constructor convention
(see `Entity__Entity`, `DreamSys.c`): calls the further-base ctor
(`Obj6EAC0__GetBaseMethods()->ctor(...)`) first, then installs this class's
own `&D_8006E99C` table, then redispatches through `finishConstruct` --
the textbook "base ctor first, then own vtable, then dispatch" idiom
already documented elsewhere in this project. Mechanics (construct an
instance of this class) fully determine the name.
