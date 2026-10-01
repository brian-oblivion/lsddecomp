/*
 * ItemList's methods (include/item_list.h: the scrolling list of strings
 * the memory card's load-file picker shows), in ROM order: its life and
 * resources (New_ItemList to ItemList__DetachTarget, introduced below),
 * closing and reporting (SetState, TickClosing), the Pad events
 * (HandleInputCode, PlaySound), the cursor and scroll methods, the four
 * visible rows (create, release, refresh, format), the view and cursor
 * helpers, the table getter GetItemListMethods, then the table and the
 * panel's cell. TextEntry, the save-title editor before it in ROM, is in
 * text_entry.c.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include "text_row.h"
#include "tim_image.h"
#include "item_list.h"
#include "vab_stream_obj.h"
#include "pad.h"
#include "bmem_pmgr.h"
#include <strings.h>
#include "frame_clock.h"
#include "full_width_sjis.h"
#include "data_source.h"

/* The y step from one row to the next (createRows). */
#define ITEMLIST_ROW_SPACING 10

extern u8 sItemListStrNameChars[]; /* a space, A to Z, a to z, 0 to 9, as TextEntry's */

/* ItemList's small data, in address order. Positions are percent of half
 * the screen from the centre (include/screen_sprite.h). */

/* Where LoadResources attaches the panel sprite. */
static ScreenSpritePos sItemListPanelPos SDATA = {-100, -60};
/* The first row's position, read by value into CreateRows's `pos`; each
 * further row is ITEMLIST_ROW_SPACING lower. */
static s32 sItemListRowOriginX SDATA = -92;
static s32 sItemListRowOriginY SDATA = -15;
/* MATCHING: TextEntry's character-table pointer, copied with its code;
 * nothing here reads it. */
static u8 *sItemListNameCharTable SDATA = sItemListStrNameChars;
/* The row colours, grey and the cursor's yellow; only their addresses are
 * taken (setColor). */
static ColorRgb sItemListRowColor SDATA = {80, 80, 80};
static ColorRgb sItemListCursorColor SDATA = {128, 128, 0};
/* LoadResources' path parts: CARD\SELECT.TIM. */
static char sStrSelect[] SDATA = "SELECT";
static char sItemListCardPathPrefix[] SDATA = "CARD\\";
static char sItemListTimExt[] SDATA = ".TIM";

/*
 * ItemList's life and resources: its allocator and ctor, BasicClass's
 * overrides (finalize, child bookkeeping, onNotify), and the view and
 * resource methods resetView, loadResources, releaseResources, attachTarget
 * and detachTarget. Its list methods follow.
 *
 * Like TextEntry it keeps its input and tick children by kind (Pad,
 * FrameClock) and draws through a ScreenSprite panel and TextRows built from
 * CARD\ TIMs; its list methods do nothing until loading has made
 * `panelSprite`. The base-class calls go through BasicClass's table
 * (include/basic_class.h) and upcast `self`.
 */

ItemList *New_ItemList(char **items, s32 mode) {
    ItemList *self = BMemPMgrAlloc(sizeof(ItemList));

    if (self != NULL) {
        GetItemListMethods()->ctor(self, items, mode);
        return self;
    }
    return NULL;
}

/*
 * The ctor. `items` is a NULL-terminated array of strings, copied into
 * buffers of the list's own: `texts[i]` holds item i and `textLens[i]` its
 * length in characters (bytes, halved for full-width SJIS), and
 * `maxTextLen` is the longest. Ends with resetView.
 */
void ItemList__ItemList(ItemList *self, char **items, s32 mode) {
    char **item;
    s32 i;
    s32 len;

    /* MATCHING: both set before the base ctor call. */
    i = 0;
    item = items;
    GetBasicClassMethods()->ctor((BasicClass *)self);
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
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

void ItemList__AddChild(ItemList *self, void *child) {
    s32 tag;

    if (child) {
        GetBasicClassMethods()->addChild((BasicClass *)self, (BasicClass *)child);
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
        GetBasicClassMethods()->removeChild((BasicClass *)self, (BasicClass *)child);
    }
}

void ItemList__RemoveAllChildren(ItemList *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->panelSprite = NULL;
    GetBasicClassMethods()->removeAllChildren((BasicClass *)self);
}

void ItemList__OnNotify(ItemList *self, void *sender, s32 event) {
    s32 tag;

    GetBasicClassMethods()->onNotify((BasicClass *)self, sender, event);
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

extern char sItemListStrFontIcon[]; /* "FONTICON" */

/*
 * Loads CARD\SELECT.TIM as the panel sprite, placed at sItemListPanelPos
 * under `parent`, and has createRows build the rows from CARD\FONTICON.TIM.
 * Does nothing without a parent or when already loaded. The same shape as
 * TextEntry__LoadCardResources, above.
 */
void ItemList__LoadResources(ItemList *self, SceneNode *parent) {
    char path[CARD_TIM_PATH_SIZE];
    char *dir;
    char *ext;
    TimImage *panelTim;
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
    self->panelSprite = New_ScreenSprite(panelTim, &sItemListPanelRect, 0);
    panelTim->methods->release(panelTim);
    self->panelSprite->methods->attachToParent(self->panelSprite, parent, (LongVec3 *)&sItemListPanelPos);

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

    /* MATCHING: the cached slot, `zero` and the do/while (0) give retail's order. */
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

void ItemList__SetState(ItemList *self, s32 state) {
    self->closeTicks = 0;
    switch (state) {
        case ITEMLIST_RESULT_CHOSEN:
        case ITEMLIST_RESULT_CANCELLED:
            self->methods->removeChild(self, self->inputSource);
            self->methods->releaseResources(self);
            self->result = state;
            break;
        case ITEMLIST_STATE_REPORT:
            self->methods->notifyParents(self, self->result);
            break;
    }
}

void ItemList__TickClosing(ItemList *self) {
    if (self->result >= ITEMLIST_STATE_REPORT) {
        return;
    }
    if (self->result < ITEMLIST_RESULT_CHOSEN) {
        return;
    }
    if (self->closeTicks++ == 0) {
        return;
    }
    self->methods->setState(self, ITEMLIST_STATE_REPORT);
}

void ItemList__HandleInputCode(ItemList *self, void *source, s32 code) {
    /* MATCHING: the cases stay in this order; retail lays their bodies out in it. */
    switch (code) {
        case PAD_EVENT_PRESSED + PAD_BUTTON_RRIGHT:
            self->methods->playSound(self, ITEMLIST_TONE_BUTTON);
            self->methods->setState(self, ITEMLIST_RESULT_CHOSEN);
            break;
        case PAD_EVENT_PRESSED + PAD_BUTTON_RDOWN:
            self->methods->playSound(self, ITEMLIST_TONE_BUTTON);
            self->methods->setState(self, ITEMLIST_RESULT_CANCELLED);
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_LRIGHT:
            self->methods->scrollRight(self);
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_LLEFT:
            self->methods->scrollLeft(self);
            break;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LUP:
            self->methods->cursorUp(self);
            break;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LDOWN:
            self->methods->cursorDown(self);
            break;
    }
}

void ItemList__PlaySound(ItemList *self, s32 tone) {
    struct VabStreamObj *target = self->target;

    if (target != NULL) {
        target->methods->playTone(target, tone, ITEMLIST_TONE_VOLUME, ITEMLIST_TONE_VOLUME);
    }
}

void ItemList__ScrollRight(ItemList *self) {
    ItemListMethods *methods;
    s32 current;
    s32 column;

    if (!self->panelSprite) {
        return;
    }
    current = self->column; /* MATCHING: read, then copied; one local compiles differently */
    column = current;
    if (column + ITEMLIST_ROW_CHARS >= self->maxTextLen) {
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

void ItemList__CursorUp(ItemList *self, s32 unused1, s32 unused2, s32 forwarded) {
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
    /* MATCHING: this test polarity, and newTop/newCursor read back after each step; the
     * other polarity swaps the arms, and passing the fields compiles differently. */
    if (cursor - self->topIndex > 0) {
        self->methods->stepCursorInView(self, 0, 1, forwarded);
    } else {
        self->topIndex--;
        newTop = self->topIndex;
        self->cursorIndex--;
        newCursor = self->cursorIndex;
        self->methods->refreshRows(self, newTop, self->column, newCursor, 1);
    }
}

void ItemList__CursorDown(ItemList *self, s32 unused1, s32 unused2, s32 forwarded) {
    s32 newTop;
    s32 newCursor;
    s32 prevTop;

    if (!self->panelSprite) {
        return;
    }
    if (self->cursorIndex + 1 >= self->itemCount) {
        return;
    }
    prevTop = self->topIndex - 1; /* MATCHING: its own statement; inline, the -1 folds away */
    if (self->cursorIndex - prevTop < ARRAY_COUNT(self->rows)) {
        self->methods->stepCursorInView(self, 1, 1, forwarded);
    } else {
        self->topIndex++;
        newTop = self->topIndex; /* MATCHING: read back after each step, as in CursorUp */
        self->cursorIndex++;
        newCursor = self->cursorIndex;
        self->methods->refreshRows(self, newTop, self->column, newCursor, 1);
    }
}

void ItemList__CreateRows(ItemList *self, SceneNode *parent, TimImage *font, s32 top, s32 column,
                          s32 cursor) {
    char buf[32]; /* MATCHING: declared first; declared later it compiles differently */
    ScreenSpritePos pos;
    TextRow **row;
    s32 count;
    s32 i;

    if (!self->panelSprite) {
        return;
    }

    pos.x = sItemListRowOriginX;
    pos.y = sItemListRowOriginY;
    count = self->itemCount;
    row = &self->rows[0];
    if (count > ARRAY_COUNT(self->rows)) {
        count = ARRAY_COUNT(self->rows);
    }

    for (i = 0; i < count; i++) {
        ItemList__FormatRowText(self, buf, i, top, column);
        *row = New_TextRow(font, ITEMLIST_ROW_CHARS, buf);
        (*row)->methods->attachToParent(*row, parent, (LongVec3 *)&pos);
        (*row)->methods->setColor(*row, &sItemListRowColor);
        pos.y += ITEMLIST_ROW_SPACING;
        row++;
    }

    ItemList__SetView(self, top, column, cursor, 1);
}

void ItemList__ReleaseRows(ItemList *self) {
    s32 count;
    s32 i;
    u8 unused[8]; /* MATCHING: never used; it gives retail's 0x28-byte stack */

    if (!self->panelSprite) {
        return;
    }
    count = self->itemCount;
    i = 0; /* MATCHING: here, and a do/while, as retail tests count once */
    if (count > ARRAY_COUNT(self->rows)) {
        count = ARRAY_COUNT(self->rows);
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

void ItemList__RefreshRows(ItemList *self, s32 top, s32 column, s32 cursor, s32 notify) {
    s32 count;
    s32 i;
    char buf[32]; /* MATCHING: 32 bytes; it gives retail's stack frame */
    TextRow **row;

    if (!self->panelSprite) {
        return;
    }
    count = self->itemCount;
    row = &self->rows[0]; /* MATCHING: set before the clamp, not after it */
    if (count > ARRAY_COUNT(self->rows)) {
        count = ARRAY_COUNT(self->rows);
    }
    for (i = 0; i < count; i++) {
        ItemList__FormatRowText(self, buf, i, top, column);
        (*row)->methods->setText(*row, buf);
        row++;
    }
    ItemList__SetView(self, top, column, cursor, 0);
    if (notify) {
        self->methods->playSound(self, ITEMLIST_TONE_CURSOR);
    }
}

char *ItemList__FormatRowText(ItemList *self, char *dest, s32 row, s32 top, s32 column) {
    s32 item = top + row; /* MATCHING: top + row, not row + top */
    s32 len;
    s32 i;

    len = strlen(self->texts[item] + column);
    if (len > ITEMLIST_ROW_CHARS) {
        len = ITEMLIST_ROW_CHARS;
    }
    memcpy(dest, self->texts[item] + column, len);
    for (i = len; i < ITEMLIST_ROW_CHARS; i++) {
        dest[i] = ' ';
    }
    dest[ITEMLIST_ROW_CHARS] = '\0';
    return dest;
}

void ItemList__SetView(ItemList *self, s32 top, s32 column, s32 cursor, s32 highlight) {
    TextRow *row;

    self->topIndex = top;
    self->column = column;
    self->cursorIndex = cursor;
    if (highlight == 0) {
        return;
    }
    cursor -= top; /* MATCHING: cursor becomes the row index; a new local compiles differently */
    row = self->rows[cursor];
    row->methods->setColor(row, &sItemListCursorColor);
}

void ItemList__StepCursorInView(ItemList *self, s32 dir, s32 notify) {
    TextRow **row;
    s32 idx;

    if (!self->panelSprite) {
        return;
    }
    idx = self->cursorIndex - self->topIndex;
    row = &self->rows[idx]; /* MATCHING: one row pointer, stepped; indexing twice differs */
    (*row)->methods->setColor(*row, &sItemListRowColor);
    if (dir) {
        self->cursorIndex++;
        row++;
    } else {
        self->cursorIndex--;
        row--;
    }
    (*row)->methods->setColor(*row, &sItemListCursorColor);
    if (notify) {
        self->methods->playSound(self, ITEMLIST_TONE_CURSOR);
    }
}

s32 ItemList__GetCursorIndex(ItemList *self) {
    return self->cursorIndex;
}

ItemListMethods *GetItemListMethods(void) {
    return &gItemListMethods;
}

/* ItemList's method table, class id 0x20: BasicClass's slots, six of them
 * overridden, then ItemList's own from +0x040. A slot whose function is
 * declared for another class's `self` takes a `void *` cast. */
/* clang-format off */
ItemListMethods gItemListMethods = {
    /* +0x000 header */ ITEMLIST_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ ItemList__ItemList,
    /* +0x00C finalize */ ItemList__Finalize,
    /* +0x010 addChild */ (void *)ItemList__AddChild,
    /* +0x014 removeChild */ (void *)ItemList__RemoveChild,
    /* +0x018 removeAllChildren */ ItemList__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ ItemList__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 resetView */ ItemList__ResetView,
    /* +0x044 loadResources */ ItemList__LoadResources,
    /* +0x048 releaseResources */ ItemList__ReleaseResources,
    /* +0x04C attachTarget */ ItemList__AttachTarget,
    /* +0x050 detachTarget */ ItemList__DetachTarget,
    /* +0x054 setState */ ItemList__SetState,
    /* +0x058 tickClosing */ (void *)ItemList__TickClosing,
    /* +0x05C handleInputCode */ ItemList__HandleInputCode,
    /* +0x060 playSound */ ItemList__PlaySound,
    /* +0x064 pad64 */ {NULL, NULL, NULL, NULL, NULL, NULL},
    /* +0x07C scrollRight */ ItemList__ScrollRight,
    /* +0x080 scrollLeft */ ItemList__ScrollLeft,
    /* +0x084 cursorUp */ (void *)ItemList__CursorUp,
    /* +0x088 cursorDown */ (void *)ItemList__CursorDown,
    /* +0x08C createRows */ ItemList__CreateRows,
    /* +0x090 releaseRows */ ItemList__ReleaseRows,
    /* +0x094 refreshRows */ ItemList__RefreshRows,
    /* +0x098 stepCursorInView */ (void *)ItemList__StepCursorInView,
    /* +0x09C getCursorIndex */ ItemList__GetCursorIndex,
};
/* clang-format on */

/* SELECT's cell, the item-list panel ItemList__LoadResources shows:
 * 256 x 160 from (0, 0). */
SpriteRect sItemListPanelRect = {0, 0, 256, 160};
