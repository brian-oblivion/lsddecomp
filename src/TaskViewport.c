/* TaskViewport -- TaskCore's own methods from +0x058 to +0x0C0: the pad
 * dispatch and its five button handlers, the state machine (update and
 * setState), the frame bound, the sound call, the view callback and the
 * fade-in/fade-out pair. Each is the default for its slot in
 * gTaskCoreMethods, which StreamTask, TitleMenu and GraphRoom inherit or
 * override; include/TaskCore.h's banner describes the class and its states.
 * The slot and item-list methods that follow are in code_2cc8c_b.c.
 *
 * Input: while inputMode is not NONE, onPadEvent maps a press to a handler;
 * each handler plays the button tone (or moves a cursor) and reports what
 * happened as a state, which setState passes to the parents and then folds
 * back into ACTIVE. Fades: tickColorFade adds frameCounter * fadeRate to
 * baseColor and pushes the result to every slot widget and the BgLayer,
 * until it passes TASKCORE_FADE_FULL.
 */

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "TaskViewport.h"
#include "VabStreamObj.h"
#include "BgLayer.h"
#include "Pad.h"
#include "TimImage.h"
#include "Viewport.h"
#include "LightRig.h"
#include "FrameClock.h"

void TaskCore__OnPadEvent(TaskCore *self, BasicClass *sender, s32 event) {
    TaskCoreMethods *methods;

    methods = self->methods;
    if (self->inputMode != TASKCORE_INPUT_NONE) {
        /* MATCHING: the case order is retail's arm order (its jump table). */
        switch (event) {
            case PAD_EVENT_PRESSED + PAD_BUTTON_LUP:
                methods->onPadPrev(self);
                break;
            case PAD_EVENT_PRESSED + PAD_BUTTON_LDOWN:
                methods->onPadNext(self);
                break;
            case PAD_EVENT_PRESSED + PAD_BUTTON_START:
                methods->onPadStart(self);
                break;
            case PAD_EVENT_PRESSED + PAD_BUTTON_RDOWN:
                methods->onPadCancel(self);
                break;
            case PAD_EVENT_PRESSED + PAD_BUTTON_RRIGHT:
                methods->onPadConfirm(self);
                break;
        }
    }
}

void TaskCore__Update(TaskCore *self, BasicClass *sender, s32 event) {
    TaskCoreMethods *methods;

    methods = self->methods;
    Get_vtable_IntermediateBase()->update((IntermediateBase *)self, sender, event);
    if (self->inputMode != TASKCORE_INPUT_NONE) {
        u32 frames;

        /* MATCHING: frameCounter is loaded before frameBound. */
        frames = self->frameCounter;
        if ((u32)self->frameBound < frames) {
            methods->setState(self, TASKCORE_STATE_TIMED_OUT);
        }
    }
    switch (self->state) {
        case INTERMEDIATEBASE_STATE_START:
            methods->setState(self, TASKCORE_STATE_FADE_IN);
            break;
        case TASKCORE_STATE_FADE_IN:
            methods->tickFadeCallback(self);
            break;
        case TASKCORE_STATE_FADE_OUT:
            methods->tickFadeOutCallback(self);
            break;
        case TASKCORE_STATE_FADED_OUT:
            methods->setState(self, INTERMEDIATEBASE_STATE_STOP);
            break;
    }
}

void TaskCore__SetState(TaskCore *self, s32 state) {
    TaskCoreMethods *methods;

    methods = self->methods;
    Get_vtable_IntermediateBase()->setState((IntermediateBase *)self, state);
    switch (state) {
        case TASKCORE_STATE_ACTIVE:
            methods->broadcastToSlots(self, self->target->unselectedColor);
            methods->setActiveSlot(self, self->target->unk8, 0);
            self->frameCounter = 0;
            self->inputMode = TASKCORE_INPUT_CHOOSING_SLOT;
            break;
        case TASKCORE_STATE_TIMED_OUT:
            self->result = 1;
            methods->refreshViewValue(self);
            break;
        case TASKCORE_STATE_FADE_IN:
        case TASKCORE_STATE_FADE_OUT:
            self->frameCounter = 0;
            self->inputMode = TASKCORE_INPUT_NONE;
            break;
        case TASKCORE_STATE_FADED_OUT:
            self->frameCounter = 0;
            break;
        case TASKCORE_STATE_CURSOR_MOVED:
        case TASKCORE_STATE_START_PRESSED:
        case TASKCORE_STATE_SLOT_CONFIRMED:
        case TASKCORE_STATE_SCROLL_OPENED:
        case TASKCORE_STATE_ITEM_CONFIRMED:
        case TASKCORE_STATE_SCROLL_COMMITTED:
        case TASKCORE_STATE_SCROLL_CANCELLED:
            self->state = TASKCORE_STATE_ACTIVE;
            self->frameCounter = 0;
            switch (state) {
                case TASKCORE_STATE_SLOT_CONFIRMED:
                    methods->tick(self);
                    break;
                case TASKCORE_STATE_ITEM_CONFIRMED:
                    methods->commitElementScroll(self);
                    break;
                case TASKCORE_STATE_SCROLL_CANCELLED:
                    methods->cancelElementScroll(self);
                    break;
            }
            break;
    }
}

void TaskCore__SetFrameBound(TaskCore *self, s32 bound) {
    self->frameBound = bound;
    if (bound >= 0) {
        self->frameBound = bound * TASKCORE_FRAMES_PER_SECOND;
    }
}

void TaskCore__PlaySound(TaskCore *self, s32 tone) {
    VabStreamObj *sound;

    sound = (VabStreamObj *)self->sound;
    if (sound != NULL) {
        sound->methods->playTone(sound, tone, TASKCORE_TONE_VOLUME, TASKCORE_TONE_VOLUME);
    }
}

void TaskCore__OnPadStart(TaskCore *self) {
    if (self->target != NULL) {
        self->methods->playSound(self, TASKCORE_TONE_BUTTON);
        self->methods->setState(self, TASKCORE_STATE_START_PRESSED);
    }
}

void TaskCore__OnPadConfirm(TaskCore *self) {
    s32 state;

    if (self->target != NULL) {
        self->methods->playSound(self, TASKCORE_TONE_BUTTON);
        state = TASKCORE_STATE_ITEM_CONFIRMED;
        if (self->inputMode == TASKCORE_INPUT_CHOOSING_SLOT) {
            state = TASKCORE_STATE_SLOT_CONFIRMED;
        }
        self->methods->setState(self, state);
    }
}

void TaskCore__OnPadCancel(TaskCore *self) {
    if (self->target != NULL && self->inputMode != TASKCORE_INPUT_CHOOSING_SLOT) {
        self->methods->playSound(self, TASKCORE_TONE_BUTTON);
        self->methods->setState(self, TASKCORE_STATE_SCROLL_CANCELLED);
    }
}

void TaskCore__OnPadPrev(TaskCore *self) {
    void (*handler)(TaskCore *self); /* MATCHING: one call site, not one per arm */

    if (self->target == NULL) {
        return;
    }
    if (self->inputMode == TASKCORE_INPUT_CHOOSING_SLOT) {
        handler = self->methods->findPrevFreeSlot;
    } else if (self->inputMode == TASKCORE_INPUT_SCROLLING) {
        handler = self->methods->retreatSlotCursor;
    } else {
        return;
    }
    handler(self);
}

void TaskCore__OnPadNext(TaskCore *self) {
    void (*handler)(TaskCore *self); /* MATCHING: one call site, not one per arm */

    if (self->target == NULL) {
        return;
    }
    if (self->inputMode == TASKCORE_INPUT_CHOOSING_SLOT) {
        handler = self->methods->findNextFreeSlot;
    } else if (self->inputMode == TASKCORE_INPUT_SCROLLING) {
        handler = self->methods->advanceSlotCursor;
    } else {
        return;
    }
    handler(self);
}

void TaskCore__Tick(TaskCore *self) {
    TaskCoreTarget *target;
    s32 slot;

    target = self->target;
    slot = self->activeSlot;
    if (target->unk24[slot] != NULL) {
        self->methods->beginElementScroll(self);
    } else if (slot == target->exitSlot) {
        self->methods->refreshViewValue(self);
    }
}

void TaskCore__RefreshViewValue(TaskCore *self) {
    if (self->viewCallback != NULL) {
        self->viewCallback(self->viewCallbackCtx);
    }
    self->methods->setState(self, TASKCORE_STATE_FADE_OUT);
}

void TaskCore__SetCallback(TaskCore *self, void (*callback)(void *ctx), void *ctx) {
    self->viewCallback = callback;
    self->viewCallbackCtx = ctx;
}

void TaskCore__SetFadeCallbackEnabled(TaskCore *self, s32 enable) {
    TaskCoreMethods *methods;

    methods = self->methods; /* MATCHING: loaded before the switch, on every path */
    switch (enable) {
        case 0:
            self->fadeInCallback = NULL;
            break;
        case 1:
            self->fadeInCallback = methods->tickColorFade;
            break;
    }
}

void TaskCore__SetFadeOutCallbackEnabled(TaskCore *self, s32 enable) {
    TaskCoreMethods *methods;

    methods = self->methods; /* MATCHING: loaded before the switch, on every path */
    switch (enable) {
        case 0:
            self->fadeOutCallback = NULL;
            break;
        case 1:
            self->fadeOutCallback = methods->tickFadeColor;
            break;
    }
}

/* baseColor is where the fade-in starts; unk93 is what onDeinit clears the
 * screen to (TaskCore__Reset's defaults: black, black, 128 grey).
 * MATCHING: each colour is copied as a BgLayerRgb (signed bytes: lb/sb). */
void TaskCore__SetColors(TaskCore *self, u8 *base, u8 *clear, u8 *color96) {
    *(BgLayerRgb *)self->baseColor = *(BgLayerRgb *)base;
    *(BgLayerRgb *)self->unk93 = *(BgLayerRgb *)clear;
    *(BgLayerRgb *)self->unk96 = *(BgLayerRgb *)color96;
}

void TaskCore__SetFadeRate(TaskCore *self, s32 rate) {
    self->fadeRate = rate;
}

s32 TaskCore__TickFadeCallback(TaskCore *self) {
    s32 done;

    done = 1;
    if (self->fadeInCallback != NULL) {
        done = self->fadeInCallback(self);
    }
    if (done != 0) {
        self->methods->setState(self, TASKCORE_STATE_ACTIVE);
    }
    return done;
}

s32 TaskCore__TickColorFade(TaskCore *self) {
    s32 level;
    u8 color[3];

    level = self->frameCounter * self->fadeRate;
    /* MATCHING: level first; base + level swaps the addu operands. */
    color[0] = level + self->baseColor[0];
    color[1] = level + self->baseColor[1];
    color[2] = level + self->baseColor[2];
    self->methods->broadcastToSlots(self, color);
    self->bgLayer->methods->setColor(self->bgLayer, 1, (BgLayerRgb *)color);
    return (u8)level > TASKCORE_FADE_FULL;
}

s32 TaskCore__TickFadeOutCallback(TaskCore *self) {
    s32 done;

    done = 1;
    if (self->fadeOutCallback != NULL) {
        done = self->fadeOutCallback(self);
        if (done == 0) {
            goto epilogue; /* MATCHING: an early return here branches differently */
        }
    }
    self->methods->setState(self, TASKCORE_STATE_FADED_OUT);
epilogue:
    return done;
}

/* ---- merged from code_2cc8c_b ---- */

/*
 * TaskCore's menu methods, gTaskCoreMethods +0x0C4 to +0x11C (the class is
 * include/TaskCore.h): the fade-out tick, the sub handle, and a two-level
 * picker over the menu description in `target`, a TaskCoreTarget.
 *
 * The first level is the slots. setTarget makes one TextRow per
 * target->names entry (slotElements); findNextFreeSlot/findPrevFreeSlot step,
 * wrapping, to the next slot whose registrationSlots entry is NULL; and
 * setActiveSlot moves the highlight from unselectedColor to selectedColor.
 * A slot whose target->unk24 entry is non-NULL also has a list of items,
 * described by a SlotEntry: createSlotElements makes its rows (itemLists,
 * itemCounts) and starts its cursor (slotCounts) at savedCursor.
 *
 * The second level scrolls that list. beginElementScroll (inputMode
 * CHOOSING_SLOT to SCROLLING) shows every row with listView's frame behind
 * them and the cursor's row in cursorColor; advanceSlotCursor/
 * retreatSlotCursor move the cursor, wrapping, through setSlotCursor;
 * commitElementScroll keeps the cursor in savedCursor and leaves only its
 * row shown, cancelElementScroll goes back to savedCursor. The list is laid
 * out so that the cursor's row sits at SlotEntry::pos.
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

/* The scrolled list's layout (RefreshSlotView, CommitElementScroll): item
 * rows are SLOT_LIST_ROW_PITCH apart, and listView, the frame behind them,
 * is SLOT_LIST_FRAME_WIDTH wide and SLOT_LIST_FRAME_ROW_HEIGHT tall a row. */
#define SLOT_LIST_ROW_PITCH 10
#define SLOT_LIST_FRAME_WIDTH 40
#define SLOT_LIST_FRAME_ROW_HEIGHT 12

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
    s32 level = TASKCORE_FADE_FULL - (self->frameCounter * self->fadeRate);
    u8 color[3];

    color[0] = level;
    color[1] = level;
    color[2] = level;
    self->methods->broadcastToSlots(self, color);
    self->bgLayer->methods->setColor(self->bgLayer, 1, (BgLayerRgb *)color);
    return (u8)level > TASKCORE_FADE_FULL;
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

void TaskCore__SetTarget(TaskCore *self, TaskCoreTarget *target) {
    char **names;
    s32 count;
    s32 size;
    TextRow **widget;
    TimImage *texture;
    s32 i;

    self->target = target;
    if (target == NULL) {
        return;
    }

    names = target->names;
    count = 0;
    while (*names++ != NULL) {
        count++;
    }
    size = count * sizeof(void *);
    widget = BMemPMgrAlloc(size);
    self->slotElements = (BasicClass **)widget;
    self->itemCounts = BMemPMgrAlloc(size);
    self->slotCounts = BMemPMgrAlloc(size);
    self->itemLists = BMemPMgrAlloc(size);
    self->slotCount = count;

    if (target->path != NULL) {
        texture = New_TimImage((char *)target->path);
        ((TimImageUploadFn)texture->methods->processBuffer)(texture);
        texture->methods->freeBuffer(texture);
    } else {
        texture = (TimImage *)target->handle;
    }

    names = target->names;
    i = 0;
    if (*names != NULL) {
        do {
            void *itemList = target->unk24[i];
            s32 len = strlen(*names);

            *widget = New_TextRow(texture, len, *names);
            widget++;
            if (itemList != NULL) {
                self->activeSlot = i;
                self->methods->createSlotElements(self, itemList, texture);
            }
            names++;
            i++;
        } while (*names != NULL);
    }

    self->listView = New_BoxFill(sListViewSize, &sListViewColor, 0);
    target->handle = (BasicClass *)texture;
}

void TaskCore__ReleaseTarget(TaskCore *self) {
    TextRow **widget;
    s32 i;

    if (self->target == NULL) {
        return;
    }
    if (self->target->path != NULL) {
        TimImage *texture = (TimImage *)self->target->handle;
        texture->methods->release(texture);
    }
    self->listView->methods->release(self->listView);
    widget = (TextRow **)self->slotElements;
    for (i = 0; i < self->slotCount; widget++) {
        TextRow *row;

        if (self->target->unk24[i] != NULL) {
            self->activeSlot = i;
            self->methods->releaseSlotElements(self);
        }
        row = *widget;
        row->methods->release(row);
        i++;
    }
    BMemPMgrFree(self->itemLists);
    BMemPMgrFree(self->slotCounts);
    BMemPMgrFree(self->itemCounts);
    BMemPMgrFree(self->slotElements);
}

void TaskCore__UpdateSlotElements(TaskCore *self, void *parent) {
    TextRow **widget;
    SlotPos *position;
    s32 i;

    if (self->target == NULL) {
        return;
    }
    widget = (TextRow **)self->slotElements;
    position = (SlotPos *)self->target->externalRecords;
    for (i = 0; i < self->slotCount; i++, widget++, position++) {
        if (self->target->registrationSlots[i] == NULL) {
            TextRow *row = *widget;

            row->methods->attachToParent(row, parent, (LongVec3 *)position);
            if (self->target->unk24[i] != NULL) {
                self->activeSlot = i;
                self->methods->refreshSlotView(self, parent, 0);
            }
        } else {
            TextRow *row = *widget;

            row->methods->detachFromParent(row);
        }
    }
}

void TaskCore__BroadcastToSlots(TaskCore *self, void *color) {
    s32 savedSlot;
    TextRow **widget;
    s32 i;

    if (self->target == NULL) {
        return;
    }
    widget = (TextRow **)self->slotElements;
    savedSlot = self->activeSlot;
    for (i = 0; i < self->slotCount;) {
        TextRow *row = *widget;

        widget++;
        row->methods->setColor(row, color);
        if (self->target->unk24[i] != NULL) {
            self->activeSlot = i;
            self->methods->broadcastToSlotElements(self, color);
        }
        i++;
        /* MATCHING: without it GCC moves i++ into the slotCount load's delay slot. */
        __asm__("");
    }
    self->activeSlot = savedSlot;
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

void TaskCore__SetActiveSlot(TaskCore *self, s32 slot, void *withSound) {
    s32 prev;
    TextRow *prevWidget;
    TextRow *nextWidget;

    if (self->target == NULL) {
        return;
    }
    prev = self->activeSlot;
    prevWidget = ((TextRow **)self->slotElements)[prev];
    nextWidget = ((TextRow **)self->slotElements)[slot];
    if (prev >= 0) {
        prevWidget->methods->setColor(prevWidget, (SpriteRgb *)self->target->unselectedColor);
    }
    nextWidget->methods->setColor(nextWidget, (SpriteRgb *)self->target->selectedColor);
    self->activeSlot = slot;
    if (withSound != NULL) {
        self->methods->playSound(self, TASKCORE_TONE_CURSOR);
    }
    self->methods->setState(self, TASKCORE_STATE_CURSOR_MOVED);
}

s32 TaskCore__GetActiveSlot(TaskCore *self) {
    return self->activeSlot;
}

void TaskCore__CreateSlotElements(TaskCore *self, void *desc, void *texture) {
    char **names;
    s32 slot;
    s32 count;
    TextRow **item;

    names = ((SrcDesc *)desc)->itemNames;
    slot = self->activeSlot;
    count = 0;
    while (*names++ != NULL) {
        count++;
    }
    item = BMemPMgrAlloc(count * sizeof(TextRow *));
    self->itemLists[slot] = (void *)item;
    self->slotCounts[slot] = ((SrcDesc *)desc)->savedCursor;
    self->itemCounts[slot] = count;

    names = ((SrcDesc *)desc)->itemNames;
    if (*names != NULL) {
        do {
            s32 len = strlen(*names);

            *item = New_TextRow(texture, len, *names);
            names++;
            item++;
        } while (*names != NULL);
    }
}

void TaskCore__ReleaseSlotElements(TaskCore *self) {
    ReleaseBasicClassArray(self->itemLists[self->activeSlot], self->itemCounts[self->activeSlot]);
    BMemPMgrFree(self->itemLists[self->activeSlot]);
}

void TaskCore__RefreshSlotView(TaskCore *self, void *parent, s32 show) {
    s32 slot;
    TextRow **item;
    s32 count;
    s32 cursor;
    SlotPos pos;
    s32 i;

    slot = self->activeSlot;
    item = (TextRow **)self->itemLists[slot];
    {
        SlotEntry *entry = (SlotEntry *)self->target->unk24[slot];

        count = self->itemCounts[slot];
        cursor = entry->savedCursor;
    }

    for (i = 0; i < count; i++) {
        (*item)->methods->detachFromParent(*item);
        item++;
    }

    pos = ((SlotEntry *)self->target->unk24[slot])->pos;
    pos.y -= cursor * SLOT_LIST_ROW_PITCH;

    if (show != 0) {
        s32 size[2];

        ((BoxFillAttachToParentFn)((BoxFill *)self->listView)->methods->attachToParent)(
            (BoxFill *)self->listView, (SceneNode *)self->unk14, (BoxFillPos *)&pos);
        size[0] = SLOT_LIST_FRAME_WIDTH;
        size[1] = count * SLOT_LIST_FRAME_ROW_HEIGHT;
        ((BoxFill *)self->listView)->methods->setSize((BoxFill *)self->listView, size);
    } else {
        ((BoxFill *)self->listView)->methods->detachFromParent((BoxFill *)self->listView);
    }

    item = (TextRow **)self->itemLists[slot];
    for (i = 0; i < count; i++) {
        (*item)->methods->attachToParent(*item, parent, (LongVec3 *)&pos);
        (*item)->methods->setDisplay(*item, show);
        pos.y += SLOT_LIST_ROW_PITCH;
        item++;
    }

    item = (TextRow **)self->itemLists[slot];
    item[cursor]->methods->setDisplay(item[cursor], 1);
}

void TaskCore__BroadcastToSlotElements(TaskCore *self, void *color) {
    s32 slot = self->activeSlot;
    TextRow **item = (TextRow **)self->itemLists[slot];
    s32 count = self->itemCounts[slot];
    s32 i;

    for (i = 0; i < count; i++) {
        TextRow *row = *item;
        item++;
        row->methods->setColor(row, color);
    }
}

void TaskCore__BeginElementScroll(TaskCore *self) {
    s32 slot;
    TextRow *item;
    SpriteRgb *cursorColor;

    if (self->inputMode != TASKCORE_INPUT_CHOOSING_SLOT) {
        return;
    }
    slot = self->activeSlot;
    self->methods->refreshSlotView(self, self->unk14, 1);
    item = ((TextRow **)self->itemLists[slot])[self->slotCounts[slot]];
    cursorColor = &((SlotEntry *)self->target->unk24[slot])->cursorColor;
    item->methods->setColor(item, cursorColor);
    self->inputMode = TASKCORE_INPUT_SCROLLING;
    self->methods->setState(self, TASKCORE_STATE_SCROLL_OPENED);
}

void TaskCore__CommitElementScroll(TaskCore *self) {
    s32 slot;
    s32 cursor;
    SlotPos pos;
    TextRow **item;
    s32 count;
    s32 i;

    if (self->inputMode != TASKCORE_INPUT_SCROLLING) {
        return;
    }
    slot = self->activeSlot;
    cursor = self->slotCounts[slot];
    pos = ((SlotEntry *)self->target->unk24[slot])->pos;
    pos.y -= cursor * SLOT_LIST_ROW_PITCH;

    item = (TextRow **)self->itemLists[slot];
    count = self->itemCounts[slot];
    for (i = 0; i < count; i++) {
        (*item)->methods->setDisplay(*item, 0);
        (*item)->methods->setPosition(*item, (ScreenSpritePos *)&pos);
        pos.y += SLOT_LIST_ROW_PITCH;
        item++;
    }

    {
        TextRow *row = ((TextRow **)self->itemLists[slot])[cursor];

        row->methods->setDisplay(row, 1);
        row->methods->setColor(row, (SpriteRgb *)self->target->unselectedColor);
    }

    ((SlotEntry *)self->target->unk24[slot])->savedCursor = cursor;

    ((BoxFill *)self->listView)->methods->detachFromParent((BoxFill *)self->listView);

    self->inputMode = TASKCORE_INPUT_CHOOSING_SLOT;
    self->methods->setState(self, TASKCORE_STATE_SCROLL_COMMITTED);
}

void TaskCore__CancelElementScroll(TaskCore *self) {
    s32 slot;
    s32 cursor;
    TextRow **items;
    TextRow *prevItem;
    TextRow *savedItem;
    s32 saved;

    if (self->inputMode != TASKCORE_INPUT_SCROLLING) {
        return;
    }
    slot = self->activeSlot;
    cursor = self->slotCounts[slot];
    self->methods->refreshSlotView(self, self->unk14, 0);
    items = (TextRow **)self->itemLists[slot];
    prevItem = items[cursor];
    prevItem->methods->setColor(prevItem, (SpriteRgb *)self->target->unselectedColor);
    saved = ((SlotEntry *)self->target->unk24[slot])->savedCursor;
    self->slotCounts[slot] = saved;
    savedItem = items[saved];
    savedItem->methods->setDisplay(savedItem, 1);
    self->inputMode = TASKCORE_INPUT_CHOOSING_SLOT;
    self->methods->setState(self, TASKCORE_STATE_SCROLL_CANCELLED);
}

void TaskCore__AdvanceSlotCursor(TaskCore *self) {
    s32 slot = self->activeSlot;
    s32 cursor = self->slotCounts[slot];

    cursor++;
    if (cursor >= self->itemCounts[slot]) {
        cursor = 0;
    }
    self->methods->setSlotCursor(self, cursor, 1);
}

void TaskCore__RetreatSlotCursor(TaskCore *self) {
    s32 slot = self->activeSlot;
    s32 cursor = self->slotCounts[slot];

    cursor--;
    if (cursor < 0) {
        cursor = self->itemCounts[slot] - 1;
    }
    self->methods->setSlotCursor(self, cursor, 1);
}

void TaskCore__SetSlotCursor(TaskCore *self, s32 cursor, void *withSound) {
    s32 slot;
    s32 prev;
    TextRow **items;
    TextRow *prevItem;
    TextRow *nextItem;
    SpriteRgb *cursorColor;

    slot = self->activeSlot;
    prev = self->slotCounts[slot];
    items = (TextRow **)self->itemLists[slot];
    prevItem = items[prev];
    nextItem = items[cursor];
    prevItem->methods->setColor(prevItem, (SpriteRgb *)self->target->unselectedColor);
    cursorColor = &((SlotEntry *)self->target->unk24[slot])->cursorColor;
    nextItem->methods->setColor(nextItem, cursorColor);
    self->slotCounts[slot] = cursor;
    if (withSound != NULL) {
        self->methods->playSound(self, TASKCORE_TONE_CURSOR);
    }
    self->methods->setState(self, TASKCORE_STATE_CURSOR_MOVED);
}

/* ---- merged from code_2cc8c_c ---- */

/* code_2cc8c_c -- IntermediateBase's methods, the start of Viewport's, and
 * three accessors ahead of them.
 *
 * TaskCore__GetActiveSlotCount, Get_vtable_TaskCore and
 * GetDefaultMovieFrame come first: one TaskCore method and two plain
 * accessors for data used far more widely (code_2c054.c, class_3bb8c_t.c).
 *
 * Then IntermediateBase (include/IntermediateBase.h, whose banner says what
 * the class does): the ctor, onNotify's split by the sender's root class,
 * the counters, init and deinit, the VSync handler that ticks the frame
 * clock and polls the pad, setState with its two state hooks (which start
 * and stop the DrawSystem), and the table getter.
 *
 * Last, Viewport (include/Viewport.h): New_Viewport, the ctor, finalize, and
 * the addChild/removeChild/removeAllChildren overrides, which cache a
 * DrawSystem child and a SceneNode child (the view node, whose coord2 the
 * reference view hangs from) by root class id. code_2cc8c_d.c holds the rest
 * of Viewport's table.
 */


s32 TaskCore__GetActiveSlotCount(TaskCore *self) {
    return self->slotCounts[self->activeSlot];
}

TaskCoreMethods *Get_vtable_TaskCore(void) {
    return &gTaskCoreMethods;
}

/* The default movie frame, {640, 0, 320, 240}: StreamTask's default initData
 * and the rect TaskCore__OnInit clears (code_2c054.h). */
extern DrawRect gDefaultMovieFrame;

DrawRect *GetDefaultMovieFrame(void) {
    return &gDefaultMovieFrame;
}

void IntermediateBase__IntermediateBase(IntermediateBase *self) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_IntermediateBase();
    self->methods->resetCounters(self);
}

void IntermediateBase__OnNotify(IntermediateBase *self, BasicClass *sender, s32 event) {
    s32 rootClass;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);
    rootClass = sender->methods->header & CLASS_ID_ROOT_MASK;
    if (rootClass == DRAWSYSTEM_CLASS_ID) {
        self->methods->onTag1Notify(self, sender, event);
    } else if (rootClass == PAD_CLASS_ID) {
        self->methods->onPadEvent(self, sender, event);
    } else if (rootClass == FRAMECLOCK_CLASS_ID) {
        self->methods->update(self, sender, event);
    }
}

void IntermediateBase__ResetCounters(IntermediateBase *self) {
    self->frameCounter = 0;
    self->state = 0;
}

void IntermediateBase__Init(IntermediateBase *self, IntermediateBaseInitArgs *args, s32 mode) {
    IntermediateBaseMethods *methods;
    BasicClass *viewport;

    methods = self->methods;
    if (args->frameClock != NULL) {
        self->unk10 = args->frameClock;
    } else {
        self->unk10 = (BasicClass *)New_FrameClock();
    }
    if (args->lightRig != NULL) {
        self->unk14 = args->lightRig;
    } else {
        self->unk14 = (BasicClass *)New_LightRig();
    }
    if (args->viewport != NULL) {
        self->viewport = args->viewport;
    } else {
        self->viewport = (BasicClass *)New_Viewport();
    }
    self->initArgs = args;
    viewport = self->viewport;
    methods->addChild(self, args->drawSystem);
    methods->addChild(self, args->pad);
    methods->addChild(self, self->unk10);
    methods->onInit(self, 0, 0, 0);
    self->initMode = mode;
    if (mode == 0) {
        viewport->methods->addChild(viewport, args->drawSystem);
        viewport->methods->addChild(viewport, self->unk10);
        self->unk14->methods->addChild(self->unk14, self->unk10);
        methods->setState(self, 2);
        methods->deinit(self);
    }
}

void IntermediateBase__Deinit(IntermediateBase *self) {
    IntermediateBaseMethods *methods;
    BasicClass *viewport;

    methods = self->methods;
    methods->onDeinit(self);
    viewport = self->viewport;
    if (self->initMode == 0) {
        self->unk14->methods->removeChild(self->unk14, self->unk10);
        viewport->methods->removeChild(viewport, self->unk10);
        viewport->methods->removeChild(viewport, self->initArgs->drawSystem);
    }
    methods->removeChild(self, self->unk10);
    methods->removeChild(self, self->initArgs->pad);
    methods->removeChild(self, self->initArgs->drawSystem);
    if (self->initArgs->viewport != viewport) {
        self->viewport = viewport->methods->release(viewport);
    }
    if (self->initArgs->lightRig != self->unk14) {
        self->unk14 = self->unk14->methods->release(self->unk14);
    }
    if (self->initArgs->frameClock != self->unk10) {
        self->unk10 = self->unk10->methods->release(self->unk10);
    }
}

void IntermediateBase__OnTag1Notify(IntermediateBase *self, BasicClass *sender, s32 event) {
    Pad *pad;

    if (event == DRAWSYSTEM_EVENT_VSYNC) {
        ((FrameClock *)self->unk10)->methods->tick((FrameClock *)self->unk10);
        pad = (Pad *)self->initArgs->pad;
        pad->methods->updateMasks(pad);
        pad->methods->dispatchEvents(pad);
    }
}

void IntermediateBase__IncrementFrameCounter(IntermediateBase *self) {
    self->frameCounter++;
}

/* MATCHING: the two state hooks are ONE call through a slot picked per arm;
 * two direct calls give self a sixth reference and swap $s0/$s1. */
void IntermediateBase__SetState(IntermediateBase *self, s32 state) {
    IntermediateBaseMethods *methods;
    void (*fn)(IntermediateBase *);

    methods = self->methods;
    self->state = state;
    methods->notifyParents(self, state);
    if (state == 2) {
        fn = methods->onState2;
    } else if (state == 3) {
        fn = methods->onState3;
    } else {
        return;
    }
    fn(self);
}

void IntermediateBase__OnState2(IntermediateBase *self) {
    DrawSystem *drawSystem;

    self->frameCounter = 0;
    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    drawSystem->methods->start(drawSystem);
}

void IntermediateBase__OnState3(IntermediateBase *self) {
    DrawSystem *drawSystem;

    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    drawSystem->methods->stop(drawSystem);
    self->frameCounter = 0;
}

IntermediateBaseMethods *Get_vtable_IntermediateBase(void) {
    return &gIntermediateBaseMethods;
}

Viewport *New_Viewport(void) {
    Viewport *self;

    self = BMemPMgrAlloc(sizeof(Viewport));
    if (self != NULL) {
        GetViewportMethods()->ctor(self);
        return self;
    }
    return NULL;
}

void Viewport__Viewport(Viewport *self) {
    SceneNode *fadeBox;

    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetViewportMethods();
    self->drawSystem = NULL;
    self->viewNode = NULL;
    self->sceneRoot = New_SceneNode();
    fadeBox = (SceneNode *)New_FadeBox(gViewportFadeBoxSize, 0, 0);
    self->fadeBox = fadeBox;
    fadeBox->methods->attachToParent(fadeBox, self->sceneRoot, (LongVec3 *)gFadeBoxAttachPos);
    self->methods->initDefaults(self);
}

void Viewport__Finalize(Viewport *self) {
    self->methods->deinitOt(self);
    self->methods->detachViewChild(self);
    self->sceneRoot->methods->release(self->sceneRoot);
    self->methods->setFadeBox(self, 0);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void Viewport__AddChild(Viewport *self, BasicClass *child) {
    s32 rootClass;

    Get_vtable_BasicClass()->addChild((BasicClass *)self, child);
    rootClass = child->methods->header & CLASS_ID_ROOT_MASK;
    if (rootClass == SCENENODE_CLASS_ID) {
        self->viewNode = (SceneNode *)child;
        self->refView.super = ((SceneNode *)child)->coord2;
    } else if (rootClass == DRAWSYSTEM_CLASS_ID) {
        self->drawSystem = (DrawSystem *)child;
    }
}

void Viewport__RemoveChild(Viewport *self, BasicClass *child) {
    s32 rootClass;

    rootClass = child->methods->header & CLASS_ID_ROOT_MASK;
    if (rootClass == SCENENODE_CLASS_ID) {
        self->refView.super = NULL;
        self->viewNode = NULL;
    } else if (rootClass == DRAWSYSTEM_CLASS_ID) {
        self->drawSystem = NULL;
    }
    Get_vtable_BasicClass()->removeChild((BasicClass *)self, child);
}

/* Viewport's removeAllChildren override (+0x018 of gViewportMethods and of
 * gNodeGuardedViewportMethods): clears the three child caches AddChild fills, then
 * the base. */
void Viewport__RemoveAllChildren(Viewport *self) {
    self->refView.super = NULL;
    self->viewNode = NULL;
    self->drawSystem = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

/* ---- merged from code_2cc8c_d ---- */

/*
 * code_2cc8c_d -- Viewport's methods from onNotify (+0x038) to the end of
 * gViewportMethods (include/Viewport.h, whose banner says what the class
 * is); the ctor, finalize and the child overrides are in TaskViewport.c,
 * drawNode in ViewportDraw.c. In table order:
 *  - onNotify and its two per-sender handlers: a FrameClock event runs
 *    update, the DrawSystem's VSync event runs flip;
 *  - initDefaults and the field setters (screen size, OT length, packet
 *    count and size, which only take before initOt, projection, light
 *    mode, clear and far colours, fog near); four slots hold empty bodies;
 *  - attachViewChild/detachViewChild and the setters of the GsRVIEW2 that
 *    GsSetRefView2 takes (viewpoint, reference point, twist);
 *  - initOt/deinitOt: one allocation holding both halves of the
 *    double-buffered GsOT, each a header, its tags and its packet area;
 *  - update, per frame: projection, near clip, light mode and fog, the
 *    reference view, then this half's packet area and cleared OT, and the
 *    scene drawn into it;
 *  - flip: takes the buffer index from the DrawSystem, swaps, sorts the
 *    clear into the OT and draws it;
 *  - setFadeBox/getFadeBox and two flag setters.
 * Then the table getter, GetRootNode (update's helper) and Sony's
 * GsSetProjection.
 *
 * refView is Viewport.h's ViewportRefView, GsRVIEW2's layout with vp and vr
 * as vectors (its comment says why), hence the (GsRVIEW2 *) cast at
 * GsSetRefView2.
 */

/* Forwards to the base onNotify, then dispatches on the sender's class-id
 * nibble: a FrameClock to onNotifyTag5, the DrawSystem to onNotifyTag1,
 * anything else nowhere. */
void Viewport__OnNotify(Viewport *self, BasicClass *sender, s32 event) {
    s32 tag;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);

    tag = sender->methods->header & CLASS_ID_ROOT_MASK;
    if (tag == FRAMECLOCK_CLASS_ID) {
        self->methods->onNotifyTag5(self, sender, event);
    } else if (tag == DRAWSYSTEM_CLASS_ID) {
        self->methods->onNotifyTag1(self, sender, event);
    }
}

extern s32 gDefaultViewportWidth;
extern s32 gDefaultViewportHeight;
extern ViewportRgb gDefaultViewportColor;
/* MATCHING: a second name for the same symbol, so cc1 cannot share one
 * address computation between the two copies; retail loads it twice. */
extern ViewportRgb gDefaultViewportColorAlias __asm__("gDefaultViewportColor");

/* InitDefaults' values. The OT has 1 << VIEWPORT_DEFAULT_OT_LENGTH (8192)
 * tags; with the near and far defaults Update's zDiv comes out 8. The packet
 * size is also TaskCore's reset value for what it passes to setPacketSize. */
#define VIEWPORT_DEFAULT_OT_LENGTH 13
#define VIEWPORT_DEFAULT_MAX_PACKETS 2000
#define VIEWPORT_DEFAULT_PACKET_SIZE 64
#define VIEWPORT_DEFAULT_PROJ_H 256 /* GsSetProjection's h */
#define VIEWPORT_DEFAULT_NEAR_Z 10
#define VIEWPORT_DEFAULT_FAR_Z 65536
#define VIEWPORT_DEFAULT_FOG_NEAR 20000

/* Every field's default; both colours start black (gDefaultViewportColor). */
void Viewport__InitDefaults(Viewport *self) {
    self->clockEventCount = 0;
    self->otReady = 0;
    /* MATCHING: without it both loads hoist above the two zero stores. */
    __asm__("");
    self->screenSize.width = gDefaultViewportWidth;
    self->screenSize.height = gDefaultViewportHeight;
    /* MATCHING: without it both stores sink below the constant stores. */
    __asm__("");
    self->otLength = VIEWPORT_DEFAULT_OT_LENGTH;
    self->maxPackets = VIEWPORT_DEFAULT_MAX_PACKETS;
    self->packetSize = VIEWPORT_DEFAULT_PACKET_SIZE;
    self->projH = VIEWPORT_DEFAULT_PROJ_H;
    self->nearZ = VIEWPORT_DEFAULT_NEAR_Z;
    self->farZ = VIEWPORT_DEFAULT_FAR_Z;
    self->lightMode = GsLMODE_NORMAL;
    self->fogNear = VIEWPORT_DEFAULT_FOG_NEAR;
    self->farColor = gDefaultViewportColor;
    self->clearColor = gDefaultViewportColorAlias;
    self->extraSwap = 0;
    self->drawEnabled = 1;
}

void Viewport__SetScreenSize(Viewport *self, ViewportSize *size) {
    self->screenSize = *size;
}

void Viewport__SetOtLength(Viewport *self, s32 length) {
    self->otLength = length;
}

/* Takes only before InitOt, which sizes the packet areas from it. */
void Viewport__SetMaxPackets(Viewport *self, s32 maxPackets) {
    if (self->otReady == 0) {
        self->maxPackets = maxPackets;
    }
}

/* The same guard, for the packet size. */
void Viewport__SetPacketSize(Viewport *self, s32 size) {
    if (self->otReady == 0) {
        self->packetSize = size;
    }
}

void Viewport__SetProjection(Viewport *self, s32 h) {
    self->projH = h;
}

void Viewport__NoOpSlot58(void) {}

void Viewport__NoOpSlot5C(void) {}

void Viewport__SetLightMode(Viewport *self, s32 mode) {
    self->lightMode = mode;
}

void Viewport__SetClearColor(Viewport *self, ViewportRgb *color) {
    self->clearColor = *color;
}

void Viewport__SetFarColor(Viewport *self, ViewportRgb *color) {
    self->farColor = *color;
}

void Viewport__SetFogNear(Viewport *self, s32 fogNear) {
    self->fogNear = fogNear;
}

/* One-time init, skipped once a view node is set: adds `node` as a child
 * (addChild caches it as viewNode), sets the viewpoint, reference point and
 * twist (gDefaultViewTwist when `twist` is NULL), then hands refView to
 * GsSetRefView2. */
void Viewport__AttachViewChild(Viewport *self, BasicClass *node, LongVec3 *vp, LongVec3 *vr,
                               Ratio16 *twist) {
    ViewportMethods *methods = self->methods;

    if (self->viewNode != NULL) {
        return;
    }
    methods->addChild(self, node);
    methods->setViewPoint(self, vp);
    methods->setViewRef(self, vr);
    methods->setTwist(self, twist != NULL ? twist : &gDefaultViewTwist);
    GsSetRefView2((GsRVIEW2 *)&self->refView);
}

/* Teardown counterpart to Viewport__AttachViewChild: removes the view node
 * as a child, if there is one. */
void Viewport__DetachViewChild(Viewport *self) {
    if (self->viewNode != NULL) {
        self->methods->removeChild(self, (BasicClass *)self->viewNode);
    }
}

/* Copies vp into refView.vp, only while a view node is set. */
void Viewport__SetViewPoint(Viewport *self, LongVec3 *vp) {
    if (self->viewNode != NULL) {
        self->refView.vp = *vp;
    }
}

/* Sibling of Viewport__SetViewPoint: copies vr into refView.vr, guarded the
 * same way. */
void Viewport__SetViewRef(Viewport *self, LongVec3 *vr) {
    if (self->viewNode != NULL) {
        self->refView.vr = *vr;
    }
}

void Viewport__SetTwist(Viewport *self, Ratio16 *twist) {
    s32 whole, rem, frac;

    if (self->viewNode != NULL) {
        whole = twist->num / twist->den;
        rem = twist->num % twist->den;
        frac = rem * ONE / twist->den;
        self->refView.rz = whole * ONE + frac;
    }
}

void Viewport__NoOpSlot84(void) {}

void Viewport__NoOpSlot88(void) {}

/* One-time allocation of the two ordering tables. Each half of the buffer
 * is a GsOT header, its 1 << otLength tags, then packetSize * maxPackets
 * bytes of packet area. */
void Viewport__InitOt(Viewport *self) {
    s32 size;
    s32 buf;
    /* MATCHING: written inline, the constant reassociates out of the sum and
     * the final addu/addiu pair swaps. */
    s32 hdrSize = sizeof(GsOT);

    if (self->otReady != 0) {
        return;
    }

    size = (sizeof(GsOT_TAG) << self->otLength) + (self->packetSize * self->maxPackets + hdrSize);

    buf = (s32)BMemPMgrAlloc(size * 2);
    if (buf == 0) {
        return;
    }

    self->ot[0] = (GsOT *)buf;
    self->otTags[0] = (GsOT_TAG *)(buf + sizeof(GsOT));
    self->workBase[0] = (PACKET *)self->otTags[0] + (sizeof(GsOT_TAG) << self->otLength);

    self->ot[1] = (GsOT *)((PACKET *)self->ot[0] + size);
    self->otTags[1] = (GsOT_TAG *)((PACKET *)self->otTags[0] + size);
    self->workBase[1] = self->workBase[0] + size;

    self->ot[0]->length = self->otLength;
    self->ot[0]->org = self->otTags[0];

    self->ot[1]->length = self->otLength;
    self->ot[1]->org = self->otTags[1];

    GsClearOt(0, 0, self->ot[0]);
    GsClearOt(0, 0, self->ot[1]);

    self->otReady = 1;
    self->otIndex = 0;
}

/* Teardown counterpart to Viewport__InitOt's init. */
void Viewport__DeinitOt(Viewport *self) {
    if (self->otReady != 0) {
        DrawSync(0);
        BMemPMgrFree(self->ot[0]);
        self->otReady = 0;
    }
}

/* A FrameClock event: counts every one in clockEventCount, and runs update
 * on a tick whether the clock is running or paused (not on
 * FRAMECLOCK_EVENT_FLAG14). */
void Viewport__OnNotifyTag5(Viewport *self, BasicClass *sender, s32 event) {
    self->clockEventCount = self->clockEventCount + 1;
    if (event == FRAMECLOCK_EVENT_RUNNING || event == FRAMECLOCK_EVENT_PAUSED) {
        self->methods->update(self);
    }
}

/* A DrawSystem event: its per-VSync event runs flip. */
void Viewport__OnNotifyTag1(Viewport *self, BasicClass *sender, s32 event) {
    if (event == DRAWSYSTEM_EVENT_VSYNC) {
        self->methods->flip(self);
    }
}

/* Per-frame update, only once Viewport__InitOt has succeeded: draws the
 * view node if it has a parent, sets the projection, near clip and light
 * mode (and the fog, in both fog modes), sets the reference view and
 * marks its super coordinate for recompute, recomputes zDiv, sets this
 * half's packet area and clears its OT, then draws sceneRoot and the root
 * of the view node's parent chain. */
void Viewport__Update(Viewport *self) {
    s32 idx;
    SceneNode *root;

    if (self->otReady == 0) {
        return;
    }

    if (self->viewNode->parent != NULL) {
        self->methods->drawNode(self, self->viewNode);
    }

    GsSetProjection(self->projH);
    GsSetNearClip(self->nearZ);
    GsSetLightMode(self->lightMode);

    if (self->lightMode == GsLMODE_FOG || self->lightMode == (GsLMODE_LOFF | GsLMODE_FOG)) {
        SetFarColor((u8)self->farColor.r, (u8)self->farColor.g, (u8)self->farColor.b);
        SetFogNear(self->fogNear, self->projH);
    }

    GsSetRefView2((GsRVIEW2 *)&self->refView);
    self->refView.super->flg = 0;

    self->zDiv = (u32)(self->farZ - self->nearZ) / (u32)(1 << self->otLength) + 1;

    idx = self->otIndex;
    GsSetWorkBase(self->workBase[idx]);

    idx = self->otIndex;
    GsClearOt(0, 0, self->ot[idx]);

    self->methods->drawNode(self, self->sceneRoot);

    if (self->viewNode != NULL) {
        root = GetRootNode(self->viewNode);
        self->methods->drawNode(self, root);
    }
}

/* Takes otIndex from the DrawSystem's getActiveBuffer; when drawing is
 * enabled, resets the GPU, swaps, sorts the clear into this half's OT and
 * draws it, with one more swap before the clear and one after the draw on
 * buffer 0 when extraSwap is set; then flips otIndex to the other
 * half. */
void Viewport__Flip(Viewport *self) {
    s32 idx;

    if (self->otReady == 0) {
        return;
    }

    self->otIndex = self->drawSystem->methods->getActiveBuffer(self->drawSystem);
    if (self->drawEnabled == 0) {
        goto tail_check;
    }

    ResetGraph(1);
    self->drawSystem->methods->swapBuffers(self->drawSystem);

    if (self->extraSwap != 0) {
        if (self->otIndex == 0) {
            self->drawSystem->methods->swapBuffers(self->drawSystem);
        }
    }

    idx = self->otIndex;
    GsSortClear(self->clearColor.r, self->clearColor.g, self->clearColor.b, self->ot[idx]);

    idx = self->otIndex;
    GsDrawOt(self->ot[idx]);

    if (self->extraSwap != 0 && self->otIndex == 0) {
        self->drawSystem->methods->swapBuffers(self->drawSystem);
    }

tail_check:
    self->otIndex = (self->otIndex == 0);
}

/* Only while no view node is set: releases the current fade box, installs
 * `fadeBox`, and attaches it under sceneRoot at gFadeBoxAttachPos
 * (-100, -100). A FadeBox's attachToParent (BoxFill__AttachToParent) takes a
 * screen position where SceneNode's slot types a LongVec3 offset, hence the
 * cast (include/Viewport.h, "Not settled here"). */
void Viewport__SetFadeBox(Viewport *self, SceneNode *fadeBox) {
    if (self->viewNode != NULL) {
        return;
    }

    if (self->fadeBox != NULL) {
        self->fadeBox->methods->release(self->fadeBox);
    }

    self->fadeBox = fadeBox;
    if (fadeBox != NULL) {
        fadeBox->methods->attachToParent(fadeBox, self->sceneRoot, (LongVec3 *)gFadeBoxAttachPos);
    }
}

SceneNode *Viewport__GetFadeBox(Viewport *self) {
    return self->fadeBox;
}

void Viewport__SetExtraSwap(Viewport *self, s32 on) {
    self->extraSwap = on;
}

void Viewport__SetDrawEnabled(Viewport *self, s32 on) {
    self->drawEnabled = on;
}

/* Viewport's table getter. */
ViewportMethods *GetViewportMethods(void) {
    return &gViewportMethods;
}

/* Follows `parent` from `node` to the top of its hierarchy. Not a method
 * (in no table): Viewport__Update passes it the view node. */
SceneNode *GetRootNode(SceneNode *node) {
    while (node->parent != NULL) {
        node = node->parent;
    }
    return node;
}

/* Sony's libgs GsSetProjection (gs_106). No SDK object places it, so it is
 * carried as C; progress.py counts it as library. */
void GsSetProjection(long h) {
    SetGeomScreen(h);
}
