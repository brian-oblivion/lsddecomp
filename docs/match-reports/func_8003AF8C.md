# func_8003AF8C

**Round 81 (delta)** · **Unit:** code_2b78c · **Size:** 38 words · **Status:** MATCHED (38/38 words, whole-image SHA1 green, first build)

## What it does

The D_8006E4F0 constructor (slot `+0x008`). Class6D3C8__Class6D3C8
(code_1677c) calls it through `func_8003B20C()->ctor(self, arg->unk00)`.

1. base ctor through BasicClass's table;
2. installs its own table (func_8003B20C);
3. one-time `CdInit()`, guarded by the sdata flag D_8008A8DC (gp_rel);
4. clears `initialized` (+0x18) and calls `SetActiveDataSource(source)`;
5. calls its own `+0x040` slot with the {320, 240} default (D_8008A8E0).

```c
void func_8003AF8C(Class6E4F0 *self, s32 source) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = func_8003B20C();
    if (D_8008A8DC == 0) {
        CdInit();
        D_8008A8DC = 1;
    }
    self->initialized = 0;
    SetActiveDataSource(source);
    self->methods->setDims(self, &D_8008A8E0, 0);
}
```

Nothing needed tuning: the `sw $zero, 0x18($s0)` lands in
SetActiveDataSource's delay slot and the table store in the `bnez` slot
by the scheduler. `D_8008A8DC` is gp-relative through
`--gp-symbols` (resolved blocker; ordinary extern). `CdInit`/`SsInit`/
`GsInit3D` declared locally from the Psy-Q prototypes.

## Naming

Kept. Suggested `Class6E4F0__Class6E4F0` (the ctor slot).
