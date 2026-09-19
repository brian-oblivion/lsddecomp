> Renamed from `func_800575E0` on 2026-09-19 (tools/rename.py). Address 0x800575e0.

# DreamSys__DispatchOffsetSlot0 -- MATCHED (12/12)

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0D4`.

## Signature

```c
void DreamSys__DispatchOffsetSlot0(DreamSys *self, s32 val, void *extra);
```

## Body

```c
void DreamSys__DispatchOffsetSlot0(DreamSys *self, s32 val, void *extra) {
    DreamSys__ApplyOffsetOrFindNearby(self, self->vt->DreamSys__ApplyOffsetSlot0, val, extra);
}
```

Sibling of `DreamSys__DispatchOffsetSlotC4`: same shape, but reads the vtable's `+0x0C8`
slot instead of `+0x0C4` -- which is `DreamSys__ApplyOffsetSlot0`, THIS unit's own
neighbouring function (a self-referential vtable read: the slot's value is
the address of another function this same unit implements).

## Naming

**`DreamSys__DispatchOffsetSlot0` -- tier B.** Sibling of
`DreamSys__DispatchOffsetSlotC4` (see that report): same shape, but reads
the vtable's `+0x0C8` slot -- `DreamSys__ApplyOffsetSlot0`, THIS unit's
own neighbour (a self-referential vtable read). Named consistently with
that function's own `Slot0` naming.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py DreamSys__DispatchOffsetSlot0   # 12/12
```
