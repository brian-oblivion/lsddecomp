> Renamed from `func_800575B0` on 2026-09-19 (tools/rename.py). Address 0x800575b0.

# DreamSys__DispatchOffsetSlotC4 -- MATCHED (12/12)

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0D0`.

## Signature

```c
void DreamSys__DispatchOffsetSlotC4(DreamSys *self, s32 val, void *extra);
```

## Body

```c
void DreamSys__DispatchOffsetSlotC4(DreamSys *self, s32 val, void *extra) {
    DreamSys__ApplyOffsetOrFindNearby(self, self->vt->BaseObjO__func_5748c, val, extra);
}
```

Reads (does NOT call) the vtable's `+0x0C4` slot -- `BaseObjO__func_5748c`, out
of this unit/runner's range -- as a raw function-pointer VALUE, and
forwards it plus its own two arguments to `DreamSys__ApplyOffsetOrFindNearby` (this unit's own
helper, see its report), which is what actually invokes it.

## Naming

**`DreamSys__DispatchOffsetSlotC4` -- tier B.** Confirmed mechanics: reads
(does not call) the vtable's `+0x0C4` slot (`BaseObjO__func_5748c`, a
sibling of this unit's own offset-slot family defined in a DIFFERENT
unit, `class_3bb8c_o.c`, and out of this runner's scope to rename) as a
raw function pointer, and forwards it to
`DreamSys__ApplyOffsetOrFindNearby`. Named by the SLOT it reads (`C4`,
objective) rather than by inventing a name for `BaseObjO__func_5748c`'s
own purpose, which belongs to another unit.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py DreamSys__DispatchOffsetSlotC4   # 12/12
```
