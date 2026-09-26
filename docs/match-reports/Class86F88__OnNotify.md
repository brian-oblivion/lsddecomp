# Class86F88__OnNotify -- MATCHED (44/44 words)

> Renamed from `Class86F88__NotifyChild` on 2026-09-26 (tools/rename.py). Address 0x80051e64.

> Renamed from `Class86F88_3bb8c_j__NotifyChild` on 2026-09-24 (tools/rename.py). Address 0x80051e64.

> Renamed from `func_80051E64` on 2026-09-24 (tools/rename.py). Address 0x80051e64.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86F88_3bb8c_j`.

## Body

```c
void Class86F88__OnNotify(Class86F88_3bb8c_j *self, void *arg1, s32 arg2)
{
    s32 tag;

    Get_vtable_BasicClass()->slot38(self, arg1, arg2);
    tag = **(s32 **)arg1 & 0xF;
    if (tag == 2) {
        self->methods->slot5C(self, arg1, arg2);
    } else if (tag == 5) {
        self->methods->slot58(self, arg1, arg2);
    }
}
```

Calls the inherited `BasicClass::slot38` (already declared in
`include/class_3bb8c.h`'s `BasicMethods866E8F`, established by
class_3bb8c_f) unconditionally first, then dispatches through Class86F88_3bb8c_j's
OWN vtable (`slot5C`/`slot58`) based on the same tag-nibble convention as
`Class86F88__AddChild`/`Class86F88__RemoveChild`. Established `Class86F88Methods_3bb8c_j::slot5C`
(+0x05C, "tag==2") and `slot58` (+0x058, "tag==5") from this function.
Matched first try.

## Naming

- `Class86F88__OnNotify` -- tier B. The slot38 occupant (classtable.py gClass86F88Methods +0x038, the offset every sibling class in this header uses for its own "OnNotify"-shaped override): chains the base slot38 unconditionally, then dispatches to this class's own slot5C/slot58 by the child's tag nibble. Mechanics (notify then tag-dispatch) are clear; the in-game meaning of the notification is not.

## Track 4 (2026-09-26, round 89)

Renamed from `Class86F88__NotifyChild`: it occupies gClass86F88Methods
+0x038, BasicClass's `onNotify` slot (`classtable.py gClass86F88Methods --vs
D_8006B58C`: OVERRIDDEN BasicClass__OnNotify), and it chains
`Get_vtable_BasicClass()->onNotify` first, so the override takes the slot's
name (FINISHING-PLAN track 4 step 6). The tag-2 child's notifications go to
`handleInputCode` (+0x05C), the tag-5 child's to `tickClosing` (+0x058).
