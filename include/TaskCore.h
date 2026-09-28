#ifndef TASKCORE_H
#define TASKCORE_H

#include "IntermediateBase.h"

/*
 * TaskCore -- class id 0x130, method table gTaskCoreMethods: the
 * IntermediateBase subclass behind the game's menu/screen tasks. Its methods
 * are in src/app/Task.c. The object is 0xA4 bytes (New_TaskCore). Three classes derive from
 * it, each ctor calling TaskCore__TaskCore first (`typeviews.py --tree`):
 * StreamTask (0x1130, gStreamTaskMethods, include/StreamTask.h), TitleMenu
 * (0x1F130, gTitleMenuMethods, include/TitleMenu.h) and GraphRoom (0x2F130, include/GraphRoom.h).
 *
 * Construction, ctor(target, soundBankPath, sound): the base ctor, then
 * setTarget(target), `sound` = New_VabStreamObj(soundBankPath) when a path
 * is given (both subclass ctors pass "ETC\ETCSE") or the caller's object,
 * setSubHandle(NULL, NULL), a TileAtlas -> TileMap -> BgLayer chain, and
 * resetCounters (TaskCore__Reset: colours, fade callbacks, fadeRate 9,
 * the three viewport words). Finalize releases what the ctor made
 * (`sound` only when it made it) and runs releaseTarget.
 *
 * init/deinit (IntermediateBase's) call onInit/onDeinit: onInit hangs the
 * slot widgets and the BgLayer under +0x014, sets the colours, configures
 * the viewport (its +0x048/+0x04C/+0x050 take otLength/unk2C/packetSize) and opens
 * its OT; onDeinit closes it. TaskCore__Init returns `result`.
 *
 * The state machine (setState, update). IntermediateBase's update counts
 * frames; TaskCore's then steps the state: 2 -> 4 (fade in: tickFadeCallback
 * runs fadeInCallback, TickColorFade, until it reports done) -> 5 (active:
 * the target's slot unk8 selected, inputMode 1) ... 7 (fade out:
 * tickFadeOutCallback, TickFadeColor) -> 8 -> 3 (IntermediateBase's
 * onState3). While inputMode is nonzero, frameCounter passing frameBound
 * is setState(6): result = 1, then refreshViewValue, which calls
 * viewCallback and goes to 7. States 9..0x11 set state 5 and reset the
 * frame counter; 0xB runs tick, 0xF commitElementScroll, 0x11
 * cancelElementScroll.
 *
 * Input. onPadEvent (IntermediateBase's Pad case) is a switch on the event
 * while inputMode is nonzero: 0x12 onPadPrev, 0x13 onPadNext, 0x17
 * onPadCancel, 0x19 onPadConfirm, 0x21 onPadStart. inputMode 1 moves between
 * the target's slots (find{Next,Prev}FreeSlot, setActiveSlot), 2 scrolls
 * the active slot's item list (beginElementScroll from tick, then
 * {advance,retreat}SlotCursor and commit/cancel). Confirm, cancel and 0x21
 * call playSound(0x10), VabStreamObj's PlayTone on `sound`; setActiveSlot
 * and setSlotCursor call playSound(0) when their last argument is nonzero.
 *
 * StreamTask (include/StreamTask.h) expands these macros; its
 * +0x044 override StreamTask__Init takes (args, streamName, streamGroup,
 * autoPlay) where IntermediateBase's init takes (args, mode): the table
 * keeps the inherited slot and GameApplicationFileResource's callers cast to
 * StreamTaskInitFn. TitleMenu (include/TitleMenu.h) expands
 * these macros too; its ctor's resetCounters call passes dreamSys and casts
 * the slot to TitleMenuResetCallFn, as GraphRoom's does. GraphRoom
 * (include/GraphRoom.h) expands these macros; its ctor is void
 * like every other.
 *
 * IntermediateBase's onInit slot is (self, s32, s32, s32), from init's
 * call; TaskCore__OnInit and StreamTask's override take self alone, and
 * the one up-call that passes self alone casts the slot.
 *
 * The sound and widget objects TaskCore holds (VabStreamObj, the slot and
 * list widgets) are declared `BasicClass *`, the parent every one of them
 * has; a unit that calls one past BasicClass's slots casts to its own view
 * of it. The viewport (IntermediateBase's field) is a Viewport
 * (include/Viewport.h); its callers cast to that type. bgLayer is a
 * `struct BgLayer *` (include/BgLayer.h), by tag, so a unit that calls it
 * includes BgLayer.h. tileMap likewise is a `struct TileMap *`
 * (include/TileMap.h), and tileAtlas a `struct TileAtlas *`
 * (include/TileAtlas.h).
 */

typedef struct TaskCore TaskCore;
typedef struct TaskCoreMethods TaskCoreMethods;
typedef struct TaskCoreTarget TaskCoreTarget;

/* TaskCore's states, above IntermediateBase's START/STOP. setState passes
 * each to the parents (notifyParents) before acting on it, so the codes from
 * CURSOR_MOVED on are also events a parent reacts to (TitleMenu__SetState's
 * START_PRESSED); every one of them returns the menu to ACTIVE. */
enum TaskCoreState {
    TASKCORE_STATE_FADE_IN = 4, /* update from START: tickFadeCallback until the fade-in is done */
    TASKCORE_STATE_ACTIVE = 5,  /* the target's unk8 slot selected, inputMode CHOOSING_SLOT */
    TASKCORE_STATE_TIMED_OUT = 6, /* update: frameCounter passed frameBound; result 1, refreshViewValue */
    TASKCORE_STATE_FADE_OUT = 7, /* refreshViewValue: tickFadeOutCallback until the fade-out is done */
    TASKCORE_STATE_FADED_OUT = 8,       /* update goes on to IntermediateBase's STOP */
    TASKCORE_STATE_CURSOR_MOVED = 9,    /* setActiveSlot, setSlotCursor */
    TASKCORE_STATE_START_PRESSED = 10,  /* onPadStart */
    TASKCORE_STATE_SLOT_CONFIRMED = 11, /* onPadConfirm while choosing a slot: runs tick */
    TASKCORE_STATE_SCROLL_OPENED = 14,  /* beginElementScroll */
    TASKCORE_STATE_ITEM_CONFIRMED = 15, /* onPadConfirm while scrolling: runs commitElementScroll */
    TASKCORE_STATE_SCROLL_COMMITTED = 16, /* commitElementScroll */
    TASKCORE_STATE_SCROLL_CANCELLED =
        17 /* onPadCancel while scrolling, then cancelElementScroll: runs cancelElementScroll */
};

/* inputMode: what the pad events move. */
enum TaskCoreInputMode {
    TASKCORE_INPUT_NONE = 0,          /* fading: onPadEvent ignores the pad */
    TASKCORE_INPUT_CHOOSING_SLOT = 1, /* Up/Down move between the target's slots */
    TASKCORE_INPUT_SCROLLING = 2      /* Up/Down move the active slot's item cursor */
};

/* playSound's tones: VabStreamObj__PlayTone indices, program << 4 | tone. */
#define TASKCORE_TONE_CURSOR 0x00 /* setActiveSlot, setSlotCursor: the cursor moved */
#define TASKCORE_TONE_BUTTON 0x10 /* onPadStart, onPadConfirm, onPadCancel */
#define TASKCORE_TONE_VOLUME 96   /* playSound's PlayTone vol and endVol */

/* The fades' end point: a GsBG/sprite colour of 128 draws the texture at its
 * own brightness. tickColorFade counts up from baseColor and is done past it,
 * tickFadeColor counts down from it and is done when it wraps below 0. */
#define TASKCORE_FADE_FULL 128

/* setFrameBound's unit: DrawSystem__Init sets vsyncCount 3, so the game
 * draws 60 / 3 = 20 frames a second and frameCounter counts them;
 * setFrameBound(bound) is `bound` seconds. (StreamTask's override uses 15.) */
#define TASKCORE_FRAMES_PER_SECOND 20

/* The menu description setTarget builds its slot widgets from (the ctor's
 * first argument; TitleMenu passes &sTitleMenuTarget). One slot per `names`
 * entry. */
struct TaskCoreTarget {
    /* +0x000 */ const char *path; /* non-NULL: setTarget loads `handle` from it (New_TimImage) and releaseTarget releases that */
    /* +0x004 */ BasicClass *handle; /* New_TimImage(path), or the caller's own when path is NULL; the slot widgets' first argument */
    /* +0x008 */ s32 unk8;     /* setState(5): setActiveSlot(unk8, 0) */
    /* +0x00C */ s32 exitSlot; /* tick: confirming this slot (one with no item list) runs refreshViewValue, which fades the menu out */
    /* +0x010 */ u8 unselectedColor[3]; /* broadcastToSlots at state 5; the colour a slot or item loses focus to */
    /* +0x013 */ u8 selectedColor[3]; /* setActiveSlot's colour for the new slot */
    /* +0x016 */ u8 pad16[2];
    /* +0x018 */ void **registrationSlots; /* NULL entries are the slots find{Next,Prev}FreeSlot stop at */
    /* +0x01C */ char **names;             /* NULL-terminated; one New_TextRow widget per name */
    /* +0x020 */ u8 *externalRecords; /* 8 bytes a slot, updateSlotElements' position for each widget */
    /* +0x024 */ void **unk24; /* per slot: NULL, or the item-list record createSlotElements and the scroll methods read */
};

/* clang-format off */
#define TASKCORE_SLOTS(Self, CtorParams)                                                           \
    INTERMEDIATEBASE_SLOTS(Self, CtorParams);                                                      \
    /* +0x06C */ void (*setFrameBound)(Self *self, s32 bound);   /* TaskCore__SetFrameBound: frameBound = bound seconds of frames (negative: kept, no bound) */ \
    /* +0x070 */ void (*playSound)(Self *self, s32 tone);        /* TaskCore__PlaySound */         \
    /* +0x074..+0x084: onPadEvent's cases. Called with self alone: $a1 still  \
     * holds the sender at that call, but no occupant in any of the four     \
     * tables reads it, and StreamTask's overrides up-call with self only. */ \
    /* +0x074 */ void (*onPadStart)(Self *self);   /* TaskCore__OnPadStart: onPadEvent's 0x21 */ \
    /* +0x078 */ void (*onPadConfirm)(Self *self); /* TaskCore__OnPadConfirm: 0x19 */ \
    /* +0x07C */ void (*onPadCancel)(Self *self);  /* TaskCore__OnPadCancel: 0x17 */ \
    /* +0x080 */ void (*onPadPrev)(Self *self);    /* TaskCore__OnPadPrev: 0x12 */ \
    /* +0x084 */ void (*onPadNext)(Self *self);    /* TaskCore__OnPadNext: 0x13 */ \
    /* +0x088 */ void *slot88;                                   /* NULL; StreamTask__NoOpSlot88 */ \
    /* +0x08C */ void *slot8C;                                   /* NULL; StreamTask__NoOpSlot8C */ \
    /* +0x090 */ void (*tick)(Self *self);                       /* TaskCore__ConfirmSlot: setState(0xB) */ \
    /* +0x094 */ void (*refreshViewValue)(Self *self);           /* TaskCore__Exit */  \
    /* +0x098 */ void (*setCallback)(Self *self, void (*callback)(void *ctx), void *ctx); /* TaskCore__SetCallback */ \
    /* +0x09C */ void (*setFadeCallbackEnabled)(Self *self, s32 enable);    /* TaskCore__SetFadeCallbackEnabled */ \
    /* +0x0A0 */ void (*setFadeOutCallbackEnabled)(Self *self, s32 enable); /* TaskCore__SetFadeOutCallbackEnabled */ \
    /* +0x0A4 */ void (*setColors)(Self *self, u8 *base, u8 *clear, u8 *color96); /* TaskCore__SetColors */ \
    /* +0x0A8 */ void (*setFadeRate)(Self *self, s32 rate);      /* TaskCore__SetFadeRate */       \
    /* +0x0AC */ s32 (*tickFadeCallback)(Self *self);            /* TaskCore__TickFadeCallback: update's state 4 */ \
    /* +0x0B0 */ s32 (*tickColorFade)(Self *self);               /* TaskCore__TickFadeIn: the fade-in callback */ \
    /* +0x0B4 */ void *slotB4;                                   /* NULL in every TaskCore table */ \
    /* +0x0B8 */ void *slotB8;                                   /* NULL */                        \
    /* +0x0BC */ void *slotBC;                                   /* NULL */                        \
    /* +0x0C0 */ s32 (*tickFadeOutCallback)(Self *self);         /* TaskCore__TickFadeOutCallback: update's state 7 */ \
    /* +0x0C4 */ s32 (*tickFadeColor)(Self *self);               /* TaskCore__TickFadeOut: the fade-out callback */ \
    /* +0x0C8 */ void *slotC8;                                   /* NULL */                        \
    /* +0x0CC */ void *slotCC;                                   /* NULL */                        \
    /* +0x0D0 */ void *slotD0;                                   /* NULL */                        \
    /* +0x0D4 */ void (*setSubHandle)(Self *self, const char *path, BasicClass *handle); /* TaskCore__SetSubHandle; GraphRoom passes "ETC\HGRAPH.TIM" */ \
    /* +0x0D8 */ void (*setTarget)(Self *self, TaskCoreTarget *target); /* TaskCore__SetTarget */  \
    /* +0x0DC */ void (*releaseTarget)(Self *self);              /* TaskCore__ReleaseTarget */     \
    /* +0x0E0 */ void (*updateSlotElements)(Self *self, void *parent); /* TaskCore__UpdateSlotElements */ \
    /* +0x0E4 */ void (*broadcastToSlots)(Self *self, u8 *color); /* TaskCore__BroadcastToSlots */ \
    /* +0x0E8 */ void (*findNextFreeSlot)(Self *self);           /* TaskCore__FindNextFreeSlot */  \
    /* +0x0EC */ void (*findPrevFreeSlot)(Self *self);           /* TaskCore__FindPrevFreeSlot */  \
    /* +0x0F0 */ void (*setActiveSlot)(Self *self, s32 slot, s32 withSound); /* TaskCore__SetActiveSlot */ \
    /* +0x0F4 */ s32 (*getActiveSlot)(Self *self);               /* TaskCore__GetActiveSlot */     \
    /* +0x0F8 */ void (*createSlotElements)(Self *self, void *desc, void *handle); /* TaskCore__CreateSlotElements */ \
    /* +0x0FC */ void (*releaseSlotElements)(Self *self);        /* TaskCore__ReleaseSlotElements */ \
    /* +0x100 */ void (*refreshSlotView)(Self *self, void *parent, s32 show); /* TaskCore__RefreshSlotView */ \
    /* +0x104 */ void (*broadcastToSlotElements)(Self *self, void *color); /* TaskCore__BroadcastToSlotElements */ \
    /* +0x108 */ void (*beginElementScroll)(Self *self);         /* TaskCore__BeginElementScroll */ \
    /* +0x10C */ void (*commitElementScroll)(Self *self);        /* TaskCore__CommitElementScroll */ \
    /* +0x110 */ void (*cancelElementScroll)(Self *self);        /* TaskCore__CancelElementScroll */ \
    /* +0x114 */ void (*advanceSlotCursor)(Self *self);          /* TaskCore__AdvanceSlotCursor */ \
    /* +0x118 */ void (*retreatSlotCursor)(Self *self);          /* TaskCore__RetreatSlotCursor */ \
    /* +0x11C */ void (*setSlotCursor)(Self *self, s32 cursor, s32 withSound); /* TaskCore__SetSlotCursor */ \
    /* +0x120 */ s32 (*getActiveSlotCount)(Self *self)           /* TaskCore__GetActiveSlotCount */
/* clang-format on */

/* clang-format off */
#define TASKCORE_FIELDS(Methods)                                                                   \
    INTERMEDIATEBASE_FIELDS(Methods);                                                              \
    /* +0x028 */ s32 otLength;          /* reset: 3; onInit: the viewport's setOtLength (+0x048) */ \
    /* +0x02C */ s32 unk2C;             /* reset: 0x12C (GraphRoom 0x190); onInit: viewport +0x04C (SetUnk44) */ \
    /* +0x030 */ s32 packetSize;        /* reset: 64; onInit: viewport +0x050 (SetUnk48), a factor of the packet area */      \
    /* +0x034 */ s32 unk34;             /* reset: 1; nonzero: onDeinit hands unk93 to initArgs->unk0's +0x078 */ \
    /* +0x038 */ s32 result;            /* TaskCore__Init returns it; onInit 0, setState(6) 1 */   \
    /* +0x03C */ s32 inputMode;         /* 0 none, 1 choosing a slot, 2 scrolling its items; onPadEvent needs nonzero */ \
    /* +0x040 */ s32 frameBound;        /* setFrameBound; update: frameCounter past it is setState(6) */ \
    /* +0x044 */ char *soundBankPath;   /* the ctor's; nonzero: finalize releases `sound` */       \
    /* +0x048 */ BasicClass *sound;     /* New_VabStreamObj(soundBankPath) or the ctor's own; playSound's target */ \
    /* +0x04C */ TaskCoreTarget *target; /* setTarget */                                           \
    /* +0x050 */ s32 slotCount;         /* target->names' length */                               \
    /* +0x054 */ BasicClass **slotElements; /* one widget a slot (New_TextRow) */                \
    /* +0x058 */ s32 activeSlot;                                                                   \
    /* +0x05C */ s32 *itemCounts;       /* per slot: its item list's length */                     \
    /* +0x060 */ s32 *slotCounts;       /* per slot: the item cursor, a ring over itemCounts */    \
    /* +0x064 */ void **itemLists;      /* per slot: its item widgets (createSlotElements) */      \
    /* +0x068 */ BasicClass *listView;  /* New_BoxFill: the frame around the scrolled list */    \
    /* +0x06C */ u8 pad06C[4];                                                                     \
    /* +0x070 */ const char *subHandlePath; /* setSubHandle's path; nonzero: the handle is owned */ \
    /* +0x074 */ BasicClass *subHandle; /* New_TimImage(subHandlePath), or the caller's; NULL: onInit also passes baseColor with gDefaultMovieFrame */ \
    /* +0x078 */ struct BgLayer *bgLayer; /* New_BgLayer(tileMap, 1); include/BgLayer.h (tag only here) */ \
    /* +0x07C */ struct TileMap *tileMap; /* New_TileMap(0, tileAtlas); include/TileMap.h (tag only here) */ \
    /* +0x080 */ struct TileAtlas *tileAtlas; /* New_TileAtlas(0); include/TileAtlas.h (tag only here) */ \
    /* +0x084 */ s32 fadeRate;          /* setFadeRate; reset: 9 */                                \
    /* +0x088 */ s32 (*fadeInCallback)(TaskCore *self);  /* setFadeCallbackEnabled: NULL or tickColorFade; nonzero: onInit sets baseColor */ \
    /* +0x08C */ s32 (*fadeOutCallback)(TaskCore *self); /* setFadeOutCallbackEnabled: NULL or tickFadeColor */ \
    /* +0x090 */ u8 baseColor[3];       /* setColors; the fade-in's start colour */                \
    /* +0x093 */ u8 unk93[3];           /* setColors; onDeinit (unk34 set) and TitleMenu's clear the screen to it */                                            \
    /* +0x096 */ u8 unk96[3];           /* setColors (reset: 128 grey); no code reads it */                                            \
    /* +0x099 */ u8 pad099[3];                                                                     \
    /* +0x09C */ void (*viewCallback)(void *ctx); /* setCallback; refreshViewValue calls it */     \
    /* +0x0A0 */ void *viewCallbackCtx  /* the object is 0xA4 bytes: StreamTask's own fields start at +0x0A4 */
/* clang-format on */

struct TaskCoreMethods {
    TASKCORE_SLOTS(TaskCore,
                   (TaskCore * self, TaskCoreTarget *target, char *soundBankPath, BasicClass *sound));
};

struct TaskCore {
    TASKCORE_FIELDS(TaskCoreMethods);
};

extern TaskCoreMethods gTaskCoreMethods;
extern TaskCoreMethods *GetTaskCoreMethods(void); /* returns &gTaskCoreMethods */

TaskCore *New_TaskCore(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound);
void TaskCore__TaskCore(TaskCore *self, TaskCoreTarget *target, char *soundBankPath, BasicClass *sound);
void TaskCore__Finalize(TaskCore *self);
void TaskCore__Reset(TaskCore *self);
s32 TaskCore__Init(TaskCore *self, IntermediateBaseInitArgs *args, s32 mode);
void TaskCore__OnInit(TaskCore *self);
void TaskCore__OnDeinit(TaskCore *self);
void TaskCore__OnPadEvent(TaskCore *self, BasicClass *sender, s32 event);
void TaskCore__Update(TaskCore *self, BasicClass *sender, s32 event);
void TaskCore__SetState(TaskCore *self, s32 state);
void TaskCore__SetFrameBound(TaskCore *self, s32 bound);
void TaskCore__PlaySound(TaskCore *self, s32 tone);
void TaskCore__OnPadStart(TaskCore *self);
void TaskCore__OnPadConfirm(TaskCore *self);
void TaskCore__OnPadCancel(TaskCore *self);
void TaskCore__OnPadPrev(TaskCore *self);
void TaskCore__OnPadNext(TaskCore *self);
void TaskCore__ConfirmSlot(TaskCore *self);
void TaskCore__Exit(TaskCore *self);
void TaskCore__SetCallback(TaskCore *self, void (*callback)(void *ctx), void *ctx);
void TaskCore__SetFadeCallbackEnabled(TaskCore *self, s32 enable);
void TaskCore__SetFadeOutCallbackEnabled(TaskCore *self, s32 enable);
void TaskCore__SetColors(TaskCore *self, u8 *base, u8 *clear, u8 *color96);
void TaskCore__SetFadeRate(TaskCore *self, s32 rate);
s32 TaskCore__TickFadeCallback(TaskCore *self);
s32 TaskCore__TickFadeIn(TaskCore *self);
s32 TaskCore__TickFadeOutCallback(TaskCore *self);
s32 TaskCore__TickFadeOut(TaskCore *self);
void TaskCore__SetSubHandle(TaskCore *self, const char *path, BasicClass *handle);
void TaskCore__SetTarget(TaskCore *self, TaskCoreTarget *target);
void TaskCore__ReleaseTarget(TaskCore *self);
void TaskCore__UpdateSlotElements(TaskCore *self, void *parent);
void TaskCore__BroadcastToSlots(TaskCore *self, void *color);
void TaskCore__FindNextFreeSlot(TaskCore *self);
void TaskCore__FindPrevFreeSlot(TaskCore *self);
void TaskCore__SetActiveSlot(TaskCore *self, s32 slot, void *withSound);
s32 TaskCore__GetActiveSlot(TaskCore *self);
void TaskCore__CreateSlotElements(TaskCore *self, void *desc, void *handle);
void TaskCore__ReleaseSlotElements(TaskCore *self);
void TaskCore__RefreshSlotView(TaskCore *self, void *parent, s32 show);
void TaskCore__BroadcastToSlotElements(TaskCore *self, void *color);
void TaskCore__BeginElementScroll(TaskCore *self);
void TaskCore__CommitElementScroll(TaskCore *self);
void TaskCore__CancelElementScroll(TaskCore *self);
void TaskCore__AdvanceSlotCursor(TaskCore *self);
void TaskCore__RetreatSlotCursor(TaskCore *self);
void TaskCore__SetSlotCursor(TaskCore *self, s32 cursor, void *withSound);
s32 TaskCore__GetActiveSlotCount(TaskCore *self);

#endif
