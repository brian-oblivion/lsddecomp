> Renamed from `BasicClass__func_18358` on 2026-09-17 (tools/rename.py). Address 0x80018358.

# BasicClass__OnNotify

**Unit:** code_8220_b · **Size:** 14 instructions · **Status:** MATCHED (14/14 words)

BasicClass vtable slot `+0x038` (`slot38` in `BasicClassMethods`). Called by
`BasicClass__NotifyParents` (slot `+0x030`, `onFinalize`, still `INCLUDE_ASM`
this round) once per entry in `self->parentRefs`, as
`parent->methods->slot38(parent, childBeingFinalized, 1)`.

## What it does

```c
void BasicClass__OnNotify(BasicClass *self, void *arg1, s32 arg2)
{
    if (arg2 == 1) {
        self->methods->removeChild(self, (BasicClass *)arg1);
    }
}
```

Reading it against its only caller: when a child object finalizes, it walks
its own `parentRefs` list and calls each parent's `slot38(parent, self, 1)`.
This function's job, from the parent's side, is to remove that now-dying
child from `self`'s own `children` list — a flag-gated notification hook
(`arg2 != 1` is a no-op; no other value is exercised by anything in this
unit, but the check exists in the retail binary so it's kept literally
rather than assumed always-true).

`arg1` is `void *` at the vtable-slot level (see `code_8220.h`'s
`BasicClassMethods::slot38`) and gets cast to `BasicClass *` only at the
`removeChild` call site — the caller and this callee agree on what's really
being passed, but the slot's own declared type stays generic.

## Provenance

round 12 (2026-09-03), runner charlie, unit code_8220_b (fresh carve).
Matched first attempt — the `if (arg2 == 1)` shape and the always-allocated
stack frame (needed because the true branch makes an indirect call) came
straight off the disassembly.
