# New_TmdModel -- MATCHED (24/24 words), round 82

> Renamed from `new_class_6bea0` on 2026-09-25 (tools/rename.py). Address 0x8001f250.

Round 82, runner charlie (matching slot, second pass on the unit). Unit `src/graphics/TmdModel.c`. Fresh ground, no prior attempt.

- **What:** the allocator of class gTmdModelMethods: `p = BMemPMgrAlloc(0x24); if (p != NULL) { GetTmdModelMethods()->ctor(p, arg); return p; } return NULL;`. The ctor (slot +0x008, `TmdModel__TmdModel`) takes the allocator's argument as its second parameter.
- **Result:** byte-exact; 24/24 words, whole-image SHA1 green. First build (the broadcast allocator shape).
- **Types:** the unit's local `TmdModel` view is now `BASICCLASS_FIELDS(TmdModelMethods)` + its own fields, with `TmdModelMethods` = `BASICCLASS_SLOTS(TmdModel, (TmdModel *self, void *arg))` (unit includes `basic_class.h`, the UNIFIED header, unchanged). The previous view had `void *vtable; u8 pad4[8];` at the same offsets. `GetTmdModelMethods` (matched earlier this round) was retyped in this unit from `void *` to `TmdModelMethods *` (same bytes; no other unit declares it).

## Source

```c
#include "basic_class.h"
typedef struct TmdModel TmdModel;
typedef struct TmdModelMethods TmdModelMethods;
struct TmdModelMethods {
    BASICCLASS_SLOTS(TmdModel, (TmdModel *self, void *arg));
};
struct TmdModel {
    BASICCLASS_FIELDS(TmdModelMethods);
    void *data;             /* +0x00C, ModelData_fa50 * in the unit */
    TmdObject_fa50 *object;  /* +0x010 */
    s32 quad[4];            /* +0x014 */
};
extern void *BMemPMgrAlloc(s32 size);
TmdModelMethods *GetTmdModelMethods(void);

TmdModel *New_TmdModel(void *arg) {
    TmdModel *p = BMemPMgrAlloc(0x24);

    if (p != NULL) {
        GetTmdModelMethods()->ctor(p, arg);
        return p;
    }
    return NULL;
}
```

## Naming

`New_TmdModel` -- tier A. Convention: `New_Class` allocator. Evidence: the
standard `BMemPMgrAlloc` + ctor-through-vtable allocator shape, chained to
`GetTmdModelMethods()->ctor`; the class it allocates is named from its
table's role (see `GetTmdModelMethods.md`).

## Naming (field)

`TmdModel::object` (was `unk10`) -- tier A. The pointer `New_TmdModel`'s own
argument becomes: `self->object = arg;` in the ctor, and every reader
(`TmdModel__ComputeBounds`, `TmdModel__NextPrimitive`) treats it as the TMD
object-table entry (`TmdObject_fa50 *`) the class wraps. Unit-local field;
renamed in the struct definition only, compiler-verified accessor list
(5 sites, all in this unit), build and check-nonmatching.sh green.

## Track 4 (2026-09-26, round 87, delta)

Parameter retyped `void *arg` -> `TmdObject *object` (include/TmdModel.h),
byte-identical. Callers checked: `LinkResource__BuildModels` (graphics_resources)
is the only caller of `New_TmdModel`, passing entry `i` of the loaded TMD's
0x1C-byte object table (buffer + 0xC); `New_TmdModel` is the only caller of
the ctor, through slot +0x008. The ctor stores the argument in `object` and
`object - 0xC` in `data` (a TmdFile, the real header when the entry is the
first).

## Track 7 (2026-09-26, round 94, bravo)

`BMemPMgrAlloc(0x24)` -> `BMemPMgrAlloc(sizeof(TmdModel))`, the codebase-wide
idiom once a class's allocator size matches its now-real struct (e.g.
`src/graphics/graphics_resources.c`'s `BMemPMgrAlloc(sizeof(TimBlockSrc))`). `sizeof(TmdModel)`
is 0x24 (`include/TmdModel.h`'s own banner already states the object is
0x24 bytes); byte-identical, build and check-nonmatching.sh green.

## Unit history

`src/code_fa50.c` has no unit report; its first function's report holds the
unit-level notes the source banner used to carry (moved here in round 94's
re-sent track 7 pass, from the banner as it stood at `b7c5a68e^`).

- **Carve.** GAME code carved from `psyq_fa50` on 2026-09-25 (FINISHING-PLAN
  revision 18): file range 0xFA50..0x10D48, vram 0x8001F250..0x80020548. It
  had been counted as Psy-Q SDK by segment name; `tools/gameinsdk.py`
  measured it as game (a call into game code, a method-table entry beside
  game methods, or contiguity with those, and no Sony fingerprint).
- **Matching.** All 18 functions matched in round 82 (runner charlie).
- **Track 4.** The TmdModel class was unified in round 87 (runner delta):
  the unit's local TmdModel view was deleted for `include/TmdModel.h`.
- **Jump table.** `TmdModel__NextPrimitive` owns `jtbl_80010354` (its
  mode `switch`).

## History (moved from src/TmdModel.c, comments pass)

The file's banner carried its edge evidence:

> File edges: this is one whole original file. It lies between two placed
> Sony objects, libgte/ratan before and libgs/gs_105 after, so both edges
> are measured file edges; tools/tuboundary.py finds no rodata crossing and
> no forced boundary inside it (the forced interval that starts at
> TmdModel__NextPrimitive's jtbl_80010354 is closed by the gs_105 edge).
> Content settles the rest: every function is TmdModel's or serves it.
>
> Tiers and match evidence for every function are in each function's own
> docs/match-reports/ file; the unit's own history is in New_TmdModel's.
