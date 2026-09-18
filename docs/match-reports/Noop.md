# Noop -- MATCHED (trivial, splat-generated)

Unit: `class_3bb8c_o` (round 17; named round 52). `void func_80056DF0(void) {}`
-- an empty body, `jr $ra; nop`. No hand derivation was needed or done; it is
included here (unlike most such stubs) because it is a deliberate no-op
target of a dispatch table, not merely an unworked artifact of extraction.

## Final source

```c
void Noop(void) {
}
```

## Derivation

Whole body is the trivial `jr $ra; nop` epilogue with nothing in between --
splat emits this shape itself for any zero-instruction function. Called from
`src/class_3bb8c_s.c`'s `func_80056640` as the `self->unk54 == 2` handler,
alongside two real handlers for `case 0` and `case 3` -- i.e. retail's own
source really does dispatch to an empty function for this state, this is not
a decompilation artifact.

## Naming

**`Noop` -- tier A.** A pure leaf whose mechanics ARE its purpose: the body
does nothing, and it is reached as one arm of an otherwise-real dispatch
table (`class_3bb8c_s.c:func_80056640`), which rules out "this is just an
unextracted stub" as the alternative reading. No class prefix: the function
takes no `self` (a bare `void(void)`), and every call site passes a dead
second argument the body never reads, so it is not meaningfully a method of
`LinkOwnerObj` despite being defined among that group's functions.
