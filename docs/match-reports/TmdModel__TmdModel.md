# TmdModel__TmdModel -- MATCHED (25/25 words), round 82

> Renamed from `func_8001F2B0` on 2026-09-25 (tools/rename.py). Address 0x8001f2b0.

Round 82, runner charlie (matching slot, second pass on the unit). Unit `src/code_fa50.c`. Fresh ground, no prior attempt.

- **What:** the constructor of class D_8006BEA0 (its slot +0x008): base ctor through `Get_vtable_BasicClass()->ctor`, install the method table, `object = arg`, `data = (u8 *)arg - 0xC`, then `MarkTmdModelConstructed(self)` (sets the flag `gTmdModelConstructed = 1`).
- **Result:** byte-exact; 25/25 words, whole-image SHA1 green. Second build.
- **Build 1:** `MarkTmdModelConstructed();` with no argument -- one word short (the `addu a0, s0, zero` before the stores was missing, and every later function shifted). Retail loads `$a0 = self` BEFORE the three stores and then stores through `$a0`: the call's argument was computed early and reused as the store base. So the callee takes `self` even though its body never reads it.
- **Change to an existing declaration:** `MarkTmdModelConstructed` (matched earlier this round, this unit, no other declarer) is now `void MarkTmdModelConstructed(TmdModel *self)` instead of `(void)`; identical bytes.

## Source

```c
void MarkTmdModelConstructed(TmdModel *self);

void TmdModel__TmdModel(TmdModel *self, void *arg) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_TmdModel();
    self->object = arg;
    self->data = (ModelData_fa50 *)((u8 *)arg - 0xC);
    MarkTmdModelConstructed(self);
}
```
(types as in `New_TmdModel.md`.)

### Proposed learning

Stores after a call that go through `$a0` (freshly `move a0, sN` just before them) rather than through the saved `sN` mean the NEXT call takes that value as its first argument: GCC computed the argument early and used it as the base. A `void f(void)` callee with this pattern really takes `self` (unused). Dropping the argument costs exactly one word.

## Naming

`TmdModel__TmdModel` -- tier A. Convention: `Class__Class` constructor. The
ctor at slot +0x008 of the TmdModel class table (D_8006BEA0): base ctor
through `Get_vtable_BasicClass`, installs the method table, stores the
object-table entry and the TMD data header, marks the class constructed.
