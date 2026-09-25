# New_TmdModel -- MATCHED (24/24 words), round 82

> Renamed from `new_class_6bea0` on 2026-09-25 (tools/rename.py). Address 0x8001f250.

Round 82, runner charlie (matching slot, second pass on the unit). Unit `src/code_fa50.c`. Fresh ground, no prior attempt.

- **What:** the allocator of class D_8006BEA0: `p = BMemPMgrAlloc(0x24); if (p != NULL) { func_8001F384()->ctor(p, arg); return p; } return NULL;`. The ctor (slot +0x008, `func_8001F2B0`) takes the allocator's argument as its second parameter.
- **Result:** byte-exact; 24/24 words, whole-image SHA1 green. First build (the broadcast allocator shape).
- **Types:** the unit's local `Class6BEA0` view is now `BASICCLASS_FIELDS(Class6BEA0Methods)` + its own fields, with `Class6BEA0Methods` = `BASICCLASS_SLOTS(Class6BEA0, (Class6BEA0 *self, void *arg))` (unit includes `BasicClass.h`, the UNIFIED header, unchanged). The previous view had `void *vtable; u8 pad4[8];` at the same offsets. `func_8001F384` (matched earlier this round) was retyped in this unit from `void *` to `Class6BEA0Methods *` (same bytes; no other unit declares it).

## Source

```c
#include "BasicClass.h"
typedef struct Class6BEA0 Class6BEA0;
typedef struct Class6BEA0Methods Class6BEA0Methods;
struct Class6BEA0Methods {
    BASICCLASS_SLOTS(Class6BEA0, (Class6BEA0 *self, void *arg));
};
struct Class6BEA0 {
    BASICCLASS_FIELDS(Class6BEA0Methods);
    void *data;             /* +0x00C, ModelData_fa50 * in the unit */
    void *unk10;            /* +0x010 */
    s32 quad[4];            /* +0x014 */
};
extern void *BMemPMgrAlloc(s32 size);
Class6BEA0Methods *func_8001F384(void);

Class6BEA0 *New_TmdModel(void *arg) {
    Class6BEA0 *p = BMemPMgrAlloc(0x24);

    if (p != NULL) {
        func_8001F384()->ctor(p, arg);
        return p;
    }
    return NULL;
}
```
