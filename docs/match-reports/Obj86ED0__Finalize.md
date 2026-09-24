# Obj86ED0__Finalize -- MATCHED (18/18 words)

> Renamed from `func_80050CE8` on 2026-09-24 (tools/rename.py). Address 0x80050ce8.

Unit `class_3bb8c_i`, carved round 14.

`Obj86ED0`'s finalize override (vtable slot 0x00C, per
`tools/classtable.py gObj86ED0Methods`): frees the owned name buffer, then
dispatches the BASE class's own finalize (`Get_vtable_BasicClass()->finalize`,
NOT `self->methods->finalize`, since that would just call this same
function again -- confirmed against `Get_vtable_BasicClass`'s local reduced view
`BasicMethods866E8F`, extended this round with the ctor/finalize/addChild/
removeChild/removeAllChildren slots that were previously opaque padding).

```c
void Obj86ED0__Finalize(Obj86ED0 *self)
{
    BMemPMgrFree(self->unk28);
    Get_vtable_BasicClass()->finalize(self);
}
```

First attempt, straight transcription, matched immediately.

### Proposed learning

`BasicMethods866E8F` (`include/class_3bb8c.h`) previously only named its
`slot38` field, leaving `ctor`/`finalize`/`addChild`/`removeChild`/
`removeAllChildren` as opaque `pad000[0x038]`. This round's functions
(`Obj86ED0__Obj86ED0`'s base-ctor call, and this function's base-finalize call,
plus `Obj86ED0__AddChild`/`Obj86ED0__RemoveChild`/`Obj86ED0__RemoveAllChildren`'s explicit
`Get_vtable_BasicClass()->addChild/removeChild/removeAllChildren`) named all five.
Since `Get_vtable_BasicClass` can only have ONE extern declaration per translation
unit (this header has exactly one), any future unit needing a currently-pad
slot of this same getter must EXTEND `BasicMethods866E8F` in place (shrink
the pad, add the field at its real offset) rather than declaring a second,
differently-typed `Get_vtable_BasicClass` -- the latter is a straight redeclaration
conflict the moment both land in one `.c` file via this shared header.

## Naming

- `Obj86ED0__Finalize` -- tier A. gObj86ED0Methods +0x00C (classtable.py), overrides BasicClass's finalize slot: frees unk28 then chains to Get_vtable_BasicClass()->finalize. Standard dtor-shaped override.
