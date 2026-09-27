/*
 * class_3bb8c_j -- the tail of TextEntry and the first half of ItemList.
 *  - TextEntry__PrevChar .. TextEntry__SetCharAt and GetTextEntryMethods
 *    finish TextEntry (include/TextEntry.h), the caller-owned string editor
 *    whose other methods are in class_3bb8c_i.
 *  - Everything else is ItemList (include/ItemList.h), the list of strings
 *    the player picks one from: its allocator and ctor, BasicClass's
 *    overrides (finalize, child bookkeeping, onNotify), and the view and
 *    resource methods resetView, loadResources, releaseResources,
 *    attachTarget and detachTarget. Its list methods are in class_3bb8c_k.
 *
 * include/class_3bb8c.h is shared with every other class_3bb8c_* unit; edits
 * to it must be strictly additive.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "TextEntry.h"
#include "ItemList.h"
#include "ScreenSprite.h"
#include "TimImage.h"
#include "TextRow.h"

void TextEntry__PrevChar(TextEntry *self) {
    s32 count;

    if (self->panelSprite) {
        count = self->charIndex - 1;
        self->charIndex = count;
        if (count > 0) {
            self->methods->setCharAt(self, self->cursorIndex, count, 1);
        } else {
            self->charIndex = self->charCount;
        }
    }
}

void TextEntry__ToggleActOnHeld(TextEntry *self) {
    if (self->panelSprite) {
        self->actOnHeld ^= 1;
    }
}

void TextEntry__ResetChar(TextEntry *self) {
    if (self->panelSprite) {
        self->charIndex = 0;
        self->methods->setCharAt(self, self->cursorIndex, 0, 1);
    }
}

void TextEntry__ResetAllChars(TextEntry *self) {
    s32 i;

    if (self->panelSprite) {
        self->charIndex = 0;
        i = self->textLen - 1;
        if (i >= 0) {
            do {
                self->cursorIndex = i;
                self->methods->setCharAt(self, i, self->charIndex, 0);
                i--;
            } while (i >= 0);
        }
        self->methods->setCursorPos(self, self->cursorIndex, 1);
    }
}

void TextEntry__SetCursorPos(TextEntry *self, s32 pos, s32 notify) {
    ScreenSpritePos local;
    CharSprite *obj;

    if (self->panelSprite) {
        local.y = gTextEntryCursorPos.y;
        local.x = pos * 7 + gTextEntryCursorPos.x;
        obj = self->cursorSprite;
        obj->methods->setPosition(obj, &local);
        self->cursorIndex = pos;
        if (notify) {
            self->methods->playSound(self, 0);
        }
    }
}

/* VALUE-of `%gp_rel`, round 45's TextEntry__SetCharAt only -- a byte lookup table
 * (ROM image initialises it to D_800115D0, still-uncarved rodata). */
extern u8 *gNameCharTable;

void TextEntry__SetCharAt(TextEntry *self, s32 pos, s32 charIndex, s32 notify) {
    TextRow *obj;

    if (self->panelSprite) {
        self->editBuf[pos] = gNameCharTable[charIndex];
        obj = self->textRow;
        ((TextRowSetCellAtFn)obj->methods->setCell)(obj, gNameCharTable[charIndex], pos);
        self->cursorIndex = pos;
        self->charIndex = charIndex;
        if (notify) {
            self->methods->playSound(self, 0);
        }
    }
}

/* TextEntry's own table getter (include/TextEntry.h), defined here in ROM
 * order; not ItemList's. */
TextEntryMethods *GetTextEntryMethods(void) {
    return &gTextEntryMethods;
}

/*
 * New_ItemList onward: ItemList's allocator, ctor, child and resource
 * methods (include/ItemList.h; the rest of the class is class_3bb8c_k).
 *
 * BasicClass's method table and its getter are include/BasicClass.h's
 * (through class_3bb8c.h); the base-class calls below upcast `self`.
 */
extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

ItemList *New_ItemList(char **items, s32 mode) {
    ItemList *self = BMemPMgrAlloc(0x54);

    if (self == NULL) {
        goto fail;
    }
    GetItemListMethods()->ctor(self, items, mode);
    return self;
fail:
    return NULL;
}

/*
 * The ctor. `items` is a NULL-terminated array of string pointers; `mode`
 * (0 or 1) is also stashed into self->mode. First pass counts entries; then
 * allocates two parallel itemCount-length arrays (texts: one
 * individually-allocated buffer per entry; textLens: one s32 length per
 * entry, computed by strlen -- halved when mode==1). Each buffer is filled
 * either via DecodeFullWidthSjis (mode==1) or strcpy (otherwise), and
 * maxTextLen tracks the running max of the computed lengths. The max is a
 * ternary, not an `if`: retail stores the old value back unconditionally
 * before the conditional store of len.
 */
extern s32 strlen(void *arg0);
extern void DecodeFullWidthSjis(void *dst, void *src);
extern char *strcpy(char *dest, char *src);

void ItemList__ItemList(ItemList *self, char **items, s32 mode) {
    char **p;
    s32 i;
    s32 len;

    i = 0;
    p = items;
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetItemListMethods();

    while (*p++ != NULL) {
        i++;
    }

    self->itemCount = i;
    self->texts = BMemPMgrAlloc(i * 4);
    p = items;
    self->textLens = BMemPMgrAlloc(self->itemCount * 4);
    self->maxTextLen = 0;

    for (i = 0; i < self->itemCount; i++) {
        len = strlen(*p);
        if (mode == 1) {
            len /= 2;
        }
        self->textLens[i] = len;
        self->texts[i] = BMemPMgrAlloc(len + 4);
        if (mode == 1) {
            DecodeFullWidthSjis(self->texts[i], *p);
        } else {
            strcpy(self->texts[i], *p);
        }
        self->maxTextLen = (self->maxTextLen < len) ? len : self->maxTextLen;
        p++;
    }

    self->mode = mode;
    ItemList__ClearCachedRefs(self);
    self->methods->resetView(self);
}

void ItemList__ClearCachedRefs(ItemList *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->panelSprite = NULL;
}

void ItemList__Finalize(ItemList *self) {
    s32 i;

    for (i = 0; i < self->itemCount; i++) {
        BMemPMgrFree(self->texts[i]);
    }
    BMemPMgrFree(self->textLens);
    BMemPMgrFree(self->texts);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void ItemList__AddChild(ItemList *self, void *child) {
    s32 tag;

    if (child) {
        Get_vtable_BasicClass()->addChild((BasicClass *)self, (BasicClass *)child);
        tag = **(s32 **)child & 0xF;
        if (tag == 2) {
            self->inputSource = child;
        } else if (tag == 5) {
            self->tickSource = child;
        }
    }
}

void ItemList__RemoveChild(ItemList *self, void *child) {
    s32 tag;

    if (child) {
        tag = **(s32 **)child & 0xF;
        if (tag == 2) {
            self->inputSource = NULL;
        } else if (tag == 5) {
            self->tickSource = NULL;
        }
        Get_vtable_BasicClass()->removeChild((BasicClass *)self, (BasicClass *)child);
    }
}

void ItemList__RemoveAllChildren(ItemList *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->panelSprite = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

void ItemList__OnNotify(ItemList *self, void *sender, s32 event) {
    s32 tag;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);
    tag = **(s32 **)sender & 0xF;
    if (tag == 2) {
        self->methods->handleInputCode(self, sender, event);
    } else if (tag == 5) {
        self->methods->tickClosing(self, sender, event);
    }
}

void ItemList__ResetView(ItemList *self) {
    self->topIndex = 0;
    self->column = 0;
    self->cursorIndex = 0;
}

extern char *BuildFileName(char *dest, const char *arg1, const char *arg2, const char *arg3);
extern const char D_8008AB14[]; /* "SELECT" */
extern const char D_8008AB1C[]; /* "CARD\\" */
extern const char D_8008AB24[]; /* ".TIM" */
extern s32 gItemListPanelRect; /* 3 words, New_ScreenSprite's rect: a SpriteRect {0, 0, 256, 160} */
extern s32 gItemListPanelPos;
extern const char D_800116E4[]; /* "FONTICON" */

/*
 * Two handle variables, not one: handle1 and handle2 are disjoint live
 * ranges, and merging them into one `h` gives the rotation filed as the
 * round-18/19 stall (75/95, both addresses and the handle swapped among
 * $s0-$s2). Same shape as class_3bb8c_i's TextEntry__LoadCardResources.
 */
void ItemList__LoadResources(ItemList *self, SceneNode *parent) {
    char path[0x20];
    const char *dir;
    const char *ext;
    TimImage *handle1;
    TimImage *handle2;

    if (parent == NULL) {
        return;
    }
    if (self->panelSprite != NULL) {
        return;
    }

    dir = D_8008AB1C;
    ext = D_8008AB24;

    handle1 = New_TimImage(BuildFileName(path, D_8008AB14, dir, ext));
    ((TimImageUploadFn)handle1->methods->processBuffer)(handle1);
    self->panelSprite = New_ScreenSprite(handle1, (SpriteRect *)&gItemListPanelRect, 0);
    handle1->methods->release(handle1);
    self->panelSprite->methods->attachToParent(self->panelSprite, parent, (LongVec3 *)&gItemListPanelPos);

    handle2 = New_TimImage(BuildFileName(path, D_800116E4, dir, ext));
    ((TimImageUploadFn)handle2->methods->processBuffer)(handle2);
    self->methods->createRows(self, parent, handle2, self->topIndex, self->column, self->cursorIndex);
    handle2->methods->release(handle2);
}

void ItemList__ReleaseResources(ItemList *self) {
    if (self->panelSprite) {
        self->methods->releaseRows(self);
        self->panelSprite = self->panelSprite->methods->release(self->panelSprite);
    }
}

/* The first addChild passes all four words through (ItemListAddChildWideFn,
 * no code): see this function's report for the do/while. */
void ItemList__AttachTarget(ItemList *self, void *child1, void *child2, struct VabStreamObj *target) {
    ItemListAddChildWideFn fn;
    s32 zero;

    zero = 0;
    fn = (ItemListAddChildWideFn)self->methods->addChild;
    do {
        fn(self, child1, child2, target);
        self->methods->addChild(self, child2);
        self->target = target;
        self->result = zero;
    } while (0);
}

void ItemList__DetachTarget(ItemList *self) {
    self->methods->removeChild(self, self->inputSource);
    self->methods->removeChild(self, self->tickSource);
    self->target = NULL;
}
