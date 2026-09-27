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
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTimImageMethods();
    self->unk48 = 0;
    self->unk4C = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}
```

`extern FileResourceMethods *GetActiveDataSourceMethods(void);` is a unit-local
declaration (the definition in `code_171e0.c` returns `void *`), following
the local-view convention `code_179d8_d.c` / `_e.c` already use.

## Naming

- **Class `TimImage`** (was `D_8006E558Obj`/`D_8006E558`), tier A. Every
  project-wide caller of `New_TimImage` (this class's `New_` helper) builds
  a `"...\ .TIM"` path and passes it in; `TimImage__GetTimInfo` calls Sony's
  `GsGetTimInfo` on the loaded buffer; `TimImage__Upload` reads the result
  and uploads the pixel/CLUT blocks. The class is a TIM-image loader/upload
  handle, not merely "a FileResource subclass".
- **`TimImage__TimImage`**, tier A (constructor: `Class__Class` convention).
  Slot +0x008, dispatched by `New_TimImage`/the table getter as `ctor`.
- **`gTimImageMethods`** (was `D_8006E558`), tier A: `g<Class>Methods`
  convention for the class's static method table, matching
  `gVabDriverMethods`/`gTitleMenuMethods`/etc. project-wide.

## Track 4 (2026-09-26, round 88)

TimImage is unified in `include/TimImage.h`; `src/code_2bb9c.c`'s local
views are gone. The field this ctor clears at +0x04C, `unk4C`, is now
`clutBase`: TimArraySrc__BuildImages (src/GraphicsResources.c) stores
`((info.cy - 0x1E0) >> gTimClutRowShift) * 16 + base` there for each
TimImage it makes, which its own view already called `clutBase`. Image
byte-identical.
