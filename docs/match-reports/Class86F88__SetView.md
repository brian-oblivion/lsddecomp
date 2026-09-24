# Class86F88__SetView -- MATCH

> Renamed from `func_800529FC` on 2026-09-24 (tools/rename.py). Address 0x800529fc.

Unit `class_3bb8c_k`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86F88__SetView`: 23/23 words match.

Called (not through a vtable) by `Class86F88__RefreshRows` (this unit, still
`INCLUDE_ASM`) as `Class86F88__SetView(self, arg1, arg2, arg3, arg4)`.

## Source

```c
void Class86F88__SetView(Class86F88 *self, s32 a1, s32 a2, s32 a3, s32 a4)
{
    Class86F88Elem *elem;
    s32 flag = a4;

    __asm__("");
    self->unk20 = a1;
    self->unk24 = a2;
    self->unk28 = a3;
    if (flag == 0) {
        return;
    }
    a3 -= a1;
    elem = self->unk40[a3];
    elem->methods->slotB8(elem, &D_8008AB10);
}
```

## Notes

`a4` is a 5th (stack-passed) parameter -- `Class86F88Methods` slot
signatures elsewhere in this unit put the stack argument last, but here
it's an ordinary call, not a vtable dispatch, so the stack argument comes
from `INCLUDE_ASM("Class86F88__RefreshRows")`'s own call setup, not a declared
struct field. Stores `self->unk20`/`unk24`/`unk28` from `a1`/`a2`/`a3`
UNCONDITIONALLY (all three sit before the `a4 == 0` guard in retail's own
instruction order -- the `unk28` store is in the guarding branch's delay
slot, so it always executes even on the early-return path), then, only if
`a4 != 0`, indexes `self->unk40[a3 - a1]` and dispatches that element's
own `slotB8(elem, &D_8008AB10)`.

**Two independent residues, both register/scheduling, no register or CFG
value was ever wrong:**

1. **The stack argument (`a4`) was read one instruction too LATE.** Retail
   loads it (`lw $v0, 0x28($sp)`) as literally the first thing after the
   prologue, before any of the `unk20`/`unk24`/`unk28` stores; the natural
   C (`if (a4 == 0) return;` written where the guard logically belongs, at
   the point of use) let GCC schedule that load right next to its first
   use instead, one word later than retail and shifting everything after
   it. A bare `__asm__("");` as the function's FIRST statement (this
   project's established, permitted lever for "same instructions, only
   the ORDER differs") pinned the load back to the top -- confirmed
   legitimate by CLAUDE.md rule 6's test: removing the barrier only moves
   WHERE the load happens, never which register holds the value.
2. **A genuine register-identity residue in `self->unk40[a3 - a1]`.**
   `elem = self->unk40[a3 - a1];` (and an equivalent named-local
   `idx = a3 - a1;` variant) computed the difference into `$v0`, where
   retail reuses the parameter register `$a3` itself (the value stored to
   `self->unk28` had already been written to memory, so the register was
   free to repurpose). `permuter.py` ran ~6500 iterations over 3+ minutes
   without finding a zero for this residue by search. It closed by
   directly mutating the parameter in place -- `a3 -= a1; elem =
   self->unk40[a3];` -- which is ordinary, legal C (parameters are
   ordinary lvalues) and let GCC keep computing the difference into the
   SAME register that already held `a3`, rather than allocating a fresh
   temporary for an expression it treated as read-only.

### Proposed learning

**When a call argument's post-use value is never needed again, reassigning
it in place (`arg -= x;` instead of `s32 tmp = arg - x;`) is a legitimate,
non-UB lever for a `$vN`-vs-parameter-register identity residue** -- it is
the mutable-lvalue analogue of the "give a value its own named local"
family already documented, but pointing the OTHER direction: instead of
introducing a fresh local to stop a fold, reuse an EXISTING one (a
parameter) to stop GCC from allocating a new temporary for an expression
whose result it otherwise treats as freshly computed. Try it before
escalating a small (1-2 word) register swap around a parameter to a
stall, especially after a permuter search comes back empty.
