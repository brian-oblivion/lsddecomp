> Renamed from `func_80057618` on 2026-09-19 (tools/rename.py). Address 0x80057618.

# DreamSys__ApplyOffsetOrFindNearby -- MATCHED (20/20)

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`. Plain internal helper --
not a vtable slot itself (checked all 6 method tables reachable from this
unit's addresses with `tools/classtable.py`, no hit).

## Signature

```c
void DreamSys__ApplyOffsetOrFindNearby(DreamSys *self, void (*callback)(DreamSys *, s32, void *), s32 val, void *extra);
```

Called only by this unit's own `DreamSys__DispatchOffsetSlotC4`/`DreamSys__DispatchOffsetSlot0`, which each
forward a raw vtable-slot value as `callback`.

## Body

```c
void DreamSys__ApplyOffsetOrFindNearby(DreamSys *self, void (*callback)(DreamSys *, s32, void *), s32 val, void *extra) {
    self->unk_0x28 = NULL;
    callback(self, val, extra);
    if (self->unk_0x28 == NULL) {
        DreamSys__FindNearbyLink(self);
    }
}
```

Zeroes `self->unk_0x28` (already typed `DreamSysUnk28Target *` in
`include/DreamSys.h`), invokes the passed-in callback with `(self, val,
extra)`, then -- only if the callback did NOT set `unk_0x28` back to
non-NULL -- calls `DreamSys__FindNearbyLink` (this unit's own, still queued at the
time this was written; see its own report).

The `self->unk_0x28 = NULL` store is scheduled by retail into the `jalr`'s
delay slot (executes unconditionally, right as the call is issued) --
reproduced by plain statement order, no barrier needed.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py DreamSys__ApplyOffsetOrFindNearby   # 20/20
```
