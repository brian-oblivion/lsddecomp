# func_8003B3FC -- MATCHED (29/29 words), round 81

Round 81, runner echo. Unit `src/code_2bb9c.c`. Fresh ground, no prior attempt.

- **Where:** D_8006E558 slot +0x008 (the ctor; `tools/classtable.py D_8006E558`).
- **What:** runs the active data-source driver's ctor on `self`
  (`GetActiveDataSourceMethods()->ctor`), installs this class's table, clears
  +0x048 and +0x04C, and when `name` is non-NULL issues
  `self->methods->requestLoadFile(self, name)` (slot +0x06C, bound at run time
  to the driver's RequestLoadFile).
- **Result:** byte-exact on the first build, whole-image SHA1 green.
- **Name:** kept `func_`.

## Source

```c
void func_8003B3FC(D_8006E558Obj *self, char *name) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = func_8003B614();
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
