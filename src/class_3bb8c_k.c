/*
 * class_3bb8c_k -- the second half of ItemList, and the start of ObjM.
 *  - ItemList (include/ItemList.h), the list of strings the player picks one
 *    from: setState, tickClosing, handleInputCode and forwardToTarget, the
 *    cursor and scroll methods, the four visible rows (createRows,
 *    releaseRows, refreshRows, and the non-virtual helpers FormatRowText and
 *    SetView), stepCursorInView, getCursorIndex and the table getter
 *    GetItemListMethods. Its ctor and resource methods are in class_3bb8c_j.
 *  - ObjM (include/ObjM.h): its allocator, ctor, finalize and onNotify,
 *    which dispatches on the sender's class id. The rest of ObjM is in
 *    class_3bb8c_l and class_3bb8c_m.
 *
 * include/class_3bb8c.h is shared with every other class_3bb8c_* unit; edits
 * to it must be strictly additive.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "class_39e08.h"
#include "TimedTask.h"
#include "TextRow.h"
#include "TimImage.h"
#include "ItemList.h"
#include "ObjM.h"
#include "VabStreamObj.h"

void ItemList__SetState(ItemList *self, s32 state) {
    self->closeTicks = 0;
    if (state < 2) {
        goto end;
    }
    if (state < 4) {
        goto case_lt4;
    }
    if (state == 4) {
        goto case_eq4;
    }
    goto end;
case_lt4:
    self->methods->removeChild(self, self->inputSource);
    self->methods->releaseResources(self);
    self->result = state;
    goto end;
case_eq4:
    self->methods->notifyParents(self, self->result);
end:
    return;
}

void ItemList__TickClosing(ItemList *self) {
    s32 old;

    if (self->result >= 4) {
        return;
    }
    if (self->result < 2) {
        return;
    }
    old = self->closeTicks;
    self->closeTicks = old + 1;
    if (old == 0) {
        return;
    }
    self->methods->setState(self, 4);
}

void ItemList__HandleInputCode(ItemList *self, void *source, s32 code) {
    switch (code) {
        case 25:
            self->methods->forwardToTarget(self, 0x10);
            self->methods->setState(self, 2);
            break;
        case 23:
            self->methods->forwardToTarget(self, 0x10);
            self->methods->setState(self, 3);
            break;
        case 5:
            self->methods->scrollRight(self);
            break;
        case 4:
            self->methods->scrollLeft(self);
            break;
        case 18:
            self->methods->cursorUp(self);
            break;
        case 19:
            self->methods->cursorDown(self);
            break;
    }
}

void ItemList__ForwardToTarget(ItemList *self, s32 code) {
    struct VabStreamObj *target = self->target;

    if (target != NULL) {
        target->methods->playTone(target, code, 0x60, 0x60);
    }
}

void ItemList__ScrollRight(ItemList *self) {
    ItemListMethods *methods;
    s32 tmp;
    s32 column;

    if (!self->panelSprite) {
        return;
    }
    tmp = self->column;
    column = tmp;
    if (column + 0x1A >= self->maxTextLen) {
        return;
    }
    methods = self->methods;
    column++;
    self->column = column;
    methods->refreshRows(self, self->topIndex, column, self->cursorIndex, 1);
}

void ItemList__ScrollLeft(ItemList *self) {
    s32 column;

    if (!self->panelSprite) {
        return;
    }
    column = self->column - 1;
    if (column < 0) {
        return;
    }
    self->column = column;
    self->methods->refreshRows(self, self->topIndex, column, self->cursorIndex, 1);
}

void ItemList__CursorUp(ItemList *self, s32 arg1, s32 arg2, s32 arg3) {
    s32 cursor;
    s32 newTop;
    s32 newCursor;

    if (!self->panelSprite) {
        return;
    }
    cursor = self->cursorIndex;
    if (cursor - 1 < 0) {
        return;
    }
    if (cursor - self->topIndex > 0) {
        self->methods->stepCursorInView(self, 0, 1, arg3);
    } else {
        self->topIndex--;
        newTop = self->topIndex;
        self->cursorIndex--;
        newCursor = self->cursorIndex;
        self->methods->refreshRows(self, newTop, self->column, newCursor, 1);
    }
}

void ItemList__CursorDown(ItemList *self, s32 arg1, s32 arg2, s32 arg3) {
    s32 newTop;
    s32 newCursor;
    s32 prevTop;

    if (!self->panelSprite) {
        return;
    }
    if (self->cursorIndex + 1 >= self->itemCount) {
        return;
    }
    prevTop = self->topIndex - 1;
    if (self->cursorIndex - prevTop < 4) {
        self->methods->stepCursorInView(self, 1, 1, arg3);
    } else {
        self->topIndex++;
        newTop = self->topIndex;
        self->cursorIndex++;
        newCursor = self->cursorIndex;
        self->methods->refreshRows(self, newTop, self->column, newCursor, 1);
    }
}

/* The first row's position, two sdata words (-0x5C, -0xF). Read by value
 * into ItemList__CreateRows's `pos`; each further row is 0xA lower. */
extern s32 gItemListRowOriginX;
extern s32 gItemListRowOriginY;

void ItemList__CreateRows(ItemList *self, SceneNode *parent, TimImage *font, s32 top, s32 column,
                          s32 cursor) {
    char buf[0x20];
    ScreenSpritePos pos;
    TextRow **p;
    s32 count;
    s32 i;

    if (!self->panelSprite) {
        return;
    }

    pos.x = gItemListRowOriginX;
    pos.y = gItemListRowOriginY;
    count = self->itemCount;
    p = &self->rows[0];
    if (count >= 5) {
        count = 4;
    }

    for (i = 0; i < count; i++) {
        ItemList__FormatRowText(self, buf, i, top, column);
        *p = New_TextRow(font, 0x1A, buf);
        (*p)->methods->attachToParent(*p, parent, (LongVec3 *)&pos);
        (*p)->methods->setColor(*p, &gItemListRowColor);
        pos.y += 0xA;
        p++;
    }

    ItemList__SetView(self, top, column, cursor, 1);
}

void ItemList__ReleaseRows(ItemList *self) {
    s32 count;
    s32 i;
    u8 unused[8];

    if (!self->panelSprite) {
        return;
    }
    count = self->itemCount;
    i = 0;
    if (count >= 5) {
        count = 4;
    }
    if (count <= 0) {
        return;
    }
    do {
        self->rows[i]->methods->release(self->rows[i]);
        self->rows[i] = NULL;
        i++;
    } while (i < count);
}

/* Psy-Q strlen and memcpy (libc2/strlen and libc2/memcpy, linked from
 * Sony's own SDK objects). libc2's memcpy guards a NULL dest, copies `n`
 * bytes a byte at a time and returns dest -- which is why this call site
 * was read as strncpy-like before the object gave it its name.
 * Both are declared LOCAL to this unit, not in a shared header, since
 * these are cross-unit prototypes for functions this unit does not define
 * (see CLAUDE.md's header-contention rule). strlen already has a
 * differently-typed local declaration in class_3bb8c_j.c
 * (`s32 strlen(void *arg0)`); this unit's own call site reads its
 * argument as a byte pointer, so it is typed `char *` here instead --
 * per-call-site typing of an undefined function's argument, same
 * convention as DecodeFullWidthSjis (see include/class_3bb8c.h HEAD NOTE).
 * memcpy's return type is `void *` to agree with include/psyq/memory.h's
 * unprototyped declaration should this unit ever include it; the result
 * is discarded at the one call site either way. */
extern s32 strlen(char *s);
extern void *memcpy(char *dest, char *src, s32 n);

void ItemList__RefreshRows(ItemList *self, s32 top, s32 column, s32 cursor, s32 notify) {
    s32 count;
    s32 i;
    char buf[0x20];
    TextRow **p;

    if (!self->panelSprite) {
        return;
    }
    count = self->itemCount;
    p = &self->rows[0];
    if (count >= 5) {
        count = 4;
    }
    for (i = 0; i < count; i++) {
        ItemList__FormatRowText(self, buf, i, top, column);
        (*p)->methods->setText(*p, buf);
        p++;
    }
    ItemList__SetView(self, top, column, cursor, 0);
    if (notify) {
        self->methods->forwardToTarget(self, 0);
    }
}

char *ItemList__FormatRowText(ItemList *self, char *dest, s32 row, s32 top, s32 column) {
    s32 idx = top + row;
    s32 len;
    s32 i;

    len = strlen(self->texts[idx] + column);
    if (len >= 0x1B) {
        len = 0x1A;
    }
    memcpy(dest, self->texts[idx] + column, len);
    i = len;
    if (i < 0x1A) {
        for (; i < 0x1A; i++) {
            dest[i] = ' ';
        }
    }
    dest[0x1A] = 0;
    return dest;
}

void ItemList__SetView(ItemList *self, s32 top, s32 column, s32 cursor, s32 highlight) {
    TextRow *elem;
    s32 flag = highlight;

    self->topIndex = top;
    self->column = column;
    self->cursorIndex = cursor;
    if (flag == 0) {
        return;
    }
    cursor -= top;
    elem = self->rows[cursor];
    elem->methods->setColor(elem, &gItemListCursorColor);
}

void ItemList__StepCursorInView(ItemList *self, s32 dir, s32 notify) {
    TextRow **p;
    s32 idx;

    if (!self->panelSprite) {
        return;
    }
    idx = self->cursorIndex - self->topIndex;
    p = &self->rows[idx];
    (*p)->methods->setColor(*p, &gItemListRowColor);
    if (dir) {
        self->cursorIndex++;
        p++;
    } else {
        self->cursorIndex--;
        p--;
    }
    (*p)->methods->setColor(*p, &gItemListCursorColor);
    if (notify) {
        self->methods->forwardToTarget(self, 0);
    }
}

s32 ItemList__GetCursorIndex(ItemList *self) {
    return self->cursorIndex;
}

ItemListMethods *GetItemListMethods(void) {
    return &gItemListMethods;
}

ObjM *New_ObjM(BasicClass *sound, struct WBgm *bgm, TimImage *etcTim,
               struct LinkResource *dreamerTmd, s32 stage) {
    ObjM *self;
    ObjMMethods *methods;

    self = BMemPMgrAlloc(0x88);
    if (self != NULL) {
        methods = GetObjMMethods();
        methods->ctor(self, sound, bgm, etcTim, dreamerTmd, stage);
        return self;
    }
    return NULL;
}

void ObjM__ObjM(ObjM *self, BasicClass *sound, struct WBgm *bgm, TimImage *etcTim,
                struct LinkResource *dreamerTmd, s32 stage) {
    GetTimedTaskMethods()->ctor((TimedTask *)self, 0, sound);
    self->methods = GetObjMMethods();
    self->unk64 = 0;
    self->inSession = 0;
    self->timBlockPending = 1;
    self->bgm = bgm;
    self->stage = stage;
    self->ctorSound = sound;
    self->etcTim = etcTim;
    self->dreamerTmd = dreamerTmd;
    self->pauseSetupStep = 0;
    self->closeReady = 0;
    self->methods->resetCounters(self);
}

void ObjM__Finalize(ObjM *self) {
    GetTimedTaskMethods()->finalize((TimedTask *)self);
}

void ObjM__OnNotify(ObjM *self, BasicClass *sender, s32 event) {
    s32 tag;

    GetTimedTaskMethods()->onNotify((TimedTask *)self, sender, event);
    tag = sender->methods->header;
    if ((tag & 0xFFF) == 0x114) {
        self->methods->onStageMapNotify(self, sender, event);
    } else if ((tag & 0xFFF) == 0x164) {
        self->methods->onFadeNotify(self, (struct FadeBox *)sender, event);
    } else if ((tag & 0xFFFF) == 0x1F34) {
        self->methods->onDreamSysNotify(self, sender, event);
    }
}
