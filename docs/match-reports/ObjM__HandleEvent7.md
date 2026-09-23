# ObjM__HandleEvent7

> Renamed from `func_800540E8` on 2026-09-23 (tools/rename.py). Address 0x800540e8.

**Unit:** class_3bb8c_m · **Size:** 14 instructions · **Status:** MATCHED (14/14 words)

## What this function does

`arg1` is never read at all — only `self` and `arg2` matter. If
`arg2 == 7`, dispatch `self->methods->slotB8(self)`; otherwise do nothing.

## The C

```c
void ObjM__HandleEvent7(ObjM *self, s32 arg1, s32 arg2) {
    if (arg2 == 7) {
        self->methods->slotB8(self);
    }
}
```

`$ra` is saved unconditionally (in the branch's own delay slot) even
though the call is conditional — ordinary GCC behaviour whenever a
function contains any call at all, nothing to reproduce deliberately.

## Residue

None — matched on the first attempt.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `class_3bb8c_m`.
