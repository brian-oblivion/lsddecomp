# TimImage__Upload -- MATCHED (65/65 words), round 81

> Renamed from `func_8003B4A8` on 2026-09-25 (tools/rename.py). Address 0x8003b4a8.

Round 81, runner echo. Unit `src/graphics/tim_image.c`. Fresh ground, no prior attempt.
Three builds.

- **Where:** TimImage's table (`gTimImageMethods`) slot +0x078 (`tools/classtable.py D_8006E558`).
- **What:** fetches the draw singleton (`GetDrawSystem()`), and when the
  buffer is loaded has slot +0x09C describe the TIM into `self->tim`
  (+0x02C, a full `GsIMAGE`), then passes `{px, py, pw, ph}` and the pixel
  pointer to the singleton's slot +0x058; when `pmode` bit 3 (TIM has a CLUT)
  is set, does the same with `{cx, cy, cw, ch}` and the CLUT pointer. The
  rectangle the singleton takes is `{s16 x, y; s32 w, h}` (`sh, sh, sw, sw`
  on the stack), so the u16 `pw`/`ph` are zero-extended into it.
- **The one lever:** the first version passed `&self->tim` directly and came
  out one word short: the buffer test loaded into `$v1`, `move $s1,$v0` was
  sunk into the `beqz` delay slot, and `a0`/`a1` setup swapped. Retail
  computes `a1 = self + 0x2C` in the branch delay slot. Hoisting the address
  into a local set BEFORE the buffer test (`tim = &self->tim;`, used only as
  the call argument) is byte-exact. Rewriting the test as an early
  `return` changed nothing; retyping `GetDrawSystem` as `void *` changed
  nothing.
- **Result:** byte-exact, whole-image SHA1 green.
- **Name:** `TimImage__Upload`, tier A (round 81 naming pass, runner bravo).
  See `## Naming` below.

## Source

```c
typedef struct DrawRect {
    s16 x;
    s16 y;
    s32 w;
    s32 h;
} DrawRect;
/* Class6C070 (the draw singleton, draw_system.c): methods at +0; slot +0x058
 * loadImage(self, DrawRect *, u32 *) -- confirmed against LIBGPU.H's
 * LoadImage(RECT *rect, u_long *p), see ## Naming below. */
extern Class6C070 *GetDrawSystem(void);

void TimImage__Upload(TimImage *self) {
    Class6C070 *draw;
    DrawRect rect;
    GsIMAGE *tim;

    draw = GetDrawSystem();
    tim = &self->tim;
    if (self->buffer != NULL) {
        self->methods->getTimInfo(self, tim);
        rect.x = self->tim.px;
        rect.y = self->tim.py;
        rect.w = self->tim.pw;
        rect.h = self->tim.ph;
        draw->methods->loadImage(draw, &rect, self->tim.pixel);
        if ((self->tim.pmode >> 3) & 1) {
            rect.x = self->tim.cx;
            rect.y = self->tim.cy;
            rect.w = self->tim.cw;
            rect.h = self->tim.ch;
            draw->methods->loadImage(draw, &rect, self->tim.clut);
        }
    }
}
```

### Proposed learning

A call argument's address computation (`&self->field`) that retail puts in
the delay slot of an EARLIER branch, ahead of a test the call is guarded by,
is a local pointer assigned before the test. Symptom when it is written
inline at the call: one word short, the guard's load lands in a different
temp register, and a pending call-result copy fills the branch delay slot
instead.

## Naming

- **`TimImage__Upload`** (was `func_8003B4A8`), tier A. Slot +0x078; every
  project-wide caller of `New_TimImage` invokes this slot
  (`handle->methods->slot78(handle)`) immediately after construction, and
  its body describes the loaded TIM (`GsGetTimInfo`) then hands the pixel
  block, and CLUT when present, to the draw singleton's `loadImage` slot --
  uploading the decoded image to VRAM is both the mechanics and the purpose.
- **`draw->methods->loadImage`** (this unit's own local field on its private
  `Class6C070Methods` view; not a rename, `draw_system.c` owns that struct):
  tier A. Signature `void (*)(Class6C070 *self, DrawRect *rect, u32 *data)`
  matches LIBGPU.H's `extern int LoadImage(RECT *rect, u_long *p)` exactly
  in shape.
- **`Class6C070`/`Class6C070Methods`** (was the local placeholder
  `DrawObj`/`DrawObjMethods`), tier B: confirmed as the real class the draw
  singleton (`GetDrawSystem`) returns -- `draw_system.c`'s own header comment
  identifies `sDrawSystem`'s class as `Class6C070` -- replacing the
  placeholder name per the round's instruction to confirm or replace it.
  `draw_system.c` itself is still mostly `INCLUDE_ASM`, so this unit's struct
  stays a partial two-slot local view (pad + `loadImage` + `moveImage`), not
  a full definition.

## Track 4 (2026-09-26, round 87, bravo)

The local view of the DrawSystem singleton quoted above is gone; the unit takes DrawSystem, its method table and GetDrawSystem from `include/draw_system.h` (gDrawSystemMethods unified). Byte-identical.

## Round 95 (alpha, track 6: Sony headers)

`self->tim` is <libgs.h>'s GsIMAGE, whose pixel and clut are `unsigned long *`; DrawSystem's loadImage slot takes `u32 *`, so the two calls cast `(u32 *)`. Byte-identical.
