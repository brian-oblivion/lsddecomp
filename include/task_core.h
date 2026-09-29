#ifndef TASK_CORE_H
#define TASK_CORE_H

#include "intermediate_base.h"

/**
 * @file task_core.h
 * @brief TaskCore, the base of the game's menu and screen tasks: a fade and
 * state machine, pad input, and a two-level picker over a menu description.
 *
 * Declares the class (TASKCORE_SLOTS, TASKCORE_FIELDS for its subclasses),
 * its states, input modes, tones and fade constants, the TaskCoreTarget menu
 * description, and its methods, defined in src/app/task.c.
 */

typedef struct TaskCore TaskCore;
typedef struct TaskCoreMethods TaskCoreMethods;
typedef struct TaskCoreTarget TaskCoreTarget;
typedef struct TaskCoreItemList TaskCoreItemList; /* defined in src/app/task.c, its one reader */

/** TaskCore's states, above IntermediateBase's START/STOP. setState passes
 * each to the parents (notifyParents) before acting on it, so the codes from
 * CURSOR_MOVED on are also events a parent reacts to (TitleMenu__SetState's
 * START_PRESSED); every one of them returns the menu to ACTIVE. */
enum TaskCoreState {
    TASKCORE_STATE_FADE_IN = 4, /* update from START: tickFadeInCallback until the fade-in is done */
    TASKCORE_STATE_ACTIVE = 5,  /* the target's initialSlot selected, inputMode CHOOSING_SLOT */
    TASKCORE_STATE_TIMED_OUT = 6,       /* update: frameCounter passed frameBound; result 1, exit */
    TASKCORE_STATE_FADE_OUT = 7,        /* exit: tickFadeOutCallback until the fade-out is done */
    TASKCORE_STATE_FADED_OUT = 8,       /* update goes on to IntermediateBase's STOP */
    TASKCORE_STATE_CURSOR_MOVED = 9,    /* setActiveSlot, setSlotCursor */
    TASKCORE_STATE_START_PRESSED = 10,  /* onPadStart */
    TASKCORE_STATE_SLOT_CONFIRMED = 11, /* onPadConfirm while choosing a slot: runs confirmSlot */
    TASKCORE_STATE_SCROLL_OPENED = 14,  /* beginElementScroll */
    TASKCORE_STATE_ITEM_CONFIRMED = 15, /* onPadConfirm while scrolling: runs commitElementScroll */
    TASKCORE_STATE_SCROLL_COMMITTED = 16, /* commitElementScroll */
    TASKCORE_STATE_SCROLL_CANCELLED =
        17 /**< onPadCancel while scrolling, then cancelElementScroll: runs cancelElementScroll */
};

/** inputMode: what the pad events move. */
enum TaskCoreInputMode {
    TASKCORE_INPUT_NONE = 0,          /* fading: onPadEvent ignores the pad */
    TASKCORE_INPUT_CHOOSING_SLOT = 1, /* Up/Down move between the target's slots */
    TASKCORE_INPUT_SCROLLING = 2      /**< Up/Down move the active slot's item cursor */
};

/** @name Tones
 * playSound's tones: VabStreamObj__PlayTone indices, program << 4 | tone. @{ */
#define TASKCORE_TONE_CURSOR 0x00 /**< setActiveSlot, setSlotCursor: the cursor moved */
#define TASKCORE_TONE_BUTTON 0x10 /**< onPadStart, onPadConfirm, onPadCancel */
#define TASKCORE_TONE_VOLUME 96   /**< playSound's PlayTone vol and endVol */
/** @} */

/** The fades' end point: a GsBG/sprite colour of 128 draws the texture at its
 * own brightness. tickFadeIn counts up from baseColor and is done past it,
 * tickFadeOut counts down from it and is done when it wraps below 0. */
#define TASKCORE_FADE_FULL 128

/** setFrameBound's unit: DrawSystem__Init sets vsyncCount 3, so the game
 * draws 60 / 3 = 20 frames a second and frameCounter counts them;
 * setFrameBound(bound) is `bound` seconds. (StreamTask's override uses 15.) */
#define TASKCORE_FRAMES_PER_SECOND 20

/** The menu description setTarget builds its slot widgets from (the ctor's
 * first argument; TitleMenu passes &sTitleMenuTarget). One slot per `names`
 * entry. */
struct TaskCoreTarget {
    /* +0x000 */ const char *path; /**< non-NULL: setTarget loads `handle` from it (New_TimImage) and releaseTarget releases that */
    /* +0x004 */ BasicClass *handle; /**< New_TimImage(path), or the caller's own when path is NULL; the slot widgets' first argument */
    /* +0x008 */ s32 initialSlot; /**< setState(5): setActiveSlot(initialSlot, 0) */
    /* +0x00C */ s32 exitSlot; /**< confirmSlot: confirming this slot (one with no item list) runs exit, which fades the menu out */
    /* +0x010 */ u8 unselectedColor[3]; /**< broadcastToSlots at state 5; the colour a slot or item loses focus to */
    /* +0x013 */ u8 selectedColor[3]; /**< setActiveSlot's colour for the new slot */
    /* +0x016 */ u8 pad16[2];
    /* +0x018 */ void **hiddenSlots; /**< NULL entries are the slots find{Next,Prev}FreeSlot stop at */
    /* +0x01C */ char **names;       /**< NULL-terminated; one New_TextRow widget per name */
    /* +0x020 */ struct ScreenSpritePos *slotPositions; /**< per slot: updateSlotElements' position for its widget */
    /* +0x024 */ TaskCoreItemList **slotLists; /**< per slot: NULL, or the item list the slot opens */
};

/** TaskCore's slots, IntermediateBase's first. +0x074..+0x084 are
 * onPadEvent's cases, called with self alone: no occupant in any TaskCore
 * table reads the sender. */
/* clang-format off */
#define TASKCORE_SLOTS(Self, CtorParams)                                                           \
    INTERMEDIATEBASE_SLOTS(Self, CtorParams);                                                      \
    /* +0x06C */ void (*setFrameBound)(Self *self, s32 bound);   /**< @see TaskCore__SetFrameBound: frameBound = bound seconds of frames (negative: kept, no bound) */ \
    /* +0x070 */ void (*playSound)(Self *self, s32 tone);        /**< @see TaskCore__PlaySound */         \
    /* +0x074 */ void (*onPadStart)(Self *self);   /**< @see TaskCore__OnPadStart: onPadEvent's 0x21 */ \
    /* +0x078 */ void (*onPadConfirm)(Self *self); /**< @see TaskCore__OnPadConfirm: 0x19 */ \
    /* +0x07C */ void (*onPadCancel)(Self *self);  /**< @see TaskCore__OnPadCancel: 0x17 */ \
    /* +0x080 */ void (*onPadPrev)(Self *self);    /**< @see TaskCore__OnPadPrev: 0x12 */ \
    /* +0x084 */ void (*onPadNext)(Self *self);    /**< @see TaskCore__OnPadNext: 0x13 */ \
    /* +0x088 */ void *slot88;                                   /**< NULL; StreamTask__NoOpSlot88 */ \
    /* +0x08C */ void *slot8C;                                   /**< NULL; StreamTask__NoOpSlot8C */ \
    /* +0x090 */ void (*confirmSlot)(Self *self);                       /**< @see TaskCore__ConfirmSlot: setState(0xB)'s call; opens the slot's list, or exits on exitSlot */ \
    /* +0x094 */ void (*exit)(Self *self);           /**< @see TaskCore__Exit */  \
    /* +0x098 */ void (*setExitCallback)(Self *self, void (*callback)(void *ctx), void *ctx); /**< @see TaskCore__SetExitCallback */ \
    /* +0x09C */ void (*setFadeInCallbackEnabled)(Self *self, s32 enable);    /**< @see TaskCore__SetFadeInCallbackEnabled */ \
    /* +0x0A0 */ void (*setFadeOutCallbackEnabled)(Self *self, s32 enable); /**< @see TaskCore__SetFadeOutCallbackEnabled */ \
    /* +0x0A4 */ void (*setColors)(Self *self, u8 *base, u8 *clear, u8 *color96); /**< @see TaskCore__SetColors */ \
    /* +0x0A8 */ void (*setFadeRate)(Self *self, s32 rate);      /**< @see TaskCore__SetFadeRate */       \
    /* +0x0AC */ s32 (*tickFadeInCallback)(Self *self);            /**< @see TaskCore__TickFadeInCallback: update's state 4 */ \
    /* +0x0B0 */ s32 (*tickFadeIn)(Self *self);               /**< @see TaskCore__TickFadeIn: the fade-in callback */ \
    /* +0x0B4 */ void *slotB4;                                   /**< NULL in every TaskCore table */ \
    /* +0x0B8 */ void *slotB8;                                   /**< NULL */                        \
    /* +0x0BC */ void *slotBC;                                   /**< NULL */                        \
    /* +0x0C0 */ s32 (*tickFadeOutCallback)(Self *self);         /**< @see TaskCore__TickFadeOutCallback: update's state 7 */ \
    /* +0x0C4 */ s32 (*tickFadeOut)(Self *self);               /**< @see TaskCore__TickFadeOut: the fade-out callback */ \
    /* +0x0C8 */ void *slotC8;                                   /**< NULL */                        \
    /* +0x0CC */ void *slotCC;                                   /**< NULL */                        \
    /* +0x0D0 */ void *slotD0;                                   /**< NULL */                        \
    /* +0x0D4 */ void (*setSubHandle)(Self *self, const char *path, BasicClass *handle); /**< @see TaskCore__SetSubHandle; GraphRoom passes "ETC\HGRAPH.TIM" */ \
    /* +0x0D8 */ void (*setTarget)(Self *self, TaskCoreTarget *target); /**< @see TaskCore__SetTarget */  \
    /* +0x0DC */ void (*releaseTarget)(Self *self);              /**< @see TaskCore__ReleaseTarget */     \
    /* +0x0E0 */ void (*updateSlotElements)(Self *self, void *parent); /**< @see TaskCore__UpdateSlotElements */ \
    /* +0x0E4 */ void (*broadcastToSlots)(Self *self, u8 *color); /**< @see TaskCore__BroadcastToSlots */ \
    /* +0x0E8 */ void (*findNextFreeSlot)(Self *self);           /**< @see TaskCore__FindNextFreeSlot */  \
    /* +0x0EC */ void (*findPrevFreeSlot)(Self *self);           /**< @see TaskCore__FindPrevFreeSlot */  \
    /* +0x0F0 */ void (*setActiveSlot)(Self *self, s32 slot, s32 withSound); /**< @see TaskCore__SetActiveSlot */ \
    /* +0x0F4 */ s32 (*getActiveSlot)(Self *self);               /**< @see TaskCore__GetActiveSlot */     \
    /* +0x0F8 */ void (*createSlotElements)(Self *self, TaskCoreItemList *list, void *handle); /**< @see TaskCore__CreateSlotElements */ \
    /* +0x0FC */ void (*releaseSlotElements)(Self *self);        /**< @see TaskCore__ReleaseSlotElements */ \
    /* +0x100 */ void (*refreshSlotView)(Self *self, void *parent, s32 show); /**< @see TaskCore__RefreshSlotView */ \
    /* +0x104 */ void (*broadcastToSlotElements)(Self *self, void *color); /**< @see TaskCore__BroadcastToSlotElements */ \
    /* +0x108 */ void (*beginElementScroll)(Self *self);         /**< @see TaskCore__BeginElementScroll */ \
    /* +0x10C */ void (*commitElementScroll)(Self *self);        /**< @see TaskCore__CommitElementScroll */ \
    /* +0x110 */ void (*cancelElementScroll)(Self *self);        /**< @see TaskCore__CancelElementScroll */ \
    /* +0x114 */ void (*advanceSlotCursor)(Self *self);          /**< @see TaskCore__AdvanceSlotCursor */ \
    /* +0x118 */ void (*retreatSlotCursor)(Self *self);          /**< @see TaskCore__RetreatSlotCursor */ \
    /* +0x11C */ void (*setSlotCursor)(Self *self, s32 cursor, s32 withSound); /**< @see TaskCore__SetSlotCursor */ \
    /* +0x120 */ s32 (*getActiveItemCursor)(Self *self)           /**< @see TaskCore__GetActiveItemCursor */
/* clang-format on */

/** TaskCore's fields, IntermediateBase's first. The sound and widget
 * objects are declared `BasicClass *`, the parent every one of them has; a
 * unit that calls one past BasicClass's slots casts to its own class. */
/* clang-format off */
#define TASKCORE_FIELDS(Methods)                                                                   \
    INTERMEDIATEBASE_FIELDS(Methods);                                                              \
    /* +0x028 */ s32 otLength;          /**< reset: 3; onInit: the viewport's setOtLength (+0x048) */ \
    /* +0x02C */ s32 maxPackets;        /**< reset: 300 (TitleMenu, GraphRoom: 400); onInit: the viewport's setMaxPackets */ \
    /* +0x030 */ s32 packetSize;        /**< reset: 64; onInit: the viewport's setPacketSize */      \
    /* +0x034 */ s32 clearOnDeinit;     /**< reset: 1 (TitleMenu 0); nonzero: onDeinit clears the screen to clearColor */ \
    /* +0x038 */ s32 result;            /**< TaskCore__Init returns it; onInit 0, setState(6) 1 */   \
    /* +0x03C */ s32 inputMode;         /**< 0 none, 1 choosing a slot, 2 scrolling its items; onPadEvent needs nonzero */ \
    /* +0x040 */ s32 frameBound;        /**< setFrameBound; update: frameCounter past it is setState(6) */ \
    /* +0x044 */ char *soundBankPath;   /**< the ctor's; nonzero: finalize releases `sound` */       \
    /* +0x048 */ BasicClass *sound;     /**< New_VabStreamObj(soundBankPath) or the ctor's own; playSound's target */ \
    /* +0x04C */ TaskCoreTarget *target; /**< setTarget */                                           \
    /* +0x050 */ s32 slotCount;         /**< target->names' length */                               \
    /* +0x054 */ BasicClass **slotElements; /**< one widget a slot (New_TextRow) */                \
    /* +0x058 */ s32 activeSlot;        /**< the highlighted slot; the slot the per-slot methods act on */ \
    /* +0x05C */ s32 *itemCounts;       /**< per slot: its item list's length */                     \
    /* +0x060 */ s32 *itemCursors;       /**< per slot: the item cursor, a ring over itemCounts */    \
    /* +0x064 */ void **itemLists;      /**< per slot: its item widgets (createSlotElements) */      \
    /* +0x068 */ BasicClass *listView;  /**< New_BoxFill: the frame around the scrolled list */    \
    /* +0x06C */ u8 pad06C[4];                                                                     \
    /* +0x070 */ const char *subHandlePath; /**< setSubHandle's path; nonzero: the handle is owned */ \
    /* +0x074 */ BasicClass *subHandle; /**< New_TimImage(subHandlePath), or the caller's; NULL: onInit also passes baseColor with sDefaultMovieFrame */ \
    /* +0x078 */ struct BgLayer *bgLayer; /**< New_BgLayer(tileMap, BGLAYER_MODE_SCREEN); bg_layer.h */ \
    /* +0x07C */ struct TileMap *tileMap; /**< New_TileMap(0, tileAtlas); tile_map.h */ \
    /* +0x080 */ struct TileAtlas *tileAtlas; /**< New_TileAtlas(0); tile_atlas.h */ \
    /* +0x084 */ s32 fadeRate;          /**< setFadeRate; reset: 9 */                                \
    /* +0x088 */ s32 (*fadeInCallback)(TaskCore *self);  /**< setFadeInCallbackEnabled: NULL or tickFadeIn; nonzero: onInit sets baseColor */ \
    /* +0x08C */ s32 (*fadeOutCallback)(TaskCore *self); /**< setFadeOutCallbackEnabled: NULL or tickFadeOut */ \
    /* +0x090 */ u8 baseColor[3];       /**< setColors; the fade-in's start colour */                \
    /* +0x093 */ u8 clearColor[3];      /**< setColors; onDeinit (clearOnDeinit set) and TitleMenu's clear the screen to it */                                            \
    /* +0x096 */ u8 unk96[3];           /**< setColors (reset: 128 grey); no code reads it */                                            \
    /* +0x099 */ u8 pad099[3];                                                                     \
    /* +0x09C */ void (*exitCallback)(void *ctx); /**< setExitCallback; exit calls it */     \
    /* +0x0A0 */ void *exitCallbackCtx  /**< exitCallback's argument */
/* clang-format on */

/** TaskCore's method table: IntermediateBase's slots and TASKCORE_SLOTS'
 * own. The ctor takes the menu description, a sound bank path and a sound
 * object. */
struct TaskCoreMethods {
    TASKCORE_SLOTS(TaskCore,
                   (TaskCore * self, TaskCoreTarget *target, char *soundBankPath, BasicClass *sound));
};

/**
 * TaskCore -- the base of the game's menu and screen tasks: class id 0x130,
 * method table gTaskCoreMethods, an IntermediateBase subclass
 * (intermediate_base.h). Methods in src/app/task.c. The object is 0xA4 bytes
 * (New_TaskCore). Three classes derive from it, each ctor calling
 * TaskCore__TaskCore first: StreamTask (0x1130, stream_task.h), TitleMenu
 * (0x1F130, title_menu.h) and GraphRoom (0x2F130, graph_room.h). Used on its
 * own, it shows one TIM image for a frame bound (GameApplication__ShowImage).
 *
 * Construction, ctor(target, soundBankPath, sound): the base ctor, then
 * setTarget(target), `sound` = New_VabStreamObj(soundBankPath) when a path
 * is given (both menu subclasses pass "ETC\ETCSE") or the caller's object,
 * setSubHandle(NULL, NULL), a TileAtlas -> TileMap -> BgLayer chain, and
 * resetCounters (TaskCore__Reset: colours, fade callbacks, fadeRate 9, the
 * three viewport words). Finalize releases what the ctor made (`sound` only
 * when it made it) and runs releaseTarget.
 *
 * init/deinit (IntermediateBase's) call onInit/onDeinit: onInit hangs the
 * slot widgets and the BgLayer under the light rig, sets the colours,
 * configures the viewport (setOtLength/setMaxPackets/setPacketSize take
 * otLength/maxPackets/packetSize) and opens its OT; onDeinit closes it.
 * TaskCore__Init returns `result`.
 *
 * The state machine (setState, update). IntermediateBase's update counts
 * frames; TaskCore's then steps the state: START -> FADE_IN
 * (tickFadeInCallback runs fadeInCallback, TickFadeIn, until it reports
 * done) -> ACTIVE (the target's initialSlot selected, inputMode
 * CHOOSING_SLOT) ... FADE_OUT (tickFadeOutCallback, TickFadeOut) ->
 * FADED_OUT -> STOP (IntermediateBase's onStop). While inputMode is not
 * NONE, frameCounter passing frameBound is setState(TIMED_OUT): result = 1,
 * then exit, which calls exitCallback and goes to FADE_OUT. The states from
 * CURSOR_MOVED on set the state back to ACTIVE and reset the frame counter;
 * SLOT_CONFIRMED runs confirmSlot, ITEM_CONFIRMED commitElementScroll,
 * SCROLL_CANCELLED cancelElementScroll.
 *
 * Input. onPadEvent (IntermediateBase's Pad case) is a switch on the event
 * while inputMode is not NONE: Up pressed is onPadPrev, Down onPadNext,
 * cross onPadCancel, circle onPadConfirm, Start onPadStart. CHOOSING_SLOT
 * moves between the target's slots (find{Next,Prev}FreeSlot, setActiveSlot);
 * SCROLLING moves through the active slot's item list (beginElementScroll
 * from confirmSlot, then {advance,retreat}SlotCursor and commit/cancel).
 * Confirm, cancel and Start call playSound(TASKCORE_TONE_BUTTON),
 * VabStreamObj's PlayTone on `sound`; setActiveSlot and setSlotCursor call
 * playSound(TASKCORE_TONE_CURSOR) when their last argument is nonzero.
 *
 * The picker. The first level is the slots: setTarget makes one TextRow per
 * target->names entry (slotElements), findNextFreeSlot/findPrevFreeSlot
 * step, wrapping, to the next slot whose hiddenSlots entry is NULL, and
 * setActiveSlot moves the highlight from unselectedColor to selectedColor. A
 * slot whose target->slotLists entry is non-NULL also has a list of items:
 * createSlotElements makes its rows (itemLists, itemCounts) and starts its
 * cursor (itemCursors) at the list's saved cursor. The second level scrolls
 * that list: beginElementScroll shows every row with listView's frame behind
 * them and the cursor's row in the list's cursor colour;
 * advanceSlotCursor/retreatSlotCursor move the cursor, wrapping, through
 * setSlotCursor; commitElementScroll saves the cursor and leaves only its row
 * shown; cancelElementScroll goes back to the saved cursor.
 *
 * Subclass conventions. StreamTask's +0x044 override StreamTask__Init takes
 * (args, streamName, streamGroup, autoPlay) where IntermediateBase's init
 * takes (args, mode): the table keeps the inherited slot and its callers
 * cast to StreamTaskInitFn. TitleMenu's and GraphRoom's ctors call
 * resetCounters with the DreamSys through a cast slot (TitleMenuResetCallFn).
 * IntermediateBase's onInit slot is (self, s32, s32, s32), from init's call;
 * TaskCore__OnInit and StreamTask's override take self alone, and the one
 * up-call that passes self alone casts the slot.
 *
 * The viewport (IntermediateBase's field) is a Viewport (viewport.h); its
 * callers cast to that type. bgLayer, tileMap and tileAtlas are declared by
 * struct tag (bg_layer.h, tile_map.h, tile_atlas.h), so a unit that calls one
 * includes that header.
 */
struct TaskCore {
    TASKCORE_FIELDS(TaskCoreMethods);
};

/** TaskCore's own method table. */
extern TaskCoreMethods gTaskCoreMethods;

/** @brief TaskCore's method-table getter.
 * @return &gTaskCoreMethods */
extern TaskCoreMethods *GetTaskCoreMethods(void);

/** @brief Allocates a TaskCore from the pool and runs its ctor.
 * @param target the menu description, or NULL for a task without slots
 * @param soundBankPath a sound bank to load into a VabStreamObj, or NULL
 * @param sound the sound object to use while soundBankPath is NULL
 * @return the new task, or NULL when the pool allocation fails */
TaskCore *New_TaskCore(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound);

/** @brief Constructor: IntermediateBase's, this table, setTarget, the sound
 * object, no sub handle, the TileAtlas -> TileMap -> BgLayer chain, and
 * resetCounters.
 * @param self the task
 * @param target the menu description, or NULL
 * @param soundBankPath a sound bank to load, or NULL
 * @param sound the sound object to use while soundBankPath is NULL */
void TaskCore__TaskCore(TaskCore *self, TaskCoreTarget *target, char *soundBankPath, BasicClass *sound);

/** @brief Finalize: releases the BgLayer chain, the sound object and sub
 * handle when this task made them, then releaseTarget and
 * IntermediateBase's finalize.
 * @param self the task */
void TaskCore__Finalize(TaskCore *self);

/** @brief resetCounters override: no frame bound, the default colours, both
 * fade callbacks enabled, fadeRate 9, the viewport's OT length 3 with 300
 * packets of 64 bytes, no exit callback, clearOnDeinit set, input off.
 * @param self the task */
void TaskCore__Reset(TaskCore *self);

/** @brief init override: IntermediateBase's init, then the result.
 * @param self the task
 * @param args the objects to work with
 * @param mode INTERMEDIATEBASE_INIT_RUN runs the task to its end inside this call
 * @return `result`: 0, or 1 when the frame bound ran out */
s32 TaskCore__Init(TaskCore *self, IntermediateBaseInitArgs *args, s32 mode);

/** @brief onInit: hangs the slot widgets and the BgLayer under the light
 * rig, sets the fade-in colour, clears the default movie frame (no sub
 * handle) and the screen to baseColor, and configures and opens the
 * viewport's OT; result = 0.
 * @param self the task */
void TaskCore__OnInit(TaskCore *self);

/** @brief onDeinit: closes the viewport's OT, detaches the view and the
 * BgLayer, and clears the screen to clearColor while clearOnDeinit is set.
 * @param self the task */
void TaskCore__OnDeinit(TaskCore *self);

/** @brief onPadEvent: while input is on, maps Up, Down, Start, cross and
 * circle presses to onPadPrev, onPadNext, onPadStart, onPadCancel and
 * onPadConfirm.
 * @param self the task
 * @param sender the Pad
 * @param event the Pad's event code */
void TaskCore__OnPadEvent(TaskCore *self, BasicClass *sender, s32 event);

/** @brief update: IntermediateBase's frame count, the frame-bound timeout,
 * then the state step (START to FADE_IN, the fade ticks, FADED_OUT to STOP).
 * @param self the task
 * @param sender the FrameClock
 * @param event its event code */
void TaskCore__Update(TaskCore *self, BasicClass *sender, s32 event);

/** @brief setState override: IntermediateBase's, then TaskCore's own
 * transitions (enum TaskCoreState).
 * @param self the task
 * @param state the new state */
void TaskCore__SetState(TaskCore *self, s32 state);

/** @brief Sets the frame bound: `bound` seconds of frames
 * (TASKCORE_FRAMES_PER_SECOND each); a negative bound is kept as is and
 * never passes.
 * @param self the task
 * @param bound seconds, or negative for no bound */
void TaskCore__SetFrameBound(TaskCore *self, s32 bound);

/** @brief Plays `tone` on the sound object, if there is one.
 * @param self the task
 * @param tone TASKCORE_TONE_CURSOR or TASKCORE_TONE_BUTTON */
void TaskCore__PlaySound(TaskCore *self, s32 tone);

/** @brief With a target: the button tone, then setState(START_PRESSED).
 * @param self the task */
void TaskCore__OnPadStart(TaskCore *self);

/** @brief With a target: the button tone, then setState(SLOT_CONFIRMED)
 * while choosing a slot, else ITEM_CONFIRMED.
 * @param self the task */
void TaskCore__OnPadConfirm(TaskCore *self);

/** @brief With a target and while not choosing a slot: the button tone, then
 * setState(SCROLL_CANCELLED).
 * @param self the task */
void TaskCore__OnPadCancel(TaskCore *self);

/** @brief With a target: findPrevFreeSlot while choosing a slot,
 * retreatSlotCursor while scrolling.
 * @param self the task */
void TaskCore__OnPadPrev(TaskCore *self);

/** @brief With a target: findNextFreeSlot while choosing a slot,
 * advanceSlotCursor while scrolling.
 * @param self the task */
void TaskCore__OnPadNext(TaskCore *self);

/** @brief Opens the active slot's item list (beginElementScroll), or runs
 * exit when the slot has no list and is the target's exitSlot.
 * @param self the task */
void TaskCore__ConfirmSlot(TaskCore *self);

/** @brief Calls the exit callback, if set, then setState(FADE_OUT).
 * @param self the task */
void TaskCore__Exit(TaskCore *self);

/** @brief Sets the callback exit calls before fading out.
 * @param self the task
 * @param callback the function, or NULL
 * @param ctx its argument */
void TaskCore__SetExitCallback(TaskCore *self, void (*callback)(void *ctx), void *ctx);

/** @brief Enables (1: tickFadeIn) or disables (0: none, the fade-in ends at
 * once) the fade-in callback; other values change nothing.
 * @param self the task
 * @param enable 0 or 1 */
void TaskCore__SetFadeInCallbackEnabled(TaskCore *self, s32 enable);

/** @brief Enables (1: tickFadeOut) or disables (0) the fade-out callback;
 * other values change nothing.
 * @param self the task
 * @param enable 0 or 1 */
void TaskCore__SetFadeOutCallbackEnabled(TaskCore *self, s32 enable);

/** @brief Copies three RGB colours: the fade-in's start, the colour onDeinit
 * clears the screen to, and a third no code reads.
 * @param self the task
 * @param base the new baseColor
 * @param clear the new clearColor
 * @param color96 the new unk96 */
void TaskCore__SetColors(TaskCore *self, u8 *base, u8 *clear, u8 *color96);

/** @brief Sets the colour step per frame of both fades.
 * @param self the task
 * @param rate the step */
void TaskCore__SetFadeRate(TaskCore *self, s32 rate);

/** @brief One fade-in step: runs the fade-in callback (done at once when
 * there is none) and goes to ACTIVE when it is done.
 * @param self the task
 * @return nonzero once the fade-in is done */
s32 TaskCore__TickFadeInCallback(TaskCore *self);

/** @brief The fade-in callback: pushes baseColor plus frameCounter * fadeRate
 * to every slot widget and the BgLayer.
 * @param self the task
 * @return nonzero once the level passes TASKCORE_FADE_FULL */
s32 TaskCore__TickFadeIn(TaskCore *self);

/** @brief One fade-out step: runs the fade-out callback (done at once when
 * there is none) and goes to FADED_OUT when it is done.
 * @param self the task
 * @return nonzero once the fade-out is done */
s32 TaskCore__TickFadeOutCallback(TaskCore *self);

/** @brief The fade-out callback: pushes the grey level TASKCORE_FADE_FULL -
 * frameCounter * fadeRate to every slot widget and the BgLayer.
 * @param self the task
 * @return nonzero once the level wraps below 0 */
s32 TaskCore__TickFadeOut(TaskCore *self);

/** @brief Sets the task's image: with a path, loads it as a TimImage,
 * uploads it and frees its file buffer (releasing an image it loaded
 * before); without, adopts `handle`.
 * @param self the task
 * @param path the TIM to load, or NULL
 * @param handle the image to use while path is NULL */
void TaskCore__SetSubHandle(TaskCore *self, const char *path, BasicClass *handle);

/** @brief Builds the slot widgets for a menu description: one TextRow per
 * name, the per-slot arrays, each slot's item list, and the list frame; with
 * target->path, loads the texture into target->handle.
 * @param self the task
 * @param target the menu description, or NULL for none */
void TaskCore__SetTarget(TaskCore *self, TaskCoreTarget *target);

/** @brief Releases everything setTarget built, and the texture when it loaded
 * it.
 * @param self the task */
void TaskCore__ReleaseTarget(TaskCore *self);

/** @brief Attaches each visible slot's widget under `parent` at its position
 * (with its item list, collapsed) and detaches each hidden one.
 * @param self the task
 * @param parent the node to attach to */
void TaskCore__UpdateSlotElements(TaskCore *self, void *parent);

/** @brief Sets every slot widget, and every item of each slot's list, to one
 * colour.
 * @param self the task
 * @param color an RGB triple */
void TaskCore__BroadcastToSlots(TaskCore *self, void *color);

/** @brief Moves the highlight to the next slot not hidden, wrapping, with the
 * cursor tone.
 * @param self the task */
void TaskCore__FindNextFreeSlot(TaskCore *self);

/** @brief Moves the highlight to the previous slot not hidden, wrapping, with
 * the cursor tone.
 * @param self the task */
void TaskCore__FindPrevFreeSlot(TaskCore *self);

/** @brief Moves the highlight: the old slot to unselectedColor, `slot` to
 * selectedColor, then setState(CURSOR_MOVED).
 * @param self the task
 * @param slot the slot to highlight
 * @param withSound nonzero plays the cursor tone */
void TaskCore__SetActiveSlot(TaskCore *self, s32 slot, s32 withSound);

/** @brief The highlighted slot.
 * @param self the task
 * @return activeSlot */
s32 TaskCore__GetActiveSlot(TaskCore *self);

/** @brief Makes one TextRow per item name of the active slot's list and
 * starts the slot's cursor at the list's saved cursor.
 * @param self the task
 * @param list the active slot's item list
 * @param texture the texture the rows draw from */
void TaskCore__CreateSlotElements(TaskCore *self, TaskCoreItemList *list, void *texture);

/** @brief Releases the active slot's item rows and frees their array.
 * @param self the task */
void TaskCore__ReleaseSlotElements(TaskCore *self);

/** @brief Lays out the active slot's item rows under `parent`, the saved
 * cursor's row at the list's position; `show` makes every row and the frame
 * behind them visible, otherwise only the saved cursor's row shows.
 * @param self the task
 * @param parent the node to attach the rows to
 * @param show nonzero opens the list */
void TaskCore__RefreshSlotView(TaskCore *self, void *parent, s32 show);

/** @brief Sets every item row of the active slot to one colour.
 * @param self the task
 * @param color an RGB triple */
void TaskCore__BroadcastToSlotElements(TaskCore *self, void *color);

/** @brief While choosing a slot: opens the active slot's list, colours the
 * cursor's row, switches input to SCROLLING and setState(SCROLL_OPENED).
 * @param self the task */
void TaskCore__BeginElementScroll(TaskCore *self);

/** @brief While scrolling: collapses the list to the cursor's row, saves the
 * cursor, hides the frame, returns input to CHOOSING_SLOT and
 * setState(SCROLL_COMMITTED).
 * @param self the task */
void TaskCore__CommitElementScroll(TaskCore *self);

/** @brief While scrolling: collapses the list back to the saved cursor,
 * returns input to CHOOSING_SLOT and setState(SCROLL_CANCELLED).
 * @param self the task */
void TaskCore__CancelElementScroll(TaskCore *self);

/** @brief Moves the active slot's item cursor one down, wrapping.
 * @param self the task */
void TaskCore__AdvanceSlotCursor(TaskCore *self);

/** @brief Moves the active slot's item cursor one up, wrapping.
 * @param self the task */
void TaskCore__RetreatSlotCursor(TaskCore *self);

/** @brief Moves the active slot's item cursor: the old row to
 * unselectedColor, the new one to the list's cursor colour, then
 * setState(CURSOR_MOVED).
 * @param self the task
 * @param cursor the item to move to
 * @param withSound nonzero plays the cursor tone */
void TaskCore__SetSlotCursor(TaskCore *self, s32 cursor, s32 withSound);

/** @brief The active slot's item cursor.
 * @param self the task
 * @return the cursor */
s32 TaskCore__GetActiveItemCursor(TaskCore *self);

#endif
