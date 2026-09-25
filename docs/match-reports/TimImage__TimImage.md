# TimImage__TimImage -- MATCHED (29/29 words), round 81

> Renamed from `func_8003B3FC` on 2026-09-25 (tools/rename.py). Address 0x8003b3fc.

Round 81, runner echo. Unit `src/code_2bb9c.c`. Fresh ground, no prior attempt.

- **Where:** TimImage's table (`gTimImageMethods`) slot +0x008 (the ctor; `tools/classtable.py D_8006E558`).
- **What:** runs the active data-source driver's ctor on `self`
  (`GetActiveDataSourceMethods()->ctor`), installs this class's table, clears
  +0x048 and +0x04C, and when `name` is non-NULL issues
  `self->methods->requestLoadFile(self, name)` (slot +0x06C, bound at run time
  to the driver's RequestLoadFile).
- **Result:** byte-exact on the first build, whole-image SHA1 green.
- **Name:** kept `func_`.

## Source

```c
void TimImage__TimImage(TimImage *self, char *name) {
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

## Naming

- **Class `TimImage`** (was `D_8006E558Obj`/`D_8006E558`), tier A. Every
  project-wide caller of `func_8003B39C` (this class's `New_` helper) builds
  a `"...\ .TIM"` path and passes it in; `TimImage__GetTimInfo` calls Sony's
  `GsGetTimInfo` on the loaded buffer; `TimImage__Upload` reads the result
  and uploads the pixel/CLUT blocks. The class is a TIM-image loader/upload
  handle, not merely "a Class6D430 subclass".
- **`TimImage__TimImage`**, tier A (constructor: `Class__Class` convention).
  Slot +0x008, dispatched by `func_8003B39C`/the table getter as `ctor`.
- **`gTimImageMethods`** (was `D_8006E558`), tier A: `g<Class>Methods`
  convention for the class's static method table, matching
  `gVabDriverMethods`/`gClass86B60Methods`/etc. project-wide.
