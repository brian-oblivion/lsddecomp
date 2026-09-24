# Obj86ED0__NotifyTarget -- MATCH (16/16 words)

> Renamed from `func_8005161C` on 2026-09-24 (tools/rename.py). Address 0x8005161c.

Unit `class_3bb8c_i`. Obj86ED0's own method: if `self->unk3C` (a
`TargetObj86ED0 *`, a wholly separate class reached only through this one
field) is non-NULL, dispatch its own `slot80` on it.

```c
void Obj86ED0__NotifyTarget(Obj86ED0 *self, s32 arg1)
{
    TargetObj86ED0 *target;

    target = self->unk3C;
    if (target != NULL) {
        target->methods->slot80(target, arg1, 0x60, 0x60);
    }
}
```

## The header fix this function required

The existing `TargetMethods86ED0::slot80` declaration (established from this
exact call site, before this round) had 2 params after `self`
(`arg1, arg2`). The retail disassembly sets `$a2 = 0x60` in the branch's
delay slot (unconditionally) and `$a3 = 0x60` in the `jalr`'s delay slot,
while `$a1` is never touched inside the function body at all.

That is the signature of a THIRD parameter being forwarded unchanged: the
function's own second parameter arrives in `$a1` and is never moved, because
it is already sitting in the exact register the call needs. This project has
the same idiom at several other call sites already (`src/class_3bb8c_k.c`,
`src/code_2cc8c.c`, `src/class_3bb8c_g.c`, `src/code_55dd4.c`:
`obj->methods->slot80(obj, arg1, 0x60, 0x60)` / `(..., 0x7F, 0x7F)` /
`(..., 0x6E, 0x6E)`), so recognizing it here just meant trusting the pattern
instead of the header's (incomplete) prior reading.

Fixed `include/class_3bb8c.h`'s `TargetMethods86ED0::slot80` to
`(TargetObj86ED0 *self, s32 arg1, s32 arg2, s32 arg3)`. `TargetObj86ED0` is
local to this unit's slice (only `Obj86ED0__NotifyTarget` reaches it), so this is
safe to retype outright rather than additively.

### Proposed learning

A vtable slot's arity established from a SINGLE call site with no
corroborating siblings is worth cross-checking against this project's
existing `self->methods->slotN(self, arg1, LITERAL, LITERAL)`
forward-and-repeat idiom before trusting it -- an unused/untouched `$a1`
register in the callee is the tell that the slot takes one more argument
than the call appears to set explicitly.
