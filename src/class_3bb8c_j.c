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
 * Both classes keep their input and tick children by kind (Pad, FrameClock)
 * and draw through a ScreenSprite panel and TextRows built from CARD\ TIMs;
 * their editing and list methods do nothing until loading has made
 * `panelSprite`.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <strings.h>
#include "class_3bb8c.h"
#include "TextEntry.h"
#include "ItemList.h"
#include "ScreenSprite.h"
#include "TimImage.h"
#include "TextRow.h"
#include "Pad.h"
#include "FrameClock.h"

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
        for (i = self->textLen - 1; i >= 0; i--) {
            self->cursorIndex = i;
            self->methods->setCharAt(self, i, self->charIndex, 0);
        }
        self->methods->setCursorPos(self, self->cursorIndex, 1);
    }
}

void TextEntry__SetCursorPos(TextEntry *self, s32 pos, s32 notify) {
    ScreenSpritePos screenPos;
    CharSprite *cursor;

    if (self->panelSprite) {
        screenPos.y = gTextEntryCursorPos.y;
        screenPos.x = pos * TEXTROW_DEFAULT_PITCH + gTextEntryCursorPos.x;
        cursor = self->cursorSprite;
        cursor->methods->setPosition(cursor, &screenPos);
        self->cursorIndex = pos;
        if (notify) {
            self->methods->playSound(self, 0);
        }
    }
}

/* The characters an entry can hold, in the order nextChar/prevChar step
 * through them: a pointer to a NUL-terminated byte string (TextEntry.h). */
extern u8 *gNameCharTable;

void TextEntry__SetCharAt(TextEntry *self, s32 pos, s32 charIndex, s32 notify) {
    TextRow *row;

    if (self->panelSprite) {
        self->editBuf[pos] = gNameCharTable[charIndex];
        row = self->textRow;
        ((TextRowSetCellAtFn)row->methods->setCell)(row, gNameCharTable[charIndex], pos);
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
 * ItemList from here on. The base-class calls go through BasicClass's table
 * (include/BasicClass.h) and upcast `self`.
 */

ItemList *New_ItemList(char **items, s32 mode) {
    ItemList *self = BMemPMgrAlloc(sizeof(ItemList));

    /* MATCHING: goto, not an early return: NULL fills the branch's delay slot. */
    if (self == NULL) {
        goto fail;
    }
    GetItemListMethods()->ctor(self, items, mode);
    return self;
fail:
    return NULL;
}

/*
 * The ctor. `items` is a NULL-terminated array of strings, copied into
 * buffers of the list's own: `texts[i]` holds item i and `textLens[i]` its
 * length in characters (bytes, halved for full-width SJIS), and
 * `maxTextLen` is the longest. Ends with resetView.
 */
extern char *DecodeFullWidthSjis(char *dest, char *src);

void ItemList__ItemList(ItemList *self, char **items, s32 mode) {
    char **item;
    s32 i;
    s32 len;

    /* MATCHING: both set before the base ctor call. */
    i = 0;
    item = items;
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetItemListMethods();

    while (*item++ != NULL) {
        i++;
    }

    self->itemCount = i;
    self->texts = BMemPMgrAlloc(i * sizeof(*self->texts));
    item = items;
    self->textLens = BMemPMgrAlloc(self->itemCount * sizeof(*self->textLens));
    self->maxTextLen = 0;

    for (i = 0; i < self->itemCount; i++) {
        len = strlen(*item);
        if (mode == ITEMLIST_MODE_FULLWIDTH) {
            len /= 2;
        }
        self->textLens[i] = len;
        self->texts[i] = BMemPMgrAlloc(len + 4);
        if (mode == ITEMLIST_MODE_FULLWIDTH) {
            DecodeFullWidthSjis(self->texts[i], *item);
        } else {
            strcpy(self->texts[i], *item);
        }
        /* MATCHING: a ternary, not an `if`: the old value is stored back. */
        self->maxTextLen = (self->maxTextLen < len) ? len : self->maxTextLen;
        item++;
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
        tag = ((BasicClass *)child)->methods->header & CLASS_ID_ROOT_MASK;
        if (tag == PAD_CLASS_ID) {
            self->inputSource = child;
        } else if (tag == FRAMECLOCK_CLASS_ID) {
            self->tickSource = child;
        }
    }
}

void ItemList__RemoveChild(ItemList *self, void *child) {
    s32 tag;

    if (child) {
        tag = ((BasicClass *)child)->methods->header & CLASS_ID_ROOT_MASK;
        if (tag == PAD_CLASS_ID) {
            self->inputSource = NULL;
        } else if (tag == FRAMECLOCK_CLASS_ID) {
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
    tag = ((BasicClass *)sender)->methods->header & CLASS_ID_ROOT_MASK;
    if (tag == PAD_CLASS_ID) {
        self->methods->handleInputCode(self, sender, event);
    } else if (tag == FRAMECLOCK_CLASS_ID) {
        self->methods->tickClosing(self, sender, event);
    }
}

void ItemList__ResetView(ItemList *self) {
    self->topIndex = 0;
    self->column = 0;
    self->cursorIndex = 0;
}

/* loadResources' path buffer, BuildFileName's dest: "CARD\\" + name + ".TIM". */
#define CARD_TIM_PATH_SIZE 32

extern char *BuildFileName(char *dest, const char *name, const char *dir, const char *ext);
extern const char sStrSelect[];              /* "SELECT" */
extern const char sItemListCardPathPrefix[]; /* "CARD\\" */
extern const char sItemListTimExt[];         /* ".TIM" */
extern SpriteRect gItemListPanelRect;        /* SELECT's cell: 256 x 160 from (0, 0) */
extern ScreenSpritePos gItemListPanelPos;    /* (-100, -60) */
extern const char sItemListStrFontIcon[];    /* "FONTICON" */

/*
 * Loads CARD\SELECT.TIM as the panel sprite, placed at gItemListPanelPos
 * under `parent`, and has createRows build the rows from CARD\FONTICON.TIM.
 * Does nothing without a parent or when already loaded. The same shape as
 * TextEntry__LoadCardResources (class_3bb8c_i).
 */
void ItemList__LoadResources(ItemList *self, SceneNode *parent) {
    char path[CARD_TIM_PATH_SIZE];
    const char *dir;
    const char *ext;
    TimImage *panelTim; /* MATCHING: two handles, not one reused */
    TimImage *fontTim;

    if (parent == NULL) {
        return;
    }
    if (self->panelSprite != NULL) {
        return;
    }

    dir = sItemListCardPathPrefix;
    ext = sItemListTimExt;

    panelTim = New_TimImage(BuildFileName(path, sStrSelect, dir, ext));
    ((TimImageUploadFn)panelTim->methods->processBuffer)(panelTim);
    self->panelSprite = New_ScreenSprite(panelTim, &gItemListPanelRect, 0);
    panelTim->methods->release(panelTim);
    self->panelSprite->methods->attachToParent(self->panelSprite, parent, (LongVec3 *)&gItemListPanelPos);

    fontTim = New_TimImage(BuildFileName(path, sItemListStrFontIcon, dir, ext));
    ((TimImageUploadFn)fontTim->methods->processBuffer)(fontTim);
    self->methods->createRows(self, parent, fontTim, self->topIndex, self->column, self->cursorIndex);
    fontTim->methods->release(fontTim);
}

void ItemList__ReleaseResources(ItemList *self) {
    if (self->panelSprite) {
        self->methods->releaseRows(self);
        self->panelSprite = self->panelSprite->methods->release(self->panelSprite);
    }
}

/* Adds the input and tick children, keeps the sound as `target` and zeroes
 * `result`. The first addChild is handed all four words (its occupant reads
 * only the first; ItemListAddChildWideFn). */
void ItemList__AttachTarget(ItemList *self, void *inputSource, void *tickSource,
                            struct VabStreamObj *target) {
    ItemListAddChildWideFn addChildWide;
    s32 zero;

    /* MATCHING: the cached slot and zero and the do/while (0) fill a delay slot. */
    zero = 0;
    addChildWide = (ItemListAddChildWideFn)self->methods->addChild;
    do {
        addChildWide(self, inputSource, tickSource, target);
        self->methods->addChild(self, tickSource);
        self->target = target;
        self->result = zero;
    } while (0);
}

void ItemList__DetachTarget(ItemList *self) {
    self->methods->removeChild(self, self->inputSource);
    self->methods->removeChild(self, self->tickSource);
    self->target = NULL;
}
