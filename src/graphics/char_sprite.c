/*
 * Two of the sprite classes, each documented in its own header, in ROM
 * order: CharSprite (include/char_sprite.h), one 8 x 8 cell of the font
 * texture, with GetCellRect, the font-cell helper, then ScreenSprite
 * (include/screen_sprite.h), a Sprite placed in screen coordinates:
 * allocators, ctors (each chains to its parent's, installs its table, then
 * calls reset) and the methods each adds or overrides. Sprite, their base,
 * follows in sprite.c. The two classes' method tables close the file.
 */
#include "common.h"
#include "char_sprite.h"
#include "bmem_pmgr.h"

/* Two constants defined at the end of the file, among the method tables. */

/* The zero offset ScreenSprite__AttachToParent attaches with. */
extern LongVec3 sVec3Zero;

/* Cell 0 of the font texture, {u 0, v 0, w 8, h 8}: GetCellRect offsets it. */
extern SpriteRect sCharSpriteCellRect;

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

/* The two classes' method tables, each followed by its constant, in the
 * order the image keeps them. Each table fills its class's header's slots
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
