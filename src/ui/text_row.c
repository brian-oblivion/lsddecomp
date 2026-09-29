/*
 * TextRow's methods (include/text_row.h: a row of CharSprite cells showing
 * a string, a CharSprite subclass), in ROM order: the allocator, ctor,
 * finalize and reset, the attach, display, colour and position overrides
 * that it applies to every cell, then SetCellAt, the empty getCell,
 * SetText, the empty slot +0x0D0 and SetCellPitch, ending with its getter
 * GetTextRowMethods; its method table closes the file. A (void *) entry in
 * it is a method whose declared type differs from its slot's, usually one
 * inherited from a parent class and declared on the parent's type.
 */
#include "common.h"
#include "text_row.h"
#include "bmem_pmgr.h"

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

/* TextRow (include/text_row.h): CharSprite's table with the text row's
 * reset, attach, display, colour, position and cell overrides, then setText
 * and setCellPitch. */
TextRowMethods gTextRowMethods = {
    /* +0x000 header */ TEXTROW_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)TextRow__TextRow,
    /* +0x00C finalize */ TextRow__Finalize,
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
    /* +0x040 reset */ (void *)TextRow__Reset,
    /* +0x044 updateRotation */ (void *)Sprite__UpdateRotation,
    /* +0x048 updateScale */ (void *)SceneNode__UpdateScale,
    /* +0x04C attachToParent */ (void *)TextRow__AttachToParent,
    /* +0x050 detachFromParent */ (void *)TextRow__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ (void *)TextRow__SetDisplay,
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
    /* +0x0B8 setColor */ TextRow__SetColor,
    /* +0x0BC setPosition */ TextRow__SetPosition,
    /* +0x0C0 setPivotAnchor */ (void *)ScreenSprite__SetPivotAnchor,
    /* +0x0C4 setCell */ (void *)TextRow__SetCellAt,
    /* +0x0C8 getCell */ (void *)TextRow__NoOpGetCell,
    /* +0x0CC setText */ TextRow__SetText,
    /* +0x0D0 slotD0 */ TextRow__NoOpSlotD0,
    /* +0x0D4 setCellPitch */ TextRow__SetCellPitch,
};
