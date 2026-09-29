/*
 * FadeBox's methods (include/fade_box.h: a BoxFill whose colour steps a
 * tick at a time into or out of a flash or a fade to black), in ROM order:
 * New_FadeBox through GetFadeBoxMethods. Its method table and colour
 * tables end the file. BoxFill, its parent, follows in box_fill.c, then
 * TextRow and the full-width Shift-JIS helpers in text_row.c and
 * full_width_sjis.c.
 */
#include "common.h"
#include "frame_clock.h"
#include "fade_box.h"
#include "bmem_pmgr.h"

/* FadeBox's colour tables, eight RGB entries each, indexed at a 3-byte
 * stride by a channel mask: sFadeBoxMaskColors holds each mask's own
 * channels at 0xFF (0 and 7 white), sFadeBoxBlackColors is all black. */
#define FADEBOX_COLOR_TABLE_SIZE (8 * 3) /* eight masks, three bytes each */
extern u8 sFadeBoxMaskColors[FADEBOX_COLOR_TABLE_SIZE];
extern u8 sFadeBoxBlackColors[FADEBOX_COLOR_TABLE_SIZE];

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
        color = &sFadeBoxMaskColors[channels * 3];
    } else {
        color = sFadeBoxBlackColors;
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

/* MATCHING: both StartFade functions pass all their arguments on to configure;
 * passing only self compiles differently. */
void FadeBox__StartFadeDown(FadeBox *self, BasicClass *source, s32 channels, s32 mode) {
    s32 storedChannels;

    if (self->state != FADEBOX_STATE_IDLE) {
        return;
    }
    storedChannels = self->methods->configure(self, source, channels, mode);
    self->methods->setColor(self, 1, &sFadeBoxMaskColors[storedChannels * 3]);
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
        self->methods->setColor(self, 1, &sFadeBoxBlackColors[channels * 3]);
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
    /* MATCHING: retail stores `channels` twice, above and here; one store is
     * four instructions shorter. */
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
                methods->setColor(self, 1, sFadeBoxBlackColors);
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
        return sFadeBoxBlackColors;
    }
    return &sFadeBoxMaskColors[self->channels * 3];
}

/* MATCHING: the position pairs are copied as whole structs; field copies compile
 * differently. */
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
    /* MATCHING: the position pair is copied whole, as in PushPosition. */
    *(BoxFillPos *)&self->posX = *(BoxFillPos *)&self->savedPosX;
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

/* The method table and the colour tables, in the order the image keeps
 * them. A (void *) entry is a method whose declared type differs from its
 * slot's, usually one inherited from a parent class and declared on the
 * parent's type. */

/* FadeBox (include/fade_box.h): BoxFill's table with reset and update,
 * then the fade's own slots. */
FadeBoxMethods gFadeBoxMethods = {
    /* +0x000 header */ FADEBOX_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)FadeBox__FadeBox,
    /* +0x00C finalize */ (void *)SceneNode__Finalize,
    /* +0x010 addChild */ (void *)SceneNode__AddChild,
    /* +0x014 removeChild */ (void *)SceneNode__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)SceneNode__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)SceneNode__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 reset */ (void *)FadeBox__Reset,
    /* +0x044 updateRotation */ (void *)SceneNode__UpdateRotation,
    /* +0x048 updateScale */ (void *)SceneNode__UpdateScale,
    /* +0x04C attachToParent */ (void *)BoxFill__AttachToParent,
    /* +0x050 detachFromParent */ (void *)SceneNode__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ (void *)BoxFill__SetDisplay,
    /* +0x064 setSemiTransOn */ (void *)BoxFill__SetSemiTrans,
    /* +0x068 setSemiTransRate */ (void *)BoxFill__SetSemiTransRate,
    /* +0x06C setLighting */ (void *)SceneNode__SetLighting,
    /* +0x070 setLightMode */ (void *)SceneNode__SetLightMode,
    /* +0x074 setLightDim */ (void *)SceneNode__SetLightDim,
    /* +0x078 setUseZ */ (void *)SceneNode__SetUseZ,
    /* +0x07C setSubdivision */ (void *)SceneNode__SetSubdivision,
    /* +0x080 setBackClip */ (void *)SceneNode__SetBackClip,
    /* +0x084 getRotMatrix */ (void *)SceneNode__GetRotMatrix,
    /* +0x088 notifyWithHull */ (void *)SceneNode__NotifyWithHull,
    /* +0x08C getModelHull */ (void *)SceneNode__GetModelHull,
    /* +0x090 transformAndNotifyParents */ (void *)SceneNode__TransformAndNotifyParents,
    /* +0x094 onPadEvent */ (void *)SceneNode__OnPadEvent,
    /* +0x098 update */ FadeBox__Update,
    /* +0x09C dispatchLinkCommand */ (void *)SceneNode__DispatchLinkCommand,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ (void *)SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ (void *)SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ SceneNode__NoOpSlotB0,
    /* +0x0B4 addToActorParents */ (void *)SceneNode__AddToActorParents,
    /* +0x0B8 setColor */ (void *)BoxFill__SetColor,
    /* +0x0BC setPosition */ (void *)BoxFill__SetPosition,
    /* +0x0C0 setSize */ (void *)BoxFill__SetSize,
    /* +0x0C4 attachAbsolute */ (void *)BoxFill__AttachAbsolute,
    /* +0x0C8 setPri */ (void *)BoxFill__SetPri,
    /* +0x0CC setMask */ (void *)BoxFill__SetMask,
    /* +0x0D0 setStep */ FadeBox__SetStep,
    /* +0x0D4 startFadeDown */ FadeBox__StartFadeDown,
    /* +0x0D8 startFadeUp */ FadeBox__StartFadeUp,
    /* +0x0DC configure */ FadeBox__Configure,
    /* +0x0E0 stop */ FadeBox__Stop,
    /* +0x0E4 getColor */ FadeBox__GetColor,
    /* +0x0E8 pushPosition */ FadeBox__PushPosition,
    /* +0x0EC popPosition */ FadeBox__PopPosition,
    /* +0x0F0 setDivisorMode */ FadeBox__SetDivisorMode,
};

/* clang-format off */
u8 sFadeBoxMaskColors[FADEBOX_COLOR_TABLE_SIZE] = {
    /* mask  r     g     b */
    /* 0 */ 0xFF, 0xFF, 0xFF,
    /* 1 */ 0x00, 0x00, 0xFF,
    /* 2 */ 0x00, 0xFF, 0x00,
    /* 3 */ 0x00, 0xFF, 0xFF,
    /* 4 */ 0xFF, 0x00, 0x00,
    /* 5 */ 0xFF, 0x00, 0xFF,
    /* 6 */ 0xFF, 0xFF, 0x00,
    /* 7 */ 0xFF, 0xFF, 0xFF,
};

u8 sFadeBoxBlackColors[FADEBOX_COLOR_TABLE_SIZE] = {
    /* mask  r     g     b */
    /* 0 */ 0x00, 0x00, 0x00,
    /* 1 */ 0x00, 0x00, 0x00,
    /* 2 */ 0x00, 0x00, 0x00,
    /* 3 */ 0x00, 0x00, 0x00,
    /* 4 */ 0x00, 0x00, 0x00,
    /* 5 */ 0x00, 0x00, 0x00,
    /* 6 */ 0x00, 0x00, 0x00,
    /* 7 */ 0x00, 0x00, 0x00,
};
/* clang-format on */
