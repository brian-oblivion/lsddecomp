# TmdModel__TmdModel -- MATCHED (25/25 words), round 82

> Renamed from `func_8001F2B0` on 2026-09-25 (tools/rename.py). Address 0x8001f2b0.

Round 82, runner charlie (matching slot, second pass on the unit). Unit `src/graphics/TmdModel.c`. Fresh ground, no prior attempt.

- **What:** the constructor of class gTmdModelMethods (its slot +0x008): base ctor through `Get_vtable_BasicClass()->ctor`, install the method table, `object = arg`, `data = (u8 *)arg - 0xC`, then `TmdModel__InitBoundsCount(self)` (sets the flag `gTmdModelBoundsCount = 1`).
- **Result:** byte-exact; 25/25 words, whole-image SHA1 green. Second build.
- **Build 1:** `TmdModel__InitBoundsCount();` with no argument -- one word short (the `addu a0, s0, zero` before the stores was missing, and every later function shifted). Retail loads `$a0 = self` BEFORE the three stores and then stores through `$a0`: the call's argument was computed early and reused as the store base. So the callee takes `self` even though its body never reads it.
- **Change to an existing declaration:** `TmdModel__InitBoundsCount` (matched earlier this round, this unit, no other declarer) is now `void TmdModel__InitBoundsCount(TmdModel *self)` instead of `(void)`; identical bytes.

## Source

```c
void TmdModel__InitBoundsCount(TmdModel *self);

void TmdModel__TmdModel(TmdModel *self, void *arg) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_TmdModel();
    self->object = arg;
    self->data = (ModelData_fa50 *)((u8 *)arg - 0xC);
    TmdModel__InitBoundsCount(self);
}
```
(types as in `New_TmdModel.md`.)

### Proposed learning

Stores after a call that go through `$a0` (freshly `move a0, sN` just before them) rather than through the saved `sN` mean the NEXT call takes that value as its first argument: GCC computed the argument early and used it as the base. A `void f(void)` callee with this pattern really takes `self` (unused). Dropping the argument costs exactly one word.

## Naming

`TmdModel__TmdModel` -- tier A. Convention: `Class__Class` constructor. The
ctor at slot +0x008 of the TmdModel class table (gTmdModelMethods): base ctor
through `Get_vtable_BasicClass`, installs the method table, stores the
object-table entry and the TMD data header, marks the class constructed.

## Track 4 (2026-09-26, round 87, delta)

Parameter retyped `void *arg` -> `TmdObject *object` (include/TmdModel.h),
byte-identical. Callers checked: `LinkResource__BuildModels` (GraphicsResources)
is the only caller of `New_TmdModel`, passing entry `i` of the loaded TMD's
0x1C-byte object table (buffer + 0xC); `New_TmdModel` is the only caller of
the ctor, through slot +0x008. The ctor stores the argument in `object` and
`object - 0xC` in `data` (a TmdFile, the real header when the entry is the
first).

## Track 7 (2026-09-26, round 94, bravo)

`(TmdFile *)((u8 *)object - 0xC)` -> `(TmdFile *)((u8 *)object - offsetof(TmdFile, objects))`,
the project's standing idiom for a byte-offset conversion between two
struct views once the target field is known (`include/common.h`'s
`offsetof`; precedent `src/ObjMStyleActor.c`, `src/TmdRenderer.c`). The `(u8
*)` cast itself stays: `object` and `TmdFile` are unrelated types with no
field expressing the relationship, so byte-granularity pointer arithmetic is
the only C form. Byte-identical, build and check-nonmatching.sh green.
