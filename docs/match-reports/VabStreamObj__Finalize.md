# VabStreamObj__Finalize -- MATCHED 49/49 (round 43)

> Renamed from `VabStreamObj__Close` on 2026-09-26 (tools/rename.py). Address 0x8002c638.

> Renamed from `func_8002C638` on 2026-09-18 (tools/rename.py). Address 0x8002c638.

Unit `PlacementGridVabSound`. Previously filed as a `gp_rel` stall (round 17, never
attempted); reopened round 42 once `--gp-symbols`/`--no-nop-mflo-mfhi`
resolved that blocker project-wide. Confirmed against `gVabStreamObjMethods`'s own
rodata (`asm/data/5E140.data.s`) as that table's own `+0x0C` slot -- this is
the class's "close" method.

## Derivation

```c
s32 VabStreamObj__Finalize(VabStreamObj *self) {
    SsVabClose(self->vabId);
    if (--gOpenVabCount < 0) {
        gOpenVabCount = 0;
    }
    if (gOpenVabCount == 0 && IsWBgmActive() == 0) {
        gVabSizeTableInited = 0;
        gVabVolumeInited = 0;
        gVabStreamInited = 0;
        SsEnd();
        SsQuit();
    }
    BMemPMgrFree(self->vagAttrPool);
    BMemPMgrFree(self->progVagTable);
    BMemPMgrFree(self->baseFilename);
    return func_80026CAC()->slot0C(self);
}
```

(Field names updated to round 52's renames -- `ObjDA34::unk54/unk4C/unk50/
unk5C` are now `vabId`/`vagAttrPool`/`progVagTable`/`baseFilename`; bytes
unchanged.)

Closes the VAB handle, decrements the module-wide open-VAB refcount
(clamped at zero), and if that count hit zero and the streaming-idle check
(`IsWBgmActive`) also says idle, tears down the shared VAB table state. Then
frees the three per-object allocations (`vagAttrPool`'s VagAtr pool,
`progVagTable`'s pointer array, `baseFilename`'s filename copy) and chains
to the base class's own
`+0x0C` slot (`func_80026CAC()->slot0C`), the same base-chain pattern
`PlacementGridVabSound.c` already established for a sibling table. The final call's
return type is a bare tail call with nothing after it -- genuinely
ambiguous between `void` and `s32`, defaulted to `s32` per CLAUDE.md's rule.

## Result

First full-image-correct build (needed `ObjDA34::unk2A`'s `u16` fix from
`VabStreamObj__VabStreamObj`/`VabStreamObj__AdvanceLoadState`'s derivation, since all three share the
struct): byte-exact.

```
VabStreamObj__Finalize: 49/49 words match (file 0x1CE38-0x1CEFC)
```

`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`.

### Proposed learning

See `VabStreamObj__VabStreamObj.md` for the `loadState` signedness lesson
(`u16`, confirmed by `lh` vs `lhu` on the switch-controlling load) -- it was
discovered while matching this function's neighbour but affects the whole
`VabStreamObj` struct, so it applied here too once all three of this
cluster's functions were built together.

## Naming

Renamed `func_8002C638` -> `VabStreamObj__Finalize`, tier A. Confirmed as
`gVabStreamObjMethods`'s own +0x0C slot (the ctor's `+0x08` sibling) and its
body is unambiguously a teardown: closes the VAB handle, decrements/clamps
the shared refcount, frees the object's three allocations, chains to the
base class's own `+0x0C` slot. "Close" (not "Delete"/"Destroy") because it
does not free `self` itself -- that's left to the caller, matching the
project's own use of "Close" elsewhere for a VAB/CD handle teardown that
doesn't own the container.

## Track 4 (2026-09-26, round 87)

Renamed `VabStreamObj__Close` -> `VabStreamObj__Finalize` with `rename.py`.
`classtable.py gVabStreamObjMethods --vs gFileResourceMethods` shows this is the
+0x00C override, FileResource's `finalize` slot (BasicClass's), and the body is
a finalize: it releases what the ctor acquired and chains to the active
driver's finalize, as `FileResource__Finalize` does. "Close" also named a
different slot: FileResource's +0x048 is `close`. Track 4 step 6 names an
override for its slot.

Return type (same day, when the unit's view became `include/VabStreamObj.h`).
The function is now `void`, like the finalize slot it fills, and the chained
`GetActiveDataSourceMethods()->slot0C(self)` is a statement rather than a
`return`. The whole image stays byte-identical. The one caller is
`FileResource__Release`, through the finalize slot, and it does not read the
return value. The unit's `DriverBaseMethods` view still declares that slot
as s32. That is FileResource's view, left as it was.

Track 4, 2026-09-26 (round 88, CdDriver). `DriverBaseMethods` is gone:
`GetActiveDataSourceMethods` returns gCdDriverMethods or gVabDriverMethods,
both FILERESOURCE_SLOTS tables, so PlacementGridVabSound.c declares it
`FileResourceMethods *` like every other caller, and the chained call is
`GetActiveDataSourceMethods()->finalize((FileResource *)self)` (void, as the
slot is). Byte-identical.
