# New_TimImage -- MATCHED (24/24 words), round 81

> Renamed from `func_8003B39C` on 2026-09-26 (tools/rename.py). Address 0x8003b39c.

Round 81, runner echo. Unit `src/graphics/TimImage.c`. Fresh ground, no prior attempt.

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

Declarations (top of `src/graphics/TimImage.c`): `TimImageMethods`, a local table
struct expanding `FILERESOURCE_SLOTS(TimImage, (TimImage *self, char *name))`
plus slots +0x07C..+0x09C; `extern void *BMemPMgrAlloc(s32 size);`;
`TimImageMethods *GetTimImageMethods(void);`.

## Naming

- **`New_TimImage`**, tier A (proposed round 81, applied 2026-09-26). Evidence: matches the project's
  `New_Class` constructor-helper convention (`New_DayTask`,
  `New_BoxFill`, ...: `alloc(size); if (self) { ctor(); return self; }
  return NULL;`); every project-wide call site (`DayTaskStageMap.c`,
  `TitleMenuTaskObjF.c`, `class_3bb8c_g.c`, `TextEntryItemList.c`, `class_3bb8c_j.c`,
  `Task.c`) builds a `"...\ .TIM"` path (via `BuildFileName` or
  literal `sEtcTimPath`/`sSaveIconTimPath`/`sDreamerTmdPath`/`sCardPathSuffix`) and hands
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

## History: the unit banner before track 7 (moved from src/code_2bb9c.c, round 100)

The banner of `src/code_2bb9c.c` as it stood before the track-7 pass; the new banner keeps only what the file holds. The carve and naming history it carried lives here.

```c
/*
 * code_2bb9c -- GAME code carved from psyq_2bb9c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2BB9C..0x2BF70 (vram 0x8003B39C..0x8003B770). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint).
 *
 * What it holds: TimImage, a FileResource (data-source) subclass -- 12 of its
 * own methods (table `gTimImageMethods`, id 0x103), plus the class's own
 * alloc-then-ctor helper (`New_TimImage`, "new TimImage(name)") and table
 * getter (`GetTimImageMethods`). Every call site project-wide that reaches
 * TimImage does so by building "CARD\\<name>.TIM" or another `.TIM` path and
 * handing it to `New_TimImage`, then calling the returned handle's slot78
 * (`TimImage__Upload`) and usually slot5C (`FileResource__FreeBuffer`) --
 * TimImage is the game's TIM-image loader: `buffer` (inherited from
 * FileResource) holds the raw file, `TimImage__GetTimInfo` describes it with
 * Sony's `GsGetTimInfo`, and `TimImage__Upload` uploads the pixel block and,
 * when present, the CLUT to the draw singleton (DrawSystem, `include/DrawSystem.h`)
 * through its loadImage slot.
 *
 * Also holds `RotateVramRectRight`, not a TimImage method (`classtable.py
 * D_8006E558` lists it nowhere): a free function that circularly scrolls a
 * VRAM rectangle right by one column at a time through the draw singleton's
 * `moveImage` slot, called from `class_3bb8c_n.c`'s `StyleScrollVramStrips`.
 *
 * Fully matched in round 81 (runner echo). Naming pass round 81 (runner
 * bravo): every function and the class table named; see each function's
 * report `## Naming` for tier and evidence. `RotateVramRectRight` kept its
 * `func_` name (proposed `ScrollImageRight`, recorded in its report);
 * `New_TimImage` was renamed in track 4 (round 88).
 */
```

## History (moved from src/TimImage.c, comments pass)

The file's banner carried its edge evidence and the reason it is parked:

> Edges: both are placed Sony objects (libcd/event before, libgs/gs_122 after),
> so the file is this whole gap. Content alone would put RotateVramRectRight in
> a file of its own; no tool splits a unit and the binary is silent on it
> (tuboundary.py: "boundary possible" at every gap), so it stays here, parked.
