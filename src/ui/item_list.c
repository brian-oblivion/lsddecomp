/*
 * ItemList's second half (include/item_list.h: the scrolling list of strings
 * the memory card's load-file picker shows); the first half, New_ItemList to
 * ItemList__DetachTarget, is in src/ui/input_dialogs.c. In ROM order:
 * closing and reporting (SetState, TickClosing), the Pad events
 * (HandleInputCode, PlaySound), the cursor and scroll methods, the four
 * visible rows (create, release, refresh, format), the view and cursor
 * helpers, and the table getter GetItemListMethods.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "text_row.h"
#include "tim_image.h"
#include "item_list.h"
#include "vab_stream_obj.h"
#include "pad.h"
#include "bmem_pmgr.h"
#include <strings.h>

/* The y step from one row to the next (createRows). */
#define ITEMLIST_ROW_SPACING 10

/* The row colours, two 3-byte RGBs in sdata, 4 bytes apart; only their
 * addresses are taken (setColor). */
extern struct ColorRgb sItemListRowColor;
extern struct ColorRgb sItemListCursorColor;

void ItemList__SetState(ItemList *self, s32 state) {
    /* MATCHING: gotos; an if/else chain tests the ranges and lays out the arms in
     * another order. */
    self->closeTicks = 0;
    if (state < ITEMLIST_RESULT_CHOSEN) {
        goto end;
    }
    if (state < ITEMLIST_STATE_REPORT) {
        goto case_lt4;
    }
    if (state == ITEMLIST_STATE_REPORT) {
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

/* The first row's position, two sdata words (-92, -15). Read by value into
 * ItemList__CreateRows's `pos`; each further row is ITEMLIST_ROW_SPACING
 * lower. */
extern s32 sItemListRowOriginX;
extern s32 sItemListRowOriginY;

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
    /* +0x000 header */ 0x20,
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
