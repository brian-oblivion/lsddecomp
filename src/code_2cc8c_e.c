#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "FrameClock.h"
#include "TaskViewport.h"

/*
 * code_2cc8c_e -- the screen fade, and the first methods of the box it draws.
 *
 * FadeBox (include/FadeBox.h), New_FadeBox to GetFadeBoxMethods: a BoxFill
 * whose colour steps once per FrameClock tick away from a channel mask's
 * colour (gFadeBoxMaskColors) or up from black (gFadeBoxBlackColors), with
 * semi-transparency on, and notifies its parents when the ramp runs out.
 * Viewport's fadeBox and Entity's are the two users.
 *
 * Then BoxFill's allocator, ctor and Reset (include/BoxFill.h: a GsBOXF
 * screen rectangle); the rest of BoxFill's methods open code_2cc8c_f.c.
 */

FadeBox *New_FadeBox(void *size, s32 channels, s32 pri) {
    FadeBox *self;

    self = BMemPMgrAlloc(sizeof(FadeBox));
    if (self != NULL) {
        GetFadeBoxMethods()->ctor(self, size, channels, pri);
        return self;
    }
    return NULL;
}

void FadeBox__FadeBox(FadeBox *self, void *size, s32 channels, s32 pri) {
    BoxFillMethods *base;
    void *color;

    base = GetBoxFillMethods();
    if (channels != 0) {
        color = &gFadeBoxMaskColors[channels * 3];
    } else {
        color = gFadeBoxBlackColors;
    }
    base->ctor((BoxFill *)self, size, color, pri);
    self->methods = GetFadeBoxMethods();
    ((FadeBoxResetFn)self->methods->reset)(self, channels);
}

void FadeBox__Reset(FadeBox *self, s32 channels) {
    self->defaultChannels = channels;
    self->state = FADEBOX_STATE_IDLE;
    self->step = FADEBOX_DEFAULT_STEP;
    self->channels = 0;
    self->mode = 0;
    self->methods->setDisplay(self, 0);
    self->methods->setSemiTransOn(self, 0);
    self->altMode = 0;
}

void FadeBox__Update(FadeBox *self, void *sender, s32 event) {
    s32 ticks;

    if (event != FRAMECLOCK_EVENT_RUNNING) {
        return;
    }
    ticks = self->ticksLeft;
    self->ticksLeft = ticks - 1;
    if (ticks > 0) {
        if (self->mode == FADEBOX_MODE_HOLD) {
            return;
        }
        if (self->channels & FADEBOX_CHANNEL_R) {
            self->color[0] += (u8)self->step;
        }
        if (self->channels & FADEBOX_CHANNEL_G) {
            self->color[1] += (u8)self->step;
        }
        if (self->channels & FADEBOX_CHANNEL_B) {
            self->color[2] += (u8)self->step;
        }
    } else {
        self->methods->stop(self, sender);
    }
}

void FadeBox__SetStep(FadeBox *self, s32 step) {
    self->step = step;
}

/* MATCHING: both StartFade functions pass their own arguments on to
 * configure; a `(self)`-only call reorders the instructions. */
void FadeBox__StartFadeDown(FadeBox *self, BasicClass *source, s32 channels, s32 mode) {
    s32 mask;

    if (self->state != FADEBOX_STATE_IDLE) {
        return;
    }
    mask = self->methods->configure(self, source, channels, mode);
    self->methods->setColor(self, 1, &gFadeBoxMaskColors[mask * 3]);
    self->state = FADEBOX_STATE_FADING_DOWN;
    self->step = -self->step;
}

void FadeBox__StartFadeUp(FadeBox *self, BasicClass *source, s32 channels, s32 mode) {
    if (self->state != FADEBOX_STATE_IDLE) {
        return;
    }
    channels = self->methods->configure(self, source, channels, mode);
    if (self->altMode != 0) {
        self->ticksLeft--;
    } else {
        self->methods->setColor(self, 1, &gFadeBoxBlackColors[channels * 3]);
    }
    self->state = FADEBOX_STATE_FADING_UP;
}

s32 FadeBox__Configure(FadeBox *self, BasicClass *source, s32 channels, s32 mode) {
    FadeBoxMethods *methods;
    s32 rate;
    s32 ticks, cut;

    methods = self->methods;
    if (channels < 0) {
        channels = self->defaultChannels;
    } else {
        self->defaultChannels = channels;
    }
    rate = BOXFILL_SEMITRANS_RATE(GsAONE);
    if (channels != 0) {
        self->channels = channels;
    } else {
        rate = BOXFILL_SEMITRANS_RATE(GsATWO);
        self->channels = FADEBOX_CHANNELS_ALL;
    }
    /* MATCHING: retail stores the mask twice; one store drops four instructions. */
    self->channels = channels;
    if (channels == 0) {
        self->channels = FADEBOX_CHANNELS_ALL;
    }
    ticks = FADEBOX_RAMP / self->step;
    self->mode = mode;
    self->ticksLeft = ticks;
    if (self->altMode != 0) {
        cut = ticks / self->divisor;
        self->ticksLeft = ticks - cut;
    }
    self->maskPerTick = self->mask / self->ticksLeft;
    methods->addChild(self, source);
    methods->setSemiTransOn(self, 1);
    methods->setSemiTransRate(self, rate);
    methods->setDisplay(self, 1);
    return channels;
}

void FadeBox__Stop(FadeBox *self, BasicClass *source) {
    FadeBoxMethods *methods;
    s32 event;

    methods = self->methods;
    if (self->state == FADEBOX_STATE_IDLE) {
        return;
    }
    if (self->state == FADEBOX_STATE_FADING_DOWN) {
        event = FADEBOX_EVENT_FADE_DOWN_DONE;
        if (self->altMode == 0) {
            methods->setDisplay(self, 0);
            methods->setSemiTransOn(self, 0);
        }
    } else {
        event = FADEBOX_EVENT_FADE_UP_DONE;
        if (self->altMode != 0) {
            if (self->channels == FADEBOX_CHANNELS_ALL) {
                methods->setColor(self, 1, gFadeBoxBlackColors);
            }
            methods->setSemiTransOn(self, 0);
        }
    }
    methods->removeChild(self, source);
    if (self->step < 0) {
        self->step = -self->step;
    }
    self->state = FADEBOX_STATE_IDLE;
    methods->notifyParents(self, event);
}

void *FadeBox__GetColor(FadeBox *self) {
    if (self->channels == FADEBOX_CHANNELS_ALL) {
        return gFadeBoxBlackColors;
    }
    return &gFadeBoxMaskColors[self->channels * 3];
}

/* MATCHING: both position pairs are copied as whole structs; the block
 * copy is what moves `self` and `size` out of their incoming registers. */
void FadeBox__PushPosition(FadeBox *self, BoxFillSize *size, BoxFillPos *pos) {
    if (self->parent != 0) {
        self->savedW = self->boxW;
        self->savedH = self->boxH;
        *(BoxFillPos *)&self->savedPosX = *(BoxFillPos *)&self->posX;
        self->boxW = size->w;
        self->boxH = size->h;
        *(BoxFillPos *)&self->posX = *pos;
    }
}

void FadeBox__PopPosition(FadeBox *self) {
    s32 x, y;

    x = self->savedPosX;
    y = self->savedPosY;
    self->posX = x;
    self->posY = y;
    /* MATCHING: without it GCC hoists the savedW/savedH loads above the
     * posX/posY stores. */
    __asm__("");
    self->boxW = self->savedW;
    self->boxH = self->savedH;
}

void FadeBox__SetDivisorMode(FadeBox *self, s32 altMode, s32 divisor) {
    self->altMode = altMode;
    self->divisor = divisor;
}

FadeBoxMethods *GetFadeBoxMethods(void) {
    return &gFadeBoxMethods;
}

BoxFill *New_BoxFill(void *size, void *color, s32 pri) {
    BoxFill *self;

    self = BMemPMgrAlloc(sizeof(BoxFill));
    if (self != NULL) {
        GetBoxFillMethods()->ctor(self, size, color, pri);
        return self;
    }
    return NULL;
}

void BoxFill__BoxFill(BoxFill *self, BoxFillSize *size, void *color, s32 pri) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetBoxFillMethods();
    ((BoxFillResetFn)self->methods->reset)(self, size, color, pri);
}

void BoxFill__Reset(BoxFill *self, BoxFillSize *size, void *color, s32 pri) {
    BoxFillMethods *methods;

    self->pri = pri;
    self->relative = 1;
    self->unk4C = 0;
    self->boxAttribute = 0;
    self->boxX = 0;
    self->boxY = 0;
    self->boxW = size->w;
    self->boxH = size->h;
    methods = self->methods;
    if (color == NULL) {
        color = gBoxFillDefaultColor;
    }
    methods->setColor(self, 1, color);
    self->methods->setMask(self, 13);
}
