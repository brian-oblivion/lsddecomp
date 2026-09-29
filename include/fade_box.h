/**
 * @file fade_box.h
 * @brief FadeBox, a BoxFill whose colour ramps a step per frame: a screen fade.
 *
 * Declares the FadeBox class (object and method table), its channel-mask,
 * state, mode and event constants, and its methods, which are defined in
 * src/ui/fade_box.c.
 */
#ifndef FADE_BOX_H
#define FADE_BOX_H

#include "box_fill.h"

typedef struct FadeBox FadeBox;
typedef struct FadeBoxMethods FadeBoxMethods;

/** FadeBox's class id (gFadeBoxMethods word +0x000). Three nibbles, so
 * `(header & CLASS_ID_LEVEL3_MASK) == FADEBOX_CLASS_ID` tests for it or a class below it
 * (ObjM__OnNotify). */
#define FADEBOX_CLASS_ID 0x164

/** @name Channel mask
 * A channel mask's bits: which of BoxFill's colour bytes update steps
 * (4 = color[0], r; 2 = g; 1 = b), and the entry of sFadeBoxMaskColors the
 * fade starts or ends at. configure stores a mask of 0 as
 * FADEBOX_CHANNELS_ALL, 0xF rather than 7, so that stop and getColor can
 * tell it from mask 7 (white) and use black.
 * @{ */
#define FADEBOX_CHANNEL_B 0x1    /**< blue, color[2] */
#define FADEBOX_CHANNEL_G 0x2    /**< green, color[1] */
#define FADEBOX_CHANNEL_R 0x4    /**< red, color[0] */
#define FADEBOX_CHANNELS_ALL 0xF /**< mask 0 as configure stores it: all three, towards black */
/** @} */

/** Reset's per-tick step. */
#define FADEBOX_DEFAULT_STEP 10
/** One colour byte's full range: configure sets ticksLeft to
 * FADEBOX_RAMP / step, the ticks the range takes at that step. */
#define FADEBOX_RAMP 256

/** FadeBox::state: which start function ran; stop returns it to idle. */
enum FadeBoxState {
    FADEBOX_STATE_IDLE = 0,        /**< no fade running */
    FADEBOX_STATE_FADING_DOWN = 1, /**< startFadeDown ran */
    FADEBOX_STATE_FADING_UP = 2    /**< startFadeUp ran */
};

/** FadeBox::mode, configure's third argument: the one value update tests.
 * Every caller passes 0. */
enum FadeBoxMode {
    FADEBOX_MODE_HOLD = 9 /**< the colour holds while ticksLeft counts down */
};

/** What stop notifies its parents with (ObjM__OnFadeNotify). */
enum FadeBoxEvent {
    FADEBOX_EVENT_FADE_DOWN_DONE = 5, /**< a fade down ran out */
    FADEBOX_EVENT_FADE_UP_DONE = 6    /**< a fade up ran out */
};

/**
 * @brief FadeBox's method table: BoxFill's slots, then FadeBox's own.
 *
 * FadeBox overrides the inherited +0x008 (FadeBox__FadeBox), +0x040
 * (FadeBox__Reset) and +0x098 (FadeBox__Update).
 */
struct FadeBoxMethods {
    BOXFILL_SLOTS(FadeBox, (FadeBox * self, void *size, s32 channels, s32 pri));
    /* +0x0D0 */ void (*setStep)(FadeBox *self, s32 step); /**< @see FadeBox__SetStep */
    /* +0x0D4 */ void (*startFadeDown)(FadeBox *self, BasicClass *source, s32 channels,
                                       s32 mode); /**< @see FadeBox__StartFadeDown */
    /* +0x0D8 */ void (*startFadeUp)(FadeBox *self, BasicClass *source, s32 channels,
                                     s32 mode); /**< @see FadeBox__StartFadeUp */
    /* +0x0DC */ s32 (*configure)(FadeBox *self, BasicClass *source, s32 channels,
                                  s32 mode);                      /**< @see FadeBox__Configure */
    /* +0x0E0 */ void (*stop)(FadeBox *self, BasicClass *source); /**< @see FadeBox__Stop */
    /* +0x0E4 */ void *(*getColor)(FadeBox *self);                /**< @see FadeBox__GetColor */
    /* +0x0E8 */ void (*pushPosition)(FadeBox *self, BoxFillSize *size,
                                      BoxFillPos *pos); /**< @see FadeBox__PushPosition */
    /* +0x0EC */ void (*popPosition)(FadeBox *self);    /**< @see FadeBox__PopPosition */
    /* +0x0F0 */ void (*setDivisorMode)(FadeBox *self, s32 altMode, s32 divisor); /**< @see FadeBox__SetDivisorMode */
};

/**
 * @brief A BoxFill whose colour ramps a step per update until a tick count runs out.
 *
 * Class id 0x164 (FADEBOX_CLASS_ID), method table gFadeBoxMethods, parent
 * BoxFill; methods in src/ui/fade_box.c; no subclasses. The ctor
 * chains to BoxFill's ctor, then Reset stores the default channel mask and
 * a step of 10 and turns the box's display and semi-transparency off.
 *
 * A channel mask (4 = r, 2 = g, 1 = b; 0 means all, stored as 0xF) picks the
 * colour: it indexes sFadeBoxMaskColors, eight 3-byte RGB entries whose
 * bytes are the mask's channels at 0xFF (0 and 7 are white);
 * sFadeBoxBlackColors's are black.
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
 *    or 6 (fade up done). getColor returns the colour the screen shows once
 *    a fade up is done: the mask's, or black for mask 0.
 *
 * In altMode (setDivisorMode) a fade runs 1/divisor fewer ticks, a fade up
 * keeps the current colour, and stop leaves the box shown after a fade
 * down. pushPosition/popPosition save and restore BoxFill's size and
 * position.
 *
 * Users: Viewport makes one as its fadeBox (320x240 at the relative
 * (-100, -100)), which ObjM fades and whose 5/6 it handles
 * (ObjM__OnFadeNotify; ObjM__EnterStyleSession sets altMode from the
 * DreamSys's flashback session). Entity makes one on demand as its
 * `fadeBox` (Entity__GetOrCreateFadeBox); the MoodCue handlers fade it with
 * the entity's ticker as the source.
 *
 * The +0x040 reset override takes the ctor's channel mask where the
 * inherited slot does not, so the ctor calls it through FadeBoxResetFn. The
 * ctor returns nothing where the inherited slot returns `void *`, as
 * BoxFill's does; every caller ignores the value.
 *
 * The object is 0xA0 bytes (New_FadeBox); BoxFill's fields end at +0x06C.
 */
struct FadeBox {
    BOXFILL_FIELDS(FadeBoxMethods);
    /* +0x06C */ s32 state; /**< a FadeBoxState: 0 idle, 1 fading down, 2 fading up */
    /* +0x070 */ s32 defaultChannels; /**< the ctor's mask (Reset); configure uses it when passed a negative mask */
    /* +0x074 */ s32 step; /**< per-tick channel delta: 10 from Reset, setStep; negated by startFadeDown */
    /* +0x078 */ s32 channels; /**< the mask configure stored (0 as 0xF): which bytes update steps, which colour getColor returns */
    /* +0x07C */ s32 mode; /**< configure's third argument; update does not step the colour while it is FADEBOX_MODE_HOLD */
    /* +0x080 */ s32 ticksLeft; /**< configure: FADEBOX_RAMP / step; update counts it down and stops at 0 */
    /* +0x084 */ s32 maskPerTick; /**< configure: BoxFill's mask / ticksLeft; no reader */
    /* +0x088 */ s32 savedW;      /**< pushPosition's copy of boxW, restored by popPosition */
    /* +0x08C */ s32 savedH;      /**< ... of boxH */
    /* +0x090 */ s32 savedPosX;   /**< ... of posX (copied with posY as one BoxFillPos) */
    /* +0x094 */ s32 savedPosY;   /**< ... of posY */
    /* +0x098 */ s32 altMode; /**< setDivisorMode: shortens ticksLeft by 1/divisor; startFadeUp keeps the colour */
    /* +0x09C */ s32 divisor; /**< setDivisorMode */
};

/** FadeBox's method table (class id FADEBOX_CLASS_ID). */
extern FadeBoxMethods gFadeBoxMethods;

/**
 * @brief Returns FadeBox's method table.
 * @return &gFadeBoxMethods.
 */
extern FadeBoxMethods *GetFadeBoxMethods(void);

/** The +0x040 reset slot's occupant, FadeBox__Reset, as the ctor calls it. */
typedef void (*FadeBoxResetFn)(FadeBox *self, s32 channels);

/* The class's own methods, in ROM order (fade_box.c). */

/**
 * @brief Allocates a FadeBox and runs its ctor through the method table.
 * @param size     the box's BoxFillSize (width, height in pixels).
 * @param channels the default channel mask; 0 starts the box black.
 * @param pri      the ordering-table priority.
 * @return the new box, or NULL when the allocation fails.
 */
FadeBox *New_FadeBox(void *size, s32 channels, s32 pri);

/**
 * @brief Constructs a FadeBox: BoxFill's ctor in the mask's colour, then Reset.
 * @param self     the object to construct.
 * @param size     the box's size in pixels.
 * @param channels the default channel mask: the box starts in its
 *                 sFadeBoxMaskColors entry, or black for 0.
 * @param pri      the ordering-table priority.
 */
void FadeBox__FadeBox(FadeBox *self, void *size, s32 channels, s32 pri);

/**
 * @brief Resets the fade: idle, step 10, no mask or mode, box hidden and opaque.
 * @param self     the box.
 * @param channels stored as `defaultChannels`.
 */
void FadeBox__Reset(FadeBox *self, s32 channels);

/**
 * @brief Steps the fade on a running frame tick, and stops it when it runs out.
 *
 * Counts `ticksLeft` down; while it was positive, adds `(u8)step` to each
 * colour byte `channels` selects (unless `mode` is FADEBOX_MODE_HOLD);
 * once it was not, calls stop(sender).
 * @param self   the box.
 * @param sender the source driving the fade, passed on to stop.
 * @param event  the notification; only FRAMECLOCK_EVENT_RUNNING acts.
 */
void FadeBox__Update(FadeBox *self, void *sender, s32 event);

/**
 * @brief Sets the per-tick colour step.
 * @param self the box.
 * @param step the step; configure's tick count is FADEBOX_RAMP / step.
 */
void FadeBox__SetStep(FadeBox *self, s32 step);

/**
 * @brief Starts a fade down from the mask's colour, when no fade is running.
 *
 * Runs configure, sets the colour to the mask's sFadeBoxMaskColors entry
 * and negates `step`.
 * @param self     the box.
 * @param source   the node whose updates drive the fade.
 * @param channels the channel mask, or negative for `defaultChannels`.
 * @param mode     stored in `mode` (FADEBOX_MODE_HOLD holds the colour).
 */
void FadeBox__StartFadeDown(FadeBox *self, BasicClass *source, s32 channels, s32 mode);

/**
 * @brief Starts a fade up from black, when no fade is running.
 *
 * Runs configure; in altMode it keeps the current colour and runs one tick
 * fewer, otherwise it sets the colour to black.
 * @param self     the box.
 * @param source   the node whose updates drive the fade.
 * @param channels the channel mask, or negative for `defaultChannels`.
 * @param mode     stored in `mode` (FADEBOX_MODE_HOLD holds the colour).
 */
void FadeBox__StartFadeUp(FadeBox *self, BasicClass *source, s32 channels, s32 mode);

/**
 * @brief Sets up a fade: mask, tick count, source child, and the box shown semi-transparent.
 *
 * Stores the mask (0 as FADEBOX_CHANNELS_ALL) and `mode`, sets `ticksLeft`
 * to FADEBOX_RAMP / step (less 1/divisor of it in altMode), adds `source`
 * as a child, and turns display and semi-transparency on: rate 1 (added)
 * for a mask, rate 2 (subtracted) for 0.
 * @param self     the box.
 * @param source   the node whose updates drive the fade.
 * @param channels the channel mask; a negative one means `defaultChannels`,
 *                 any other becomes the new default.
 * @param mode     stored in `mode`.
 * @return the mask used, 0 for all channels (before it is stored as 0xF).
 */
s32 FadeBox__Configure(FadeBox *self, BasicClass *source, s32 channels, s32 mode);

/**
 * @brief Ends the running fade and notifies the parents.
 *
 * After a fade down hides the box (unless in altMode); after a fade up in
 * altMode turns semi-transparency off, and sets black for mask 0. Removes
 * `source`, makes `step` positive and returns to idle, then notifies
 * FADEBOX_EVENT_FADE_DOWN_DONE or FADEBOX_EVENT_FADE_UP_DONE. Does nothing
 * while idle.
 * @param self   the box.
 * @param source the child configure added.
 */
void FadeBox__Stop(FadeBox *self, BasicClass *source);

/**
 * @brief Returns the colour a finished fade up leaves: the mask's, or black.
 * @param self the box.
 * @return three bytes r, g, b: black for FADEBOX_CHANNELS_ALL, else the
 *         mask's sFadeBoxMaskColors entry.
 */
void *FadeBox__GetColor(FadeBox *self);

/**
 * @brief Saves the box's size and position and sets new ones, while it is attached.
 * @param self the box.
 * @param size the new size in pixels.
 * @param pos  the new position.
 */
void FadeBox__PushPosition(FadeBox *self, BoxFillSize *size, BoxFillPos *pos);

/**
 * @brief Restores the size and position pushPosition saved.
 * @param self the box.
 */
void FadeBox__PopPosition(FadeBox *self);

/**
 * @brief Sets altMode and the divisor it shortens each fade by.
 * @param self    the box.
 * @param altMode nonzero for altMode (see the class doc).
 * @param divisor a fade runs 1/divisor fewer ticks in altMode.
 */
void FadeBox__SetDivisorMode(FadeBox *self, s32 altMode, s32 divisor);

#endif
