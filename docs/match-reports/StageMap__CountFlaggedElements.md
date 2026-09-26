# StageMap__CountFlaggedElements — MATCHED (13/13 words)

> Renamed from `Class866E8__CountFlaggedElements` on 2026-09-26 (tools/rename.py). Address 0x8004bce0.

> Renamed from `func_8004BCE0` on 2026-09-24 (tools/rename.py). Address 0x8004bce0.

Not a vtable slot — `StageMap__CountFlaggedElements` does not appear in `gStageMapMethods`
(confirmed via `tools/classtable.py 0x800866E8`), so it is a plain,
non-virtual helper. Takes `self` directly as its only argument.

## Disassembly

```
addu $a1, $zero, $zero       ; a1 = count = 0
addu $v1, $zero, $zero       ; v1 = i = 0
.loop:
lhu  $v0, 0xEC($a0)          ; self->arr[i].flag  (a0 walks by 0x1C/iteration)
nop
beqz $v0, .skip
 addiu $a0, $a0, 0x1C        ; a0 += 0x1C (delay slot -- ALWAYS executes)
addiu $a1, $a1, 0x1          ; count++ (only reached when flag != 0)
.skip:
addiu $v1, $v1, 0x1          ; i++
slti $v0, $v1, 0x7
bnez $v0, .loop
 nop
jr $ra
 addu $v0, $a1, $zero        ; return count
```

The moving-pointer codegen (`a0` incremented by `0x1C` each iteration, and
`+0xEC` always read relative to the CURRENT `a0`) is GCC's natural -O2
expansion of a plain indexed loop over a fixed-size array — no manual
pointer-walking needed in the source; see the "final C" below.

## Final C

```c
s32 StageMap__CountFlaggedElements(Obj866E8 *self) {
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < 7; i++) {
        if (self->arr[i].flag != 0) {
            count++;
        }
    }
    return count;
}
```

## New struct knowledge (`include/class_3bb8c.h`)

- `Obj866E8::arr` — a 7-element array at `+0x0EC`, each element `0x1C`
  bytes (new `Elem` type). Only `Elem::flag` (`u16`, +0x000) is established
  so far, from this function's nonzero check.

## Attempts

1 (matched on first attempt).

### Proposed learning

Confirms (doesn't newly establish) the project's existing "let GCC hoist
its own loop invariants" guidance: writing the natural indexed-array-access
loop, rather than hand-rolling pointer arithmetic to mimic the observed
`lhu 0xEC($a0)` / `addiu $a0,$a0,0x1C` register moves, reproduces retail
exactly. The instinct to transcribe the moving-pointer shape literally
would have been wrong and unnecessary.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004BCE0` | `StageMap__CountFlaggedElements` | A | Pure leaf: loops `self->arr[7]`, counts entries with `flag != 0`, returns the count. A pure count is tier A by the naming rule's own "getter/clamp/list-push" clause -- the mechanics ARE the purpose. Called by `StageMap__ApplyRateEntries` to refresh `self->unk1B4` after flagging/unflagging elements. |
