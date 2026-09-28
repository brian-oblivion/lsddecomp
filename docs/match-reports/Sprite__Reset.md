# Sprite__Reset -- MATCHED (24/24 words), round 82

Round 82, runner alpha (fourth slot on Sprite). Unit `src/graphics/sprite.c`. Named this round from its slot by the types runner; first body attempt.

- **Where:** gSpriteMethods slot +0x040 (reset), declared in `include/sprite.h` as `void Sprite__Reset(Sprite *self, void *texture, s32 abr, SpriteRect *rect)`.
- **What:** `image = texture + 0x2C` (the TimImage's GsIMAGE), copies the 12-byte cell into `rect`, calls `InitGsSprite(&self->sprite, abr, rect, self->image)` (image RELOADED from the object, which is what the retail `lw a3,0x48(s0)` is), then zeroes `unk58`.
- **Result:** byte-exact, 24/24 words, 0 ins / 0 del, whole-image SHA1 green. First build, against the header's prototype unchanged.
- **Track 4:** confirms sprite.h's reading of reset's three parameters (texture, abr, rect). The SceneNode slot +0x040 type question in the sprite.h banner is unchanged: this function is only defined here, never called through the slot. No header touched.

## Source

```c
void Sprite__Reset(Sprite *self, void *texture, s32 abr, SpriteRect *rect) {
    self->image = (struct GsIMAGE *)((u8 *)texture + 0x2C);
    self->rect = *rect;
    InitGsSprite(&self->sprite, abr, rect, self->image);
    self->unk58 = 0;
}
```

## Track 4 (2026-09-26, round 88)

TimImage is unified (`include/TimImage.h`, which now holds GsIMAGE).
`src/graphics/sprite.c`'s own copy of `struct GsIMAGE` is deleted and the unit
includes TimImage.h; `self->image = (struct GsIMAGE *)((u8 *)texture +
0x2C)` is now `&((TimImage *)texture)->tim` (`texture` stays `void *`, as
Sprite's slot types it). Same `addiu 0x2C`; image byte-identical.

## Round 95 (alpha, track 6: Sony headers)

TimImage::tim is now <libgs.h>'s anonymous GsIMAGE, while include/sprite.h still types +0x048 `image` as the tag `struct GsIMAGE *` (Sony's typedef has no tag, so it is never completed). The assignment casts `(struct GsIMAGE *)` until sprite.h can say `GsIMAGE *`, which needs all its includers on <libgs.h>; task.c cannot take it yet (its local GsClearOt/DrawSync/... prototypes, the ViewportOt job). Byte-identical.

Round 96 (alpha, track 6). include/sprite.h types +0x048 `image` as Sony's
`GsIMAGE *` and its includers take Sony's headers (`common.h`, <libgte.h>,
<libgpu.h>, <libgs.h>), so the cast is gone: the body is now

```c
void Sprite__Reset(Sprite *self, void *texture, s32 abr, SpriteRect *rect) {
    self->image = &((TimImage *)texture)->tim;
    self->rect = *rect;
    InitGsSprite(&self->sprite, abr, rect, self->image);
    self->unk58 = 0;
}
```

Byte-identical (whole image green).
