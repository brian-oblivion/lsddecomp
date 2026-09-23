# NoOpIgnoreArgs -- MATCHED (trivial, splat-generated)

> Renamed from `LinkOwnerObj__NoOp` on 2026-09-18 (tools/rename.py). Address 0x80056df0.

> Renamed from `Noop` on 2026-09-18 (tools/rename.py). Address 0x80056df0.

Unit: `class_3bb8c_o` (round 17; named round 52). `void func_80056DF0(void) {}`
-- an empty body, `jr $ra; nop`. No hand derivation was needed or done; it is
included here (unlike most such stubs) because it is a deliberate no-op
target of a dispatch table, not merely an unworked artifact of extraction.

## Final source

```c
void NoOpIgnoreArgs(void) {
}
```

## Derivation

Whole body is the trivial `jr $ra; nop` epilogue with nothing in between --
splat emits this shape itself for any zero-instruction function. Called from
`src/class_3bb8c_s.c`'s `Class876FC__UpdateByKind` as the `self->unk54 == 2` handler,
alongside two real handlers for `case 0` and `case 3` -- i.e. retail's own
source really does dispatch to an empty function for this state, this is not
a decompilation artifact.

## Naming

**`NoOpIgnoreArgs` -- tier A.** A pure leaf whose mechanics ARE its purpose: the body
does nothing, and it is reached as the `case 2:` arm of the state switch on
`self->unk54` in `class_3bb8c_s.c:Class876FC__UpdateByKind`, beside real arms
(`Class876FC__DriftModelChildren`, `LinkOwnerObj__RandomizeLinks`) that take the same
`(self, arg1)` shape -- which rules out "this is just an unextracted stub" as
the alternative reading. No class prefix: the function takes no `self` (a
bare `void(void)`), and every call site passes a dead second argument the
body never reads, so it is not meaningfully a method of `LinkOwnerObj`
despite being defined among that group's functions.

HEAD (round 52), at merge: bravo named this `Noop`, which collided with
`NoOp` (0x80026C80, runner alpha's `code_171e0` unit) in nothing but the case
of one letter -- two distinct empty functions a reader cannot tell apart.
Resolved here, not in the runner's unit, because the collision only existed
once both branches were in one tree. `NoOp` keeps the plain name: it is the
shared vtable filler, measured in 10 method tables
(`tools/classtable.py --scan`, slots resolving to 0x80026C80). This one is
reached by a direct call and sits in NO table -- the same measurement found
zero tables referencing 0x80056DF0 -- so the discriminator in the name is
the dead arguments its callers pass. An earlier head attempt at
`LinkOwnerObj__NoOp` was withdrawn: the prefix asserts a methodhood the bare
`void(void)` signature denies, which is bravo's own argument above and it is
correct.
