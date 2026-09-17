# BasicClass__func_182cc — MATCHED (33/33 words)

Unit: `src/code_8220_b.c`. This is `BasicClassMethods` vtable slot `+0x030`,
`onFinalize(self, s32 flag)` (already documented in
`include/code_8220.h`'s struct comment). Walks `self->parentRefs` and, for
each parent, calls that parent's own `slot38` (`BasicClass__func_18358`,
already matched in this unit) with `self` as its `arg1` and `flag` passed
through as `arg2`. Since `slot38` only acts `if (arg2 == 1)`, calling
`onFinalize(self, 1)` (as `BasicClass__func_17f2c`'s finalize path does)
tells every parent holding a reference to `self` to remove `self` as its
own child.

## Final source

```c
void BasicClass__func_182cc(BasicClass *self, s32 arg1)
{
    BasicClassListNode *cursor = self->parentRefs;
    BasicClass *value;

    for (GetNextBasicClass(&value, &cursor); value != NULL; GetNextBasicClass(&value, &cursor)) {
        value->methods->slot38(value, self, arg1);
    }
}
```

## Notes

Byte-exact on the first attempt. The disassembly's `j` straight to the loop
condition before the first iteration is the classic `for`/`while`
lowering — GCC emits `init; goto cond; body: ...; cond: test; branch`. Since
this function's "condition" is itself a call with a side-effecting output
parameter (`GetNextBasicClass` pops the cursor and writes `*value`), the natural
C spelling is a `for` loop whose init AND increment clauses are both that
same call, with the body running only the vtable dispatch. Writing it as a
`while (GetNextBasicClass(&value, &cursor), value != NULL)`-style comma
expression was unnecessary — the plain `for (call; test; call) { body }`
form reproduces the goto-to-condition shape directly and needs no unusual
syntax.

Register mapping confirms the earlier read of the call site: `a0` = the
value popped off the cursor (`this` for the `slot38` call), `a1` = `self`
(the finalizing object, cast to `void *` for `slot38`'s `arg1`), `a2` =
`arg1` (the finalize flag, passed straight through as `slot38`'s `arg2`) —
matching `BasicClassMethods.slot38`'s signature
`void (*)(BasicClass *self, void *arg1, s32 arg2)` already in the header.
