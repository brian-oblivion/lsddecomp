# Obj865C8__AdvanceState — MATCHED (94/94 words)

> Renamed from `func_80049CA8` on 2026-09-23 (tools/rename.py). Address 0x80049ca8.

`Class865C8Methods` slot +0x054.

## Disassembly shape

```
addiu $sp, $sp, -0x20
sw    $s1, 0x14($sp)
addu  $s1, $a0, $zero        ; s1 = self
sw    $s0, 0x10($sp)
addu  $s0, $a1, $zero        ; s0 = arg1
sw    $s2, 0x18($sp)
sw    $ra, 0x1C($sp)
jal   GetClass86668Methods
 addu $s2, $a2, $zero        ; s2 = arg2
addu  $a0, $s1, $zero
addu  $a1, $s0, $zero
lw    $v0, 0x54($v0)         ; gClass86668Methods's own +0x054
jalr  $v0
 addu $a2, $s2, $zero        ; GetClass86668Methods()->slot54(self, arg1, arg2), return discarded
ori   $v0, $zero, 0x2
bne   $s2, $v0, END          ; if (arg2 != 2) goto END
 nop
lw    $v1, 0x3C($s1)         ; self->unk3C
nop
beq   $v1, $s2, END          ; if (self->unk3C == arg2) goto END   (arg2==2 here)
slti  $v0, $v1, 0x3          ; v0 = (self->unk3C < 3)
beqz  $v0, .L80049D18        ; if not, go test the high branch
 ori  $v0, $zero, 0x1
beq   $v1, $v0, .L80049D2C   ; if (self->unk3C == 1) goto CASE1
 nop
j     END                    ; else (self->unk3C == 0): nothing to do
 nop
.L80049D18:
ori   $v0, $zero, 0x3
beq   $v1, $v0, .L80049D98   ; if (self->unk3C == 3) goto CASE3
 nop
j     END                    ; else (self->unk3C == 2): nothing to do
 nop
.L80049D2C:                  ; CASE1
lw    $a0, 0x38($s1)         ; self->unk38
lw    $v0, 0x0($a0)
lw    $v0, 0x1B4($v0)        ; methods->slot1B4
jalr  $v0
 nop                          ; result = self->unk38->methods->slot1B4(self->unk38)
bgez  $v0, .L80049DFC        ; if (result >= 0) goto shared tail
 addu $a0, $s1, $zero
lw    $a0, 0x38($s1)
lw    $v0, 0x0($a0)
lw    $v0, 0x1B8($v0)        ; methods->slot1B8
jalr  $v0
 addu $a1, $zero, $zero       ; self->unk38->methods->slot1B8(self->unk38, 0)
lw    $v0, 0x0($s1)          ; self->methods
addu  $a0, $s1, $zero
sw    $s2, 0x28($a0)          ; self->unk28 = arg2
lw    $v0, 0x60($v0)           ; methods->onEventArg
jalr  $v0
 ori  $a1, $zero, 0x3            ; self->methods->onEventArg(self, 3)
j     END
 nop
.L80049D98:                  ; CASE3
lw    $a0, 0x4C($s1)         ; self->unk4C
lw    $v0, 0x0($a0)
lw    $v0, 0x48($v0)         ; methods->slot48
jalr  $v0
 nop                          ; self->unk4C->methods->slot48(self->unk4C), discarded
lw    $a0, 0x4C($s1)
lw    $v0, 0x0($a0)
lw    $v0, 0x4($v0)          ; methods->slot4
jalr  $v0
 nop                          ; self->unk4C->methods->slot4(self->unk4C), discarded
lw    $a0, 0x38($s1)
lw    $v0, 0x0($a0)
lw    $v0, 0x1E0($v0)        ; methods->slot1E0
jalr  $v0
 nop                          ; result = self->unk38->methods->slot1E0(self->unk38)
addu  $a0, $s1, $zero
.L80049DFC:                  ; shared tail
jal   Obj865C8__EnterState2
 addu $a1, $v0, $zero        ; Obj865C8__EnterState2(self, result)
END:
...
jr $ra
```

## Final C

```c
extern void Obj865C8__EnterState2(Obj865C8 *self, s32 arg1);

void Obj865C8__AdvanceState(Obj865C8 *self, s32 arg1, s32 arg2) {
    s32 result;

    GetClass86668Methods()->slot54(self, arg1, arg2);
    if (arg2 == 2 && self->unk3C != arg2) {
        switch (self->unk3C) {
        case 1:
            result = self->unk38->methods->slot1B4(self->unk38);
            if (result < 0) {
                self->unk38->methods->slot1B8(self->unk38, 0);
                self->unk28 = arg2;
                self->methods->onEventArg(self, 3);
                return;
            }
            Obj865C8__EnterState2(self, result);
            break;
        case 2:
            break;
        case 3:
            self->unk4C->methods->slot48(self->unk4C);
            self->unk4C->methods->slot4(self->unk4C);
            result = self->unk38->methods->slot1E0(self->unk38);
            Obj865C8__EnterState2(self, result);
            break;
        }
    }
}
```

`Obj865C8__EnterState2` is defined LATER in this file (ROM order requires the C
definition to stay where it is), so a local `extern` forward prototype was
added right above this function -- same "calling into a function that is
still being written elsewhere" convention CLAUDE.md documents, applied to a
same-unit forward reference rather than a cross-unit one.

## Residue and how it closed (9 attempts)

The whole function matched immediately except a 2-word gap at the very top
of the dispatch: retail has an extra `slti $v0,$v1,0x3` / `beqz` pair before
testing `self->unk3C == 1`, and BOTH "no match" exits use a real `beq
.../nop/j END/nop` (4 words) rather than a single inverted branch. Every
attempt that reproduced the range test failed to ALSO reproduce the
double-branch tail, or vice versa:

1. **Plain `switch` with only `case 1`/`case 3`.** 20/94 (94-2=92 matched,
   everything else perfectly aligned once the systematic 2-word shift is
   accounted for) — GCC 2.6.3 uses a flat 2-way equality chain for a
   2-value switch, no range pivot at all.
2. **Added `case 0: break;`** (case values `{0,1,3}`, 3 real labels). Worse
   (19/94) — GCC's switch-balancing picks the MEDIAN case value (1) as an
   EQUALITY-test root regardless of source declaration order, then
   recurses into a SECOND range test (`slti v1,2`) nested inside the
   already-false branch to distinguish `{0}` from... this is structurally
   a different (correctly balanced, just different) tree from retail's.
3. **Nested `if (unk3C < 3) { if (unk3C == 1) {...} } else if (unk3C == 3)
   {...}`.** Got the top-level range test right, but GCC merged the inner
   single-armed `if` into ONE inverted branch (`bne ...,END`) since nothing
   else follows in that scope — losing retail's 4-word double-branch shape.
   36/94.
4. **Same nested if, with an explicit early `return;` on the "false"
   path** (`if (unk3C != 1) return;`) instead of relying on implicit
   fallthrough. Byte-identical to attempt 3 — GCC treats the two spellings
   as the same control-flow graph, so this doesn't distinguish "genuinely
   falls through" from "explicit exit" the way it does elsewhere in this
   codebase (contrast the project's own `goto`/`return` lever, which DOES
   matter when the two exits return DIFFERENT values — here both exits are
   `void`, so there's nothing for the compiler to disambiguate on).
5. **`case 0: case 2: break;` (fallthrough-combined).** Picked yet another
   pivot (`slti v1,2` again, plus a redundant `beq v1,s2,END` since `s2==2`
   was still in a register) — worse.
6. **Four distinct labels `case 0: break; case 2: break;` (not
   fallthrough-combined) with case1/case3 unchanged.** Same wrong pivot,
   19/94.
7. **`default: break;`** instead of any numbered low case. Identical to
   attempt 1 (20/94) — an empty default changes nothing here.
8. **Reordered case labels in SOURCE** (case3 before case1, case0 last) —
   confirmed GCC sorts case values internally before balancing regardless
   of declaration order (same wrong pivot as attempt 2's family;
   reordering source doesn't matter). This ruled out declaration-order
   tie-breaking as a lever.
9. **`case 1: ...; case 2: break; case 3: ...;`** (real value set `{1,2,3}`,
   the one dense range NOT yet tried) — full match, 94/94. GCC's
   case-balancer picks the pivot from the ACTUAL case value set, and for
   `{1,2,3}` (as opposed to `{1,3}` or `{0,1,3}`) it produces a genuine
   `< pivot` / `>= pivot` split at the value 3 (the max), with the low
   partition `{1,2}` distinguished by one more equality test on 1 (case 2's
   empty body needs no code, but its PRESENCE as a real case value is what
   shapes the tree) — exactly retail's instruction sequence.

## New struct knowledge (`include/class_39e08.h`)

- `Class86668Methods::slot54` added — inherited, shared verbatim with
  `D_800865C8`'s own occupant of this slot (this function itself); the
  sibling's own occupant is `Obj86B60__OnTag1Notify`, out of this unit's scope.
- `SubObjDMethods` extended with `slot1B4` (`s32 (*)(SubObjD *self)`,
  return value used — genuinely non-void) and `slot1B8` (`void (*)(SubObjD
  *self, s32 arg1)`), and `slot1E0` (`s32 (*)(SubObjD *self)`, return value
  forwarded straight into `Obj865C8__EnterState2`'s own argument).
- `Obj4CMethods` extended with `slot4` and `slot48` (both `void (*)(Obj4C
  *self)`, return discarded at both call sites here).

## Attempts

9 (see residue above; final form 94/94).

### Proposed learning — direct answer to the head's question

**Still no instance of "field name describes layout, not which function
runs once `self->methods` is reassigned"** — no vtable-pointer reassignment
anywhere in this function. Five functions into this round now, reporting
negative consistently; the one confirmed instance remains `func_8004A19C`'s
ctor from the earlier round.

**New, well-tested lever worth generalizing:** for a small dense-ish switch
where only some case values have real bodies, GCC 2.6.3's case-balancing
tree is sensitive to the EXACT SET of case values present (including
otherwise-empty ones), not just the "meaningful" ones, and NOT sensitive to
their declaration order. When a switch's comparison instructions include a
range test (`slti`) that a plain "only the cases with real code" switch
doesn't reproduce, try adding the case value(s) that would make the real
values into a genuinely DENSE, CONSECUTIVE run (here, `{1,3}` -> `{1,2,3}`
by adding the empty `case 2:`) rather than a sparse run with a gap (`{0,1,3}`
does NOT work the same way `{1,2,3}` does, confirmed by two different
negative attempts). This is a sharper, testable version of the existing
CLAUDE.md switch note ("picks its own comparison order... do not transcribe
the observed order as case order") — it now also says: don't transcribe
the observed VALUE SET as the real case set either; a gap in the middle of
the visible cases is itself informative about a missing empty case.
