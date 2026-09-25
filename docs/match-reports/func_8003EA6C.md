# func_8003EA6C — MATCHED

Unit: `code_2cc8c_d`. Round 73, runner charlie (report written retroactively;
the function itself was already byte-exact under `INCLUDE_ASM` before this
round -- an empty body, `jr $ra; nop`, that splat's own extraction produced
without any hand-written C being required).

## Signature

```c
void func_8003EA6C(void);
```

Not a `gViewportMethods` vtable slot occupant (`tools/classtable.py gViewportMethods`
lists no entry at any offset resolving to this address) and not a method --
it takes no `self` at all. No caller found anywhere in `src/` or the
remaining `asm/*.s` segments as of this round.

## What it does

Nothing: the whole body is `jr $ra; nop`. CLAUDE.md's own caution applies
verbatim here -- "not every matched function was work."

## Naming

Kept `func_8003EA6C`. No evidence of any kind: no `self` parameter to tie it
to `Unk18Obj` or any other class, no caller, no vtable slot. Tier-C's
`Class__func_xxxxx` form does not apply because no class is established.
Nothing to propose.
