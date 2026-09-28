# RotateVramRectRight -- MATCHED (83/83 words), round 81

> Renamed from `func_8003B624` on 2026-09-26 (tools/rename.py). Address 0x8003b624.

Round 81, runner echo. Unit `src/TimImage.c`. Fresh ground, no prior attempt.
Byte-exact on the first build.

- **Where:** after the table getter GetTimImageMethods; it is not a slot of
  TimImage's table (`tools/classtable.py D_8006E558` lists it nowhere).
- **What:** takes a `DrawRect *r` (`{s16 x, y; s32 w, h}`), a count and an
  s16 point `p`. It loads the draw singleton's slot +0x064 ONCE into a local
  (`$s4`; a memory load across calls is not loop-invariant, so the local is
  in the source), and when `count != 0` runs `count` iterations of three
  calls `fn(draw, &rect, x, y)`: right edge `{r->x + r->w - 1, r->y, 1, r->h}`
  at `p`; `{r->x, r->y, r->w - 1, r->h}` at `(r->x + 1, r->y)`; and
  `{p->x, p->y, 1, r->h}` at `(r->x, r->y)`. Only `i` changes between
  iterations. The `beqz` then `blez` pair is `if (count != 0)` around the
  `for`.
- **Result:** byte-exact, whole-image SHA1 green.
- **Name:** kept `func_`; see `## Naming` below.

## Source

```c
typedef struct DrawPoint { s16 x; s16 y; } DrawPoint;
/* DrawRect { s16 x; s16 y; s32 w; s32 h; }; Class6C070 (the draw singleton,
 * DrawSystem.c) methods slot +0x064:
 * void (*moveImage)(Class6C070 *self, DrawRect *rect, s32 x, s32 y); --
 * confirmed against LIBGPU.H's MoveImage(RECT *rect, int x, int y), see
 * ## Naming below. */

void RotateVramRectRight(DrawRect *r, s32 count, DrawPoint *p) {
    Class6C070 *draw;
    void (*fn)(Class6C070 *, DrawRect *, s32, s32);
    DrawRect rect;
    s32 i;

    draw = GetDrawSystem();
    fn = draw->methods->moveImage;
    if (count != 0) {
        for (i = 0; i < count; i++) {
            rect.x = r->x + r->w - 1;
            rect.y = r->y;
            rect.w = 1;
            rect.h = r->h;
            fn(draw, &rect, p->x, p->y);
            rect.x = r->x;
            rect.y = r->y;
            rect.w = r->w - 1;
            rect.h = r->h;
            fn(draw, &rect, r->x + 1, r->y);
            rect.x = p->x;
            rect.y = p->y;
            rect.w = 1;
            rect.h = r->h;
            fn(draw, &rect, r->x, r->y);
        }
    }
}
```

## Naming

- **`RotateVramRectRight` -- RENAME BLOCKED: rename.py explicit-placeholder bug.**
  Proposed name `ScrollImageRight`, tier B. Evidence (mechanics, not the
  in-game purpose): the loop body issues three `moveImage` calls per
  iteration that circularly shift a VRAM rectangle's content right by one
  column -- copy `r`'s rightmost column out to `p` (a caller-owned scratch
  slot), shift the rest of `r` (width `r->w - 1`) right by one pixel
  in-place, then copy the saved column back in as the new leftmost column.
  `count` repeats the whole three-step shift, so it scrolls by `count`
  columns. Only caller: `class_3bb8c_n.c`'s `StyleScrollVramStrips`, always with
  `count = 1`; what visual "table" this scrolls (`gStyleStage`-selected) is
  unestablished, per that unit's own header comment, so tier B rather than
  A. Same rename.py-bug block as `func_8003B39C` (now `New_TimImage`) had; that bug is fixed (FINISHING-PLAN revision 19).
- **`draw->methods->moveImage`** (this unit's own local field, not a
  cross-unit rename -- `Class6C070Methods` here is `TimImage.c`'s private
  partial view of the singleton `DrawSystem.c` owns): tier A. The wrapped
  slot's signature, `void (*)(Class6C070 *self, DrawRect *rect, s32 x, s32
  y)`, matches LIBGPU.H's `extern int MoveImage(RECT *rect, int x, int y)`
  exactly in shape (a rect argument plus a destination x/y), one slot below
  `loadImage`'s equally exact match to `extern int LoadImage(RECT *rect,
  u_long *p)`. Proposed for `DrawSystem.c`'s own eventual naming of
  `Class6C070`'s table (not renamed here: that struct is owned by
  `DrawSystem.c`, still mostly `INCLUDE_ASM`, and this unit's copy is an
  independent local view per the project's convention).

## Track 4 (2026-09-26, round 87, bravo)

The local view of the DrawSystem singleton quoted above is gone; the unit takes DrawSystem, its method table and GetDrawSystem from `include/DrawSystem.h` (gDrawSystemMethods unified). Byte-identical.

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
