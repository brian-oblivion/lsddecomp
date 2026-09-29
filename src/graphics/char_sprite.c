/*
 * CharSprite's methods (include/char_sprite.h: one character of the 8 x 8
 * font, a ScreenSprite whose texture cell is picked by a one-byte code), in
 * ROM order: the allocator, ctor, reset, setCell and getCell, its getter
 * GetCharSpriteMethods, then GetCellRect, the font-cell helper. Its method
 * table and the font's cell 0 close the file; ScreenSprite, its parent,
 * follows in screen_sprite.c and Sprite in sprite.c.
 */
#include "common.h"
#include "char_sprite.h"
#include "bmem_pmgr.h"

/* Defined at the end of the file, after the method table. */

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

/* The method table, then the font's cell 0, in the order the image keeps
 * them. The table fills the header's slots with the class's own method or
 * the parent's. A (void *) entry is a
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
