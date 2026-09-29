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
    /* MATCHING: the gotos keep retail's branch polarity and block order. */
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
            self->methods->playSound(self, 1 << 4); /* VAB program 1, tone 0 */
            self->methods->setState(self, ITEMLIST_RESULT_CHOSEN);
            break;
        case PAD_EVENT_PRESSED + PAD_BUTTON_RDOWN:
            self->methods->playSound(self, 1 << 4); /* VAB program 1, tone 0 */
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
        target->methods->playTone(target, tone, 96, 96);
    }
}

void ItemList__ScrollRight(ItemList *self) {
    ItemListMethods *methods;
    s32 current;
    s32 column;

    if (!self->panelSprite) {
        return;
    }
    current = self->column; /* MATCHING: the double read keeps retail's registers */
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
    /* MATCHING: this polarity, and newTop/newCursor, keep retail's block order and registers. */
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
    prevTop = self->topIndex - 1; /* MATCHING: its own statement, or cc1 folds the -1 */
    if (self->cursorIndex - prevTop < ARRAY_COUNT(self->rows)) {
        self->methods->stepCursorInView(self, 1, 1, forwarded);
    } else {
        self->topIndex++;
        newTop = self->topIndex; /* MATCHING: newTop/newCursor keep retail's registers */
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
    char buf[32]; /* MATCHING: declared first, or cc1 keeps its address in a register */
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
    u8 unused[8]; /* MATCHING: retail's 0x28-byte frame */

    if (!self->panelSprite) {
        return;
    }
    count = self->itemCount;
    i = 0; /* MATCHING: here and a do/while, as retail tests count once */
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
    char buf[32]; /* MATCHING: retail's frame size */
    TextRow **row;

    if (!self->panelSprite) {
        return;
    }
    count = self->itemCount;
    row = &self->rows[0]; /* MATCHING: before the clamp, in its delay slot */
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
        self->methods->playSound(self, 0);
    }
}

char *ItemList__FormatRowText(ItemList *self, char *dest, s32 row, s32 top, s32 column) {
    s32 item = top + row; /* MATCHING: this operand order */
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
    cursor -= top; /* MATCHING: reuses cursor's register for the index */
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
    row = &self->rows[idx]; /* MATCHING: one address, stepped, as retail */
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
        self->methods->playSound(self, 0);
    }
}

s32 ItemList__GetCursorIndex(ItemList *self) {
    return self->cursorIndex;
}

ItemListMethods *GetItemListMethods(void) {
    return &gItemListMethods;
}
