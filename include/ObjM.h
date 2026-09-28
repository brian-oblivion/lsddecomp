#ifndef OBJM_H
#define OBJM_H

#include "TimedTask.h"

/*
 * ObjM -- class id 0x2F230, method table gObjMMethods: TimedTask's second
 * subclass (its ctor calls TimedTask__TimedTask first; the first is
 * DayTask). No class derives from it. Methods, in ROM order:
 * src/world/ObjMStyleActor.c, New_ObjM through GetObjMMethods. The object is
 * 0x88 bytes (New_ObjM); its own fields run from TimedTask's 0x38.
 *
 * Built by DayTask__StartObjM (src/world/DayTaskStageMap.c): New_ObjM(DayTask's
 * sound, bgm, etcTim, dreamerTmd, stage), added as a child and init'ed with
 * DayTask's init args and its DreamSys. So the inherited
 * IntermediateBase fields hold that DayTask's init-arg objects:
 * frameClock its FrameClock, lightRig its StageMap, viewport its NodeGuardedViewport, and
 * TimedTask's sound its VabStreamObj. Those fields keep their parents'
 * `BasicClass *` types and ObjM's methods cast them (no code).
 *
 * What its methods do, measured:
 *  - init (ObjM__AttachTarget) registers ObjM__GetGridRecord as the
 *    StageMap's callback, keeps the DreamSys and adds it as a child;
 *    onInit (ObjM__InitStyleAndWorld) builds `timBlockSrc`
 *    (New_TimBlockSrc of the day's variant) and `styleConfig`
 *    (RegisterStyleConfig);
 *  - onDrawSystemEvent's event 2 (ObjM__PollTimBlockLoad) waits for the
 *    TimBlockSrc: loaded, it fades its CLUT rows to a styleConfig colour;
 *    either way releases it and runs setupSceneStyle, then, once the
 *    StageMap has nothing pending, enterStyleSession (`inSession`);
 *  - update (ObjM__Update) counts frames and runs tickStyle, or
 *    advancePauseSetup while the "Pause" overlay is up; onPadEvent maps
 *    pad codes onto togglePause and the close-ready slots;
 *  - onNotify splits by the sender's class id: the DreamSys's codes
 *    0xA..0x11 go to enterState4..A, the viewport's fade object
 *    (FadeBox) reports fade down/up done (5/6), and the StageMap's
 *    event 7 runs checkAuxTrigger. The enterState/close methods set
 *    IntermediateBase::state and notifyParents it; DayTask's
 *    onObjMNotify acts on those codes.
 * A day's play loop is the reading the evidence invites, but none of it
 * names the class.
 *
 * Overrides whose parameter list differs from the inherited slot keep the
 * slot's type:
 *  - +0x044 init: ObjM__AttachTarget takes (args, DreamSys *);
 *    DayTask__StartObjM passes the DreamSys as the slot's s32 `mode`.
 *  - +0x04C onInit: ObjM__InitStyleAndWorld takes (gridSpan, style
 *    override, unk4C); IntermediateBase__Init calls it with (0, 0, 0).
 *  - +0x054 onDrawSystemEvent, +0x058 onPadEvent: the occupants take `void *`
 *    for the unused sender.
 *
 * The +0x06C..+0x07B words are read as one block from outside:
 * ObjM__InitStyleAndWorld passes &ctorSound to RegisterStyleConfig, which
 * keeps it in gStyleSceneRefs, and ApplyStyleDecorationIfSet
 * (ObjMStyleActor) calls +0x0AC on that block's +0x00C, cachedViewport
 * (Viewport's getFadeBox). The fields are kept flat.
 */

typedef struct ObjM ObjM;
typedef struct ObjMMethods ObjMMethods;

/* ObjM's class id (gObjMMethods word +0x000). Five nibbles, so
 * `(header & 0xFFFFF) == OBJM_CLASS_ID` tests for it or a class below it
 * (DayTask__OnNotify). */
#define OBJM_CLASS_ID 0x2F230

struct DreamSys;
struct WBgm;
struct TimBlockSrc;
struct TimImage;
struct LinkResource;
struct NodeGuardedViewport;
struct FadeBox;
struct TextRow;

/* ObjM::styleConfig's pointee: the day's scene style, a
 * plain record. RegisterStyleConfig returns sStyleConfig after
 * FillStyleFromConfig fills its last four words from the stage's config
 * bytes (ObjMStyleActor, whose local StyleM views the same words), or
 * InitStyleAndWorld's caller supplies one. ObjM__SetupSceneStyle hands the
 * first three to the StageMap's lights, ObjM__EnterStyleSession the rest to
 * the viewport, ObjM__PollTimBlockLoad a colour to the TimBlockSrc. */
typedef struct StyleConfig {
    s32 lightDirs;    /* +0x000, SetupSceneStyle: the StageMap's setChildParams `dirs` */
    s32 lightColors;  /* +0x004, SetupSceneStyle: setChildParams `colors` */
    s32 ambientColor; /* +0x008, SetupSceneStyle: setAmbientColor's rgb (a pointer) */
    void *clearColor; /* +0x00C, EnterStyleSession: the viewport's setClearColor; a gStylePalette entry */
    u8 pad10[0x014 - 0x010];
    s32 colorMode; /* +0x014, EnterStyleSession: 1 makes the far colour clearColor; PollTimBlockLoad: 2 fades to clearColor, else farColor */
    void *farColor; /* +0x018, EnterStyleSession: setFarColor unless colorMode is 1; a gStylePalette entry */
    s32 fogNear; /* +0x01C, EnterStyleSession: the viewport's setFogNear; a sStyleFogNears value */
} StyleConfig;

/* IntermediateBase::state as ObjM sets it, which is also the code it
 * notifies its parent with (DayTask__OnObjMNotify). A link state is the
 * DreamSys code that started it less 6 (ObjM__OnDreamSysNotify); IDLE is
 * the only state in which DreamSys codes are acted on, and a fade down
 * returns to it. TELEPORT, CLOSE and CLOSE_NEW_GAME are notified without
 * being kept. DayTask ends the day on TIME_UP (endDay(0)) and on the two
 * closes (endDay(1), and endDay(2), which starts a new game), starts the
 * next ObjM on the link states, and ignores TELEPORT. */
enum ObjMState {
    OBJM_STATE_IDLE = 0,
    OBJM_STATE_TIME_UP = 4,
    OBJM_STATE_LINK_DYNAMIC = 5,
    OBJM_STATE_LINK_WALL = 6,
    OBJM_STATE_LINK_FLASHBACK = 7,
    OBJM_STATE_LINK_TUNNEL = 8,
    OBJM_STATE_LINK_STAGE_TIMER = 10,
    OBJM_NOTIFY_LINK_TELEPORT = 11,
    OBJM_NOTIFY_CLOSE = 12,
    OBJM_NOTIFY_CLOSE_NEW_GAME = 13
};

/* Overridden: ctor, finalize, onNotify, resetCounters (NoOpSlot40), init
 * (AttachTarget), deinit (DetachTarget), onInit (InitStyleAndWorld),
 * onDeinit (TeardownStyle), onDrawSystemEvent, onPadEvent (DispatchPadEvent),
 * update, togglePause (TogglePause) and onTimedOut (NoOpSlot7C). */
struct ObjMMethods {
    TIMEDTASK_SLOTS(ObjM, (ObjM * self, BasicClass *sound, struct WBgm *bgm,
                           struct TimImage *etcTim, struct LinkResource *dreamerTmd, s32 stage));
    /* +0x080 */ void (*setupSceneStyle)(ObjM *self); /* ObjM__SetupSceneStyle: PollTimBlockLoad, once the TimBlockSrc is done */
    /* +0x084 */ void (*exitSceneStyle)(ObjM *self); /* ObjM__ExitSceneStyle: TeardownStyle */
    /* +0x088 */ void (*enterStyleSession)(ObjM *self); /* ObjM__EnterStyleSession: PollTimBlockLoad; sets inSession */
    /* +0x08C */ void (*tickStyle)(ObjM *self); /* ObjM__TickStyle: update, no pause overlay */
    /* +0x090 */ void (*onDreamSysNotify)(ObjM *self, BasicClass *sender,
                                          s32 event); /* ObjM__OnDreamSysNotify: onNotify's 0x1F34 sender */
    /* +0x094 */ void (*enterState4)(ObjM *self); /* ObjM__EnterTimeUp: OnDreamSysNotify's code 0xA */
    /* +0x098 */ void (*enterState5)(ObjM *self); /* ObjM__EnterState5: code 0xC */
    /* +0x09C */ void (*enterState6)(ObjM *self); /* ObjM__EnterState6: code 0xD; EnterState5 before a stage */
    /* +0x0A0 */ void (*enterState7)(ObjM *self);        /* ObjM__EnterState7: code 0xE */
    /* +0x0A4 */ void (*enterState8)(ObjM *self);        /* ObjM__EnterState8: code 0xF */
    /* +0x0A8 */ void (*enterStateA)(ObjM *self);        /* ObjM__EnterStateA: code 0x10 */
    /* +0x0AC */ void (*notifyParentsCodeB)(ObjM *self); /* ObjM__NotifyParentsCodeB: code 0x11 */
    /* +0x0B0 */ void (*onFadeNotify)(ObjM *self, struct FadeBox *sender,
                                      s32 event); /* ObjM__OnFadeNotify: onNotify's 0x164 sender; 5 fade down done, 6 up */
    /* +0x0B4 */ void (*onStageMapNotify)(ObjM *self, BasicClass *sender,
                                          s32 event); /* ObjM__OnStageMapNotify: onNotify's 0x114 sender */
    /* +0x0B8 */ s32 (*checkAuxTrigger)(ObjM *self); /* ObjM__CheckAuxTrigger: OnStageMapNotify's event 7 */
    /* +0x0BC */ void (*slotBC)(void);               /* ObjM__NoOpSlotBC, empty; never called */
    /* +0x0C0 */ void (*updateCloseReadyFlag)(ObjM *self); /* ObjM__UpdateCloseReadyFlag: DispatchPadEvent's 0xC */
    /* +0x0C4 */ void (*clearCloseReadyFlag)(ObjM *self); /* ObjM__ClearCloseReadyFlag: DispatchPadEvent's 0x2C, TogglePause */
    /* +0x0C8 */ void (*closeAndNotifyD)(ObjM *self); /* ObjM__CloseAndNotifyD: DispatchPadEvent's 0x16 */
    /* +0x0CC */ void (*closeAndNotifyC)(ObjM *self); /* ObjM__CloseAndNotifyC: no caller in C */
    /* +0x0D0 */ void (*advancePauseSetup)(ObjM *self); /* ObjM__AdvancePauseSetup: update and TogglePause */
    /* +0x0D4 */ void (*teardownPauseOverlay)(ObjM *self); /* ObjM__TeardownPauseOverlay: TogglePause, ExitSceneStyle, CloseAndNotifyC/D */
};

struct ObjM {
    TIMEDTASK_FIELDS(ObjMMethods);
    /* +0x038 */ s32 stage; /* the ctor's; DayTask__StartObjM's stage. PickStageBgm, GetStageMapChunkRecord, GetStageGridDimensions, gStagePendingExtras[stage], EnterState4 */
    /* +0x03C */ struct DreamSys *dreamSys; /* init's third argument (AttachTarget); a child. Every DreamSys slot ObjM calls */
    /* +0x040 */ s32 tickPeriod; /* InitStyleAndWorld: 16; the DreamSys's resetLinkState's tickPeriod (EnterStyleSession) */
    /* +0x044 */ s32 moveMode; /* InitStyleAndWorld: 2 or 3; resetLinkState's moveMode, a sMoveModeSpeeds index (EnterStyleSession) */
    /* +0x048 */ s32 gridSpan; /* onInit's arg1, 0 meaning 0xA000; the StageMap's setGridSpan (SetupSceneStyle) */
    /* +0x04C */ s32 unk4C;                       /* onInit's arg3; no reader */
    /* +0x050 */ struct StyleConfig *styleConfig; /* RegisterStyleConfig's result, or onInit's arg2 */
    /* +0x054 */ struct WBgm *bgm; /* the ctor's (DayTask's bgm): setSeq, stop, pause, resume */
    /* +0x058 */ struct TimBlockSrc *timBlockSrc; /* InitStyleAndWorld's New_TimBlockSrc; PollTimBlockLoad releases it */
    /* +0x05C */ u8 pad05C[0x060 - 0x05C];
    /* +0x060 */ s32 timBlockPending; /* the ctor and InitStyleAndWorld set it; PollTimBlockLoad clears it */
    /* +0x064 */ s32 unk64; /* the ctor zeroes it; PollTimBlockLoad sets 1 before enterStyleSession */
    /* +0x068 */ s32 inSession; /* the ctor zeroes it; EnterStyleSession sets it; gates update, onPadEvent, enterStyleSession */
    /* +0x06C */ BasicClass *ctorSound; /* the ctor's sound again (also TimedTask::sound); &ctorSound is RegisterStyleConfig's arg2 (see the banner) */
    /* +0x070 */ struct LinkResource *dreamerTmd; /* the ctor's (DayTask's "ETC\DREAMER.TMD"); read through gStyleSceneRefs (ObjMStyleActor) */
    /* +0x074 */ struct TimImage *etcTim; /* the ctor's (DayTask's "ETC\ETC.TIM"); AdvancePauseSetup's New_TextRow font */
    /* +0x078 */ struct NodeGuardedViewport *cachedViewport; /* InitStyleAndWorld: IntermediateBase::viewport */
    /* +0x07C */ struct TextRow *pauseText; /* AdvancePauseSetup's New_TextRow(etcTim, 5, "Pause"); TeardownPauseOverlay releases it */
    /* +0x080 */ s32 pauseSetupStep; /* the ctor zeroes it; AdvancePauseSetup counts 0..4, TeardownPauseOverlay clears it */
    /* +0x084 */ s32 closeReady; /* UpdateCloseReadyFlag sets, ClearCloseReadyFlag clears; CloseAndNotifyC/D test it */
}; /* 0x88 bytes: New_ObjM */

extern ObjMMethods gObjMMethods;
extern ObjMMethods *GetObjMMethods(void); /* returns &gObjMMethods */

/* The class's own methods, in address order. */
ObjM *New_ObjM(BasicClass *sound, struct WBgm *bgm, struct TimImage *etcTim,
               struct LinkResource *dreamerTmd, s32 stage); /* BMemPMgrAlloc(0x88), then ctor */
void ObjM__ObjM(ObjM *self, BasicClass *sound, struct WBgm *bgm, struct TimImage *etcTim,
                struct LinkResource *dreamerTmd, s32 stage);
void ObjM__Finalize(ObjM *self);
void ObjM__OnNotify(ObjM *self, BasicClass *sender, s32 event);
void ObjM__NoOpSlot40(void);
void ObjM__AttachTarget(ObjM *self, IntermediateBaseInitArgs *args, struct DreamSys *dreamSys);
void ObjM__GetGridRecord(ObjM *self, s32 cell, s32 x, s32 y);
void ObjM__DetachTarget(ObjM *self);
void ObjM__InitStyleAndWorld(ObjM *self, s32 gridSpan, struct StyleConfig *style, s32 arg3);
void ObjM__TeardownStyle(ObjM *self);
void ObjM__OnDrawSystemEvent(ObjM *self, void *sender, s32 event);
void ObjM__PollTimBlockLoad(ObjM *self, struct TimBlockSrc *src);
void ObjM__DispatchPadEvent(ObjM *self, void *sender, s32 code);
void ObjM__Update(ObjM *self);
void ObjM__TogglePause(ObjM *self);
void ObjM__NoOpSlot7C(void);
void ObjM__SetupSceneStyle(ObjM *self);
void ObjM__ExitSceneStyle(ObjM *self);
void ObjM__EnterStyleSession(ObjM *self);
void ObjM__TickStyle(ObjM *self);
void ObjM__OnDreamSysNotify(ObjM *self, BasicClass *sender, s32 code);
void ObjM__EnterTimeUp(ObjM *self);
void ObjM__EnterState5(ObjM *self);
void ObjM__EnterState6(ObjM *self);
void ObjM__EnterState7(ObjM *self);
void ObjM__EnterState8(ObjM *self);
void ObjM__EnterStateA(ObjM *self);
void ObjM__NotifyParentsCodeB(ObjM *self);
void ObjM__StartFadeUp(ObjM *self, s32 channels, s32 fadeMode, s32 step, s32 addChild);
void ObjM__OnFadeNotify(ObjM *self, struct FadeBox *sender, s32 event);
void ObjM__OnStageMapNotify(ObjM *self, BasicClass *sender, s32 event);
s32 ObjM__CheckAuxTrigger(ObjM *self);
void ObjM__NoOpSlotBC(void);
void ObjM__UpdateCloseReadyFlag(ObjM *self);
void ObjM__ClearCloseReadyFlag(ObjM *self);
void ObjM__CloseAndNotifyD(ObjM *self);
void ObjM__CloseAndNotifyC(ObjM *self);
void ObjM__AdvancePauseSetup(ObjM *self);
void ObjM__TeardownPauseOverlay(ObjM *self);

#endif
