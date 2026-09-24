#include "common.h"
#include "code_2cc8c.h"

/*
 * code_2cc8c_b -- 20 of Obj86B60's own methods (class table gClass86B60Methods,
 * see include/code_2cc8c.h for the class's own provenance). Round 78 naming
 * pass (runner echo): every function in this file MATCHED before this round;
 * the pass renamed all 20 and five exclusively-owned fields, no stalls.
 *
 * What this slice of the class implements: a tab/slot picker with a
 * scrollable item list inside each tab. `self->activeSlot` selects the tab;
 * `self->slotElements[i]` is each tab's own representative widget (walked/
 * broadcast to by Obj86B60__BroadcastToSlots, switched by
 * Obj86B60__SetActiveSlot); `self->itemLists[idx]`/`self->itemCounts[idx]`
 * hold the item list WITHIN tab idx (built by Obj86B60__CreateSlotElements,
 * torn down by Obj86B60__ReleaseSlotElements, positioned/shown by
 * Obj86B60__RefreshSlotView through `self->listView`); `self->slotCounts[idx]`
 * is a ring cursor into that per-tab item list.
 * Obj86B60__BeginElementScroll/Obj86B60__CommitElementScroll/
 * Obj86B60__CancelElementScroll form a `self->unk3C` state-1<->2 trio that
 * opens interactive scrolling, then either commits the new cursor position
 * back into the target descriptor (`SlotEntry::savedCursor`) or cancels back
 * to the last-committed one; Obj86B60__AdvanceSlotCursor/
 * Obj86B60__RetreatSlotCursor step the cursor by one (wrapping) and forward
 * through Obj86B60__SetSlotCursor (vtable slot11C), which does the actual
 * old/new element highlight swap -- the same shape Obj86B60__SetActiveSlot
 * uses one level up, switching which TAB is active instead of which item.
 * Obj86B60__SetTarget/Obj86B60__ReleaseTarget are the constructor/teardown
 * pair for `self->unk4C` (the "target" descriptor, a cross-unit field --
 * see Unk4CObj's own comment in the header).
 */

/* SlotEntry's +0x10/+0x14 word pair, read as ONE 8-byte struct. Retail
 * copies it with a whole-struct assignment (lw/lw into two fresh
 * temporaries, sw/sw, then a RELOAD of .y before adjusting it) -- see
 * docs/match-reports/Obj86B60__CommitElementScroll.md, round 75. Local view: the shared
 * header still spells the pair as two s32 fields. */
typedef struct {
    s32 x;
    s32 y;
} SlotPos;

#define SLOT_POS(target) (*(SlotPos *)&(target)->unk10)

s32 Obj86B60__TickFadeColor(Obj86B60 *self)
{
    s32 c = 0x80 - (self->frameCounter * self->unk84);
    u8 buf[3];

    buf[0] = c;
    buf[1] = c;
    buf[2] = c;
    self->methods->slotE4(self, buf);
    self->unk78->methods->slotB8(self->unk78, 1, buf);
    return (u8)c >= 0x81;
}

void Obj86B60__SetSubHandle(Obj86B60 *self, const char *a1, Unk74Obj *a2)
{
    if (a1 != NULL) {
        if (self->unk70 != NULL) {
            self->unk74->methods->slot4(self->unk74);
        }
        self->unk74 = func_8003B39C(a1);
        self->unk74->methods->slot78(self->unk74);
        self->unk74->methods->slot5C(self->unk74);
    } else {
        self->unk74 = a2;
    }
    self->unk70 = a1;
}

void Obj86B60__SetTarget(Obj86B60 *self, Unk4CObj *a1)
{
    char **list;
    s32 count;
    s32 size;
    Unk64Elem **arr;
    Unk74Obj *handle;
    s32 i;

    self->unk4C = a1;
    if (a1 == NULL) {
        return;
    }

    list = a1->unk1C;
    count = 0;
    while (*list++ != NULL) {
        count++;
    }
    size = count * 4;
    arr = BMemPMgrAlloc(size);
    self->slotElements = arr;
    self->itemCounts = BMemPMgrAlloc(size);
    self->slotCounts = BMemPMgrAlloc(size);
    self->itemLists = BMemPMgrAlloc(size);
    self->slotCount = count;

    if (a1->unk0 != NULL) {
        handle = func_8003B39C(a1->unk0);
        handle->methods->slot78(handle);
        handle->methods->slot5C(handle);
    } else {
        handle = a1->unk4;
    }

    list = a1->unk1C;
    i = 0;
    if (*list != NULL) {
        do {
            void *extra = a1->unk24[i];
            s32 len = strlen(*list);

            *arr = New_Obj6EAC0(handle, len, *list);
            arr++;
            if (extra != NULL) {
                self->activeSlot = i;
                self->methods->slotF8(self, extra, handle);
            }
            list++;
            i++;
        } while (*list != NULL);
    }

    self->listView = New_ClassEAC0(D_8008A8E8, D_8008A8F0, 0);
    a1->unk4 = handle;
}

void Obj86B60__ReleaseTarget(Obj86B60 *self)
{
    Unk64Elem **arr;
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    if (self->unk4C->unk0 != NULL) {
        Unk74Obj *o = self->unk4C->unk4;
        o->methods->slot4(o);
    }
    self->listView->methods->slot4(self->listView);
    arr = self->slotElements;
    for (i = 0; i < self->slotCount; arr++) {
        Unk64Elem *elem;

        if (self->unk4C->unk24[i] != NULL) {
            self->activeSlot = i;
            self->methods->slotFC(self);
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

void Obj86B60__UpdateSlotElements(Obj86B60 *self, void *a1)
{
    Unk64Elem **arr;
    u8 *ptr;
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    arr = self->slotElements;
    ptr = self->unk4C->unk20;
    for (i = 0; i < self->slotCount; i++, arr++, ptr += 8) {
        if (self->unk4C->unk18[i] == NULL) {
            Unk64Elem *elem = *arr;

            elem->methods->slot4C(elem, a1, ptr);
            if (self->unk4C->unk24[i] != NULL) {
                self->activeSlot = i;
                self->methods->slot100(self, a1, 0);
            }
        } else {
            Unk64Elem *elem = *arr;

            elem->methods->slot50(elem);
        }
    }
}

void Obj86B60__BroadcastToSlots(Obj86B60 *self, void *a1)
{
    s32 origIdx;
    Unk64Elem **arr;
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    arr = self->slotElements;
    origIdx = self->activeSlot;
    for (i = 0; i < self->slotCount;) {
        Unk64Elem *elem = *arr;

        arr++;
        elem->methods->slotB8(elem, a1);
        if (self->unk4C->unk24[i] != NULL) {
            self->activeSlot = i;
            self->methods->slot104(self, a1);
        }
        i++;
        __asm__("");
    }
    self->activeSlot = origIdx;
}

void Obj86B60__FindNextFreeSlot(Obj86B60 *self)
{
    s32 i;

    if (self->unk4C == NULL) {
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
        if (self->unk4C->unk18[i++] != NULL) {
            continue;
        }
        i--;
        break;
    }
    self->methods->slotF0(self, i, 1);
}

void Obj86B60__FindPrevFreeSlot(Obj86B60 *self)
{
    s32 i;

    if (self->unk4C == NULL) {
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
        if (self->unk4C->unk18[i--] != NULL) {
            continue;
        }
        i++;
        break;
    }
    self->methods->slotF0(self, i, 1);
}

void Obj86B60__SetActiveSlot(Obj86B60 *self, s32 a1, void *a2)
{
    s32 idx;
    Unk64Elem *elemB;
    Unk64Elem *elemA;

    if (self->unk4C == NULL) {
        return;
    }
    idx = self->activeSlot;
    elemB = self->slotElements[idx];
    elemA = self->slotElements[a1];
    if (idx >= 0) {
        elemB->methods->slotB8(elemB, self->unk4C->unk10);
    }
    elemA->methods->slotB8(elemA, (u8 *)self->unk4C + 0x13);
    self->activeSlot = a1;
    if (a2 != NULL) {
        self->methods->slot70(self, 0);
    }
    self->methods->slot60(self, 9);
}

s32 Obj86B60__GetActiveSlot(Obj86B60 *self)
{
    return self->activeSlot;
}

void Obj86B60__CreateSlotElements(Obj86B60 *self, SrcDesc *a1, void *a2)
{
    char **list;
    s32 idx;
    s32 count;
    Unk64Elem **buf;

    list = a1->unk18;
    idx = self->activeSlot;
    count = 0;
    while (*list++ != NULL) {
        count++;
    }
    buf = BMemPMgrAlloc(count * 4);
    self->itemLists[idx] = (void *)buf;
    self->slotCounts[idx] = a1->unk4;
    self->itemCounts[idx] = count;

    list = a1->unk18;
    if (*list != NULL) {
        do {
            s32 len = strlen(*list);

            *buf = New_Obj6EAC0(a2, len, *list);
            list++;
            buf++;
        } while (*list != NULL);
    }
}

void Obj86B60__ReleaseSlotElements(Obj86B60 *self)
{
    ReleaseBasicClassArray(self->itemLists[self->activeSlot], self->itemCounts[self->activeSlot]);
    BMemPMgrFree(self->itemLists[self->activeSlot]);
}

void Obj86B60__RefreshSlotView(Obj86B60 *self, void *a1, s32 a2)
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
        SlotEntry *target = (SlotEntry *)self->unk4C->unk24[idx];

        count = self->itemCounts[idx];
        counter = target->savedCursor;
    }

    for (i = 0; i < count; i++) {
        (*arr)->methods->slot50(*arr);
        arr++;
    }

    pos = SLOT_POS((SlotEntry *)self->unk4C->unk24[idx]);
    pos.y -= counter * 10;

    if (a2 != 0) {
        s32 buf[2];

        self->listView->methods->slot4C(self->listView, self->unk14, &pos);
        buf[0] = 0x28;
        buf[1] = count * 12;
        self->listView->methods->slotC0(self->listView, buf);
    } else {
        self->listView->methods->slot50(self->listView);
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

void Obj86B60__BroadcastToSlotElements(Obj86B60 *self, void *a1)
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

void Obj86B60__BeginElementScroll(Obj86B60 *self)
{
    s32 idx;
    Unk64Elem *elem;
    u8 *buf;

    if (self->unk3C != 1) {
        return;
    }
    idx = self->activeSlot;
    self->methods->slot100(self, self->unk14, 1);
    elem = ((Unk64Elem **)self->itemLists[idx])[self->slotCounts[idx]];
    buf = (u8 *)self->unk4C->unk24[idx] + 8;
    elem->methods->slotB8(elem, buf);
    self->unk3C = 2;
    self->methods->slot60(self, 14);
}

void Obj86B60__CommitElementScroll(Obj86B60 *self)
{
    s32 idx;
    s32 counter;
    SlotPos pos;
    Unk64Elem **arr;
    s32 count;
    s32 i;

    if (self->unk3C != 2) {
        return;
    }
    idx = self->activeSlot;
    counter = self->slotCounts[idx];
    pos = SLOT_POS((SlotEntry *)self->unk4C->unk24[idx]);
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
        elem->methods->slotB8(elem, self->unk4C->unk10);
    }

    ((SlotEntry *)self->unk4C->unk24[idx])->savedCursor = counter;

    self->listView->methods->slot50(self->listView);

    self->unk3C = 1;
    self->methods->slot60(self, 0x10);
}

void Obj86B60__CancelElementScroll(Obj86B60 *self)
{
    s32 idx;
    s32 counter;
    Unk64Elem **arr;
    Unk64Elem *elem1;
    Unk64Elem *elem2;
    s32 newVal;

    if (self->unk3C != 2) {
        return;
    }
    idx = self->activeSlot;
    counter = self->slotCounts[idx];
    self->methods->slot100(self, self->unk14, 0);
    arr = (Unk64Elem **)self->itemLists[idx];
    elem1 = arr[counter];
    elem1->methods->slotB8(elem1, self->unk4C->unk10);
    newVal = ((s32 *)self->unk4C->unk24[idx])[1];
    self->slotCounts[idx] = newVal;
    elem2 = arr[newVal];
    elem2->methods->slot60(elem2, 1);
    self->unk3C = 1;
    self->methods->slot60(self, 17);
}

void Obj86B60__AdvanceSlotCursor(Obj86B60 *self)
{
    s32 idx = self->activeSlot;
    s32 v = self->slotCounts[idx];

    v++;
    if (v >= self->itemCounts[idx]) {
        v = 0;
    }
    self->methods->slot11C(self, v, 1);
}

void Obj86B60__RetreatSlotCursor(Obj86B60 *self)
{
    s32 idx = self->activeSlot;
    s32 v = self->slotCounts[idx];

    v--;
    if (v < 0) {
        v = self->itemCounts[idx] - 1;
    }
    self->methods->slot11C(self, v, 1);
}

void Obj86B60__SetSlotCursor(Obj86B60 *self, s32 a1, void *a2)
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
    elem1->methods->slotB8(elem1, self->unk4C->unk10);
    buf = (u8 *)self->unk4C->unk24[idx] + 8;
    elem2->methods->slotB8(elem2, buf);
    self->slotCounts[idx] = a1;
    if (a2 != NULL) {
        self->methods->slot70(self, 0);
    }
    self->methods->slot60(self, 9);
}
