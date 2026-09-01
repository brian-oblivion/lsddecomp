# func_8004A3EC -- MATCHED (27/27 words)

`ClassA`'s +0x060 vtable slot (`onEvent`), shared verbatim by `ClassB` --
the same function pointer appears at +0x060 in both `D_800865C8` and
`D_80086668`, not overridden. Forwards to the base implementation of this
same slot, then handles event 4 specially:

```c
void func_8004A3EC(ClassA *self, s32 event) {
    func_8003E5C8()->onEvent(self, event);
    if (event == 4) {
        self->unk28 = 1;
        self->methods->slot7C(self);
    }
}
```

Getting the branch shape right took one correction: the initial reading of
the disassembly (before checking instruction-by-instruction against the
label placement) assumed `self->unk28 = 1` was unconditional, sitting in
the `bne`'s delay slot. It is not -- the delay slot only materialises the
constant 1 into `$v1`; the `sw $v1, 0x28($s1)` that actually stores it is
the next sequential instruction, which only executes on the not-taken path
(`event == 4`). Lining up the branch *target* against where each
instruction actually falls (CLAUDE.md/MATCHING-GUIDE.md: "branch targets
disagree... outranks everything else in this list") is what caught it
before any build was attempted.

## The apparent self-reference

`func_8003E5C8()->onEvent(self, event)` reads, at face value, as this
function calling itself: `func_8003E5C8()` behaves, everywhere else in
this unit, as returning `ClassA`'s own table (see `func_8004A228.md`), and
slot +0x060 of that table is `func_8004A3EC` itself. Unlike the `ClassB`
override cases (`func_8004A2C4`/`func_8004A324`, where the *caller* is a
different override than the callee it reaches), here the caller and the
apparent callee are the exact same function.

This doesn't block the C from compiling or matching -- it's an opaque
`extern` call regardless of what's conceptually at that address -- but it
is a genuine open question for whoever eventually carves/decompiles
`func_8003E5C8` itself: either this really is intentional recursion that
never triggers in practice (a dead/unreachable path from every observed
call site), or `func_8003E5C8()` is not a constant "return ClassA's table"
function at all and actually depends on some ambient/global context that
differs at this call site specifically. Not resolved this round; flagged
for whoever carves `code_2cc8c.s` (where `func_8003E5C8` lives) or
attempts `func_80049A1C`/`func_80049AC0` (ClassA's own un-overridden
implementations at the slots ClassB's overrides reach through this same
getter) next.
