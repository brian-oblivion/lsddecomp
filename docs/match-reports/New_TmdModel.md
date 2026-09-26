# New_TmdModel -- MATCHED (24/24 words), round 82

> Renamed from `new_class_6bea0` on 2026-09-25 (tools/rename.py). Address 0x8001f250.

Round 82, runner charlie (matching slot, second pass on the unit). Unit `src/code_fa50.c`. Fresh ground, no prior attempt.

- **What:** the allocator of class gTmdModelMethods: `p = BMemPMgrAlloc(0x24); if (p != NULL) { Get_vtable_TmdModel()->ctor(p, arg); return p; } return NULL;`. The ctor (slot +0x008, `TmdModel__TmdModel`) takes the allocator's argument as its second parameter.
- **Result:** byte-exact; 24/24 words, whole-image SHA1 green. First build (the broadcast allocator shape).
- **Types:** the unit's local `TmdModel` view is now `BASICCLASS_FIELDS(TmdModelMethods)` + its own fields, with `TmdModelMethods` = `BASICCLASS_SLOTS(TmdModel, (TmdModel *self, void *arg))` (unit includes `BasicClass.h`, the UNIFIED header, unchanged). The previous view had `void *vtable; u8 pad4[8];` at the same offsets. `Get_vtable_TmdModel` (matched earlier this round) was retyped in this unit from `void *` to `TmdModelMethods *` (same bytes; no other unit declares it).

## Source

```c
#include "BasicClass.h"
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
TmdModelMethods *Get_vtable_TmdModel(void);

TmdModel *New_TmdModel(void *arg) {
    TmdModel *p = BMemPMgrAlloc(0x24);

    if (p != NULL) {
        Get_vtable_TmdModel()->ctor(p, arg);
        return p;
    }
    return NULL;
}
```

## Naming

`New_TmdModel` -- tier A. Convention: `New_Class` allocator. Evidence: the
standard `BMemPMgrAlloc` + ctor-through-vtable allocator shape, chained to
`Get_vtable_TmdModel()->ctor`; the class it allocates is named from its
table's role (see `Get_vtable_TmdModel.md`).

## Naming (field)

`TmdModel::object` (was `unk10`) -- tier A. The pointer `New_TmdModel`'s own
argument becomes: `self->object = arg;` in the ctor, and every reader
(`TmdModel__ComputeBounds`, `TmdModel__NextPrimitive`) treats it as the TMD
object-table entry (`TmdObject_fa50 *`) the class wraps. Unit-local field;
renamed in the struct definition only, compiler-verified accessor list
(5 sites, all in this unit), build and check-nonmatching.sh green.

## Track 4 (2026-09-26, round 87, delta)

Parameter retyped `void *arg` -> `TmdObject *object` (include/TmdModel.h),
byte-identical. Callers checked: `LinkResource__BuildModels` (code_33808)
is the only caller of `New_TmdModel`, passing entry `i` of the loaded TMD's
0x1C-byte object table (buffer + 0xC); `New_TmdModel` is the only caller of
the ctor, through slot +0x008. The ctor stores the argument in `object` and
`object - 0xC` in `data` (a TmdFile, the real header when the entry is the
first).
