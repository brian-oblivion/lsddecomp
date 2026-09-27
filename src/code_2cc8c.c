/* code_2cc8c -- TaskCore's own methods from +0x058 to +0x0C0: the pad
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
#include "code_2cc8c.h"
#include "VabStreamObj.h"
#include "BgLayer.h"
#include "Pad.h"

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
        self->frameBound = bound * 20;
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
