# func_8005FDFC -- MATCHED (51/51 words)

Unit: `Entity_c`. Runner: bravo.

## Shape

```c
void func_8005FDFC(Entity *this) {
    u8 *arg2;
    s32 roll;

    if (this->unk44 == 0) {
        roll = rand();
        arg2 = D_80089E38;
        if ((roll & 1) != 0) {
            arg2 = SCALE_DOUBLE;
        }
        this->methods->slot48(this, 1, arg2);
        this->unk44 = 0xB;
    }
    Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
    if (this->methods->slot144(this, this->unk94) < 0x7000) {
        this->methods->slotC4(this, 0x100, 0);
    }
}
```

## Notes

- **Attempt 1 (stored `arg2 = D_80089E38;` BEFORE calling `rand()`) scored
  2/51 with an oversized frame** (retail saves only `$s0`/`$ra` in a 0x20
  frame; the first attempt added a spurious `$s1` save). Cause: with the
  default pointer assigned before `rand()`, that pointer local has to
  survive the `jal rand` call, and GCC promoted it to a callee-saved
  register to do so.
- **Fix: capture `rand()`'s result into a plain `s32 roll` FIRST, then set up
  `arg2` afterward.** This matches retail's actual instruction order (`jal
  rand` first, `lui/addiu` for the default address in the delay slot
  *after* the call returns, `andi $v0,$v0,1` testing the raw return value
  directly, THEN the conditional override) and needs no register to survive
  the call at all -- `roll` lives entirely in `$v0`/caller-saved space
  between the call and its one use. Reordering the two statements (call
  first, pointer setup second) fixed the frame and matched first try after
  the fix.
- `Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0)` is the established
  five-argument call shape already used throughout `Entity_b.c`.
- `this->methods->slot144(this, this->unk94)` matches the two-argument
  `slot144` signature already established in `include/Entity.h`
  (`Entity__IsTargetInRange`'s residue).
- Extern added: `SCALE_DOUBLE` (already declared as `u8[]` in `Entity_b.c`;
  this unit needs its own file-scope declaration).
- Clean of both open toolchain blockers.

Matched on the 2nd attempt (2/30).

### Proposed learning

**Statement ORDER around a call decides whether a "default value, then
conditionally overridden" local survives the call in a register at all.**
Assigning the default pointer/value BEFORE the call that also determines the
override condition (e.g. `rand()`) forces that value to live across the
call, and GCC 2.6.3 promotes it to a callee-saved register to do so --
growing the frame relative to retail even though the C is logically
equivalent. Capturing the call's result into a plain scalar FIRST, and only
computing the default/override value AFTER, keeps the value entirely in
caller-saved space and reproduces retail's frame exactly. This is a
companion to the project's established "default value, then conditionally
overwritten" idiom: that idiom's delay-slot-fusion benefit only applies
when nothing between the default assignment and its use can clobber the
value's register -- a call in between is exactly the case where it breaks,
and reordering around the call (not adding a barrier) is the fix.
