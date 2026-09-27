#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_2cc8c.h"

/*
 * code_2cc8c_e -- FadeBox's methods (include/FadeBox.h: a BoxFill whose
 * colour ramps a step per update, the screen fade), New_FadeBox to
 * GetFadeBoxMethods, then BoxFill's allocator, ctor and Reset
 * (include/BoxFill.h: a GsBOXF screen rectangle; the rest of its methods
 * open code_2cc8c_f.c).
 */

FadeBox *New_FadeBox(void *size, s32 channels, s32 pri) {
    FadeBox *self;

    self = BMemPMgrAlloc(0xA0);
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
        color = &D_8006EA90[channels * 3];
    } else {
        color = D_8006EAA8;
    }
    base->ctor((BoxFill *)self, size, color, pri);
    self->methods = GetFadeBoxMethods();
    ((FadeBoxResetFn)self->methods->reset)(self, channels);
}

void FadeBox__Reset(FadeBox *self, s32 channels) {
    self->defaultChannels = channels;
    self->state = 0;
    self->step = 0xA;
    self->channels = 0;
    self->unk7C = 0;
    self->methods->setDisplay(self, 0);
    self->methods->setSemiTransOn(self, 0);
    self->altMode = 0;
}

void FadeBox__Update(FadeBox *self, void *sender, s32 event) {
    s32 old;

    if (event != 2) {
        return;
    }
    old = self->ticksLeft;
    self->ticksLeft = old - 1;
    if (old > 0) {
        if (self->unk7C == 9) {
            return;
        }
        if (self->channels & 4) {
            self->color[0] += (u8)self->step;
        }
        if (self->channels & 2) {
            self->color[1] += (u8)self->step;
        }
        if (self->channels & 1) {
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
void FadeBox__StartFadeDown(FadeBox *self, BasicClass *source, s32 channels, s32 arg3) {
    s32 idx;

    if (self->state != 0) {
        return;
    }
    idx = self->methods->configure(self, source, channels, arg3);
    self->methods->setColor(self, 1, &D_8006EA90[idx * 3]);
    self->state = 1;
    self->step = -self->step;
}

void FadeBox__StartFadeUp(FadeBox *self, BasicClass *source, s32 channels, s32 arg3) {
    if (self->state != 0) {
        return;
    }
    channels = self->methods->configure(self, source, channels, arg3);
    if (self->altMode != 0) {
        self->ticksLeft--;
    } else {
        self->methods->setColor(self, 1, &D_8006EAA8[channels * 3]);
    }
    self->state = 2;
}

s32 FadeBox__Configure(FadeBox *self, BasicClass *source, s32 channels, s32 arg3) {
    FadeBoxMethods *methods;
    s32 rate;
    s32 q1, q2;

    methods = self->methods;
    if (channels < 0) {
        channels = self->defaultChannels;
    } else {
        self->defaultChannels = channels;
    }
    rate = 1;
    if (channels != 0) {
        self->channels = channels;
    } else {
        rate = 2;
        self->channels = 0xF;
    }
    self->channels = channels;
    if (channels == 0) {
        self->channels = 0xF;
    }
    q1 = 0x100 / self->step;
    self->unk7C = arg3;
    self->ticksLeft = q1;
    if (self->altMode != 0) {
        q2 = q1 / self->divisor;
        self->ticksLeft = q1 - (s16)q2;
    }
    q2 = self->mask;
    q2 = q2 / self->ticksLeft;
    self->maskPerTick = q2;
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
    if (self->state == 0) {
        return;
    }
    if (self->state == 1) {
        event = 5;
        if (self->altMode == 0) {
            methods->setDisplay(self, 0);
            methods->setSemiTransOn(self, 0);
        }
    } else {
        event = 6;
        if (self->altMode != 0) {
            if (self->channels == 0xF) {
                methods->setColor(self, 1, D_8006EAA8);
            }
            methods->setSemiTransOn(self, 0);
        }
    }
    methods->removeChild(self, source);
    if (self->step < 0) {
        self->step = -self->step;
    }
    self->state = 0;
    methods->notifyParents(self, event);
}

void *FadeBox__GetColor(FadeBox *self) {
    if (self->channels == 0xF) {
        return D_8006EAA8;
    }
    return &D_8006EA90[self->channels * 3];
}

/* MATCHING: both position pairs are copied as whole structs; the block
 * copy is what moves `self` and `size` out of their incoming registers. */
void FadeBox__PushPosition(FadeBox *self, SkipShort2 *size, Pair32E99C *pos) {
    if (self->parent != 0) {
        self->savedW = self->boxW;
        self->savedH = self->boxH;
        *(Pair32E99C *)&self->savedPosX = *(Pair32E99C *)&self->posX;
        self->boxW = size->x;
        self->boxH = size->y;
        *(Pair32E99C *)&self->posX = *pos;
    }
}

void FadeBox__PopPosition(FadeBox *self) {
    s32 t0, t1;

    t0 = self->savedPosX;
    t1 = self->savedPosY;
    self->posX = t0;
    self->posY = t1;
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

    self = BMemPMgrAlloc(0x6C);
    if (self != NULL) {
        GetBoxFillMethods()->ctor(self, size, color, pri);
        return self;
    }
    return NULL;
}

void BoxFill__BoxFill(BoxFill *self, SkipShort2 *size, void *color, s32 pri) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetBoxFillMethods();
    ((BoxFillResetFn)self->methods->reset)(self, size, color, pri);
}

void BoxFill__Reset(BoxFill *self, SkipShort2 *size, void *color, s32 pri) {
    BoxFillMethods *methods;

    self->pri = pri;
    self->relative = 1;
    self->unk4C = 0;
    self->boxAttribute = 0;
    self->boxX = 0;
    self->boxY = 0;
    self->boxW = size->x;
    self->boxH = size->y;
    methods = self->methods;
    if (color == NULL) {
        color = D_8008A924;
    }
    methods->setColor(self, 1, color);
    self->methods->setMask(self, 0xD);
}
