# New_CdStream -- MATCHED (exact length, 31/31 words), round 82

> Renamed from `New_CdStreamObj` on 2026-09-26 (tools/rename.py). Address 0x80046f0c.

> Renamed from `func_80046F0C` on 2026-09-25 (tools/rename.py). Address 0x80046f0c.

Round 82, runner delta. Unit `src/cd/cd_stream.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build of the real body; whole-image SHA1 green.

- **Where:** the class's allocator (`New_` shape).
- **What:** `BMemPMgrAlloc(0x5C)`, and if non-NULL run the ctor (slot +0x08, `CdStream__CdStream`) with three arguments, return the object, else NULL.
- **Levers:** the documented `if (obj != NULL) { ctor; return obj; } return NULL;` shape. The early-return form (`if (obj == NULL) return NULL; ctor; return obj;`) measured 2 words LONG: it adds `j` + `move v0,s0` in the delay slot instead of retail's `move v0,zero` in the `beqz` delay slot.
- **Context:** round 82 extended the unit's local `CdStreamObj` view: `loc` (+0x0C, the seek location passed to slot +0x4C), `s32 muted` (+0x30), `void *cbArg` (+0x44), callbacks `cb48`/`cb4C`/`cb54` (+0x48/+0x4C/+0x54, each called with `cbArg`), object size 0x5C (the allocator's request); method slots +0x040..+0x070 typed; ctor slot takes `(self, s32, s32, s32)`. The active stream object is the sdata global `sActiveCdStream` (`CdStreamObj *`). libcd externs `CdSyncCallback`, `CdControl`, `CdControlF` declared in the unit from the Psy-Q prototypes.

## Naming

Tier A. `New_CdStream` -- the class allocator (`New_Class` convention): `BMemPMgrAlloc(0x5C)` then runs the ctor slot, returning the object or NULL. Evidence: the body itself (allocate, ctor, return-or-NULL), the `New_Pad`/`New_SceneNode` precedent in include/pad.h and include/scene_node.h.

## Source

```c
CdStreamObj *New_CdStream(s32 arg1, s32 arg2, s32 arg3) {
    CdStreamObj *obj = BMemPMgrAlloc(0x5C);

    if (obj != NULL) {
        GetCdStreamMethods()->ctor(obj, arg1, arg2, arg3);
        return obj;
    }
    return NULL;
}
```

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/cd_stream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from New_CdStreamObj (tools/rename.py), the class rename. The size 0x5C here is the class's size. One caller, MoviePlayer__MoviePlayer.

## History (moved from the unit banner of src/code_3770c.c, 2026-09-27)

The unit banner carried this until track 7's polish pass (round 99), which
rewrote the banner to say only what the file holds:

- Carved from `psyq_3770c` on 2026-09-25 (FINISHING-PLAN revision 18): the
  segment was counted as Psy-Q SDK by name, but `tools/gameinsdk.py`
  measured it as game code (a method-table entry beside game methods,
  contiguous with them, no Sony fingerprint).
- Extent 0x3770C..0x38110 (vram 0x80046F0C..0x80047910): all 18 methods of
  gCdStreamMethods plus their helpers, matched and named in round 82, no
  `INCLUDE_ASM` left.
- The class has been declared once, in include/CdStream.h, since track 4
  (round 87).

## History (moved from src/CdStream.c, comments pass)

The file's banner carried its edge evidence:

> the stream down from any state.
>
> The file's edges are Sony objects on both sides (libpress/vlc2 before,
> libcd/c_002 after); tools/tuboundary.py finds no rodata anchor and no
> forced boundary inside, so content decided it: one class, one file.
