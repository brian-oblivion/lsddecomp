# TimImage__TimImage -- MATCHED (29/29 words), round 81

> Renamed from `func_8003B3FC` on 2026-09-25 (tools/rename.py). Address 0x8003b3fc.

Round 81, runner echo. Unit `src/code_2bb9c.c`. Fresh ground, no prior attempt.

- **Where:** gTimImageMethods slot +0x008 (the ctor; `tools/classtable.py gTimImageMethods`).
- **What:** runs the active data-source driver's ctor on `self`
  (`GetActiveDataSourceMethods()->ctor`), installs this class's table, clears
  +0x048 and +0x04C, and when `name` is non-NULL issues
  `self->methods->requestLoadFile(self, name)` (slot +0x06C, bound at run time
  to the driver's RequestLoadFile).
- **Result:** byte-exact on the first build, whole-image SHA1 green.
- **Name:** kept `func_`.

## Source

```c
void TimImage__TimImage(D_8006E558Obj *self, char *name) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTimImageMethods();
    self->unk48 = 0;
    self->unk4C = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}
```

`extern Class6D430Methods *GetActiveDataSourceMethods(void);` is a unit-local
declaration (the definition in `code_171e0.c` returns `void *`), following
the local-view convention `code_179d8_d.c` / `_e.c` already use.
