> Renamed from `func_80056E1C` on 2026-09-18 (tools/rename.py). Address 0x80056e1c.

# LinkOwnerObj__func_56e1c -- MATCHED (10/10 words)

Unit: `class_3bb8c_o` (round 17). A 4-argument forward to `func_80056D18`.

## Final source

```c
extern void func_80056D18(void *arg0, s32 arg1, s32 arg2, s32 arg3);

void LinkOwnerObj__func_56e1c(void *this) {
    func_80056D18(this, 0, 0, 0);
}
```

## Derivation

Whole body:

```
addiu $a1, $zero, 0
addiu $a2, $zero, 0
jal   func_80056D18
 addiu $a3, $zero, 0
```

Nothing touches `$a0` before the call, so the caller's own first argument
is forwarded unchanged -- `func_80056D18(this, 0, 0, 0)`.
`func_80056D18` is OUTSIDE this unit's carved range (part of the still-
uncarved `class_3bb8c_n` monolithic segment immediately in front of this
slice), so no prototype for it exists anywhere yet; declared locally here
per the established "calling into a function in another/uncarved unit is
fine" convention (`DECOMPILATION_LEARNINGS.md`). `$v0` is never read at
this call site, so `void` is the conservative return type -- subject to the
usual "a discarded return is never evidence of `void`" caveat if a second
caller of `func_80056D18` turns up with a different answer. `this` is left
generic (`void *`) since nothing in this function's own body constrains it
further.

### Proposed learning

None beyond what's already documented -- a plain forwarding wrapper.

## Naming

**`LinkOwnerObj__func_56e1c` -- tier C.** Class is known (`LinkOwnerObj`,
confirmed by its caller's dispatch context in `class_3bb8c_s.c`), but the
function is a pure forward to `func_80056D18(this, 0, 0, 0)`, a function
outside this unit's carved range with no prototype or report anywhere yet.
Three literal zero arguments carry no evidence of what they mean, so
naming this wrapper would just be naming a guess about `func_80056D18`.
Kept the tier-C `Class__func_xxxxx` form per FINISHING-PLAN track 3.
