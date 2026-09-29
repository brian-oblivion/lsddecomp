/*
 * The sprite classes, and three small classes beside them, each documented
 * in its own header:
 *  - Sprite (include/sprite.h), ScreenSprite (include/screen_sprite.h) and
 *    CharSprite (include/char_sprite.h): allocators, ctors (each chains to
 *    its parent's, installs its table, then calls reset) and the methods
 *    each adds or overrides, with GetCellRect, the font-cell helper.
 *    TextRow and VariantSprite derive from them, with methods elsewhere.
 *  - RequestedFile (include/requested_file.h): one named file requested
 *    from the active data-source driver.
 *  - FrameClock (include/frame_clock.h): the per-frame clock.
 *  - LightRig (include/light_rig.h): three flat lights and the ambient
 *    colour.
 * The six classes' method tables close the file.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "char_sprite.h"
#include "light_rig.h"
#include "flat_light_obj.h"
#include "requested_file.h"
#include "tim_image.h"
#include "frame_clock.h"
#include "bmem_pmgr.h"
#include <strings.h>
#include "data_source.h"
#include "scene_node.h"

/* Two constants defined at the end of the file, among the method tables. */

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
 * arguments. The slot is SceneNode's, typed without them (sprite.h, "Not
 * settled"), and Sprite__Reset reads the first three. Local, where
 * CharSpriteResetFn and VariantSpriteResetFn sit in their headers, because
 * only this ctor calls through it. */
typedef void (*SpriteResetFn)(Sprite *self, void *texture, s32 abr, SpriteRect *rect,
                              void *resetArg, s32 resetWord);

/* gSpriteMethods slot +0x008 (ctor): the SceneNode ctor, install the table,
 * and hand every argument to reset. */
void Sprite__Sprite(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *resetArg,
                    s32 resetWord) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetSpriteMethods();
    ((SpriteResetFn)self->methods->reset)(self, texture, abr, rect, resetArg, resetWord);
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
    s32 mode = tim->pmode & TIM_PMODE_DEPTH_MASK;
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

/* Slot +0x098 (update) of every sprite class's table but VariantSprite's: empty override. */
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

/* gFrameClockMethods slot +0x048 (getFrameCount): the frames counted so far. */
s32 FrameClock__GetFrameCount(FrameClock *self) {
    return self->frameCount;
}

/* gFrameClockMethods slot +0x04C (pause): tick stops counting and notifies PAUSED. */
void FrameClock__Pause(FrameClock *self) {
    self->paused = 1;
}

/* gFrameClockMethods slot +0x050 (resume): tick counts again, unless stopped. */
void FrameClock__Resume(FrameClock *self) {
    self->paused = 0;
}

/* gFrameClockMethods slot +0x054 (isPaused). */
s32 FrameClock__IsPaused(FrameClock *self) {
    return self->paused;
}

/* gFrameClockMethods slot +0x058 (stop): tick notifies STOPPED until the next reset. */
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

/* The six classes' method tables and the two constants between the sprite
 * classes' tables, in the order the image keeps them. Each table fills its class's header's slots
 * with the class's own method or the parent's. A (void *) entry is a
 * method whose declared parameters differ from the slot's, usually one
 * inherited from a parent class and declared on the parent's type. */

/* CharSprite (include/char_sprite.h): ScreenSprite's table with the ctor
 * and reset, then setCell and getCell. */
CharSpriteMethods gCharSpriteMethods = {
    /* +0x000 header */ CHARSPRITE_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)CharSprite__CharSprite,
    /* +0x00C finalize */ (void *)SceneNode__Finalize,
    /* +0x010 addChild */ (void *)SceneNode__AddChild,
    /* +0x014 removeChild */ (void *)SceneNode__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)SceneNode__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)SceneNode__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 reset */ (void *)CharSprite__Reset,
    /* +0x044 updateRotation */ (void *)Sprite__UpdateRotation,
    /* +0x048 updateScale */ (void *)SceneNode__UpdateScale,
    /* +0x04C attachToParent */ (void *)ScreenSprite__AttachToParent,
    /* +0x050 detachFromParent */ (void *)SceneNode__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ (void *)Sprite__SetDisplay,
    /* +0x064 setSemiTransOn */ (void *)Sprite__SetSemiTrans,
    /* +0x068 setSemiTransRate */ (void *)Sprite__SetSemiTransRate,
    /* +0x06C setLighting */ (void *)SceneNode__SetLighting,
    /* +0x070 setLightMode */ (void *)SceneNode__SetLightMode,
    /* +0x074 setLightDim */ (void *)SceneNode__SetLightDim,
    /* +0x078 setUseZ */ (void *)SceneNode__SetUseZ,
    /* +0x07C setSubdivision */ (void *)SceneNode__SetSubdivision,
    /* +0x080 setBackClip */ (void *)SceneNode__SetBackClip,
    /* +0x084 getRotMatrix */ (void *)SceneNode__GetRotMatrix,
    /* +0x088 notifyWithHull */ (void *)SceneNode__NotifyWithHull,
    /* +0x08C getModelHull */ (void *)SceneNode__GetModelHull,
    /* +0x090 transformAndNotifyParents */ (void *)SceneNode__TransformAndNotifyParents,
    /* +0x094 onPadEvent */ (void *)SceneNode__OnPadEvent,
    /* +0x098 update */ (void *)Sprite__Update,
    /* +0x09C dispatchLinkCommand */ (void *)SceneNode__DispatchLinkCommand,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ (void *)SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ (void *)SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ SceneNode__NoOpSlotB0,
    /* +0x0B4 addToActorParents */ (void *)SceneNode__AddToActorParents,
    /* +0x0B8 setColor */ (void *)Sprite__SetColor,
    /* +0x0BC setPosition */ (void *)ScreenSprite__SetPosition,
    /* +0x0C0 setPivotAnchor */ (void *)ScreenSprite__SetPivotAnchor,
    /* +0x0C4 setCell */ CharSprite__SetCell,
    /* +0x0C8 getCell */ CharSprite__GetCell,
};

SpriteRect sCharSpriteCellRect = {0, 0, CHARSPRITE_CELL_SIZE, CHARSPRITE_CELL_SIZE};

/* ScreenSprite (include/screen_sprite.h): Sprite's table with the ctor,
 * reset and attachToParent, then setPosition and setPivotAnchor. */
ScreenSpriteMethods gScreenSpriteMethods = {
    /* +0x000 header */ SCREENSPRITE_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)ScreenSprite__ScreenSprite,
    /* +0x00C finalize */ (void *)SceneNode__Finalize,
    /* +0x010 addChild */ (void *)SceneNode__AddChild,
    /* +0x014 removeChild */ (void *)SceneNode__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)SceneNode__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)SceneNode__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 reset */ ScreenSprite__Reset,
    /* +0x044 updateRotation */ (void *)Sprite__UpdateRotation,
    /* +0x048 updateScale */ (void *)SceneNode__UpdateScale,
    /* +0x04C attachToParent */ (void *)ScreenSprite__AttachToParent,
    /* +0x050 detachFromParent */ (void *)SceneNode__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ (void *)Sprite__SetDisplay,
    /* +0x064 setSemiTransOn */ (void *)Sprite__SetSemiTrans,
    /* +0x068 setSemiTransRate */ (void *)Sprite__SetSemiTransRate,
    /* +0x06C setLighting */ (void *)SceneNode__SetLighting,
    /* +0x070 setLightMode */ (void *)SceneNode__SetLightMode,
    /* +0x074 setLightDim */ (void *)SceneNode__SetLightDim,
    /* +0x078 setUseZ */ (void *)SceneNode__SetUseZ,
    /* +0x07C setSubdivision */ (void *)SceneNode__SetSubdivision,
    /* +0x080 setBackClip */ (void *)SceneNode__SetBackClip,
    /* +0x084 getRotMatrix */ (void *)SceneNode__GetRotMatrix,
    /* +0x088 notifyWithHull */ (void *)SceneNode__NotifyWithHull,
    /* +0x08C getModelHull */ (void *)SceneNode__GetModelHull,
    /* +0x090 transformAndNotifyParents */ (void *)SceneNode__TransformAndNotifyParents,
    /* +0x094 onPadEvent */ (void *)SceneNode__OnPadEvent,
    /* +0x098 update */ (void *)Sprite__Update,
    /* +0x09C dispatchLinkCommand */ (void *)SceneNode__DispatchLinkCommand,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ (void *)SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ (void *)SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ SceneNode__NoOpSlotB0,
    /* +0x0B4 addToActorParents */ (void *)SceneNode__AddToActorParents,
    /* +0x0B8 setColor */ (void *)Sprite__SetColor,
    /* +0x0BC setPosition */ ScreenSprite__SetPosition,
    /* +0x0C0 setPivotAnchor */ ScreenSprite__SetPivotAnchor,
};

LongVec3 sVec3Zero = {0, 0, 0};

/* Sprite (include/sprite.h): SceneNode's table with Sprite's ctor, reset,
 * rotation, display, semi-transparency and update, then setColor. */
SpriteMethods gSpriteMethods = {
    /* +0x000 header */ SPRITE_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)Sprite__Sprite,
    /* +0x00C finalize */ (void *)SceneNode__Finalize,
    /* +0x010 addChild */ (void *)SceneNode__AddChild,
    /* +0x014 removeChild */ (void *)SceneNode__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)SceneNode__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)SceneNode__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 reset */ (void *)Sprite__Reset,
    /* +0x044 updateRotation */ (void *)Sprite__UpdateRotation,
    /* +0x048 updateScale */ (void *)SceneNode__UpdateScale,
    /* +0x04C attachToParent */ (void *)SceneNode__AttachToParent,
    /* +0x050 detachFromParent */ (void *)SceneNode__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ Sprite__SetDisplay,
    /* +0x064 setSemiTransOn */ (void *)Sprite__SetSemiTrans,
    /* +0x068 setSemiTransRate */ (void *)Sprite__SetSemiTransRate,
    /* +0x06C setLighting */ (void *)SceneNode__SetLighting,
    /* +0x070 setLightMode */ (void *)SceneNode__SetLightMode,
    /* +0x074 setLightDim */ (void *)SceneNode__SetLightDim,
    /* +0x078 setUseZ */ (void *)SceneNode__SetUseZ,
    /* +0x07C setSubdivision */ (void *)SceneNode__SetSubdivision,
    /* +0x080 setBackClip */ (void *)SceneNode__SetBackClip,
    /* +0x084 getRotMatrix */ (void *)SceneNode__GetRotMatrix,
    /* +0x088 notifyWithHull */ (void *)SceneNode__NotifyWithHull,
    /* +0x08C getModelHull */ (void *)SceneNode__GetModelHull,
    /* +0x090 transformAndNotifyParents */ (void *)SceneNode__TransformAndNotifyParents,
    /* +0x094 onPadEvent */ (void *)SceneNode__OnPadEvent,
    /* +0x098 update */ Sprite__Update,
    /* +0x09C dispatchLinkCommand */ (void *)SceneNode__DispatchLinkCommand,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ (void *)SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ (void *)SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ SceneNode__NoOpSlotB0,
    /* +0x0B4 addToActorParents */ (void *)SceneNode__AddToActorParents,
    /* +0x0B8 setColor */ Sprite__SetColor,
};

/* RequestedFile (include/requested_file.h): FileResource's slots up to
 * +0x074 with its ctor, finalize and onRequestDone; the file-I/O slots are
 * NULL until SetActiveDataSource binds the active driver's. */
RequestedFileMethods gRequestedFileMethods = {
    /* +0x000 header */ REQUESTEDFILE_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ RequestedFile__RequestedFile,
    /* +0x00C finalize */ RequestedFile__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 slot40 */ NULL,
    /* +0x044 open */ NULL,
    /* +0x048 close */ NULL,
    /* +0x04C seek */ NULL,
    /* +0x050 slot50 */ NULL,
    /* +0x054 read */ NULL,
    /* +0x058 loadFile */ NULL,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ RequestedFile__MarkLoaded,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
};

/* FrameClock (include/frame_clock.h): BasicClass's slots with its ctor,
 * finalize, removeParentRef and notifyParents, then its seven clock slots. */
FrameClockMethods gFrameClockMethods = {
    /* +0x000 header */ FRAMECLOCK_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ FrameClock__FrameClock,
    /* +0x00C finalize */ FrameClock__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ FrameClock__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ FrameClock__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 reset */ FrameClock__Reset,
    /* +0x044 tick */ FrameClock__Tick,
    /* +0x048 getFrameCount */ FrameClock__GetFrameCount,
    /* +0x04C pause */ FrameClock__Pause,
    /* +0x050 resume */ FrameClock__Resume,
    /* +0x054 isPaused */ FrameClock__IsPaused,
    /* +0x058 stop */ FrameClock__Stop,
};

/* LightRig (include/light_rig.h): SceneNode's table with its ctor, finalize,
 * reset and dispatchLinkCommand, then getLight and setAmbientColor. */
LightRigMethods gLightRigMethods = {
    /* +0x000 header */ LIGHTRIG_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)LightRig__LightRig,
    /* +0x00C finalize */ LightRig__Finalize,
    /* +0x010 addChild */ (void *)SceneNode__AddChild,
    /* +0x014 removeChild */ (void *)SceneNode__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)SceneNode__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)SceneNode__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 reset */ LightRig__Reset,
    /* +0x044 updateRotation */ (void *)SceneNode__UpdateRotation,
    /* +0x048 updateScale */ (void *)SceneNode__UpdateScale,
    /* +0x04C attachToParent */ (void *)SceneNode__AttachToParent,
    /* +0x050 detachFromParent */ (void *)SceneNode__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ (void *)SceneNode__SetDisplay,
    /* +0x064 setSemiTransOn */ (void *)SceneNode__SetSemiTrans,
    /* +0x068 setSemiTransRate */ (void *)SceneNode__SetSemiTransRate,
    /* +0x06C setLighting */ (void *)SceneNode__SetLighting,
    /* +0x070 setLightMode */ (void *)SceneNode__SetLightMode,
    /* +0x074 setLightDim */ (void *)SceneNode__SetLightDim,
    /* +0x078 setUseZ */ (void *)SceneNode__SetUseZ,
    /* +0x07C setSubdivision */ (void *)SceneNode__SetSubdivision,
    /* +0x080 setBackClip */ (void *)SceneNode__SetBackClip,
    /* +0x084 getRotMatrix */ (void *)SceneNode__GetRotMatrix,
    /* +0x088 notifyWithHull */ (void *)SceneNode__NotifyWithHull,
    /* +0x08C getModelHull */ (void *)SceneNode__GetModelHull,
    /* +0x090 transformAndNotifyParents */ (void *)SceneNode__TransformAndNotifyParents,
    /* +0x094 onPadEvent */ (void *)SceneNode__OnPadEvent,
    /* +0x098 update */ (void *)SceneNode__Update,
    /* +0x09C dispatchLinkCommand */ LightRig__DispatchLinkCommand,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ (void *)SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ (void *)SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ SceneNode__NoOpSlotB0,
    /* +0x0B4 addToActorParents */ (void *)SceneNode__AddToActorParents,
    /* +0x0B8 getLight */ LightRig__GetLight,
    /* +0x0BC setAmbientColor */ LightRig__SetAmbientColor,
};
