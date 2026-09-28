#ifndef FADE_BOX_H
#define FADE_BOX_H

#include "box_fill.h"

/*
 * FadeBox -- a BoxFill whose colour ramps a step per update until a tick
 * count runs out: a screen fade (class id 0x164, method table
 * gFadeBoxMethods, parent BoxFill; methods in src/ui/screen_widgets.c; no
 * subclasses). The ctor chains to BoxFill's ctor, then Reset stores the
 * default channel mask, a step of 10, and turns the box's display and
 * semi-transparency off.
 *
 * A channel mask (4 = r, 2 = g, 1 = b; 0 means all, stored as 0xF) picks
 * the colour: it indexes sFadeBoxMaskColors, eight 3-byte RGB entries whose bytes
 * are the mask's channels at 0xFF (0 and 7 are white); sFadeBoxBlackColors's are
 * black.
 *
 * One fade:
 *  - configure(source, channels, mode) adds `source` as a child (the node
 *    whose updates drive the fade), stores the mask (a negative one means
 *    `defaultChannels`), sets `ticksLeft` to 0x100 / step, and turns the
 *    box's display and semi-transparency on: rate 1 (added to what is
 *    behind it) for a mask, rate 2 (subtracted) for 0.
 *  - startFadeDown starts at the mask's colour and negates `step`;
 *    startFadeUp starts at black. So with mask 0 a fade down is a fade in
 *    from black and a fade up a fade out to black; with a mask, a coloured
 *    flash that fades out, or builds up.
 *  - update(sender, 2) adds `(u8)step` to each selected byte of BoxFill's
 *    `color`, and calls stop(sender) once `ticksLeft` runs out.
 *  - stop removes the source child, hides the box after a fade down, makes
 *    `step` positive again and notifies its parents with 5 (fade down done)
 *    or 6 (fade up done). getColor returns the colour the screen shows
 *    once a fade up is done: the mask's, or black for mask 0.
 * In altMode (setDivisorMode) a fade runs 1/divisor fewer ticks, a fade up
 * keeps the current colour, and stop leaves the box shown after a fade down.
 * pushPosition/popPosition save and restore BoxFill's size and position.
 *
 * Its users: Viewport makes one as its fadeBox (320x240 at the relative
 * (-100, -100)), which ObjM fades and whose 5/6 it handles
 * (ObjM__OnFadeNotify; ObjM__EnterStyleSession sets altMode from the
 * DreamSys's flashback session). Entity makes one on demand as its
 * `fadeBox` (Entity__GetOrCreateFadeBox); the MoodCue handlers fade it with
 * the entity's ticker as the source.
 *
 * Overrides whose parameter list differs from the inherited slot keep the
 * slot's type; the caller casts:
 *  - +0x040 reset: FadeBox__Reset takes the ctor's channel mask; the ctor
 *    calls it through FadeBoxResetFn.
 * The ctor returns nothing where the inherited slot returns `void *`, as
 * BoxFill's does; every caller ignores the value.
 *
 * The object is 0xA0 bytes (New_FadeBox); BoxFill's fields end at +0x06C.
 */

typedef struct FadeBox FadeBox;
typedef struct FadeBoxMethods FadeBoxMethods;

/* FadeBox's class id (gFadeBoxMethods word +0x000). Three nibbles, so
 * `(header & 0xFFF) == FADEBOX_CLASS_ID` tests for it or a class below it
 * (ObjM__OnNotify). */
#define FADEBOX_CLASS_ID 0x164

/* A channel mask's bits: which of BoxFill's colour bytes update steps
 * (4 = color[0], r; 2 = g; 1 = b), and the entry of sFadeBoxMaskColors the
 * fade starts or ends at. configure stores a mask of 0 as
 * FADEBOX_CHANNELS_ALL, 0xF rather than 7 so that stop and getColor can tell
 * it from mask 7 (white) and use black. */
#define FADEBOX_CHANNEL_B 0x1
#define FADEBOX_CHANNEL_G 0x2
#define FADEBOX_CHANNEL_R 0x4
#define FADEBOX_CHANNELS_ALL 0xF

/* Reset's per-tick step; configure sets ticksLeft to FADEBOX_RAMP / step,
 * the ticks one colour byte's full range takes at that step. */
#define FADEBOX_DEFAULT_STEP 10
#define FADEBOX_RAMP 256

/* `state`: which start function ran; stop returns it to idle. */
enum FadeBoxState {
    FADEBOX_STATE_IDLE = 0,
    FADEBOX_STATE_FADING_DOWN = 1,
    FADEBOX_STATE_FADING_UP = 2
};

/* `mode` (configure's third argument): the one value update tests. With it
 * the colour holds while ticksLeft counts down; every caller passes 0. */
enum FadeBoxMode { FADEBOX_MODE_HOLD = 9 };

/* What stop notifies its parents with (ObjM__OnFadeNotify). */
enum FadeBoxEvent { FADEBOX_EVENT_FADE_DOWN_DONE = 5, FADEBOX_EVENT_FADE_UP_DONE = 6 };

/* BoxFill's slots (overrides: +0x008 FadeBox__FadeBox, +0x040
 * FadeBox__Reset, +0x098 FadeBox__Update; `tools/classtable.py
 * gFadeBoxMethods --vs gBoxFillMethods`), then this class's own. */
struct FadeBoxMethods {
    BOXFILL_SLOTS(FadeBox, (FadeBox * self, void *size, s32 channels, s32 pri));
    /* +0x0D0 */ void (*setStep)(FadeBox *self, s32 step); /* FadeBox__SetStep */
    /* +0x0D4 */ void (*startFadeDown)(FadeBox *self, BasicClass *source, s32 channels,
                                       s32 mode); /* FadeBox__StartFadeDown */
    /* +0x0D8 */ void (*startFadeUp)(FadeBox *self, BasicClass *source, s32 channels,
                                     s32 mode); /* FadeBox__StartFadeUp */
    /* +0x0DC */ s32 (*configure)(FadeBox *self, BasicClass *source, s32 channels,
                                  s32 mode); /* FadeBox__Configure: returns the mask it stored */
    /* +0x0E0 */ void (*stop)(FadeBox *self, BasicClass *source); /* FadeBox__Stop */
    /* +0x0E4 */ void *(*getColor)(FadeBox *self); /* FadeBox__GetColor: the mask's table entry */
    /* +0x0E8 */ void (*pushPosition)(FadeBox *self, BoxFillSize *size,
                                      BoxFillPos *pos); /* FadeBox__PushPosition: only while attached */
    /* +0x0EC */ void (*popPosition)(FadeBox *self); /* FadeBox__PopPosition */
    /* +0x0F0 */ void (*setDivisorMode)(FadeBox *self, s32 altMode, s32 divisor); /* FadeBox__SetDivisorMode */
};

struct FadeBox {
    BOXFILL_FIELDS(FadeBoxMethods);
    /* +0x06C */ s32 state; /* 0 idle, 1 fading down (startFadeDown), 2 up (startFadeUp); stop returns it to 0 */
    /* +0x070 */ s32 defaultChannels; /* the ctor's mask (Reset); configure uses it when passed a negative mask */
    /* +0x074 */ s32 step; /* per-tick channel delta: 10 from Reset, setStep; negated by startFadeDown */
    /* +0x078 */ s32 channels; /* the mask configure stored (0 as 0xF): which bytes update steps, which colour getColor returns */
    /* +0x07C */ s32 mode; /* configure's third argument; update does not step the colour while it is FADEBOX_MODE_HOLD */
    /* +0x080 */ s32 ticksLeft; /* configure: FADEBOX_RAMP / step; update counts it down and stops at 0 */
    /* +0x084 */ s32 maskPerTick; /* configure: BoxFill's mask / ticksLeft; no reader */
    /* +0x088 */ s32 savedW;      /* pushPosition's copy of boxW, restored by popPosition */
    /* +0x08C */ s32 savedH;      /* ... of boxH */
    /* +0x090 */ s32 savedPosX;   /* ... of posX (copied with posY as one BoxFillPos) */
    /* +0x094 */ s32 savedPosY;   /* ... of posY */
    /* +0x098 */ s32 altMode; /* setDivisorMode: shortens ticksLeft by 1/divisor; startFadeUp keeps the colour */
    /* +0x09C */ s32 divisor; /* setDivisorMode */
};

extern FadeBoxMethods gFadeBoxMethods;
extern FadeBoxMethods *GetFadeBoxMethods(void); /* returns &gFadeBoxMethods */

/* +0x040's occupant, as the ctor calls it through the inherited slot. */
typedef void (*FadeBoxResetFn)(FadeBox *self, s32 channels);

/* The class's own methods, in ROM order (screen_widgets). */
FadeBox *New_FadeBox(void *size, s32 channels, s32 pri);
void FadeBox__FadeBox(FadeBox *self, void *size, s32 channels, s32 pri);
void FadeBox__Reset(FadeBox *self, s32 channels);
void FadeBox__Update(FadeBox *self, void *sender, s32 event);
void FadeBox__SetStep(FadeBox *self, s32 step);
void FadeBox__StartFadeDown(FadeBox *self, BasicClass *source, s32 channels, s32 mode);
void FadeBox__StartFadeUp(FadeBox *self, BasicClass *source, s32 channels, s32 mode);
s32 FadeBox__Configure(FadeBox *self, BasicClass *source, s32 channels, s32 mode);
void FadeBox__Stop(FadeBox *self, BasicClass *source);
void *FadeBox__GetColor(FadeBox *self);
void FadeBox__PushPosition(FadeBox *self, BoxFillSize *size, BoxFillPos *pos);
void FadeBox__PopPosition(FadeBox *self);
void FadeBox__SetDivisorMode(FadeBox *self, s32 altMode, s32 divisor);

#endif
