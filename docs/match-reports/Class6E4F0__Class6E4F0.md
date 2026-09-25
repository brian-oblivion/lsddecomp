# Class6E4F0__Class6E4F0

> Renamed from `func_8003AF8C` on 2026-09-25 (tools/rename.py). Address 0x8003af8c.

**Round 81 (delta)** · **Unit:** code_2b78c · **Size:** 38 words · **Status:** MATCHED (38/38 words, whole-image SHA1 green, first build)

## What it does

The D_8006E4F0 constructor (slot `+0x008`). Class6D3C8__Class6D3C8
(code_1677c) calls it through `GetClass6E4F0Methods()->ctor(self, arg->unk00)`.

1. base ctor through BasicClass's table;
2. installs its own table (GetClass6E4F0Methods);
3. one-time `CdInit()`, guarded by the sdata flag gCdInitDone (gp_rel);
4. clears `initialized` (+0x18) and calls `SetActiveDataSource(source)`;
5. calls its own `+0x040` slot with the {320, 240} default (gDefaultScreenDims).

```c
void Class6E4F0__Class6E4F0(Class6E4F0 *self, s32 source) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetClass6E4F0Methods();
    if (gCdInitDone == 0) {
        CdInit();
        gCdInitDone = 1;
    }
    self->initialized = 0;
    SetActiveDataSource(source);
    self->methods->setDims(self, &gDefaultScreenDims, 0);
}
```

Nothing needed tuning: the `sw $zero, 0x18($s0)` lands in
SetActiveDataSource's delay slot and the table store in the `bnez` slot
by the scheduler. `gCdInitDone` is gp-relative through
`--gp-symbols` (resolved blocker; ordinary extern). `CdInit`/`SsInit`/
`GsInit3D` declared locally from the Psy-Q prototypes.

## Naming

**Round 81 (delta), track 3.** Renamed `func_8003AF8C` -> `Class6E4F0__Class6E4F0`
(constructor convention, `Class__Class`). **Tier A**: it is the +0x008 ctor
slot (`classtable.py 0x8006E4F0 --vs 0x8006B58C`), confirmed by
`Class6D3C8__Class6D3C8` (code_1677c) calling it through
`GetClass6E4F0Methods()->ctor(self, arg->unk00)` as the base-constructor step
before installing its own vtable -- the base-ctor-through-slot+8 shape from
docs/research/class-framework.md. The body is substantive ctor work (base
ctor, install own table, one-time CdInit, clear `initialized`,
SetActiveDataSource, default screen dims), not a guess about purpose.

Globals `gCdInitDone` (tier A: guards the one-time `CdInit()` call, mechanics
is the purpose) and `gDefaultScreenDims` (tier B: a `{0x140, 0xF0}` = {320, 240}
ScreenDims constant, the "default" claim is evident from being the ctor's
own default argument to `setDims`) renamed via `tools/rename.py`: referenced
only from this unit (`grep -rn` over `src/`), so in this unit's ownership
per the field/global rule.

`func_8003B20C` (the table getter) is **NOT renamed this round**: proposed
`GetClass6E4F0Methods`, but `tools/rename.py` cannot apply it -- see that
function's own report for the blocker and the broadcast post.
