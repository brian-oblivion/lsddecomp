# Class6D3C8__LoadIntroLogoSequence

> Renamed from `func_80026170` on 2026-09-24 (tools/rename.py). Address 0x80026170.

**Unit:** code_1677c · **Size:** 57 instructions (0xE4 bytes) · **Status:** MATCHED (57/57 words, whole-image SHA1 green)

## What it does

`Class6D3C8Methods` slot `+0x050`. Gated entirely by `self->arg->unk0C != 0`
(the ctor argument's `+0x0C` field, opaque until this function): registers a
"loader" task for `"ETC\ASMKLOGO.TIM"` (`Class6D3C8__StartLoaderTask`), then builds a
separate "stream" task, initializes it with a filename
(`"ETC\ASMK.STR"`) and a type/format code looked up from a table, starts it,
then registers a second loader task for `"ETC\OSDLOGO.TIM"`.

## Derivation

```
lw   $v0, 0x20($s2)          ; s2 = self; v0 = self->arg
lw   $v0, 0xC($v0)            ; v0 = arg->unk0C
beqz $v0, .L80026238            ; whole body gated on this
 ...
jal  SetActiveDataSourceDriverMode(0, 0, 0)
jal  Class6D3C8__StartLoaderTask(self, sLogoPathAsmk)     ; "ETC\ASMKLOGO.TIM"
jal  New_StreamTaskObj(0, 0, 0, 0)            ; -> s1 = task (New_X shape, 0xDC bytes)
addiu $a0, $sp, 0x18
jal  func_800490F4                          ; writes 0x31 to local, returns &D_800113DC
 (delay: s1 = v0, i.e. the PRECEDING call's return = task)
lw   $a0, 0x18($sp)                          ; reload the type code (0x31) func_800490F4 just wrote
jal  func_800493C8(a0=0x31)                    ; halfword lookup in D_80086170
 (delay: s0 = v0, i.e. the PRECEDING call's return = streamName, &D_800113DC)
move $a0, $s1                                    ; a0 = task
move $a2, $s0                                     ; a2 = streamName
lw   $a3, 0x0($s1)                                 ; a3 = task->methods (scratch, to fetch the slot)
li   $v1, 1
sw   $v1, 0x10($sp)                                  ; 5th argument, stack-spilled
lw   $a1, 0x1C($s2)                                    ; a1 = self->unk1C
lw   $v1, 0x44($a3)                                     ; v1 = task->methods->slot44
jalr $v1
 (delay: a3 = v0)                                        ; a3 OVERWRITTEN with func_800493C8's
                                                            ; return value -- the REAL 4th argument
lw   $v0, 0x0($s1)                                          ; reload task->methods
lw   $v0, 0x4($v0)                                           ; slot4
jalr $v0
 (delay: a0 = s1 = task)
jal  Class6D3C8__StartLoaderTask(self, sLogoPathOsd)                          ; "ETC\OSDLOGO.TIM"
```

```c
void Class6D3C8__LoadIntroLogoSequence(Class6D3C8 *self) {
    const char *streamName;
    s32 typeCode;
    s32 typeLookup;
    StreamTask *task;

    if (self->arg->unk0C != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        Class6D3C8__StartLoaderTask(self, sLogoPathAsmk);
        task = New_StreamTaskObj(0, 0, 0, 0);
        streamName = func_800490F4(&typeCode);
        typeLookup = func_800493C8(typeCode);
        task->methods->slot44(task, self->unk1C, streamName, typeLookup, 1);
        task->methods->slot4(task);
        Class6D3C8__StartLoaderTask(self, sLogoPathOsd);
    }
}
```

## Two things that were not obvious from a first read of the disassembly

**1. The delay slot after `jalr` is not a scratch reload — it is the real
4th argument, and it overwrites what looks like the argument in the
preceding instructions.** `lw $a3, 0x0($s1)` (task->methods) is used to
*compute the call target* (`v1 = task->methods->slot44`), but by the time
the `jalr` actually transfers control, its own delay slot (`move $a3, $v0`)
has already replaced `$a3` with `func_800493C8`'s return value. A naive read
sees "`a3` = task's own vtable pointer, passed as an argument" and that
reading is wrong — the vtable pointer was only ever a scratch value for the
indirect call computation, immediately clobbered before the callee sees it.
The first C attempt (discarding `func_800493C8`'s return and passing
`task->methods` as the 4th argument) built and even scored 44/57 — plausible
enough to be mistaken for a near-miss — before the diff against retail's
actual delay-slot value (`move a3,v0` where `v0` is the *call's* return, not
a memory load) exposed the real data flow. Always check what the delay slot
*is* a move of, not just that a move exists there.

**2. Register (s0 vs s1) assignment for two callee-saved temporaries tracked
declaration order, not statement/liveness order.** `task` (from
`New_StreamTaskObj`) and `streamName` (from `func_800490F4`) are both live
across further calls, `task` for longer (reloaded twice more). The first
attempt declared `task` before `streamName` and code assigned `task` first
(chronologically first live); GCC put `task` in `s0` and `streamName` in
`s1` — backwards from retail (`s1`=task, `s0`=streamName). Simply reordering
the **declarations** (not the statements) — `streamName`/`typeCode` first,
`task` last — flipped the allocation to match, with no other change. Same
`__asm__("")`-relevant caveat as always: this only ever moved WHICH
callee-saved register held which value, never introduced a
`register asm("$N")` binding, so it's within the rules.

## Proposed learning

When two callee-saved locals' physical register assignment (not their
control flow) is the only residue, try reordering their **C declarations**
before reaching for anything else — this GCC's simple linear allocator
appears to hand out `$s0`, `$s1`, ... in declaration order rather than
first-use order, at least for this shape (mirrors, but is distinct from,
`Pad__DispatchEvents`'s head-broadcast finding that prologue *store* order is
unreachable from C at all — this is about which variable gets which
register, a different axis).

## HEAD BROADCAST cross-check (this round's two levers)

- **goto/return early-exit lever:** not applicable — this function has no
  early-return-with-different-value shape; its single guard wraps the
  entire body and falls through to a shared epilogue either way.
- **hand-hoisted loop invariant lever:** not applicable — no loop in this
  function.

## Naming

**`Class6D3C8__LoadIntroLogoSequence` -- tier B.** Mechanics established
from the body and its string constants: gated by `arg->unk0C`, registers a
loader task for `"ETC\ASMKLOGO.TIM"`, then a stream task for whatever
`func_800490F4` resolves (`"ETC\ASMK.STR"` per the header comment), then a
second loader task for `"ETC\OSDLOGO.TIM"` -- three named boot-time assets
loaded in sequence. "Intro logo sequence" describes what the function DOES
(loads these three specific assets, gated, in this order); it does not
assert why the game shows them (splash-screen framing is a reasonable
inference from the filenames but not confirmed by any code in this unit).
