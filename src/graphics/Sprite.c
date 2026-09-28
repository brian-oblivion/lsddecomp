/*
 * The sprite classes, and three small classes beside them.
 *
 * Sprites (include/Sprite.h and its subclasses' headers): Sprite (class id
 * 0x44) is a SceneNode that draws an embedded GsSPRITE; ScreenSprite
 * (0x144) places it in screen space with a position and a pivot anchor;
 * CharSprite (0x1144) is one character of an 8x8 font, its texture cell
 * picked by a character code. Here are their allocators, ctors (each chains
 * to its parent's, installs its table, then calls reset), and the methods
 * each adds or overrides: texture binding (Sprite__Reset, InitGsSprite),
 * rotation, the display and semitransparency attribute bits, colour, screen
 * position and pivot, and the cell. GetCellRect is the free helper that turns
 * a character code into its cell in the font texture. TextRow and
 * VariantSprite are Sprite subclasses too, with their methods elsewhere.
 *
 * RequestedFile (include/RequestedFile.h, 0xB03): a FileResource that asks
 * the active data-source driver for one named file at construction and
 * records when it has arrived.
 *
 * FrameClock (include/FrameClock.h, 0x5): a BasicClass ticked once per
 * DrawSystem frame that counts frames and tells its parents whether it is
 * running, paused or stopped.
 *
 * LightRig (include/LightRig.h, 0x14): a SceneNode that owns three flat
 * lights and the ambient colour. StageMap inherits its getLight unchanged.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "CharSprite.h"
#include "LightRig.h"
#include "flat_light_obj.h"
#include "RequestedFile.h"
#include "TimImage.h"
#include "FrameClock.h"
#include "bmem_pmgr.h"
#include <strings.h>
#include "GameApplicationFileResource.h"
#include "scene_node.h"

/* Defined in other units. */

/* The zero offset ScreenSprite__AttachToParent attaches with. */
extern LongVec3 sVec3Zero;

/* Cell 0 of the font texture, {u 0, v 0, w 8, h 8}: GetCellRect offsets it. */
extern SpriteRect sCharSpriteCellRect;

/* A 0..255 colour channel to GsSetAmbient's 0..ONE scale (255 << 4 is 4080). */
#define AMBIENT_TO_FIX12_SHIFT 4

/* Allocate and construct a CharSprite showing character `cell`. */
CharSprite *New_CharSprite(void *texture, u8 cell) {
    CharSprite *obj = BMemPMgrAlloc(sizeof(CharSprite));

    if (obj != NULL) {
        GetCharSpriteMethods()->ctor(obj, texture, cell);
        return obj;
    }
    return NULL;
}

/* CharSprite slot +0x008 (ctor): the ScreenSprite ctor sized by the space
 * character's cell, install the table, then reset to the caller's cell. */
void CharSprite__CharSprite(CharSprite *self, void *texture, u8 cell) {
    SpriteRect cellRect;

    GetCellRect(&cellRect, ' ');
    GetScreenSpriteMethods()->ctor((ScreenSprite *)self, texture, &cellRect, 0);
    self->methods = GetCharSpriteMethods();
    ((CharSpriteResetFn)self->methods->reset)(self, cell);
}

/* CharSprite slot +0x040 (reset): re-select the cell through slot +0x0C4. */
void CharSprite__Reset(CharSprite *self, u8 cell) {
    self->methods->setCell(self, cell);
}

/* CharSprite slot +0x0C4: store the cell index and point u,v at its 8x8 cell. */
void CharSprite__SetCell(CharSprite *self, u8 cell) {
    SpriteRect cellRect;

    self->cellIndex = cell;
    GetCellRect(&cellRect, cell);
    self->sprite.u = cellRect.u;
    self->sprite.v = cellRect.v;
}

/* CharSprite slot +0x0C8: the character setCell stored. */
u8 CharSprite__GetCell(CharSprite *self) {
    u8 pad[16]; /* MATCHING: unused, but it gives the function its 16-byte frame */

    return self->cellIndex;
}

/* Returns the CharSprite method table. */
CharSpriteMethods *GetCharSpriteMethods(void) {
    return &gCharSpriteMethods;
}

/* The rect of character `cell` in the font texture: sCharSpriteCellRect moved
 * to the cell's column and row. Only the low byte of `cell` counts. */
void GetCellRect(SpriteRect *dst, u32 cell) {
    *dst = sCharSpriteCellRect;
    cell &= 0xFF;
    dst->u += (cell % CHARSPRITE_GRID_COLUMNS) * CHARSPRITE_CELL_SIZE;
    dst->v += (cell / CHARSPRITE_GRID_COLUMNS) * CHARSPRITE_CELL_SIZE;
}

/* Allocate and construct a ScreenSprite on the texture cell `rect`. */
ScreenSprite *New_ScreenSprite(void *texture, SpriteRect *rect, s32 resetWord) {
    ScreenSprite *obj = BMemPMgrAlloc(sizeof(ScreenSprite));

    if (obj != NULL) {
        GetScreenSpriteMethods()->ctor(obj, texture, rect, resetWord);
        return obj;
    }
    return NULL;
}

/* gScreenSpriteMethods slot +0x008 (ctor): the Sprite ctor with abr 0 and resetArg NULL,
 * install the table, then reset. */
void ScreenSprite__ScreenSprite(ScreenSprite *self, void *texture, SpriteRect *rect, s32 resetWord) {
    GetSpriteMethods()->ctor((Sprite *)self, texture, 0, rect, NULL, resetWord);
    self->methods = GetScreenSpriteMethods();
    self->methods->reset(self);
}

/* gScreenSpriteMethods slot +0x040 (reset): empty override. */
void ScreenSprite__Reset(ScreenSprite *self) {}

/* gCharSpriteMethods and gScreenSpriteMethods slot +0x04C (attachToParent): when not yet
 * attached, attach through Sprite's with a zero offset, then hand `pos` to
 * setPosition (+0x0BC). */
void ScreenSprite__AttachToParent(ScreenSprite *self, SceneNode *parent, ScreenSpritePos *pos) {
    if (self->parent == NULL) {
        GetSpriteMethods()->attachToParent((Sprite *)self, parent, &sVec3Zero);
        self->methods->setPosition(self, pos);
    }
}

/* gCharSpriteMethods and gScreenSpriteMethods slot +0x0BC (setPosition): store
 * the screen position, once attached. */
void ScreenSprite__SetPosition(ScreenSprite *self, ScreenSpritePos *pos) {
    if (self->parent != NULL) {
        self->screenPos = *pos;
    }
}

/* gCharSpriteMethods and gScreenSpriteMethods slot +0x0C0: when attached, move the sprite's
 * pivot (mx, my) to the edge or centre `anchor` names (enum ScreenSpriteAnchor). */
void ScreenSprite__SetPivotAnchor(ScreenSprite *self, u32 anchor) {
    if (self->parent != NULL) {
        switch (anchor) {
            case SCREENSPRITE_ANCHOR_CENTRE:
                self->sprite.mx = self->sprite.w >> 1;
                self->sprite.my = self->sprite.h >> 1;
                break;
            case SCREENSPRITE_ANCHOR_LEFT:
                self->sprite.mx = 0;
                break;
            case SCREENSPRITE_ANCHOR_RIGHT:
                self->sprite.mx = self->sprite.w;
                break;
            case SCREENSPRITE_ANCHOR_TOP:
                self->sprite.my = 0;
                break;
            case SCREENSPRITE_ANCHOR_BOTTOM:
                self->sprite.my = self->sprite.h;
                break;
        }
    }
}

/* Returns the gScreenSpriteMethods method table. */
ScreenSpriteMethods *GetScreenSpriteMethods(void) {
    return &gScreenSpriteMethods;
}

/* Allocate and construct a Sprite on the texture cell `rect`. */
Sprite *New_Sprite(void *texture, s32 abr, SpriteRect *rect, void *resetArg, s32 resetWord) {
    Sprite *obj = BMemPMgrAlloc(sizeof(Sprite));

    if (obj != NULL) {
        GetSpriteMethods()->ctor(obj, texture, abr, rect, resetArg, resetWord);
        return obj;
    }
    return NULL;
}

/* Sprite's reset (+0x040) as its ctor calls it: with all five ctor
 * arguments, returning what the ctor returns. The slot is SceneNode's, typed
 * without them (Sprite.h, "Not settled"), and Sprite__Reset reads the first
 * three. Local, where CharSpriteResetFn and VariantSpriteResetFn sit in their
 * headers, because only this ctor calls through it. */
typedef void *(*SpriteResetFn)(Sprite *self, void *texture, s32 abr, SpriteRect *rect,
                               void *resetArg, s32 resetWord);

/* gSpriteMethods slot +0x008 (ctor): the SceneNode ctor, install the table,
 * and hand every argument to reset. */
void *Sprite__Sprite(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *resetArg,
                     s32 resetWord) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetSpriteMethods();
    return ((SpriteResetFn)self->methods->reset)(self, texture, abr, rect, resetArg, resetWord);
}

/* gSpriteMethods slot +0x040 (reset): bind the texture and cell, rebuild the GsSPRITE. */
void Sprite__Reset(Sprite *self, void *texture, s32 abr, SpriteRect *rect) {
    self->image = &((TimImage *)texture)->tim;
    self->rect = *rect;
    InitGsSprite(&self->sprite, abr, rect, self->image);
    self->accumulateScale = 0;
}

/* Fill a GsSPRITE from a texture image and a cell: colour mode and tpage
 * from the image, size and u,v from the cell, the pivot at its centre,
 * neutral colour, scale 1.0 and no rotation. `tim` is a TimImage's GsIMAGE. */
void InitGsSprite(SpriteGs *sprite, s32 abr, SpriteRect *rect, GsIMAGE *tim) {
    s32 mode = tim->pmode & 0x3;
    s32 grey = SPRITE_RGB_NEUTRAL;

    sprite->attribute = mode << SPRITE_ATTR_MODE_SHIFT;
    sprite->x = 0;
    sprite->y = 0;
    sprite->w = rect->w;
    sprite->h = rect->h;
    sprite->mx = sprite->w >> 1;
    sprite->my = sprite->h >> 1;
    sprite->tpage = GetTPage(mode, abr, tim->px, tim->py);
    sprite->u = rect->u;
    sprite->v = rect->v;
    sprite->cx = tim->cx;
    sprite->cy = tim->cy;
    sprite->rgb.b = grey;
    sprite->rgb.g = grey;
    sprite->rgb.r = grey;
    sprite->rotate = 0;
    sprite->scalex = ONE;
    sprite->scaley = ONE;
}

/* gSpriteMethods slot +0x044 (updateRotation): table[2] as a fraction of
 * degrees, in 4096ths; set or add to the GsSPRITE's rotate. */
void Sprite__UpdateRotation(Sprite *self, s32 set, Ratio16 *table) {
    s32 angle;

    angle = ((table[2].num / table[2].den) << FIX12_SHIFT) +
            ((table[2].num % table[2].den) << FIX12_SHIFT) / table[2].den;
    if (set) {
        self->sprite.rotate = angle;
    } else {
        self->sprite.rotate += angle;
    }
}

/* Sprite classes slot +0x060 (setDisplay): display on or off (GsDOFF, inverted);
 * returns whether it was on. */
s32 Sprite__SetDisplay(Sprite *self, s32 on) {
    return GetSetBitField(&self->sprite.attribute, SPRITE_ATTR_DOFF_SHIFT, 1, on == 0) == 0;
}

/* Sprite classes slot +0x064: semitransparency on or off (GsALON); returns the
 * old bit. */
s32 Sprite__SetSemiTrans(Sprite *self, s32 on) {
    return GetSetBitField(&self->sprite.attribute, SPRITE_ATTR_ALON_SHIFT, 1, on != 0);
}

/* Sprite classes slot +0x068: the semitransparency rate (2 bits); returns the
 * old rate. */
s32 Sprite__SetSemiTransRate(Sprite *self, s32 rate) {
    return GetSetBitField(&self->sprite.attribute, SPRITE_ATTR_RATE_SHIFT, 2, rate);
}

/* gTextRowMethods and gCharSpriteMethods slot +0x098 (update): empty override. */
void Sprite__Update(Sprite *self, void *sender, s32 event) {}

/* Slot +0x0B8 of gCharSpriteMethods, gScreenSpriteMethods, gSpriteMethods and gVariantSpriteMethods (the
 * sprite classes): copy three bytes into the embedded GsSPRITE's r,g,b. */
void Sprite__SetColor(Sprite *self, ColorRgb *rgb) {
    self->sprite.rgb = *rgb;
}

/* Returns the gSpriteMethods method table. */
SpriteMethods *GetSpriteMethods(void) {
    return &gSpriteMethods;
}

/* Allocate and construct a RequestedFile, requesting the file `name`. */
RequestedFile *New_RequestedFile(char *name) {
    RequestedFile *obj = BMemPMgrAlloc(sizeof(RequestedFile));

    if (obj != NULL) {
        GetRequestedFileMethods()->ctor(obj, name);
        return obj;
    }
    return NULL;
}

/* gRequestedFileMethods slot +0x008 (ctor): the active driver's ctor, install
 * the table, clear `loaded`, and pass a stack copy of the name to
 * requestLoadFile (+0x06C). */
void RequestedFile__RequestedFile(RequestedFile *self, char *name) {
    char nameCopy[REQUESTEDFILE_NAME_SIZE];

    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetRequestedFileMethods();
    self->loaded = 0;
    if (name != NULL) {
        strcpy(nameCopy, name);
        self->methods->requestLoadFile(self, nameCopy);
    }
}

/* gRequestedFileMethods slot +0x00C (finalize): clear `loaded`, then the active
 * driver's finalize. */
void RequestedFile__Finalize(RequestedFile *self) {
    self->loaded = 0;
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* gRequestedFileMethods slot +0x064 (onRequestDone): the driver reports the requested
 * file loaded. */
void RequestedFile__MarkLoaded(RequestedFile *self) {
    self->loaded = 1;
}

/* Returns the gRequestedFileMethods method table. */
RequestedFileMethods *GetRequestedFileMethods(void) {
    return &gRequestedFileMethods;
}

/* Allocate and construct a FrameClock at frame 0. */
FrameClock *New_FrameClock(void) {
    FrameClock *obj = BMemPMgrAlloc(sizeof(FrameClock));

    if (obj != NULL) {
        GetFrameClockMethods()->ctor(obj);
        return obj;
    }
    return NULL;
}

/* gFrameClockMethods slot +0x008 (ctor): the BasicClass ctor, install the table, reset(0). */
void FrameClock__FrameClock(FrameClock *self) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetFrameClockMethods();
    self->methods->reset(self, 0);
}

/* gFrameClockMethods slot +0x00C (finalize): the BasicClass finalize. */
void FrameClock__Finalize(FrameClock *self) {
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

/* gFrameClockMethods slot +0x024 (removeParentRef): step the cursor past the parent
 * being removed, then the BasicClass removeParentRef. */
void FrameClock__RemoveParentRef(FrameClock *self, BasicClass *parent) {
    if (self->parentCursor != NULL && parent == self->parentCursor->value) {
        self->parentCursor = self->parentCursor->next;
    }
    GetBasicClassMethods()->removeParentRef((BasicClass *)self, parent);
}

/* gFrameClockMethods slot +0x030 (notifyParents): walk the parent refs with
 * parentCursor (which removeParentRef keeps valid) and pass each the
 * event through its onNotify. */
void FrameClock__NotifyParents(FrameClock *self, s32 event) {
    BasicClass *parent;

    self->parentCursor = self->parentRefs;
    for (GetNextBasicClass(&parent, &self->parentCursor); parent != NULL;
         GetNextBasicClass(&parent, &self->parentCursor)) {
        parent->methods->onNotify(parent, self, event);
    }
    self->parentCursor = NULL;
}

/* gFrameClockMethods slot +0x040 (reset): set the frame count, clear both
 * flags and the cursor. */
void FrameClock__Reset(FrameClock *self, s32 frameCount) {
    self->frameCount = frameCount;
    self->stopped = 0;
    self->paused = 0;
    self->parentCursor = 0;
}

/* gFrameClockMethods slot +0x044 (tick): notify STOPPED if stopped is set, else
 * PAUSED if paused, else count the frame and notify RUNNING. */
void FrameClock__Tick(FrameClock *self) {
    s32 event;

    if (self->stopped != 0) {
        event = FRAMECLOCK_EVENT_STOPPED;
    } else if (self->paused != 0) {
        event = FRAMECLOCK_EVENT_PAUSED;
    } else {
        self->frameCount++;
        event = FRAMECLOCK_EVENT_RUNNING;
    }
    self->methods->notifyParents(self, event);
}

/* gFrameClockMethods slot +0x048. */
s32 FrameClock__GetFrameCount(FrameClock *self) {
    return self->frameCount;
}

/* gFrameClockMethods slot +0x04C. */
void FrameClock__Pause(FrameClock *self) {
    self->paused = 1;
}

/* gFrameClockMethods slot +0x050. */
void FrameClock__Resume(FrameClock *self) {
    self->paused = 0;
}

/* gFrameClockMethods slot +0x054. */
s32 FrameClock__IsPaused(FrameClock *self) {
    return self->paused;
}

/* gFrameClockMethods slot +0x058. */
void FrameClock__Stop(FrameClock *self) {
    self->stopped = 1;
}

/* Returns the gFrameClockMethods method table. */
FrameClockMethods *GetFrameClockMethods(void) {
    return &gFrameClockMethods;
}

/* Allocate and construct a LightRig with its three lights. */
LightRig *New_LightRig(void) {
    LightRig *obj = BMemPMgrAlloc(sizeof(LightRig));

    if (obj != NULL) {
        GetLightRigMethods()->ctor(obj);
        return obj;
    }
    return NULL;
}

/* LightRig slot +0x008 (ctor): the SceneNode ctor, install the table,
 * create and add the three flat lights, then reset. */
void LightRig__LightRig(LightRig *self) {
    s32 i;
    BasicClass **light;

    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetLightRigMethods();
    for (i = 0, light = self->lights; i < ARRAY_COUNT(self->lights); i++, light++) {
        *light = (BasicClass *)New_FlatLightObj(i);
        self->methods->addChild(self, *light);
    }
    self->methods->reset(self);
}

/* LightRig slot +0x00C (finalize): release the three lights, then the
 * SceneNode finalize. */
void LightRig__Finalize(LightRig *self) {
    s32 i;
    BasicClass *light;

    for (i = 0; i < ARRAY_COUNT(self->lights); i++) {
        light = self->methods->getLight(self, i);
        light->methods->release(light);
    }
    GetSceneNodeMethods()->finalize((SceneNode *)self);
}

/* LightRig slot +0x040 (reset): mark the coordinate for recompute. */
void LightRig__Reset(LightRig *self) {
    self->coord2->flg = 0;
}

/* LightRig slot +0x09C (dispatchLinkCommand): empty override. */
void LightRig__DispatchLinkCommand(LightRig *self, void *sender, s32 event) {}

/* LightRig slot +0x0B8 (getLight), inherited unchanged by gStageMapMethods. */
BasicClass *LightRig__GetLight(LightRig *self, s32 index) {
    return self->lights[index];
}

/* LightRig slot +0x0BC: set the ambient colour (swapping the old one out
 * into *rgb when asked) and hand it to GsSetAmbient. */
void LightRig__SetAmbientColor(LightRig *self, ColorRgb *rgb, s32 swap) {
    ColorRgb old;

    if (swap) {
        old = self->ambient;
        self->ambient = *rgb;
        *rgb = old;
    } else {
        self->ambient = *rgb;
    }
    GsSetAmbient(self->ambient.r << AMBIENT_TO_FIX12_SHIFT, self->ambient.g << AMBIENT_TO_FIX12_SHIFT,
                 self->ambient.b << AMBIENT_TO_FIX12_SHIFT);
}

/* Returns the LightRig method table. */
LightRigMethods *GetLightRigMethods(void) {
    return &gLightRigMethods;
}
