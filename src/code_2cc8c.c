/* code_2cc8c -- first 20-function slice of the 0x2CC8C block (153 functions
 * total; the remainder is the code_2cc8c_b asm segment).
 *
 * Carved in round 10 on the belief that this block had the LOWEST
 * toolchain-blocker density of any uncarved segment. That belief was
 * retracted the same round for two of the 20 -- `TaskCore__OnPadEvent` and
 * `TaskCore__SetState`, whose jump-table dispatch hits
 * `addiu $at, $at, %lo(jtbl_*)`. The retraction was RIGHT: a `jtbl_*` symbol
 * is not a safe exception to that screen, because cc1 emits the same generic
 * pseudo-op for an indexed data global and a switch jump table and the fold
 * happens in maspsx, below cc1, which cannot tell them apart.
 *
 * BOTH OF THOSE ARE NOW MATCHED (round 23). The analysis above stands; only
 * the VERDICT expired, because round 21 resolved `addiu_at` itself -- maspsx
 * gained a `--addiu-at` flag that emits retail's unfolded four-instruction
 * indexed form directly (docs/research/addiu-at-blocker.md). `addiu_at` is
 * no longer a blocker anywhere; do not screen for it and do not file a stall
 * against it -- and as of round 63 (CLAUDE.md, "Open toolchain blockers")
 * there are no open toolchain blockers of any kind left in this project.
 *
 * The history is kept rather than deleted because it is this unit that
 * established the jtbl-is-not-an-exception discriminator, and that finding
 * outlived the blocker it was about.
 *
 * Shape: this is class-framework code. Every function in this file is
 * TaskCore's own (include/TaskCore.h, track 4 round 84; its object was
 * viewed here as `Obj86B60` until then), the default for its slot in
 * gTaskCoreMethods, which StreamTask, TitleMenu and GraphRoom
 * inherit or override. TaskCore__OnPadEvent is IntermediateBase's Pad
 * case, switching on the event to the five onPad* handlers;
 * TaskCore__Update and TaskCore__SetState drive the state machine the
 * header's banner describes; the rest are the frame bound, the sound
 * call, the view callback and the fade-in/fade-out pair (a running colour
 * from `frameCounter * fadeRate` against `baseColor`).
 *
 * Round 78 (delta): full track-3 naming pass. All 20 functions were already
 * matched (rounds 10-23); this round named every one via `tools/rename.py`
 * and corrected a pre-existing error in the header's slot74..slot84
 * occupant mapping (it had the five message handlers reversed -- see
 * include/code_2cc8c.h's own comment on that struct field). See each
 * function's own match report's `## Naming` section for evidence.
 */

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_2cc8c.h"
#include "VabStreamObj.h"
#include "BgLayer.h"

void TaskCore__OnPadEvent(TaskCore *self, BasicClass *sender, s32 event) {
    TaskCoreMethods *methods;

    methods = self->methods;
    if (self->inputMode != 0) {
        switch (event) {
            case 0x12:
                methods->onPadPrev(self);
                break;
            case 0x13:
                methods->onPadNext(self);
                break;
            case 0x21:
                methods->onPad21(self);
                break;
            case 0x17:
                methods->onPadCancel(self);
                break;
            case 0x19:
                methods->onPadConfirm(self);
                break;
        }
    }
}

void TaskCore__Update(TaskCore *self, BasicClass *sender, s32 event) {
    TaskCoreMethods *methods;

    methods = self->methods;
    Get_vtable_IntermediateBase()->update((IntermediateBase *)self, sender, event);
    if (self->inputMode != 0) {
        u32 bound;

        bound = self->frameCounter;
        if ((u32)self->frameBound < bound) {
            methods->setState(self, 6);
        }
    }
    switch (self->state) {
        case 2:
            methods->setState(self, 4);
            break;
        case 4:
            methods->tickFadeCallback(self);
            break;
        case 7:
            methods->tickFadeOutCallback(self);
            break;
        case 8:
            methods->setState(self, 3);
            break;
    }
}

void TaskCore__SetState(TaskCore *self, s32 state) {
    TaskCoreMethods *methods;

    methods = self->methods;
    Get_vtable_IntermediateBase()->setState((IntermediateBase *)self, state);
    switch (state) {
        case 5:
            methods->broadcastToSlots(self, self->target->unselectedColor);
            methods->setActiveSlot(self, self->target->unk8, 0);
            self->frameCounter = 0;
            self->inputMode = 1;
            break;
        case 6:
            self->result = 1;
            methods->refreshViewValue(self);
            break;
        case 4:
        case 7:
            self->frameCounter = 0;
            self->inputMode = 0;
            break;
        case 8:
            self->frameCounter = 0;
            break;
        case 9:
        case 0xA:
        case 0xB:
        case 0xE:
        case 0xF:
        case 0x10:
        case 0x11:
            self->state = 5;
            self->frameCounter = 0;
            switch (state) {
                case 0xB:
                    methods->tick(self);
                    break;
                case 0xF:
                    methods->commitElementScroll(self);
                    break;
                case 0x11:
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
        sound->methods->playTone(sound, tone, 0x60, 0x60);
    }
}

void TaskCore__OnPadStart(TaskCore *self) {
    if (self->target != NULL) {
        self->methods->playSound(self, 0x10);
        self->methods->setState(self, 0xA);
    }
}

void TaskCore__OnPadConfirm(TaskCore *self) {
    s32 reason;

    if (self->target != NULL) {
        self->methods->playSound(self, 0x10);
        reason = 0xF;
        if (self->inputMode == 1) {
            reason = 0xB;
        }
        self->methods->setState(self, reason);
    }
}

void TaskCore__OnPadCancel(TaskCore *self) {
    if (self->target != NULL && self->inputMode != 1) {
        self->methods->playSound(self, 0x10);
        self->methods->setState(self, 0x11);
    }
}

void TaskCore__OnPadPrev(TaskCore *self) {
    void (*handler)(TaskCore *self);

    if (self->target == NULL) {
        return;
    }
    if (self->inputMode == 1) {
        handler = self->methods->findPrevFreeSlot;
    } else if (self->inputMode == 2) {
        handler = self->methods->retreatSlotCursor;
    } else {
        return;
    }
    handler(self);
}

void TaskCore__OnPadNext(TaskCore *self) {
    void (*handler)(TaskCore *self);

    if (self->target == NULL) {
        return;
    }
    if (self->inputMode == 1) {
        handler = self->methods->findNextFreeSlot;
    } else if (self->inputMode == 2) {
        handler = self->methods->advanceSlotCursor;
    } else {
        return;
    }
    handler(self);
}

void TaskCore__Tick(TaskCore *self) {
    TaskCoreTarget *target;
    s32 idx;

    target = self->target;
    idx = self->activeSlot;
    if (target->unk24[idx] != NULL) {
        self->methods->beginElementScroll(self);
    } else if (idx == target->unkC) {
        self->methods->refreshViewValue(self);
    }
}

void TaskCore__RefreshViewValue(TaskCore *self) {
    if (self->viewCallback != NULL) {
        self->viewCallback(self->viewCallbackCtx);
    }
    self->methods->setState(self, 7);
}

void TaskCore__SetCallback(TaskCore *self, void (*callback)(void *ctx), void *ctx) {
    self->viewCallback = callback;
    self->viewCallbackCtx = ctx;
}

void TaskCore__SetFadeCallbackEnabled(TaskCore *self, s32 enable) {
    TaskCoreMethods *methods;

    methods = self->methods;
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

    methods = self->methods;
    switch (enable) {
        case 0:
            self->fadeOutCallback = NULL;
            break;
        case 1:
            self->fadeOutCallback = methods->tickFadeColor;
            break;
    }
}

/* Each colour is copied as a BgLayerRgb (GsBG r, g, b; lb/sb, so signed):
 * baseColor is what TickColorFade's base and the BgLayer's setColor take. */
void TaskCore__SetColors(TaskCore *self, u8 *a1, u8 *a2, u8 *a3) {
    *(BgLayerRgb *)self->baseColor = *(BgLayerRgb *)a1;
    *(BgLayerRgb *)self->unk93 = *(BgLayerRgb *)a2;
    *(BgLayerRgb *)self->unk96 = *(BgLayerRgb *)a3;
}

void TaskCore__SetFadeRate(TaskCore *self, s32 rate) {
    self->fadeRate = rate;
}

s32 TaskCore__TickFadeCallback(TaskCore *self) {
    s32 result;

    result = 1;
    if (self->fadeInCallback != NULL) {
        result = self->fadeInCallback(self);
    }
    if (result != 0) {
        self->methods->setState(self, 5);
    }
    return result;
}

s32 TaskCore__TickColorFade(TaskCore *self) {
    s32 prod;
    u8 buffer[3];

    prod = self->frameCounter * self->fadeRate;
    buffer[0] = prod + self->baseColor[0];
    buffer[1] = prod + self->baseColor[1];
    buffer[2] = prod + self->baseColor[2];
    self->methods->broadcastToSlots(self, buffer);
    self->bgLayer->methods->setColor(self->bgLayer, 1, (BgLayerRgb *)buffer);
    return (u8)prod >= 0x81;
}

s32 TaskCore__TickFadeOutCallback(TaskCore *self) {
    s32 result;

    result = 1;
    if (self->fadeOutCallback != NULL) {
        result = self->fadeOutCallback(self);
        if (result == 0) {
            goto epilogue;
        }
    }
    self->methods->setState(self, 8);
epilogue:
    return result;
}
