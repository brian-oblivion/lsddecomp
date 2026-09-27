# New_TimImage -- MATCHED (24/24 words), round 81

> Renamed from `func_8003B39C` on 2026-09-26 (tools/rename.py). Address 0x8003b39c.

Round 81, runner echo. Unit `src/code_2bb9c.c`. Fresh ground, no prior attempt.

- **What:** `new TimImage(name)`: `BMemPMgrAlloc(0x50)`, and when that is
  non-NULL, calls the class ctor through the table getter
  (`GetTimImageMethods()->ctor(self, name)`, slot +0x008 = TimImage__TimImage) and
  returns the object; otherwise NULL. Pins the object size at 0x50.
- **Lever used:** the round's posted alloc-then-ctor shape
  `if (p != NULL) { ctor; return p; } return NULL;` (alpha, round 81).
- **Result:** byte-exact on the first build, whole-image SHA1 green.
- **Name:** `New_TimImage` (renamed 2026-09-26, track 4); see `## Naming` below.

## Source

```c
TimImage *New_TimImage(char *name) {
    TimImage *self;

    self = BMemPMgrAlloc(0x50);
    if (self != NULL) {
        GetTimImageMethods()->ctor(self, name);
        return self;
    }
    return NULL;
}
```

Declarations (top of `src/code_2bb9c.c`): `TimImageMethods`, a local table
struct expanding `FILERESOURCE_SLOTS(TimImage, (TimImage *self, char *name))`
plus slots +0x07C..+0x09C; `extern void *BMemPMgrAlloc(s32 size);`;
`TimImageMethods *GetTimImageMethods(void);`.

## Naming

- **`New_TimImage`**, tier A (proposed round 81, applied 2026-09-26). Evidence: matches the project's
  `New_Class` constructor-helper convention (`New_DayTask`,
  `New_BoxFill`, ...: `alloc(size); if (self) { ctor(); return self; }
  return NULL;`); every project-wide call site (`class_39e08.c`,
  `class_3bb8c_d.c`, `class_3bb8c_g.c`, `class_3bb8c_i.c`, `class_3bb8c_j.c`,
  `TaskViewport.c`) builds a `"...\ .TIM"` path (via `BuildFileName` or
  literal `sEtcTimPath`/`sSaveIconTimPath`/`sDreamerTmdPath`/`gCardPathSuffix`) and hands
  it straight to this function, then calls the result's slot78
  (`TimImage__Upload`) and usually slot5C (`FileResource__FreeBuffer`) -- the
  exact shape the class's own methods implement. The round-81 rename was blocked by a
  `tools/rename.py` explicit-placeholder bug (a stale track-2
  `// unidentified:` line for `func_8003B39C`), fixed in FINISHING-PLAN
  revision 19.

## Track 4 (2026-09-26, round 88)

Renamed `func_8003B39C` -> `New_TimImage` with `tools/rename.py` during
TimImage's unification (`include/TimImage.h`): the round-81 evidence above
stands (0x50-byte allocation = `sizeof(TimImage)`, then
`GetTimImageMethods()->ctor(self, name)`), and the rename.py bug that
blocked it is fixed. Image byte-identical. The six per-unit local externs of
this function (each with its own return type) are replaced by the one
prototype in `include/TimImage.h`.
