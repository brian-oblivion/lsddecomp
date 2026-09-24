# Obj86ED0__RemoveChild -- MATCHED (32/32 words)

> Renamed from `func_80050DB4` on 2026-09-24 (tools/rename.py). Address 0x80050db4.

Unit `class_3bb8c_i`, carved round 14.

`Obj86ED0`'s removeChild override (vtable slot 0x014) -- the mirror image of
`Obj86ED0__AddChild`'s addChild override. Unlike the add side, the tag check runs
FIRST (clearing whichever of `unk34`/`unk38` matches, unconditionally to
`NULL` rather than checking it was actually the same pointer), then the
BASE class's `removeChild` is dispatched last, in the shared fallthrough of
both tag branches.

```c
void Obj86ED0__RemoveChild(Obj86ED0 *self, void *arg1)
{
    s32 tag;
    s32 mask;

    if (arg1 != NULL) {
        tag = **(s32 **)arg1;
        mask = tag & 0xF;
        if (mask == 2) {
            self->unk34 = NULL;
        } else if (mask == 5) {
            self->unk38 = NULL;
        }
        Get_vtable_BasicClass()->removeChild(self, arg1);
    }
}
```

First attempt matched immediately -- the only trick was getting the
tag-check-before-base-call ORDER right (confirmed by reading the raw
disassembly's instruction sequence, not assumed by symmetry with
Obj86ED0__AddChild).
