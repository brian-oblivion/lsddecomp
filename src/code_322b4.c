/*
 * code_322b4 -- GAME code carved from psyq_322b4 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x322B4..0x330F4 (vram 0x80041AB4..0x800428F4). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). Owns jtbl_80011290 (attached rodata
 * sub-slot 0x1A90).
 *
 * Round 82 matched every function in the unit (getters, accessors, empty
 * overrides, then the 11- to 20-word bodies: the sprite attribute-bit
 * setters, cell selection, finalize chains, the FrameClock allocator), then
 * named it (track 3): every function is real C, not `func_`.
 *
 * Sprite (include/Sprite.h, gSpriteMethods), its direct subclass
 * ScreenSprite (include/ScreenSprite.h, gScreenSpriteMethods, id 0x144: the
 * screen-space sprite, adding setPosition (+0x0BC, screenPos) and a
 * pivot-anchor setter (+0x0C0, centre/left/right/top/bottom)) and ITS
 * subclass CharSprite (include/CharSprite.h, gCharSpriteMethods, id 0x1144:
 * one 8x8 font character, adding setCell/getCell (+0x0C4/+0x0C8); GetCellRect
 * is the free helper its ctor and setCell use to turn a cell index into a
 * rect) are unified; their own methods (New_Sprite, Sprite__*, InitGsSprite,
 * GetSpriteMethods, New_ScreenSprite, ScreenSprite__*, GetScreenSpriteMethods,
 * New_CharSprite, CharSprite__*, GetCharSpriteMethods) live here. Every
 * class owning methods in this unit is now unified (track 4; FrameClock, the
 * last, round 88). gTextRowMethods (0x11144, below CharSprite) and
 * gVariantSpriteMethods (0x1F44, class_3bb8c_p/q/t) are Sprite subclasses too but own
 * no methods in this unit.
 * FrameClock (include/FrameClock.h, gFrameClockMethods, id 0x5) is unified:
 * a BasicClass subclass ticked once per DrawSystem frame that tells its
 * parents event 2 (counted), 3 (paused) or 4 (flag14); its own methods
 * (New_FrameClock, FrameClock__*, Get_vtable_FrameClock) live here.
 * RequestedFile (include/RequestedFile.h, gRequestedFileMethods, id 0xB03) is
 * unified: a FileResource that requests one named file from the active driver
 * at construction and sets `loaded` when the driver's setFlag reports it
 * read (WBgm's SEQ file); its own methods (New_RequestedFile,
 * RequestedFile__*, GetRequestedFileMethods) live here.
 * LightRig (include/LightRig.h, gLightRigMethods, id 0x14) is unified too: a
 * SceneNode subclass owning three FlatLightObj children and an ambient
 * colour (SetAmbientColor -> GsSetAmbient); its own methods (New_LightRig,
 * LightRig__*, GetLightRigMethods) live here. Its getLight (+0x0B8) is
 * inherited unchanged by StageMap's own table (gStageMapMethods), which is why
 * one function occupies the same slot in both.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "CharSprite.h"
#include "LightRig.h"
#include "FlatLightObj.h"
#include "RequestedFile.h"
#include "TimImage.h"
#include "FrameClock.h"

extern void GsSetAmbient(long r, long g, long b);

extern u16 GetTPage(s32 tp, s32 abr, s32 x, s32 y);


/* The zero offset ScreenSprite__AttachToParent attaches with. */
extern LongVec3 gVec3Zero;
extern char *strcpy(char *dst, char *src);

/* The cell origin GetCellRect copies: {0, 0, 8, 8}. */
extern SpriteRect D_8006ED40;

extern u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value);
extern void *BMemPMgrAlloc(s32 size);

extern FileResourceMethods *GetActiveDataSourceMethods(void);

/* Allocate and construct a CharSprite (0xAC bytes): one character cell. */
CharSprite *New_CharSprite(void *texture, u8 cell) {
    CharSprite *obj = BMemPMgrAlloc(0xAC);

    if (obj != NULL) {
        GetCharSpriteMethods()->ctor(obj, texture, cell);
        return obj;
    }
    return NULL;
}

/* CharSprite slot +0x008 (ctor): the ScreenSprite ctor with cell 0x20's rect,
 * install the table, then reset to the caller's cell. */
void CharSprite__CharSprite(CharSprite *self, void *texture, u8 cell) {
    SpriteRect r;

    GetCellRect(&r, 0x20);
    GetScreenSpriteMethods()->ctor((ScreenSprite *)self, texture, &r, 0);
    self->methods = GetCharSpriteMethods();
    ((CharSpriteResetFn)self->methods->reset)(self, cell);
}

/* CharSprite slot +0x040 (reset): re-select the cell through slot +0x0C4. */
void CharSprite__Reset(CharSprite *self, u8 cell) {
    self->methods->setCell(self, cell);
}

/* CharSprite slot +0x0C4: store the cell index and point u,v at its 8x8 cell. */
void CharSprite__SetCell(CharSprite *self, u8 cell) {
    SpriteRect r;

    self->cellIndex = cell;
    GetCellRect(&r, cell);
    self->sprite.u = r.u;
    self->sprite.v = r.v;
}

/* CharSprite slot +0x0C8: read the byte at +0x0A8. */
u8 CharSprite__GetCell(CharSprite *self) {
    u8 pad[16]; /* unused: it is what gives retail its 0x10-byte frame */

    return self->cellIndex;
}

/* Returns the CharSprite method table. */
CharSpriteMethods *GetCharSpriteMethods(void) {
    return &gCharSpriteMethods;
}

/* Cell index -> 8x8 rect in a 32-wide grid, offset from D_8006ED40. */
void GetCellRect(SpriteRect *dst, u32 cell) {
    *dst = D_8006ED40;
    cell &= 0xFF;
    dst->u += (cell & 0x1F) * 8;
    dst->v += (cell >> 5) * 8;
}

/* Allocate and construct a ScreenSprite (0xA8 bytes). */
ScreenSprite *New_ScreenSprite(void *texture, SpriteRect *rect, s32 arg3) {
    ScreenSprite *obj = BMemPMgrAlloc(0xA8);

    if (obj != NULL) {
        GetScreenSpriteMethods()->ctor(obj, texture, rect, arg3);
        return obj;
    }
    return NULL;
}

/* gScreenSpriteMethods slot +0x008 (ctor): the Sprite ctor with abr 0 and arg4 NULL,
 * install the table, then reset. */
void ScreenSprite__ScreenSprite(ScreenSprite *self, void *texture, SpriteRect *rect, s32 arg3) {
    GetSpriteMethods()->ctor((Sprite *)self, texture, 0, rect, NULL, arg3);
    self->methods = GetScreenSpriteMethods();
    self->methods->reset(self);
}

/* gScreenSpriteMethods slot +0x040 (reset): empty override. */
void ScreenSprite__Reset(ScreenSprite *self) {}

/* gCharSpriteMethods and gScreenSpriteMethods slot +0x04C (attachToParent): when not yet
 * attached, attach through Sprite's with a zero offset, then hand the
 * caller's third argument to slot +0x0BC. */
void ScreenSprite__AttachToParent(ScreenSprite *self, SceneNode *parent, ScreenSpritePos *pos) {
    if (self->parent == NULL) {
        GetSpriteMethods()->attachToParent((Sprite *)self, parent, &gVec3Zero);
        self->methods->setPosition(self, pos);
    }
}

/* gCharSpriteMethods and gScreenSpriteMethods slot +0x0BC. */
void ScreenSprite__SetPosition(ScreenSprite *self, ScreenSpritePos *pos) {
    if (self->parent != NULL) {
        self->screenPos = *pos;
    }
}

/* gCharSpriteMethods and gScreenSpriteMethods slot +0x0C0: when attached, move the sprite's
 * pivot: 0 centre, 1 left, 2 right, 3 top, 4 bottom. */
void ScreenSprite__SetPivotAnchor(ScreenSprite *self, u32 anchor) {
    if (self->parent != NULL) {
        switch (anchor) {
            case 0:
                self->sprite.mx = self->sprite.w >> 1;
                self->sprite.my = self->sprite.h >> 1;
                break;
            case 1:
                self->sprite.mx = 0;
                break;
            case 2:
                self->sprite.mx = self->sprite.w;
                break;
            case 3:
                self->sprite.my = 0;
                break;
            case 4:
                self->sprite.my = self->sprite.h;
                break;
        }
    }
}

/* Returns the gScreenSpriteMethods method table. */
ScreenSpriteMethods *GetScreenSpriteMethods(void) {
    return &gScreenSpriteMethods;
}

/* Allocate and construct a Sprite (0xA0 bytes). */
Sprite *New_Sprite(void *texture, s32 abr, SpriteRect *rect, void *arg3, s32 arg4) {
    Sprite *obj = BMemPMgrAlloc(0xA0);

    if (obj != NULL) {
        GetSpriteMethods()->ctor(obj, texture, abr, rect, arg3, arg4);
        return obj;
    }
    return NULL;
}

/* Sprite's reset (+0x040) as its ctor calls it: with all five ctor
 * arguments, returning what the ctor returns. The slot is SceneNode's, typed
 * without them (Sprite.h, "Not settled"), and Sprite__Reset reads the first
 * three. Local, where CharSpriteResetFn and VariantSpriteResetFn sit in their
 * headers, because only this ctor calls through it. */
typedef void *(*SpriteResetFn)(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *arg4,
                               s32 arg5);

/* gSpriteMethods slot +0x008 (ctor): the SceneNode ctor, install the table,
 * and hand every argument to reset. */
void *Sprite__Sprite(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *arg4, s32 arg5) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetSpriteMethods();
    return ((SpriteResetFn)self->methods->reset)(self, texture, abr, rect, arg4, arg5);
}

/* gSpriteMethods slot +0x040 (reset): bind the texture and cell, rebuild the GsSPRITE. */
void Sprite__Reset(Sprite *self, void *texture, s32 abr, SpriteRect *rect) {
    self->image = &((TimImage *)texture)->tim;
    self->rect = *rect;
    InitGsSprite(&self->sprite, abr, rect, self->image);
    self->unk58 = 0;
}

/* Fill a GsSPRITE from a texture image and a cell: colour mode and tpage
 * from the image, size and u,v from the cell, the pivot at its centre,
 * neutral colour, scale 1.0 and no rotation. `tim` is a TimImage's GsIMAGE. */
void InitGsSprite(SpriteGs *sprite, s32 abr, SpriteRect *rect, GsIMAGE *tim) {
    s32 mode = tim->pmode & 3;
    s32 grey = 0x80;

    sprite->attribute = mode << 24;
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
    sprite->scalex = 0x1000;
    sprite->scaley = 0x1000;
}

/* gSpriteMethods slot +0x044 (updateRotation): table[2] as a fraction of
 * degrees, in 4096ths; set or add to the GsSPRITE's rotate. */
void Sprite__UpdateRotation(Sprite *self, s32 set, Ratio16 *table) {
    s32 angle;

    angle = ((table[2].num / table[2].den) << 12) + ((table[2].num % table[2].den) << 12) / table[2].den;
    if (set) {
        self->sprite.rotate = angle;
    } else {
        self->sprite.rotate += angle;
    }
}

/* Sprite classes slot +0x060: display on/off (attribute bit 31, inverted). */
s32 Sprite__SetDisplay(Sprite *self, s32 a1) {
    return GetSetBitField(&self->sprite.attribute, 0x1F, 1, a1 == 0) == 0;
}

/* Sprite classes slot +0x064: attribute bit 30. */
s32 Sprite__SetSemiTrans(Sprite *self, s32 a1) {
    return GetSetBitField(&self->sprite.attribute, 0x1E, 1, a1 != 0);
}

/* Sprite classes slot +0x068: attribute bits 28..29. */
s32 Sprite__SetSemiTransRate(Sprite *self, s32 a1) {
    return GetSetBitField(&self->sprite.attribute, 0x1C, 2, a1);
}

/* gTextRowMethods and gCharSpriteMethods slot +0x098 (update): empty override. */
void Sprite__Update(Sprite *self, void *sender, s32 event) {}

/* Slot +0x0B8 of gCharSpriteMethods, gScreenSpriteMethods, gSpriteMethods and gVariantSpriteMethods (the
 * sprite classes): copy three bytes into the embedded GsSPRITE's r,g,b. */
void Sprite__SetColor(Sprite *self, SpriteRgb *rgb) {
    self->sprite.rgb = *rgb;
}

/* Returns the gSpriteMethods method table. */
SpriteMethods *GetSpriteMethods(void) {
    return &gSpriteMethods;
}

/* Allocate and construct a RequestedFile (0x30 bytes). */
RequestedFile *New_RequestedFile(char *name) {
    RequestedFile *obj = BMemPMgrAlloc(0x30);

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
    char buf[32];

    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetRequestedFileMethods();
    self->loaded = 0;
    if (name != NULL) {
        strcpy(buf, name);
        self->methods->requestLoadFile(self, buf);
    }
}

/* gRequestedFileMethods slot +0x00C (finalize): clear `loaded`, then the active
 * driver's finalize. */
void RequestedFile__Finalize(RequestedFile *self) {
    self->loaded = 0;
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* gRequestedFileMethods slot +0x064 (setFlag): the driver reports the requested
 * file loaded. */
void RequestedFile__MarkLoaded(RequestedFile *self) {
    self->loaded = 1;
}

/* Returns the gRequestedFileMethods method table. */
RequestedFileMethods *GetRequestedFileMethods(void) {
    return &gRequestedFileMethods;
}

/* Allocate and construct a FrameClock (0x1C bytes). */
FrameClock *New_FrameClock(void) {
    FrameClock *obj = BMemPMgrAlloc(0x1C);

    if (obj != NULL) {
        Get_vtable_FrameClock()->ctor(obj);
        return obj;
    }
    return NULL;
}

/* gFrameClockMethods slot +0x008 (ctor): the BasicClass ctor, install the table, reset(0). */
void FrameClock__FrameClock(FrameClock *self) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_FrameClock();
    self->methods->reset(self, 0);
}

/* gFrameClockMethods slot +0x00C (finalize): the BasicClass finalize. */
void FrameClock__Finalize(FrameClock *self) {
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

/* gFrameClockMethods slot +0x024 (removeParentRef): step the cursor past the parent
 * being removed, then the BasicClass removeParentRef. */
void FrameClock__RemoveParentRef(FrameClock *self, BasicClass *parent) {
    if (self->parentCursor != NULL && parent == self->parentCursor->value) {
        self->parentCursor = self->parentCursor->next;
    }
    Get_vtable_BasicClass()->removeParentRef((BasicClass *)self, parent);
}

/* gFrameClockMethods slot +0x030 (notifyParents): walk the parent refs with the
 * cursor at +0x018 (which removeParentRef keeps valid) and pass each the
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

/* gFrameClockMethods slot +0x040 (reset). */
void FrameClock__Reset(FrameClock *self, s32 frameCount) {
    self->frameCount = frameCount;
    self->flag14 = 0;
    self->paused = 0;
    self->parentCursor = 0;
}

/* gFrameClockMethods slot +0x044: notify event 4 if flag14, else 3 if paused, else
 * count up and notify 2. */
void FrameClock__Tick(FrameClock *self) {
    s32 event;

    if (self->flag14 != 0) {
        event = 4;
    } else if (self->paused != 0) {
        event = 3;
    } else {
        self->frameCount++;
        event = 2;
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
void FrameClock__SetFlag14(FrameClock *self) {
    self->flag14 = 1;
}

/* Returns the gFrameClockMethods method table. */
FrameClockMethods *Get_vtable_FrameClock(void) {
    return &gFrameClockMethods;
}

/* Allocate and construct a LightRig (0x54 bytes). */
LightRig *New_LightRig(void) {
    LightRig *obj = BMemPMgrAlloc(0x54);

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
    for (i = 0, light = self->lights; i < 3; i++, light++) {
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

    for (i = 0; i < 3; i++) {
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
void LightRig__SetAmbientColor(LightRig *self, LightRigRgb *rgb, s32 swap) {
    LightRigRgb old;

    if (swap) {
        old = self->ambient;
        self->ambient = *rgb;
        *rgb = old;
    } else {
        self->ambient = *rgb;
    }
    GsSetAmbient((u8)self->ambient.r << 4, (u8)self->ambient.g << 4, (u8)self->ambient.b << 4);
}

/* Returns the LightRig method table. */
LightRigMethods *GetLightRigMethods(void) {
    return &gLightRigMethods;
}
