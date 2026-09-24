# Obj86ED0__AddChild -- MATCHED (33/33 words)

> Renamed from `func_80050D30` on 2026-09-24 (tools/rename.py). Address 0x80050d30.

Unit `class_3bb8c_i`, carved round 14.

`Obj86ED0`'s addChild override (vtable slot 0x010). Dispatches the BASE
class's own `addChild` first, then reads the new child's method-table
`header` word (matches `TaskObjFMethods::header`/`BasicClassMethods::header`
elsewhere in this project -- a per-class type tag, first word of every
BasicClass-family vtable) and stashes the child pointer into one of two
typed slots depending on the tag, mirroring the ALREADY-MATCHED
`TaskObjF__Notify` (`src/class_3bb8c_f.c`) which reads the identical tag the
identical way (`**(s32 **)arg1`, masked `& 0xF`).

```c
void Obj86ED0__AddChild(Obj86ED0 *self, void *arg1)
{
    s32 tag;
    s32 mask;

    if (arg1 != NULL) {
        Get_vtable_BasicClass()->addChild(self, arg1);
        tag = **(s32 **)arg1;
        mask = tag & 0xF;
        if (mask == 2) {
            self->unk34 = arg1;
        } else if (mask == 5) {
            self->unk38 = arg1;
        }
    }
}
```

First attempt, mirroring `TaskObjF__Notify`'s proven idiom, matched
immediately.
