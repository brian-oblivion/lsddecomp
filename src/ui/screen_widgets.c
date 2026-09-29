/*
 * screen_widgets.c -- the screen widgets FadeBox, BoxFill and TextRow, and
 * the full-width Shift-JIS string helpers.
 *
 * In ROM order: FadeBox (include/fade_box.h), a BoxFill whose colour steps
 * a tick at a time into or out of a flash or a fade to black; BoxFill
 * (include/box_fill.h), a flat-coloured GsBOXF rectangle; TextRow
 * (include/text_row.h), a row of CharSprite cells showing a string, which
 * derives from CharSprite, not BoxFill. Each class runs from its New_ to
 * its method-table getter. Then DecodeFullWidthSjis, EncodeFullWidthSjis
 * and FormatFullWidthNumber (include/full_width_sjis.h), free functions
 * over plain byte buffers. The three method tables and FadeBox's colour
 * tables end the file.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "frame_clock.h"
#include "task.h"
#include "text_row.h"
#include "bmem_pmgr.h"
#include "full_width_sjis.h"
#include <strings.h>
#include "scene_node.h"

/* FadeBox's colour tables, eight RGB entries each, indexed at a 3-byte
 * stride by a channel mask: sFadeBoxMaskColors holds each mask's own
 * channels at 0xFF (0 and 7 white), sFadeBoxBlackColors is all black.
 * sBoxFillDefaultColor ({128, 128, 128}) is BoxFill__Reset's colour when it
 * is given none. */
#define FADEBOX_COLOR_TABLE_SIZE (8 * 3) /* eight masks, three bytes each */
extern u8 sFadeBoxMaskColors[FADEBOX_COLOR_TABLE_SIZE];
extern u8 sFadeBoxBlackColors[FADEBOX_COLOR_TABLE_SIZE];
extern u8 sBoxFillDefaultColor[3];

FadeBox *New_FadeBox(void *size, s32 channels, s32 pri) {
    FadeBox *self;

    self = BMemPMgrAlloc(sizeof(FadeBox));
    if (self != NULL) {
        GetFadeBoxMethods()->ctor(self, size, channels, pri);
        return self;
    }
    return NULL;
}

void FadeBox__FadeBox(FadeBox *self, void *size, s32 channels, s32 pri) {
    BoxFillMethods *base;
    void *color;

    base = GetBoxFillMethods();
    if (channels != 0) {
        color = &sFadeBoxMaskColors[channels * 3];
    } else {
        color = sFadeBoxBlackColors;
    }
    base->ctor((BoxFill *)self, size, color, pri);
    self->methods = GetFadeBoxMethods();
    ((FadeBoxResetFn)self->methods->reset)(self, channels);
}

void FadeBox__Reset(FadeBox *self, s32 channels) {
    self->defaultChannels = channels;
    self->state = FADEBOX_STATE_IDLE;
    self->step = FADEBOX_DEFAULT_STEP;
    self->channels = 0;
    self->mode = 0;
    self->methods->setDisplay(self, 0);
    self->methods->setSemiTransOn(self, 0);
    self->altMode = 0;
}

void FadeBox__Update(FadeBox *self, void *sender, s32 event) {
    s32 ticks;

    if (event != FRAMECLOCK_EVENT_RUNNING) {
        return;
    }
    ticks = self->ticksLeft;
    self->ticksLeft = ticks - 1;
    if (ticks > 0) {
        if (self->mode == FADEBOX_MODE_HOLD) {
            return;
        }
        if (self->channels & FADEBOX_CHANNEL_R) {
            self->color[0] += (u8)self->step;
        }
        if (self->channels & FADEBOX_CHANNEL_G) {
            self->color[1] += (u8)self->step;
        }
        if (self->channels & FADEBOX_CHANNEL_B) {
            self->color[2] += (u8)self->step;
        }
    } else {
        self->methods->stop(self, sender);
    }
}

void FadeBox__SetStep(FadeBox *self, s32 step) {
    self->step = step;
}

/* MATCHING: both StartFade functions pass all their arguments on to configure;
 * passing only self compiles differently. */
void FadeBox__StartFadeDown(FadeBox *self, BasicClass *source, s32 channels, s32 mode) {
    s32 storedChannels;

    if (self->state != FADEBOX_STATE_IDLE) {
        return;
    }
    storedChannels = self->methods->configure(self, source, channels, mode);
    self->methods->setColor(self, 1, &sFadeBoxMaskColors[storedChannels * 3]);
    self->state = FADEBOX_STATE_FADING_DOWN;
    self->step = -self->step;
}

void FadeBox__StartFadeUp(FadeBox *self, BasicClass *source, s32 channels, s32 mode) {
    if (self->state != FADEBOX_STATE_IDLE) {
        return;
    }
    channels = self->methods->configure(self, source, channels, mode);
    if (self->altMode != 0) {
        self->ticksLeft--;
    } else {
        self->methods->setColor(self, 1, &sFadeBoxBlackColors[channels * 3]);
    }
    self->state = FADEBOX_STATE_FADING_UP;
}

s32 FadeBox__Configure(FadeBox *self, BasicClass *source, s32 channels, s32 mode) {
    FadeBoxMethods *methods;
    s32 rate;
    s32 ticks, cut;

    methods = self->methods;
    if (channels < 0) {
        channels = self->defaultChannels;
    } else {
        self->defaultChannels = channels;
    }
    rate = BOXFILL_SEMITRANS_RATE(GsAONE);
    if (channels != 0) {
        self->channels = channels;
    } else {
        rate = BOXFILL_SEMITRANS_RATE(GsATWO);
        self->channels = FADEBOX_CHANNELS_ALL;
    }
    /* MATCHING: retail stores `channels` twice, above and here; one store is
     * four instructions shorter. */
    self->channels = channels;
    if (channels == 0) {
        self->channels = FADEBOX_CHANNELS_ALL;
    }
    ticks = FADEBOX_RAMP / self->step;
    self->mode = mode;
    self->ticksLeft = ticks;
    if (self->altMode != 0) {
        cut = ticks / self->divisor;
        self->ticksLeft = ticks - cut;
    }
    self->maskPerTick = self->mask / self->ticksLeft;
    methods->addChild(self, source);
    methods->setSemiTransOn(self, 1);
    methods->setSemiTransRate(self, rate);
    methods->setDisplay(self, 1);
    return channels;
}

void FadeBox__Stop(FadeBox *self, BasicClass *source) {
    FadeBoxMethods *methods;
    s32 event;

    methods = self->methods;
    if (self->state == FADEBOX_STATE_IDLE) {
        return;
    }
    if (self->state == FADEBOX_STATE_FADING_DOWN) {
        event = FADEBOX_EVENT_FADE_DOWN_DONE;
        if (self->altMode == 0) {
            methods->setDisplay(self, 0);
            methods->setSemiTransOn(self, 0);
        }
    } else {
        event = FADEBOX_EVENT_FADE_UP_DONE;
        if (self->altMode != 0) {
            if (self->channels == FADEBOX_CHANNELS_ALL) {
                methods->setColor(self, 1, sFadeBoxBlackColors);
            }
            methods->setSemiTransOn(self, 0);
        }
    }
    methods->removeChild(self, source);
    if (self->step < 0) {
        self->step = -self->step;
    }
    self->state = FADEBOX_STATE_IDLE;
    methods->notifyParents(self, event);
}

void *FadeBox__GetColor(FadeBox *self) {
    if (self->channels == FADEBOX_CHANNELS_ALL) {
        return sFadeBoxBlackColors;
    }
    return &sFadeBoxMaskColors[self->channels * 3];
}

/* MATCHING: the position pairs are copied as whole structs; field copies compile
 * differently. */
void FadeBox__PushPosition(FadeBox *self, BoxFillSize *size, BoxFillPos *pos) {
    if (self->parent != 0) {
        self->savedW = self->boxW;
        self->savedH = self->boxH;
        *(BoxFillPos *)&self->savedPosX = *(BoxFillPos *)&self->posX;
        self->boxW = size->w;
        self->boxH = size->h;
        *(BoxFillPos *)&self->posX = *pos;
    }
}

void FadeBox__PopPosition(FadeBox *self) {
    s32 x, y;

    x = self->savedPosX;
    y = self->savedPosY;
    self->posX = x;
    self->posY = y;
    /* MATCHING: an ordering barrier; without it the savedW/savedH reads move above
     * the posX/posY writes. */
    __asm__("");
    self->boxW = self->savedW;
    self->boxH = self->savedH;
}

void FadeBox__SetDivisorMode(FadeBox *self, s32 altMode, s32 divisor) {
    self->altMode = altMode;
    self->divisor = divisor;
}

FadeBoxMethods *GetFadeBoxMethods(void) {
    return &gFadeBoxMethods;
}

BoxFill *New_BoxFill(void *size, void *color, s32 pri) {
    BoxFill *self;

    self = BMemPMgrAlloc(sizeof(BoxFill));
    if (self != NULL) {
        GetBoxFillMethods()->ctor(self, size, color, pri);
        return self;
    }
    return NULL;
}

void BoxFill__BoxFill(BoxFill *self, BoxFillSize *size, void *color, s32 pri) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetBoxFillMethods();
    ((BoxFillResetFn)self->methods->reset)(self, size, color, pri);
}

void BoxFill__Reset(BoxFill *self, BoxFillSize *size, void *color, s32 pri) {
    BoxFillMethods *methods;

    self->pri = pri;
    self->relative = 1;
    self->attachArg = 0;
    self->boxAttribute = 0;
    self->boxX = 0;
    self->boxY = 0;
    self->boxW = size->w;
    self->boxH = size->h;
    methods = self->methods;
    if (color == NULL) {
        color = sBoxFillDefaultColor;
    }
    methods->setColor(self, 1, color);
    self->methods->setMask(self, 13);
}

void BoxFill__AttachToParent(BoxFill *self, SceneNode *parent, BoxFillPos *pos) {
    if (self->parent == NULL) {
        GetSceneNodeMethods()->attachToParent((SceneNode *)self, parent, 0);
        self->methods->setPosition(self, pos);
    }
}

s32 BoxFill__SetDisplay(BoxFill *self, s32 on) {
    return GetSetBitField(&self->boxAttribute, BOXFILL_ATTR_DOFF_SHIFT, 1, on == 0) == 0;
}

s32 BoxFill__SetSemiTrans(BoxFill *self, s32 on) {
    return GetSetBitField(&self->boxAttribute, BOXFILL_ATTR_ALON_SHIFT, 1, on != 0);
}

s32 BoxFill__SetSemiTransRate(BoxFill *self, s32 rate) {
    return GetSetBitField(&self->boxAttribute, BOXFILL_ATTR_RATE_SHIFT, 2, rate);
}

void BoxFill__SetColor(BoxFill *self, s32 overwrite, u8 *rgb) {
    BoxFill__ApplyColor(self, self->color, rgb, overwrite);
}

void BoxFill__ApplyColor(BoxFill *self, u8 *dst, u8 *src, s32 overwrite) {
    if (overwrite) {
        *(ColorRgb *)dst = *(ColorRgb *)src;
    } else {
        dst[0] += src[0];
        dst[1] += src[1];
        dst[2] += src[2];
    }
}

void BoxFill__SetPosition(BoxFill *self, BoxFillPos *pos) {
    if (self->parent != NULL) {
        *(BoxFillPos *)&self->posX = *pos;
    }
}

void BoxFill__SetSize(BoxFill *self, s32 *size) {
    if (self->parent != NULL) {
        self->boxW = ((BoxFillSize *)size)->w;
        self->boxH = ((BoxFillSize *)size)->h;
    }
}

/* attachToParent is called through an unprototyped pointer with attachArg
 * as a fourth argument, which its occupant ignores. */
void BoxFill__AttachAbsolute(BoxFill *self, SceneNode *parent, BoxFillPos *pos, s32 attachArg) {
    void (*fn)();

    fn = (void (*)())self->methods->attachToParent;
    /* MATCHING: without the do/while(0) the entry code comes out in another order. */
    do {
        fn(self, parent, pos, attachArg);
        self->relative = 0;
        self->attachArg = attachArg;
    } while (0);
}

void BoxFill__SetPri(BoxFill *self, s32 pri) {
    self->pri = pri;
}

s32 BoxFill__SetMask(BoxFill *self, s32 bits) {
    return self->mask = (1 << bits) - 1;
}

BoxFillMethods *GetBoxFillMethods(void) {
    return &gBoxFillMethods;
}

TextRow *New_TextRow(void *texture, s32 count, char *text) {
    TextRow *self = BMemPMgrAlloc(sizeof(TextRow));
    if (self != NULL) {
        GetTextRowMethods()->ctor(self, texture, count, text);
        return self;
    }
    return NULL;
}

void TextRow__TextRow(TextRow *self, void *texture, s32 count, char *text) {
    s32 i;
    CharSprite **cursor;

    GetCharSpriteMethods()->ctor((CharSprite *)self, texture, ' ');
    self->methods = GetTextRowMethods();
    self->cellCount = count;
    self->visibleCount = count;
    self->firstVisible = 0;
    self->gapIndex = 0;
    cursor = BMemPMgrAlloc(count * sizeof(CharSprite *));
    if (cursor != NULL) {
        self->cells = cursor;
        i = 0;
        if (i < count) {
            do {
                *cursor = New_CharSprite(texture, ' ');
                i++;
                cursor++;
            } while (i < count);
        }
        ((TextRowResetFn)self->methods->reset)(self, text);
    }
}

void TextRow__Finalize(TextRow *self) {
    ReleaseBasicClassArray((BasicClass **)self->cells, self->cellCount);
    self->cells = BMemPMgrFree(self->cells);
    GetCharSpriteMethods()->finalize((CharSprite *)self);
}

void TextRow__Reset(TextRow *self, char *text) {
    self->methods->setCellPitch(self, TEXTROW_DEFAULT_PITCH);
    self->methods->setText(self, text);
}

void TextRow__AttachToParent(TextRow *self, SceneNode *parent, ScreenSpritePos *pos) {
    ScreenSpritePos buf;
    s32 i, bound;
    CharSprite **elemp;

    if (self->parent != NULL) {
        return;
    }
    GetCharSpriteMethods()->attachToParent((CharSprite *)self, parent, (LongVec3 *)pos);
    buf = *pos;
    elemp = self->cells + self->firstVisible;
    i = self->firstVisible;
    bound = i;
    if (i < bound + self->visibleCount) {
        do {
            if (self->gapIndex != 0 && i == self->gapIndex) {
                buf.x += TEXTROW_GAP_WIDTH;
            }
            (*elemp)->methods->attachToParent(*elemp, (SceneNode *)self, (LongVec3 *)&buf);
            buf.x += self->cellPitch;
            bound = self->firstVisible;
            elemp++;
            i++;
        } while (i < bound + self->visibleCount);
    }
}

void TextRow__DetachFromParent(TextRow *self) {
    CharSprite **elemp;
    s32 i, bound;

    if (self->parent != NULL) {
        if (self->cells != NULL) {
            elemp = self->cells + self->firstVisible;
            i = self->firstVisible;
            bound = i;
            if (i < bound + self->visibleCount) {
                do {
                    (*elemp)->methods->detachFromParent(*elemp);
                    elemp++;
                    bound = self->firstVisible;
                    i++;
                } while (i < bound + self->visibleCount);
            }
        }
        GetCharSpriteMethods()->detachFromParent((CharSprite *)self);
    }
}

s32 TextRow__SetDisplay(TextRow *self, s32 on, s32 result) {
    CharSprite **elemp = self->cells + self->firstVisible;
    s32 i = self->firstVisible;
    s32 bound = i;
    if (i < bound + self->visibleCount) {
        do {
            CharSprite *elem = *elemp;
            s32 r;
            elemp++;
            i++;
            r = elem->methods->setDisplay(elem, on);
            bound = self->firstVisible;
            result = r;
        } while (i < bound + self->visibleCount);
    }
    return result;
}

void TextRow__SetColor(TextRow *self, ColorRgb *rgb) {
    CharSprite **elemp = self->cells + self->firstVisible;
    s32 i = self->firstVisible;
    s32 bound = i;
    if (i < bound + self->visibleCount) {
        do {
            CharSprite *elem = *elemp;
            elemp++;
            elem->methods->setColor(elem, rgb);
            i++;
            bound = self->firstVisible;
        } while (i < bound + self->visibleCount);
    }
}

void TextRow__SetPosition(TextRow *self, ScreenSpritePos *pos) {
    if (self->parent != NULL) {
        ScreenSpritePos buf;
        s32 i;
        s32 bound;
        CharSprite **elemp;

        GetCharSpriteMethods()->setPosition((CharSprite *)self, pos);
        buf = *pos;
        i = 0;
        elemp = self->cells;
        if (i < self->cellCount) {
            do {
                (*elemp)->methods->setPosition(*elemp, &buf);
                buf.x += self->cellPitch;
                bound = self->cellCount;
                elemp++;
                i++;
            } while (i < bound);
        }
    }
}

void TextRow__SetCellAt(TextRow *self, s32 cell, s32 index) {
    CharSprite *elem = self->cells[index];
    elem->methods->setCell(elem, cell & 0xFF);
}

void TextRow__NoOpGetCell(void) {}

void TextRow__SetText(TextRow *self, char *text) {
    CharSprite **elemp = self->cells;
    char *p = text;
    if (p != NULL && *p != 0) {
        do {
            CharSprite *elem = *elemp;
            elem->methods->setCell(elem, *p);
            p++;
            elemp++;
        } while (*p != 0);
    }
}

void TextRow__NoOpSlotD0(void) {}

void TextRow__SetCellPitch(TextRow *self, s32 pitch) {
    self->cellPitch = pitch;
}

TextRowMethods *GetTextRowMethods(void) {
    return &gTextRowMethods;
}

/* The full-width Shift-JIS forms of printable ASCII, as these three convert
 * them: two bytes a character, a lead byte and ASCII + SJIS_TRAIL_OFFSET as
 * the trail, one more from SJIS_TRAIL_GAP up because Shift-JIS never uses
 * 0x7F as a trail byte. */
#define SJIS_LEAD_SYMBOL 0x81  /* the lead for space and the symbols below '0' */
#define SJIS_LEAD_ALNUM 0x82   /* the lead from '0' up: digits and Latin letters */
#define SJIS_TRAIL_OFFSET 0x1F /* ASCII + this is the trail below the gap */
#define SJIS_TRAIL_GAP 0x7F    /* the trail value Shift-JIS skips */
#define SJIS_TRAIL_SPACE 0x40 /* the full-width space 0x8140's trail: ' ' + SJIS_TRAIL_OFFSET + 1 */

/* `d` and `dst` are two cursors over one buffer; `special` holds
 * SJIS_TRAIL_SPACE. */
/* MATCHING: `special` is set before `d` is copied from `dst`, retail's order. */
u8 *DecodeFullWidthSjis(u8 *dst, u8 *src) {
    u8 *d;
    u32 special;
    u32 c;
    u32 v;
    u32 peek;

    if (*src++ != 0) {
        special = SJIS_TRAIL_SPACE;
        d = dst;
        do {
            d++;
            c = *src;
            dst++;
            if (c <= SJIS_TRAIL_GAP && c != special) {
                v = c - SJIS_TRAIL_OFFSET;
            } else {
                v = c - (SJIS_TRAIL_OFFSET + 1);
            }
            src++;
            d[-1] = v;
            peek = *src;
            src++;
        } while (peek != 0);
    }
    *dst = 0;
    return dst;
}

/* MATCHING: the `d = dst; dst++;` cursor pairs and the `trail` copy of `c` give
 * retail's code; `*dst++` and `c` alone compile differently. */
u8 *EncodeFullWidthSjis(u8 *dst, u8 *src) {
    u8 *d;
    u32 c;
    u32 v;
    u32 lead;
    u32 trail;

    if (*src != 0) {
        do {
            d = dst;
            dst++;
            c = *src;
            if (c >= '0') {
                lead = SJIS_LEAD_ALNUM;
            } else {
                lead = SJIS_LEAD_SYMBOL;
            }
            *d = lead;
            d = dst;
            dst++;
            c = *src;
            trail = c;
            if (trail < SJIS_TRAIL_GAP - SJIS_TRAIL_OFFSET && trail != ' ') {
                v = trail + SJIS_TRAIL_OFFSET;
            } else {
                v = trail + (SJIS_TRAIL_OFFSET + 1);
            }
            src++;
            *d = v;
        } while (*src != 0);
    }
    *dst = 0;
    return dst;
}

/* Sony's itoa (libc2): the decimal digits of `n`, in the library's own buffer. */
extern char *itoa(int n);

/* MATCHING: the declaration order text/fill/padded and `fill` set in two
 * statements give retail's code. */
void FormatFullWidthNumber(u8 *dst, s32 value, s32 width, s32 unpadded) {
    char text[width + 1];
    s32 fill;
    char padded[width + 1];

    fill = strlen(strcpy(text, itoa(value)));
    fill = width - fill;
    if (unpadded == 0) {
        memset((unsigned char *)padded, '0', width);
        strcpy(&padded[fill], text);
    }
    EncodeFullWidthSjis(dst, (u8 *)(unpadded != 0 ? text : padded));
}

/* The three widgets' method tables and FadeBox's colour tables, in the
 * order the image keeps them. A (void *) entry is a method whose declared
 * type differs from its slot's, usually one inherited from a parent class
 * and declared on the parent's type. */

/* FadeBox (include/fade_box.h): BoxFill's table with reset and update,
 * then the fade's own slots. */
FadeBoxMethods gFadeBoxMethods = {
    FADEBOX_CLASS_ID,
    (void *)BasicClass__Release,
    (void *)FadeBox__FadeBox,
    (void *)SceneNode__Finalize,
    (void *)SceneNode__AddChild,
    (void *)SceneNode__RemoveChild,
    (void *)SceneNode__RemoveAllChildren,
    (void *)BasicClass__GetNextChild,
    (void *)BasicClass__AddParentRef,
    (void *)BasicClass__RemoveParentRef,
    (void *)BasicClass__ClearParentRefs,
    (void *)BasicClass__GetNextParentRef,
    (void *)BasicClass__NotifyParents,
    BasicClass__NoOpSlot34,
    (void *)SceneNode__OnNotify,
    NULL,
    (void *)FadeBox__Reset,
    (void *)SceneNode__UpdateRotation,
    (void *)SceneNode__UpdateScale,
    (void *)BoxFill__AttachToParent,
    (void *)SceneNode__DetachFromParent,
    (void *)SceneNode__DetachAttachedChildren,
    (void *)SceneNode__GetNextAttachedChild,
    (void *)SceneNode__NoOpFinalizeHook,
    (void *)BoxFill__SetDisplay,
    (void *)BoxFill__SetSemiTrans,
    (void *)BoxFill__SetSemiTransRate,
    (void *)SceneNode__SetLighting,
    (void *)SceneNode__SetLightMode,
    (void *)SceneNode__SetLightDim,
    (void *)SceneNode__SetUseZ,
    (void *)SceneNode__SetSubdivision,
    (void *)SceneNode__SetBackClip,
    (void *)SceneNode__GetRotMatrix,
    (void *)SceneNode__NotifyWithHull,
    (void *)SceneNode__GetModelHull,
    (void *)SceneNode__TransformAndNotifyParents,
    (void *)SceneNode__OnPadEvent,
    FadeBox__Update,
    (void *)SceneNode__DispatchLinkCommand,
    (void *)SceneNode__TryAttachNearby,
    (void *)SceneNode__ComposeAndApplyRotation,
    (void *)SceneNode__CheckBoundsOverlap,
    (void *)SceneNode__RaycastHullAgainstFaces,
    SceneNode__NoOpSlotB0,
    (void *)SceneNode__AddToActorParents,
    (void *)BoxFill__SetColor,
    (void *)BoxFill__SetPosition,
    (void *)BoxFill__SetSize,
    (void *)BoxFill__AttachAbsolute,
    (void *)BoxFill__SetPri,
    (void *)BoxFill__SetMask,
    FadeBox__SetStep,
    FadeBox__StartFadeDown,
    FadeBox__StartFadeUp,
    FadeBox__Configure,
    FadeBox__Stop,
    FadeBox__GetColor,
    FadeBox__PushPosition,
    FadeBox__PopPosition,
    FadeBox__SetDivisorMode,
};

/* clang-format off */
u8 sFadeBoxMaskColors[FADEBOX_COLOR_TABLE_SIZE] = {
    /* mask  r     g     b */
    /* 0 */ 0xFF, 0xFF, 0xFF,
    /* 1 */ 0x00, 0x00, 0xFF,
    /* 2 */ 0x00, 0xFF, 0x00,
    /* 3 */ 0x00, 0xFF, 0xFF,
    /* 4 */ 0xFF, 0x00, 0x00,
    /* 5 */ 0xFF, 0x00, 0xFF,
    /* 6 */ 0xFF, 0xFF, 0x00,
    /* 7 */ 0xFF, 0xFF, 0xFF,
};

u8 sFadeBoxBlackColors[FADEBOX_COLOR_TABLE_SIZE] = {
    /* mask  r     g     b */
    /* 0 */ 0x00, 0x00, 0x00,
    /* 1 */ 0x00, 0x00, 0x00,
    /* 2 */ 0x00, 0x00, 0x00,
    /* 3 */ 0x00, 0x00, 0x00,
    /* 4 */ 0x00, 0x00, 0x00,
    /* 5 */ 0x00, 0x00, 0x00,
    /* 6 */ 0x00, 0x00, 0x00,
    /* 7 */ 0x00, 0x00, 0x00,
};
/* clang-format on */

/* BoxFill (include/box_fill.h): SceneNode's table with reset, the
 * attach, display and semi-transparency overrides, then its colour, position,
 * size, priority and mask setters. */
BoxFillMethods gBoxFillMethods = {
    BOXFILL_CLASS_ID,
    (void *)BasicClass__Release,
    (void *)BoxFill__BoxFill,
    (void *)SceneNode__Finalize,
    (void *)SceneNode__AddChild,
    (void *)SceneNode__RemoveChild,
    (void *)SceneNode__RemoveAllChildren,
    (void *)BasicClass__GetNextChild,
    (void *)BasicClass__AddParentRef,
    (void *)BasicClass__RemoveParentRef,
    (void *)BasicClass__ClearParentRefs,
    (void *)BasicClass__GetNextParentRef,
    (void *)BasicClass__NotifyParents,
    BasicClass__NoOpSlot34,
    (void *)SceneNode__OnNotify,
    NULL,
    (void *)BoxFill__Reset,
    (void *)SceneNode__UpdateRotation,
    (void *)SceneNode__UpdateScale,
    (void *)BoxFill__AttachToParent,
    (void *)SceneNode__DetachFromParent,
    (void *)SceneNode__DetachAttachedChildren,
    (void *)SceneNode__GetNextAttachedChild,
    (void *)SceneNode__NoOpFinalizeHook,
    BoxFill__SetDisplay,
    (void *)BoxFill__SetSemiTrans,
    (void *)BoxFill__SetSemiTransRate,
    (void *)SceneNode__SetLighting,
    (void *)SceneNode__SetLightMode,
    (void *)SceneNode__SetLightDim,
    (void *)SceneNode__SetUseZ,
    (void *)SceneNode__SetSubdivision,
    (void *)SceneNode__SetBackClip,
    (void *)SceneNode__GetRotMatrix,
    (void *)SceneNode__NotifyWithHull,
    (void *)SceneNode__GetModelHull,
    (void *)SceneNode__TransformAndNotifyParents,
    (void *)SceneNode__OnPadEvent,
    (void *)SceneNode__Update,
    (void *)SceneNode__DispatchLinkCommand,
    (void *)SceneNode__TryAttachNearby,
    (void *)SceneNode__ComposeAndApplyRotation,
    (void *)SceneNode__CheckBoundsOverlap,
    (void *)SceneNode__RaycastHullAgainstFaces,
    SceneNode__NoOpSlotB0,
    (void *)SceneNode__AddToActorParents,
    (void *)BoxFill__SetColor,
    BoxFill__SetPosition,
    BoxFill__SetSize,
    BoxFill__AttachAbsolute,
    BoxFill__SetPri,
    BoxFill__SetMask,
};

/* TextRow (include/text_row.h): CharSprite's table with the text row's
 * reset, attach, display, colour, position and cell overrides, then setText
 * and setCellPitch. */
TextRowMethods gTextRowMethods = {
    TEXTROW_CLASS_ID,
    (void *)BasicClass__Release,
    (void *)TextRow__TextRow,
    TextRow__Finalize,
    (void *)SceneNode__AddChild,
    (void *)SceneNode__RemoveChild,
    (void *)SceneNode__RemoveAllChildren,
    (void *)BasicClass__GetNextChild,
    (void *)BasicClass__AddParentRef,
    (void *)BasicClass__RemoveParentRef,
    (void *)BasicClass__ClearParentRefs,
    (void *)BasicClass__GetNextParentRef,
    (void *)BasicClass__NotifyParents,
    BasicClass__NoOpSlot34,
    (void *)SceneNode__OnNotify,
    NULL,
    (void *)TextRow__Reset,
    (void *)Sprite__UpdateRotation,
    (void *)SceneNode__UpdateScale,
    (void *)TextRow__AttachToParent,
    (void *)TextRow__DetachFromParent,
    (void *)SceneNode__DetachAttachedChildren,
    (void *)SceneNode__GetNextAttachedChild,
    (void *)SceneNode__NoOpFinalizeHook,
    (void *)TextRow__SetDisplay,
    (void *)Sprite__SetSemiTrans,
    (void *)Sprite__SetSemiTransRate,
    (void *)SceneNode__SetLighting,
    (void *)SceneNode__SetLightMode,
    (void *)SceneNode__SetLightDim,
    (void *)SceneNode__SetUseZ,
    (void *)SceneNode__SetSubdivision,
    (void *)SceneNode__SetBackClip,
    (void *)SceneNode__GetRotMatrix,
    (void *)SceneNode__NotifyWithHull,
    (void *)SceneNode__GetModelHull,
    (void *)SceneNode__TransformAndNotifyParents,
    (void *)SceneNode__OnPadEvent,
    (void *)Sprite__Update,
    (void *)SceneNode__DispatchLinkCommand,
    (void *)SceneNode__TryAttachNearby,
    (void *)SceneNode__ComposeAndApplyRotation,
    (void *)SceneNode__CheckBoundsOverlap,
    (void *)SceneNode__RaycastHullAgainstFaces,
    SceneNode__NoOpSlotB0,
    (void *)SceneNode__AddToActorParents,
    TextRow__SetColor,
    TextRow__SetPosition,
    (void *)ScreenSprite__SetPivotAnchor,
    (void *)TextRow__SetCellAt,
    (void *)TextRow__NoOpGetCell,
    TextRow__SetText,
    TextRow__NoOpSlotD0,
    TextRow__SetCellPitch,
};
