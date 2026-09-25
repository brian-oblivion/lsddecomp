# StreamTaskObj__Configure

> Renamed from `func_8003BA58` on 2026-09-23 (tools/rename.py). Address 0x8003ba58.

**Unit:** code_2c054 · **Size:** 23 instructions (0x5C bytes) · **Status:** MATCHED (23/23 words, whole-image SHA1 green), first attempt

## What it does

Stores three of its own arguments (plus a fourth, stack-spilled one) into
`self`'s fields, then delegates to the sibling class `gTaskCoreMethods`
(`Get_vtable_TaskCore()`, `LoaderTaskMethods` in `Class6D3C8.h`) at slot `+0x044`,
passing `self` and its own first argument, hardcoding the third argument to
0. Occupies `gStreamTaskObjMethods` slot `+0x044` itself.

**This function's signature was independently cross-checked and confirmed
against `include/Class6D3C8.h`.** That header already documents
`StreamTaskMethods::slot44` (established from a *different* unit's call
sites, `Class6D3C8__LoadIntroLogoSequence`/`Class6D3C8__StartWeeklyStreamTask`/`Class6D3C8__StartGraphRoomStreamTask` in `code_1677c`) as
`void (*slot44)(void *self, s32 a1, s32 arg2, s32 typeLookup, s32 flag)` --
five arguments, the fifth spilled to the stack, exactly matching what this
function's own disassembly reads at entry (`lw $v0, 0x30($sp)`, the 5th
argument). The parameter names below (`arg2`, `typeLookup`, `flag`) are
taken verbatim from that header's comment to keep the two independent
derivations consistent.

## Derivation

```
lw    $v0, 0x30($sp)          ; 5th arg (stack)
sw    $a2, 0xB8($s0)          ; self->unkB8 = arg2
sw    $a3, 0xBC($s0)          ; self->unkBC = typeLookup
jal   Get_vtable_TaskCore
 sw   $v0, 0xC0($s0)          ; self->unkC0 = flag (delay slot, independent store)
addu  $a0, $s0, zero
addu  $a1, $s1, zero
lw    $v0, 0x44($v0)
jalr  $v0
 addu $a2, $zero, zero        ; hardcoded 0
...epilogue
```

```c
void StreamTaskObj__Configure(StreamTaskObj *self, s32 a1, s32 arg2, s32 typeLookup, s32 flag) {
    self->unkB8 = arg2;
    self->unkBC = typeLookup;
    self->unkC0 = flag;
    Get_vtable_TaskCore()->slot44(self, a1, 0);
}
```

Matched first attempt. The three independent field stores were written in
straight source order; the compiler slides the third (`unkC0 = flag`) into
the following `jal`'s delay slot on its own, no manual reshaping needed.

## New struct/header knowledge

Named `StreamTaskObj::unkB8`/`unkBC`/`unkC0` in `include/code_2c054.h`, and
`TaskCoreMethods::slot44` (`s32 (*)(StreamTaskObj *self, s32 a1, s32 a2)` --
see the note below on why this unit's local typing differs from
`Class6D3C8.h`'s for the FUNCTION `TaskCore__Init` that occupies this same
`gTaskCoreMethods` slot, as opposed to this function's OWN slot `+0x044` of
`gStreamTaskObjMethods`, which is void and confirmed by the cross-check above).

## Proposed learning

**A slot number and even a matching argument count do not imply a matching
signature across TWO DIFFERENT tables**, even when one calls straight into
the other at the identical offset: `gStreamTaskObjMethods::slot44` (this function,
void, 5 args including a stack-spilled 5th) forwards to `gTaskCoreMethods::slot44`
(`TaskCore__Init`, this unit's own local typing says it returns `s32`, only 3
args) -- the offset coincidence is a delegation convenience, not evidence of
identical calling convention. Cross-checking an unfamiliar function's
signature against an already-established header (here, `Class6D3C8.h`) before
typing it from scratch is worth doing whenever the class or table is shared
across units -- it turned an otherwise-unverifiable parameter-name guess
into a corroborated one.

## Naming

**StreamTaskObj__Configure** -- tier B. Occupies `gStreamTaskObjMethods` slot
`+0x044`. Cross-unit call sites in `src/code_1677c.c`
(`Class6D3C8__LoadIntroLogoSequence`/`Class6D3C8__StartWeeklyStreamTask`, via `include/Class6D3C8.h`'s independent
`StreamTaskMethods::slot44` view) show this stores a resource
name-or-derived-value, a type lookup and a flag before up-calling the base
slot -- the mechanics (store configuration words, then delegate) are clear,
but the game meaning of the second argument varies by caller (a filename in
one call, a plain derived count in another), so no more specific verb is
supported yet.
