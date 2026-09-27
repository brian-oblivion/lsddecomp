#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_2cc8c.h"
#include "TimImage.h"
#include "BgLayer.h"

/*
 * code_2cc8c_b -- 20 of TaskCore's own methods (gTaskCoreMethods +0x0C4 to
 * +0x11C; the class is include/TaskCore.h, track 4 round 84). Round 78 naming
 * pass (runner echo): every function in this file MATCHED before this round;
 * the pass renamed all 20 and five exclusively-owned fields, no stalls.
 *
 * What this slice of the class implements: a tab/slot picker with a
 * scrollable item list inside each tab. `self->activeSlot` selects the tab;
 * `self->slotElements[i]` is each tab's own representative widget (walked/
 * broadcast to by TaskCore__BroadcastToSlots, switched by
 * TaskCore__SetActiveSlot); `self->itemLists[idx]`/`self->itemCounts[idx]`
 * hold the item list WITHIN tab idx (built by TaskCore__CreateSlotElements,
 * torn down by TaskCore__ReleaseSlotElements, positioned/shown by
 * TaskCore__RefreshSlotView through `self->listView`); `self->slotCounts[idx]`
 * is a ring cursor into that per-tab item list.
 * TaskCore__BeginElementScroll/TaskCore__CommitElementScroll/
 * TaskCore__CancelElementScroll form a `self->inputMode` state-1<->2 trio that
 * opens interactive scrolling, then either commits the new cursor position
 * back into the target descriptor (`SlotEntry::savedCursor`) or cancels back
 * to the last-committed one; TaskCore__AdvanceSlotCursor/
 * TaskCore__RetreatSlotCursor step the cursor by one (wrapping) and forward
 * through TaskCore__SetSlotCursor (setSlotCursor, +0x11C), which does the actual
 * old/new element highlight swap -- the same shape TaskCore__SetActiveSlot
 * uses one level up, switching which TAB is active instead of which item.
 * TaskCore__SetTarget/TaskCore__ReleaseTarget are the constructor/teardown
 * pair for `self->target` (a TaskCoreTarget, include/TaskCore.h).
 */

/* Psy-Q libc2's strlen, linked from Sony's object. */
extern s32 strlen(char *s);

/* New_BoxFill's size and colour for listView: (320, 240), (32, 32, 64). */
extern s32 sListViewSize[2];
extern BoxFillRgb sListViewColor;

/* A screen position, x then y. */
typedef struct {
    s32 x;
    s32 y;
} SlotPos;

/* The item-list record TaskCoreTarget::unk24[slot] points to, for a slot
 * that opens a scrolled list of items (TitleMenu's: D_80086CA8). */
typedef struct SlotEntry {
    u8 pad000[0x004];
    s32 savedCursor;       /* +0x004 the committed item cursor */
    SpriteRgb cursorColor; /* +0x008 the colour of the item under the cursor while scrolling */
    u8 pad00B[0x010 - 0x00B];
    /* +0x010 where the cursor's row is drawn; the list starts savedCursor rows
     * above. MATCHING: a struct, so the copy is lw/lw, sw/sw, then a reload of
     * .y (CommitElementScroll); two s32 fields compile differently. */
    SlotPos pos;
} SlotEntry;

/* The same record as createSlotElements reads it. */
typedef struct SrcDesc {
    u8 pad000[0x004];
    s32 savedCursor; /* +0x004 the item cursor the list opens at */
    u8 pad008[0x018 - 0x008];
    char **itemNames; /* +0x018 NULL-terminated; one New_TextRow per name */
} SrcDesc;

s32 TaskCore__TickFadeColor(TaskCore *self) {
    s32 c = 0x80 - (self->frameCounter * self->fadeRate);
    u8 buf[3];

    buf[0] = c;
    buf[1] = c;
    buf[2] = c;
    self->methods->broadcastToSlots(self, buf);
    self->bgLayer->methods->setColor(self->bgLayer, 1, (BgLayerRgb *)buf);
    return (u8)c >= 0x81;
}

void TaskCore__SetSubHandle(TaskCore *self, const char *path, BasicClass *handle) {
    if (path != NULL) {
        if (self->subHandlePath != NULL) {
            self->subHandle->methods->release(self->subHandle);
        }
        self->subHandle = (BasicClass *)New_TimImage((char *)path);
        ((TimImageUploadFn)((TimImage *)self->subHandle)->methods->processBuffer)(
            (TimImage *)self->subHandle);
        ((TimImage *)self->subHandle)->methods->freeBuffer((TimImage *)self->subHandle);
    } else {
        self->subHandle = handle;
    }
    self->subHandlePath = path;
}

void TaskCore__SetTarget(TaskCore *self, TaskCoreTarget *a1) {
    char **list;
    s32 count;
    s32 size;
    TextRow **arr;
    TimImage *handle;
    s32 i;

    self->target = a1;
    if (a1 == NULL) {
        return;
    }

    list = a1->names;
    count = 0;
    while (*list++ != NULL) {
        count++;
    }
    size = count * 4;
    arr = BMemPMgrAlloc(size);
    self->slotElements = (BasicClass **)arr;
    self->itemCounts = BMemPMgrAlloc(size);
    self->slotCounts = BMemPMgrAlloc(size);
    self->itemLists = BMemPMgrAlloc(size);
    self->slotCount = count;

    if (a1->path != NULL) {
        handle = New_TimImage((char *)a1->path);
        ((TimImageUploadFn)handle->methods->processBuffer)(handle);
        handle->methods->freeBuffer(handle);
    } else {
        handle = (TimImage *)a1->handle;
    }

    list = a1->names;
    i = 0;
    if (*list != NULL) {
        do {
            void *extra = a1->unk24[i];
            s32 len = strlen(*list);

            *arr = New_TextRow(handle, len, *list);
            arr++;
            if (extra != NULL) {
                self->activeSlot = i;
                self->methods->createSlotElements(self, extra, handle);
            }
            list++;
            i++;
        } while (*list != NULL);
    }

    self->listView = New_BoxFill(sListViewSize, &sListViewColor, 0);
    a1->handle = (BasicClass *)handle;
}

void TaskCore__ReleaseTarget(TaskCore *self) {
    TextRow **arr;
    s32 i;

    if (self->target == NULL) {
        return;
    }
    if (self->target->path != NULL) {
        TimImage *o = (TimImage *)self->target->handle;
        o->methods->release(o);
    }
    self->listView->methods->release(self->listView);
    arr = (TextRow **)self->slotElements;
    for (i = 0; i < self->slotCount; arr++) {
        TextRow *elem;

        if (self->target->unk24[i] != NULL) {
            self->activeSlot = i;
            self->methods->releaseSlotElements(self);
        }
        elem = *arr;
        elem->methods->release(elem);
        i++;
    }
    BMemPMgrFree(self->itemLists);
    BMemPMgrFree(self->slotCounts);
    BMemPMgrFree(self->itemCounts);
    BMemPMgrFree(self->slotElements);
}

void TaskCore__UpdateSlotElements(TaskCore *self, void *a1) {
    TextRow **arr;
    u8 *ptr;
    s32 i;

    if (self->target == NULL) {
        return;
    }
    arr = (TextRow **)self->slotElements;
    ptr = self->target->externalRecords;
    for (i = 0; i < self->slotCount; i++, arr++, ptr += 8) {
        if (self->target->registrationSlots[i] == NULL) {
            TextRow *elem = *arr;

            elem->methods->attachToParent(elem, a1, (LongVec3 *)ptr);
            if (self->target->unk24[i] != NULL) {
                self->activeSlot = i;
                self->methods->refreshSlotView(self, a1, 0);
            }
        } else {
            TextRow *elem = *arr;

            elem->methods->detachFromParent(elem);
        }
    }
}

void TaskCore__BroadcastToSlots(TaskCore *self, void *a1) {
    s32 origIdx;
    TextRow **arr;
    s32 i;

    if (self->target == NULL) {
        return;
    }
    arr = (TextRow **)self->slotElements;
    origIdx = self->activeSlot;
    for (i = 0; i < self->slotCount;) {
        TextRow *elem = *arr;

        arr++;
        elem->methods->setColor(elem, a1);
        if (self->target->unk24[i] != NULL) {
            self->activeSlot = i;
            self->methods->broadcastToSlotElements(self, a1);
        }
        i++;
        /* Keeps i++ ahead of the self->slotCount reload, leaving retail's nop
         * in that load's delay slot; without it GCC moves i++ into the slot. */
        __asm__("");
    }
    self->activeSlot = origIdx;
}

void TaskCore__FindNextFreeSlot(TaskCore *self) {
    s32 i;

    if (self->target == NULL) {
        return;
    }
    i = self->activeSlot;
    i++;
    for (;;) {
        if (i >= self->slotCount) {
            i = 0;
        }
        if (i == self->activeSlot) {
            break;
        }
        if (self->target->registrationSlots[i++] != NULL) {
            continue;
        }
        i--;
        break;
    }
    self->methods->setActiveSlot(self, i, 1);
}

void TaskCore__FindPrevFreeSlot(TaskCore *self) {
    s32 i;

    if (self->target == NULL) {
        return;
    }
    i = self->activeSlot;
    i--;
    for (;;) {
        if (i < 0) {
            i = self->slotCount - 1;
        }
        if (i == self->activeSlot) {
            break;
        }
        if (self->target->registrationSlots[i--] != NULL) {
            continue;
        }
        i++;
        break;
    }
    self->methods->setActiveSlot(self, i, 1);
}

void TaskCore__SetActiveSlot(TaskCore *self, s32 a1, void *a2) {
    s32 idx;
    TextRow *elemB;
    TextRow *elemA;

    if (self->target == NULL) {
        return;
    }
    idx = self->activeSlot;
    elemB = ((TextRow **)self->slotElements)[idx];
    elemA = ((TextRow **)self->slotElements)[a1];
    if (idx >= 0) {
        elemB->methods->setColor(elemB, (SpriteRgb *)self->target->unselectedColor);
    }
    elemA->methods->setColor(elemA, (SpriteRgb *)self->target->selectedColor);
    self->activeSlot = a1;
    if (a2 != NULL) {
        self->methods->playSound(self, 0);
    }
    self->methods->setState(self, 9);
}

s32 TaskCore__GetActiveSlot(TaskCore *self) {
    return self->activeSlot;
}

void TaskCore__CreateSlotElements(TaskCore *self, void *desc, void *a2) {
    char **list;
    s32 idx;
    s32 count;
    TextRow **buf;

    list = ((SrcDesc *)desc)->itemNames;
    idx = self->activeSlot;
    count = 0;
    while (*list++ != NULL) {
        count++;
    }
    buf = BMemPMgrAlloc(count * 4);
    self->itemLists[idx] = (void *)buf;
    self->slotCounts[idx] = ((SrcDesc *)desc)->savedCursor;
    self->itemCounts[idx] = count;

    list = ((SrcDesc *)desc)->itemNames;
    if (*list != NULL) {
        do {
            s32 len = strlen(*list);

            *buf = New_TextRow(a2, len, *list);
            list++;
            buf++;
        } while (*list != NULL);
    }
}

void TaskCore__ReleaseSlotElements(TaskCore *self) {
    ReleaseBasicClassArray(self->itemLists[self->activeSlot], self->itemCounts[self->activeSlot]);
    BMemPMgrFree(self->itemLists[self->activeSlot]);
}

void TaskCore__RefreshSlotView(TaskCore *self, void *a1, s32 a2) {
    s32 idx;
    TextRow **arr;
    s32 count;
    s32 counter;
    SlotPos pos;
    s32 i;

    idx = self->activeSlot;
    arr = (TextRow **)self->itemLists[idx];
    {
        SlotEntry *target = (SlotEntry *)self->target->unk24[idx];

        count = self->itemCounts[idx];
        counter = target->savedCursor;
    }

    for (i = 0; i < count; i++) {
        (*arr)->methods->detachFromParent(*arr);
        arr++;
    }

    pos = ((SlotEntry *)self->target->unk24[idx])->pos;
    pos.y -= counter * 10;

    if (a2 != 0) {
        s32 buf[2];

        ((BoxFillAttachToParentFn)((BoxFill *)self->listView)->methods->attachToParent)(
            (BoxFill *)self->listView, (SceneNode *)self->unk14, (BoxFillPos *)&pos);
        buf[0] = 0x28;
        buf[1] = count * 12;
        ((BoxFill *)self->listView)->methods->setSize((BoxFill *)self->listView, buf);
    } else {
        ((BoxFill *)self->listView)->methods->detachFromParent((BoxFill *)self->listView);
    }

    arr = (TextRow **)self->itemLists[idx];
    for (i = 0; i < count; i++) {
        (*arr)->methods->attachToParent(*arr, a1, (LongVec3 *)&pos);
        (*arr)->methods->setDisplay(*arr, a2);
        pos.y += 10;
        arr++;
    }

    arr = (TextRow **)self->itemLists[idx];
    arr[counter]->methods->setDisplay(arr[counter], 1);
}

void TaskCore__BroadcastToSlotElements(TaskCore *self, void *a1) {
    s32 idx = self->activeSlot;
    TextRow **arr = (TextRow **)self->itemLists[idx];
    s32 count = self->itemCounts[idx];
    s32 i;

    for (i = 0; i < count; i++) {
        TextRow *elem = *arr;
        arr++;
        elem->methods->setColor(elem, a1);
    }
}

void TaskCore__BeginElementScroll(TaskCore *self) {
    s32 idx;
    TextRow *elem;
    SpriteRgb *cursorColor;

    if (self->inputMode != 1) {
        return;
    }
    idx = self->activeSlot;
    self->methods->refreshSlotView(self, self->unk14, 1);
    elem = ((TextRow **)self->itemLists[idx])[self->slotCounts[idx]];
    cursorColor = &((SlotEntry *)self->target->unk24[idx])->cursorColor;
    elem->methods->setColor(elem, cursorColor);
    self->inputMode = 2;
    self->methods->setState(self, 14);
}

void TaskCore__CommitElementScroll(TaskCore *self) {
    s32 idx;
    s32 counter;
    SlotPos pos;
    TextRow **arr;
    s32 count;
    s32 i;

    if (self->inputMode != 2) {
        return;
    }
    idx = self->activeSlot;
    counter = self->slotCounts[idx];
    pos = ((SlotEntry *)self->target->unk24[idx])->pos;
    pos.y -= counter * 10;

    arr = (TextRow **)self->itemLists[idx];
    count = self->itemCounts[idx];
    for (i = 0; i < count; i++) {
        (*arr)->methods->setDisplay(*arr, 0);
        (*arr)->methods->setPosition(*arr, (ScreenSpritePos *)&pos);
        pos.y += 10;
        arr++;
    }

    {
        TextRow *elem = ((TextRow **)self->itemLists[idx])[counter];

        elem->methods->setDisplay(elem, 1);
        elem->methods->setColor(elem, (SpriteRgb *)self->target->unselectedColor);
    }

    ((SlotEntry *)self->target->unk24[idx])->savedCursor = counter;

    ((BoxFill *)self->listView)->methods->detachFromParent((BoxFill *)self->listView);

    self->inputMode = 1;
    self->methods->setState(self, 0x10);
}

void TaskCore__CancelElementScroll(TaskCore *self) {
    s32 idx;
    s32 counter;
    TextRow **arr;
    TextRow *elem1;
    TextRow *elem2;
    s32 newVal;

    if (self->inputMode != 2) {
        return;
    }
    idx = self->activeSlot;
    counter = self->slotCounts[idx];
    self->methods->refreshSlotView(self, self->unk14, 0);
    arr = (TextRow **)self->itemLists[idx];
    elem1 = arr[counter];
    elem1->methods->setColor(elem1, (SpriteRgb *)self->target->unselectedColor);
    newVal = ((SlotEntry *)self->target->unk24[idx])->savedCursor;
    self->slotCounts[idx] = newVal;
    elem2 = arr[newVal];
    elem2->methods->setDisplay(elem2, 1);
    self->inputMode = 1;
    self->methods->setState(self, 17);
}

void TaskCore__AdvanceSlotCursor(TaskCore *self) {
    s32 idx = self->activeSlot;
    s32 v = self->slotCounts[idx];

    v++;
    if (v >= self->itemCounts[idx]) {
        v = 0;
    }
    self->methods->setSlotCursor(self, v, 1);
}

void TaskCore__RetreatSlotCursor(TaskCore *self) {
    s32 idx = self->activeSlot;
    s32 v = self->slotCounts[idx];

    v--;
    if (v < 0) {
        v = self->itemCounts[idx] - 1;
    }
    self->methods->setSlotCursor(self, v, 1);
}

void TaskCore__SetSlotCursor(TaskCore *self, s32 a1, void *a2) {
    s32 idx;
    s32 counter;
    TextRow **arr;
    TextRow *elem1;
    TextRow *elem2;
    SpriteRgb *cursorColor;

    idx = self->activeSlot;
    counter = self->slotCounts[idx];
    arr = (TextRow **)self->itemLists[idx];
    elem1 = arr[counter];
    elem2 = arr[a1];
    elem1->methods->setColor(elem1, (SpriteRgb *)self->target->unselectedColor);
    cursorColor = &((SlotEntry *)self->target->unk24[idx])->cursorColor;
    elem2->methods->setColor(elem2, cursorColor);
    self->slotCounts[idx] = a1;
    if (a2 != NULL) {
        self->methods->playSound(self, 0);
    }
    self->methods->setState(self, 9);
}
