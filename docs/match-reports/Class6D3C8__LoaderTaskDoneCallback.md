# Class6D3C8__LoaderTaskDoneCallback

> Renamed from `func_80026328` on 2026-09-24 (tools/rename.py). Address 0x80026328.

**Unit:** code_1677c · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

One-line forwarding wrapper: `return func_8004A070(0);`

## Derivation

```
addiu $sp, $sp, -0x18
sw    $ra, 0x10($sp)
jal   func_8004A070
 addu $a0, $zero, $zero
lw    $ra, 0x10($sp)
addiu $sp, $sp, 0x18
jr    $ra
 nop
```

No instruction touches `$v0` between the `jal` and the `jr $ra` — the
callee's return value passes straight through in the register. Per
CLAUDE.md ("the byte match tells you NOTHING about the return type" for a
one-line tail-call wrapper), the *bytes* alone don't distinguish `void
Class6D3C8__LoaderTaskDoneCallback(void)` from a value-returning one. Positive evidence that
`func_8004A070` itself returns a value: its own body (`asm/class_39e08.s`,
around `func_8004A070`) ends with `beqz $v0, .L8004A104` gating the loop
exit on `$v0`, i.e. it's actively computed and meaningful, not incidentally
left in the register. Written to forward it:

```c
extern s32 func_8004A070(s32 a0);

s32 Class6D3C8__LoaderTaskDoneCallback(void) {
    return func_8004A070(0);
}
```

## Proposed learning

None beyond the standing "wrapper return type" rule already in CLAUDE.md —
this is a clean instance of it: the callee's own use of `$v0` as a real
result (a loop-exit condition) is what tips the type call, not the wrapper's
bytes.
