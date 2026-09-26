# ObjM__OnFadeNotify

> Renamed from `ObjM__HandleEvent5Or6` on 2026-09-26 (tools/rename.py). Address 0x80053f84.

> Renamed from `func_80053F84` on 2026-09-23 (tools/rename.py). Address 0x80053f84.

**Unit:** class_3bb8c_m · **Size:** 89 instructions · **Status:** MATCHED (89/89 words)

## What this function does

A two-case dispatch on its third parameter (`sel`), doing nothing for any
other value. Establishes `self->unk18`'s vtable slot `0x64`, `self->unk3C`'s
slot `0x17C`, `ObjMMethods::slot14`, and a brand-new parameter type
`ParamM` (this function's own `arg1`, dispatched through its own vtable
slot `0xE4` — unrelated to `ObjM::unk14`'s `FieldM14` despite the numeric
proximity of "14"; nothing ties the two together).

## The C

```c
void ObjM__OnFadeNotify(ObjM *self, ParamM *p1, s32 sel) {
    s32 v;
    switch (sel) {
    case 5:
        self->methods->slot14(self, p1);
        self->unk3C->methods->slotF4(self->unk3C, 0);
        self->unk20 = 0;
        break;
    case 6:
        self->methods->slot14(self, p1);
        v = p1->methods->slotE4(p1);
        self->unk18->methods->slot64(self->unk18, v);
        if (self->unk20 != 5 && self->unk20 != 8 && self->unk20 == 0xA) {
            self->unk3C->methods->slot17C(self->unk3C, 1);
            self->unk3C->methods->slotF4(self->unk3C, 0);
            self->unk20 = 4;
        }
        self->methods->slot30(self, self->unk20);
        break;
    }
}
```

## Residue, and the fix

**First attempt (0 attempts spent building before catching it, but worth
recording): wrote this as two independent `if (sel == 5) { ...; return; }
if (sel == 6) { ... }` blocks instead of a `switch`.** That compiled to a
*shorter* function — GCC emitted a `bne`-skip for the first `if` and then a
completely separate fresh `li`+`bne` re-comparison for the second, instead
of retail's shared `beq a2,5,case5 / li v0,6 / beq a2,v0,case6 / else
return` three-way dispatch. The missing instructions shifted every later
function in the unit (funcdiff reported ~110KB of address drift outside
this function's own range, and every downstream function came back looking
totally unrelated).

**Rewriting the two `if`s as a literal `switch (sel) { case 5: ... case
6: ... }` fixed it exactly**, reproducing retail's shared-comparison
dispatch shape and case ordering (retail lays out `case 5`'s body before
`case 6`'s body, matching source order, per the project's established
"GCC 2.6.3 lays out case bodies in textual source order" rule).

## The three-conjunct inner guard is load-bearing, not redundant (head follow-up)

`if (self->unk20 != 5 && self->unk20 != 8 && self->unk20 == 0xA)` looks
like two dead conjuncts — if `unk20 == 0xA` it is necessarily `!= 5` and
`!= 8` — and reads as if it should simplify to a sparse `switch` with
explicit no-op cases (`case 5: case 8: break; case 0xA: ...`). The head
measured that hypothesis directly: it built clean but the whole-image
SHA1 went red with a 109627-byte drift (a size change shifting every
later address), and was reverted. **The three sequential compares are
exactly what GCC 2.6.3 emits from the short-circuit `&&` chain as
written**, and the semantically-equivalent sparse-switch form does not
reproduce it. Do not "clean up" this condition — the redundant-looking
`!= 5 && != 8` conjuncts must stay.

## Proposed learning

**A two-armed early-exit dispatch on the SAME scrutinee, where each arm
does unrelated work, is a `switch`, not two independent `if`s — even when
one arm returns and the other doesn't.** Two separate `if (x == A) {
...; return; }` / `if (x == B) { ... }` statements make GCC 2.6.3
re-materialize and re-compare the scrutinee for the second check; a real
`switch` shares one dispatch. The tell in the disassembly: retail compares
the SAME register against both literals back-to-back (`beq`/`li`/`beq`)
with no intervening `lw`/reload of the scrutinee, and lays the case
bodies out in one contiguous block rather than duplicating a skip-branch
pattern per arm.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `class_3bb8c_m`. This
residue's fix unblocked accurate scoring for the rest of the unit's
functions in ROM order after it (`ObjM__OnStageMapNotify` onward), which had all
been reading as near-total mismatches purely from this function's address
drift.

## Naming

**ObjM__OnFadeNotify** -- tier B. Two-case switch on its own `sel` parameter (5 and 6), each calling `self->methods->slot14` then adjusting `dreamSys`/`ObjM::mode`, ending case 6 by forwarding the (possibly just updated) mode to `notifyParents`. Named for the mechanical shape (a selector-driven event handler on two codes); the codes' own meaning is unknown.


## Track 4 (2026-09-26, round 89, echo)

Renamed from `ObjM__HandleEvent5Or6` (rename.py): it occupies +0x0B0, which `ObjM__OnNotify` runs for a sender of class id 0x164, FadeBox (include/FadeBox.h), whose stop notifies its parents with 5 after a fade down and 6 after a fade up. The ObjM added that fade box as a child (`ObjM__StartFadeUp`, `ObjM__EnterStyleSession`: the viewport's getSubHandle). On 5 it drops the child, clears the DreamSys's move override and state 0; on 6 it drops the child, sets the viewport's clear colour from the box's getColor (+0x0E4), maps state 0xA to 4 (stopDrift, move override 0) and notifies the parents with the state. Former views ParamM (sender) and FieldM18 (viewport) replaced by FadeBox and NodeGuardedViewport. Tier A for the mechanics.
