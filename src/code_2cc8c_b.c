#include "common.h"
#include "code_2cc8c.h"

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

/* SlotEntry's +0x10/+0x14 word pair, read as ONE 8-byte struct. Retail
 * copies it with a whole-struct assignment (lw/lw into two fresh
 * temporaries, sw/sw, then a RELOAD of .y before adjusting it) -- see
 * docs/match-reports/TaskCore__CommitElementScroll.md, round 75. Local view: the shared
 * header still spells the pair as two s32 fields. */
typedef struct {
    s32 x;
    s32 y;
} SlotPos;

#define SLOT_POS(target) (*(SlotPos *)&(target)->unk10)

s32 TaskCore__TickFadeColor(TaskCore *self)
{
    s32 c = 0x80 - (self->frameCounter * self->fadeRate);
    u8 buf[3];

    buf[0] = c;
    buf[1] = c;
    buf[2] = c;
    self->methods->broadcastToSlots(self, buf);
    ((Unk78Obj *)self->bgLayer)->methods->slotB8((Unk78Obj *)self->bgLayer, 1, buf);
    return (u8)c >= 0x81;
}

void TaskCore__SetSubHandle(TaskCore *self, const char *path, BasicClass *handle)
{
    if (path != NULL) {
        if (self->subHandlePath != NULL) {
            self->subHandle->methods->release(self->subHandle);
        }
        self->subHandle = (BasicClass *)func_8003B39C(path);
        ((Unk74Obj *)self->subHandle)->methods->slot78((Unk74Obj *)self->subHandle);
        ((Unk74Obj *)self->subHandle)->methods->slot5C((Unk74Obj *)self->subHandle);
    } else {
        self->subHandle = handle;
    }
    self->subHandlePath = path;
}

void TaskCore__SetTarget(TaskCore *self, TaskCoreTarget *a1)
{
    char **list;
    s32 count;
    s32 size;
    Unk64Elem **arr;
    Unk74Obj *handle;
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
        handle = func_8003B39C(a1->path);
        handle->methods->slot78(handle);
        handle->methods->slot5C(handle);
    } else {
        handle = (Unk74Obj *)a1->handle;
    }

    list = a1->names;
    i = 0;
    if (*list != NULL) {
        do {
            void *extra = a1->unk24[i];
            s32 len = strlen(*list);

            *arr = New_Obj6EAC0(handle, len, *list);
            arr++;
            if (extra != NULL) {
                self->activeSlot = i;
                self->methods->createSlotElements(self, extra, handle);
            }
            list++;
            i++;
        } while (*list != NULL);
    }

    self->listView = New_ClassEAC0(D_8008A8E8, D_8008A8F0, 0);
    a1->handle = (BasicClass *)handle;
}

void TaskCore__ReleaseTarget(TaskCore *self)
{
    Unk64Elem **arr;
    s32 i;

    if (self->target == NULL) {
        return;
    }
    if (self->target->path != NULL) {
        Unk74Obj *o = (Unk74Obj *)self->target->handle;
        o->methods->slot4(o);
    }
    self->listView->methods->release(self->listView);
    arr = (Unk64Elem **)self->slotElements;
    for (i = 0; i < self->slotCount; arr++) {
        Unk64Elem *elem;

        if (self->target->unk24[i] != NULL) {
            self->activeSlot = i;
            self->methods->releaseSlotElements(self);
        }
        elem = *arr;
        elem->methods->slot4(elem);
        i++;
    }
    BMemPMgrFree(self->itemLists);
    BMemPMgrFree(self->slotCounts);
    BMemPMgrFree(self->itemCounts);
    BMemPMgrFree(self->slotElements);
}

void TaskCore__UpdateSlotElements(TaskCore *self, void *a1)
{
    Unk64Elem **arr;
    u8 *ptr;
    s32 i;

    if (self->target == NULL) {
        return;
    }
    arr = (Unk64Elem **)self->slotElements;
    ptr = self->target->externalRecords;
    for (i = 0; i < self->slotCount; i++, arr++, ptr += 8) {
        if (self->target->registrationSlots[i] == NULL) {
            Unk64Elem *elem = *arr;

            elem->methods->slot4C(elem, a1, ptr);
            if (self->target->unk24[i] != NULL) {
                self->activeSlot = i;
                self->methods->refreshSlotView(self, a1, 0);
            }
        } else {
            Unk64Elem *elem = *arr;

            elem->methods->slot50(elem);
        }
    }
}

void TaskCore__BroadcastToSlots(TaskCore *self, void *a1)
{
    s32 origIdx;
    Unk64Elem **arr;
    s32 i;

    if (self->target == NULL) {
        return;
    }
    arr = (Unk64Elem **)self->slotElements;
    origIdx = self->activeSlot;
    for (i = 0; i < self->slotCount;) {
        Unk64Elem *elem = *arr;

        arr++;
        elem->methods->slotB8(elem, a1);
        if (self->target->unk24[i] != NULL) {
            self->activeSlot = i;
            self->methods->broadcastToSlotElements(self, a1);
        }
        i++;
        __asm__("");
    }
    self->activeSlot = origIdx;
}

void TaskCore__FindNextFreeSlot(TaskCore *self)
{
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

void TaskCore__FindPrevFreeSlot(TaskCore *self)
{
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

void TaskCore__SetActiveSlot(TaskCore *self, s32 a1, void *a2)
{
    s32 idx;
    Unk64Elem *elemB;
    Unk64Elem *elemA;

    if (self->target == NULL) {
        return;
    }
    idx = self->activeSlot;
    elemB = ((Unk64Elem **)self->slotElements)[idx];
    elemA = ((Unk64Elem **)self->slotElements)[a1];
    if (idx >= 0) {
        elemB->methods->slotB8(elemB, self->target->unselectedColor);
    }
    elemA->methods->slotB8(elemA, self->target->selectedColor);
    self->activeSlot = a1;
    if (a2 != NULL) {
        self->methods->playSound(self, 0);
    }
    self->methods->setState(self, 9);
}

s32 TaskCore__GetActiveSlot(TaskCore *self)
{
    return self->activeSlot;
}

void TaskCore__CreateSlotElements(TaskCore *self, void *desc, void *a2)
{
    char **list;
    s32 idx;
    s32 count;
    Unk64Elem **buf;

    list = ((SrcDesc *)desc)->unk18;
    idx = self->activeSlot;
    count = 0;
    while (*list++ != NULL) {
        count++;
    }
    buf = BMemPMgrAlloc(count * 4);
    self->itemLists[idx] = (void *)buf;
    self->slotCounts[idx] = ((SrcDesc *)desc)->unk4;
    self->itemCounts[idx] = count;

    list = ((SrcDesc *)desc)->unk18;
    if (*list != NULL) {
        do {
            s32 len = strlen(*list);

            *buf = New_Obj6EAC0(a2, len, *list);
            list++;
            buf++;
        } while (*list != NULL);
    }
}

void TaskCore__ReleaseSlotElements(TaskCore *self)
{
    ReleaseBasicClassArray(self->itemLists[self->activeSlot], self->itemCounts[self->activeSlot]);
    BMemPMgrFree(self->itemLists[self->activeSlot]);
}

void TaskCore__RefreshSlotView(TaskCore *self, void *a1, s32 a2)
{
    s32 idx;
    Unk64Elem **arr;
    s32 count;
    s32 counter;
    SlotPos pos;
    s32 i;

    idx = self->activeSlot;
    arr = (Unk64Elem **)self->itemLists[idx];
    {
        SlotEntry *target = (SlotEntry *)self->target->unk24[idx];

        count = self->itemCounts[idx];
        counter = target->savedCursor;
    }

    for (i = 0; i < count; i++) {
        (*arr)->methods->slot50(*arr);
        arr++;
    }

    pos = SLOT_POS((SlotEntry *)self->target->unk24[idx]);
    pos.y -= counter * 10;

    if (a2 != 0) {
        s32 buf[2];

        ((Unk68Obj *)self->listView)->methods->slot4C((Unk68Obj *)self->listView, self->unk14, &pos);
        buf[0] = 0x28;
        buf[1] = count * 12;
        ((Unk68Obj *)self->listView)->methods->slotC0((Unk68Obj *)self->listView, buf);
    } else {
        ((Unk68Obj *)self->listView)->methods->slot50((Unk68Obj *)self->listView);
    }

    arr = (Unk64Elem **)self->itemLists[idx];
    for (i = 0; i < count; i++) {
        (*arr)->methods->slot4C(*arr, a1, &pos);
        (*arr)->methods->slot60(*arr, a2);
        pos.y += 10;
        arr++;
    }

    arr = (Unk64Elem **)self->itemLists[idx];
    arr[counter]->methods->slot60(arr[counter], 1);
}

void TaskCore__BroadcastToSlotElements(TaskCore *self, void *a1)
{
    s32 idx = self->activeSlot;
    Unk64Elem **arr = (Unk64Elem **)self->itemLists[idx];
    s32 count = self->itemCounts[idx];
    s32 i;

    for (i = 0; i < count; i++) {
        Unk64Elem *elem = *arr;
        arr++;
        elem->methods->slotB8(elem, a1);
    }
}

void TaskCore__BeginElementScroll(TaskCore *self)
{
    s32 idx;
    Unk64Elem *elem;
    u8 *buf;

    if (self->inputMode != 1) {
        return;
    }
    idx = self->activeSlot;
    self->methods->refreshSlotView(self, self->unk14, 1);
    elem = ((Unk64Elem **)self->itemLists[idx])[self->slotCounts[idx]];
    buf = (u8 *)self->target->unk24[idx] + 8;
    elem->methods->slotB8(elem, buf);
    self->inputMode = 2;
    self->methods->setState(self, 14);
}

void TaskCore__CommitElementScroll(TaskCore *self)
{
    s32 idx;
    s32 counter;
    SlotPos pos;
    Unk64Elem **arr;
    s32 count;
    s32 i;

    if (self->inputMode != 2) {
        return;
    }
    idx = self->activeSlot;
    counter = self->slotCounts[idx];
    pos = SLOT_POS((SlotEntry *)self->target->unk24[idx]);
    pos.y -= counter * 10;

    arr = (Unk64Elem **)self->itemLists[idx];
    count = self->itemCounts[idx];
    for (i = 0; i < count; i++) {
        (*arr)->methods->slot60(*arr, 0);
        (*arr)->methods->slotBC(*arr, &pos);
        pos.y += 10;
        arr++;
    }

    {
        Unk64Elem *elem = ((Unk64Elem **)self->itemLists[idx])[counter];

        elem->methods->slot60(elem, 1);
        elem->methods->slotB8(elem, self->target->unselectedColor);
    }

    ((SlotEntry *)self->target->unk24[idx])->savedCursor = counter;

    ((Unk68Obj *)self->listView)->methods->slot50((Unk68Obj *)self->listView);

    self->inputMode = 1;
    self->methods->setState(self, 0x10);
}

void TaskCore__CancelElementScroll(TaskCore *self)
{
    s32 idx;
    s32 counter;
    Unk64Elem **arr;
    Unk64Elem *elem1;
    Unk64Elem *elem2;
    s32 newVal;

    if (self->inputMode != 2) {
        return;
    }
    idx = self->activeSlot;
    counter = self->slotCounts[idx];
    self->methods->refreshSlotView(self, self->unk14, 0);
    arr = (Unk64Elem **)self->itemLists[idx];
    elem1 = arr[counter];
    elem1->methods->slotB8(elem1, self->target->unselectedColor);
    newVal = ((s32 *)self->target->unk24[idx])[1];
    self->slotCounts[idx] = newVal;
    elem2 = arr[newVal];
    elem2->methods->slot60(elem2, 1);
    self->inputMode = 1;
    self->methods->setState(self, 17);
}

void TaskCore__AdvanceSlotCursor(TaskCore *self)
{
    s32 idx = self->activeSlot;
    s32 v = self->slotCounts[idx];

    v++;
    if (v >= self->itemCounts[idx]) {
        v = 0;
    }
    self->methods->setSlotCursor(self, v, 1);
}

void TaskCore__RetreatSlotCursor(TaskCore *self)
{
    s32 idx = self->activeSlot;
    s32 v = self->slotCounts[idx];

    v--;
    if (v < 0) {
        v = self->itemCounts[idx] - 1;
    }
    self->methods->setSlotCursor(self, v, 1);
}

void TaskCore__SetSlotCursor(TaskCore *self, s32 a1, void *a2)
{
    s32 idx;
    s32 counter;
    Unk64Elem **arr;
    Unk64Elem *elem1;
    Unk64Elem *elem2;
    u8 *buf;

    idx = self->activeSlot;
    counter = self->slotCounts[idx];
    arr = (Unk64Elem **)self->itemLists[idx];
    elem1 = arr[counter];
    elem2 = arr[a1];
    elem1->methods->slotB8(elem1, self->target->unselectedColor);
    buf = (u8 *)self->target->unk24[idx] + 8;
    elem2->methods->slotB8(elem2, buf);
    self->slotCounts[idx] = a1;
    if (a2 != NULL) {
        self->methods->playSound(self, 0);
    }
    self->methods->setState(self, 9);
}
