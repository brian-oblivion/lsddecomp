# func_8003FE2C -- MATCH (43/43 words, first attempt)

Unit `code_2cc8c_e`, carved round 14. `Class6E99CObj`'s own constructor
(`Class6E99CMethods::ctor`, slot `+0x008`).

```c
void func_8003FE2C(Class6E99CObj *self, void *a1, s32 a2, s32 a3) {
    void *tableEntry;

    if (a2 != 0) {
        tableEntry = &D_8006EA90[a2 * 3];
    } else {
        tableEntry = D_8006EAA8;
    }
    func_800408BC()->ctor((ClassEAC0Obj *)self, a1, tableEntry, a3);
    self->methods = func_800404C0();
    self->methods->slot40(self, a2);
}
```

Textbook "call the further-base ctor first (through a getter for its
table, not by direct name -- `func_800408BC` returns `&D_8006EAC0`), THEN
overwrite `self->methods` with this class's own table, THEN dispatch
through it immediately" idiom (DECOMPILATION_LEARNINGS' `func_8004D578`
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
