#ifndef OBJM_H
#define OBJM_H

#include "timed_task.h"

/**
 * @file objm.h
 * @brief ObjM, the TimedTask that runs one scene of a dream day: its world,
 *        its style, its pause overlay and the links that end it.
 *
 * Methods in src/world/dream_scene.c, New_ObjM through GetObjMMethods.
 */

typedef struct ObjM ObjM;
typedef struct ObjMMethods ObjMMethods;

/** ObjM's class id (gObjMMethods word +0x000). Five nibbles, so
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

/**
 * @brief The day's scene style, a plain record (ObjM::styleConfig).
 *
 * RegisterStyleConfig returns sStyleConfig after FillStyleFromConfig fills
 * its last four words from the stage's config bytes (dream_scene.c, whose
 * local StyleM views the same words), or InitStyleAndWorld's caller supplies
 * one. ObjM__SetupSceneStyle hands the first three to the StageMap's lights,
 * ObjM__EnterStyleSession the rest to the viewport, ObjM__PollTimBlockLoad a
 * colour to the TimBlockSrc.
 */
typedef struct StyleConfig {
    s32 lightDirs;    /**< +0x000: the StageMap's setChildParams `dirs` (SetupSceneStyle). */
    s32 lightColors;  /**< +0x004: setChildParams `colors` (SetupSceneStyle). */
    s32 ambientColor; /**< +0x008: setAmbientColor's rgb, a pointer (SetupSceneStyle). */
    void *clearColor; /**< +0x00C: the viewport's clear colour (EnterStyleSession); a sStylePalette entry. */
    u8 pad10[0x014 - 0x010];
    s32 colorMode; /**< +0x014: 1 makes the far colour clearColor; 2 fades the TIM block to clearColor, else farColor. */
    void *farColor; /**< +0x018: the viewport's far colour unless colorMode is 1; a sStylePalette entry. */
    s32 fogNear; /**< +0x01C: the viewport's setFogNear; a sStyleFogNears value. */
} StyleConfig;

/**
 * @brief IntermediateBase::state as ObjM sets it, which is also the code it
 * notifies its parent with (DayTask__OnObjMNotify).
 *
 * A link state is the DreamSys code that started it less 6
 * (ObjM__OnDreamSysNotify); IDLE is the only state in which DreamSys codes
 * are acted on, and a fade down returns to it. TELEPORT, CLOSE and
 * CLOSE_NEW_GAME are notified without being kept. DayTask ends the day on
 * TIME_UP (endDay(0)) and on the two closes (endDay(1), and endDay(2), which
 * starts a new game), starts the next ObjM on the link states, and ignores
 * TELEPORT.
 */
enum ObjMState {
    OBJM_STATE_IDLE = 0,           /**< In play; DreamSys codes are acted on. */
    OBJM_STATE_TIME_UP = 4,        /**< The dream's time ran out (enterTimeUp). */
    OBJM_STATE_LINK_DYNAMIC = 5,   /**< A random-spawn link; also InitStyleAndWorld's state. */
    OBJM_STATE_LINK_WALL = 6,      /**< A static wall link. */
    OBJM_STATE_LINK_FLASHBACK = 7, /**< A flashback link. */
    OBJM_STATE_LINK_TUNNEL = 8,    /**< A tunnel link. */
    OBJM_STATE_LINK_STAGE_TIMER = 10, /**< A stage-timer link; becomes TIME_UP when its fade up ends. */
    OBJM_NOTIFY_LINK_TELEPORT = 11, /**< Notified for an instant teleport; not kept. */
    OBJM_NOTIFY_CLOSE = 12,         /**< Notified by closeAndNotify; not kept. */
    OBJM_NOTIFY_CLOSE_NEW_GAME = 13 /**< Notified by closeAndNotifyNewGame; not kept. */
};

/**
 * @brief ObjM's method table: TimedTask's slots, with a ctor that takes
 * (sound, bgm, etcTim, dreamerTmd, stage), then its own.
 *
 * Overrides of TimedTask's slots: +0x008 ObjM__ObjM, +0x00C ObjM__Finalize,
 * +0x038 ObjM__OnNotify, +0x040 resetCounters (ObjM__NoOpSlot40), +0x044
 * init (ObjM__AttachTarget), +0x048 deinit (ObjM__DetachTarget), +0x04C
 * onInit (ObjM__InitStyleAndWorld), +0x050 onDeinit (ObjM__TeardownStyle),
 * +0x054 ObjM__OnDrawSystemEvent, +0x058 onPadEvent
 * (ObjM__DispatchPadEvent), +0x05C ObjM__Update, +0x074 ObjM__TogglePause
 * and +0x07C onTimedOut (ObjM__NoOpSlot7C).
 *
 * Overrides whose parameter list differs from the inherited slot keep the
 * slot's type: init's occupant takes (args, DreamSys *), and
 * DayTask__StartObjM passes the DreamSys as the slot's s32 `mode`; onInit's
 * takes (gridSpan, style override, initOption), which IntermediateBase__Init
 * passes as (0, 0, 0); onDrawSystemEvent's and onPadEvent's take `void *`
 * for the unused sender.
 */
struct ObjMMethods {
    TIMEDTASK_SLOTS(ObjM, (ObjM * self, BasicClass *sound, struct WBgm *bgm,
                           struct TimImage *etcTim, struct LinkResource *dreamerTmd, s32 stage));
    /* +0x080 */ void (*setupSceneStyle)(ObjM *self);   /**< @see ObjM__SetupSceneStyle */
    /* +0x084 */ void (*exitSceneStyle)(ObjM *self);    /**< @see ObjM__ExitSceneStyle */
    /* +0x088 */ void (*enterStyleSession)(ObjM *self); /**< @see ObjM__EnterStyleSession */
    /* +0x08C */ void (*tickStyle)(ObjM *self);         /**< @see ObjM__TickStyle */
    /* +0x090 */ void (*onDreamSysNotify)(ObjM *self, BasicClass *sender,
                                          s32 event);     /**< @see ObjM__OnDreamSysNotify */
    /* +0x094 */ void (*enterTimeUp)(ObjM *self);         /**< @see ObjM__EnterTimeUp */
    /* +0x098 */ void (*enterLinkDynamic)(ObjM *self);    /**< @see ObjM__EnterLinkDynamic */
    /* +0x09C */ void (*enterLinkWall)(ObjM *self);       /**< @see ObjM__EnterLinkWall */
    /* +0x0A0 */ void (*enterLinkFlashback)(ObjM *self);  /**< @see ObjM__EnterLinkFlashback */
    /* +0x0A4 */ void (*enterLinkTunnel)(ObjM *self);     /**< @see ObjM__EnterLinkTunnel */
    /* +0x0A8 */ void (*enterLinkStageTimer)(ObjM *self); /**< @see ObjM__EnterLinkStageTimer */
    /* +0x0AC */ void (*notifyLinkTeleport)(ObjM *self);  /**< @see ObjM__NotifyLinkTeleport */
    /* +0x0B0 */ void (*onFadeNotify)(ObjM *self, struct FadeBox *sender, s32 event); /**< @see ObjM__OnFadeNotify */
    /* +0x0B4 */ void (*onStageMapNotify)(ObjM *self, BasicClass *sender,
                                          s32 event); /**< @see ObjM__OnStageMapNotify */
    /* +0x0B8 */ s32 (*checkAuxTrigger)(ObjM *self);  /**< @see ObjM__CheckAuxTrigger */
    /* +0x0BC */ void (*slotBC)(void);                /**< @see ObjM__NoOpSlotBC; never called */
    /* +0x0C0 */ void (*updateCloseReadyFlag)(ObjM *self);  /**< @see ObjM__UpdateCloseReadyFlag */
    /* +0x0C4 */ void (*clearCloseReadyFlag)(ObjM *self);   /**< @see ObjM__ClearCloseReadyFlag */
    /* +0x0C8 */ void (*closeAndNotifyNewGame)(ObjM *self); /**< @see ObjM__CloseAndNotifyNewGame */
    /* +0x0CC */ void (*closeAndNotify)(ObjM *self);    /**< @see ObjM__CloseAndNotify; no caller */
    /* +0x0D0 */ void (*advancePauseSetup)(ObjM *self); /**< @see ObjM__AdvancePauseSetup */
    /* +0x0D4 */ void (*teardownPauseOverlay)(ObjM *self); /**< @see ObjM__TeardownPauseOverlay */
};

/**
 * @brief The TimedTask that runs one scene of a dream day (class id
 * 0x2F230): TimedTask's second subclass, after DayTask. No class derives from
 * it.
 *
 * Built by DayTask__StartObjM (src/world/dream_day.c): New_ObjM(DayTask's
 * sound, bgm, etcTim, dreamerTmd, stage), added as a child and init'ed with
 * DayTask's init args and its DreamSys. So the inherited IntermediateBase
 * fields hold that DayTask's init-arg objects: frameClock its FrameClock,
 * lightRig its StageMap, viewport its NodeGuardedViewport, and TimedTask's
 * sound its VabStreamObj. Those fields keep their parents' `BasicClass *`
 * types and ObjM's methods cast them.
 *
 * Lifecycle:
 *  - init (ObjM__AttachTarget) registers ObjM__GetGridRecord as the
 *    StageMap's chunk callback, keeps the DreamSys and adds it as a child;
 *    onInit (ObjM__InitStyleAndWorld) picks the stage's BGM, builds
 *    `timBlockSrc` (the day's TIM block) and `styleConfig`
 *    (RegisterStyleConfig);
 *  - each vsync (ObjM__PollTimBlockLoad) waits for the TimBlockSrc: loaded,
 *    it fades its CLUT rows to a styleConfig colour; either way releases it
 *    and runs setupSceneStyle, then, once the StageMap has nothing pending,
 *    enterStyleSession, which sets `inSession`;
 *  - in session, update counts frames and runs tickStyle, or
 *    advancePauseSetup while the "Pause" overlay is up; onPadEvent maps
 *    Start, Select and Triangle onto togglePause and the close slots;
 *  - onNotify splits by the sender's class id: the DreamSys's codes go to
 *    enterTimeUp .. notifyLinkTeleport, the viewport's FadeBox reports fade
 *    down and up done, and the StageMap's slot-data event runs
 *    checkAuxTrigger. The enter and close methods set IntermediateBase::state
 *    (enum ObjMState) and notifyParents it; DayTask's onObjMNotify acts on
 *    those codes.
 *
 * A day's play loop is the reading the evidence invites, but none of it
 * names the class.
 *
 * The +0x06C..+0x07B words are read as one block from outside:
 * ObjM__InitStyleAndWorld passes &ctorSound to RegisterStyleConfig, which
 * keeps it in sStyleSceneRefs, and ApplyStyleDecorationIfSet calls
 * getFadeBox on that block's +0x00C, cachedViewport. The fields are kept
 * flat.
 */
struct ObjM {
    TIMEDTASK_FIELDS(ObjMMethods);
    /* +0x038 */ s32 stage; /**< The ctor's stage: picks the BGM, the chunk records, the grid, sStagePendingExtras[stage]. */
    /* +0x03C */ struct DreamSys *dreamSys; /**< init's DreamSys (AttachTarget), kept as a child. */
    /* +0x040 */ s32 tickPeriod; /**< 16 (InitStyleAndWorld); the DreamSys's resetLinkState tick period. */
    /* +0x044 */ s32 moveMode; /**< 2 on stage 0, else 3 (InitStyleAndWorld); resetLinkState's move mode. */
    /* +0x048 */ s32 gridSpan; /**< onInit's gridSpan, 0 meaning DEFAULT_GRID_SPAN; the StageMap's setGridSpan. */
    /* +0x04C */ s32 initOption; /**< onInit's third argument (IntermediateBase__Init passes 0); no reader. */
    /* +0x050 */ struct StyleConfig *styleConfig; /**< RegisterStyleConfig's result, or onInit's override. */
    /* +0x054 */ struct WBgm *bgm; /**< The ctor's (DayTask's) BGM: setSeq, stop, pause, resume. */
    /* +0x058 */ struct TimBlockSrc *timBlockSrc; /**< The day's TIM block (InitStyleAndWorld); PollTimBlockLoad releases it. */
    /* +0x05C */ u8 pad05C[0x060 - 0x05C];
    /* +0x060 */ s32 timBlockPending; /**< Set by the ctor and InitStyleAndWorld; PollTimBlockLoad clears it. */
    /* +0x064 */ s32 loadsComplete; /**< Set once the TIM block and the StageMap's loads are done, before enterStyleSession; no reader. */
    /* +0x068 */ s32 inSession; /**< Set by EnterStyleSession; gates update, onPadEvent and enterStyleSession. */
    /* +0x06C */ BasicClass *ctorSound; /**< The ctor's sound again (also TimedTask::sound); the start of the block RegisterStyleConfig keeps. */
    /* +0x070 */ struct LinkResource *dreamerTmd; /**< The ctor's (DayTask's "ETC\\DREAMER.TMD"); read through sStyleSceneRefs. */
    /* +0x074 */ struct TimImage *etcTim; /**< The ctor's (DayTask's "ETC\\ETC.TIM"); the "Pause" row's font. */
    /* +0x078 */ struct NodeGuardedViewport *cachedViewport; /**< IntermediateBase::viewport, kept by InitStyleAndWorld. */
    /* +0x07C */ struct TextRow *pauseText; /**< The "Pause" row (AdvancePauseSetup); TeardownPauseOverlay releases it. */
    /* +0x080 */ s32 pauseSetupStep; /**< 0 when no overlay; AdvancePauseSetup counts it up, TeardownPauseOverlay clears it. */
    /* +0x084 */ s32 closeReady; /**< Set by UpdateCloseReadyFlag, cleared by ClearCloseReadyFlag; the close slots test it. */
}; /* 0x88 bytes: New_ObjM */

/** @brief ObjM's method table (see ObjMMethods). */
extern ObjMMethods gObjMMethods;

/**
 * @brief Returns ObjM's method table.
 * @return &gObjMMethods.
 */
extern ObjMMethods *GetObjMMethods(void);

/**
 * @brief Allocates an ObjM from the BMemPMgr pool and constructs it.
 * @param sound      The scene's VabStreamObj.
 * @param bgm        The BGM player.
 * @param etcTim     ETC\\ETC.TIM, the "Pause" row's font.
 * @param dreamerTmd ETC\\DREAMER.TMD.
 * @param stage      The stage the scene plays in.
 * @return The new object, or NULL when the pool is exhausted.
 */
ObjM *New_ObjM(BasicClass *sound, struct WBgm *bgm, struct TimImage *etcTim,
               struct LinkResource *dreamerTmd, s32 stage);

/**
 * @brief Constructor (slot +0x008): TimedTask's ctor with `sound`, then keeps
 * the arguments, marks the TIM block pending, clears the session and pause
 * state and resets the counters.
 * @param self       The object being constructed.
 * @param sound      The scene's VabStreamObj.
 * @param bgm        The BGM player.
 * @param etcTim     ETC\\ETC.TIM.
 * @param dreamerTmd ETC\\DREAMER.TMD.
 * @param stage      The stage the scene plays in.
 */
void ObjM__ObjM(ObjM *self, BasicClass *sound, struct WBgm *bgm, struct TimImage *etcTim,
                struct LinkResource *dreamerTmd, s32 stage);

/**
 * @brief Finalizer (slot +0x00C): TimedTask's.
 * @param self The object.
 */
void ObjM__Finalize(ObjM *self);

/**
 * @brief onNotify (slot +0x038): TimedTask's, then a StageMap sender goes to
 * onStageMapNotify, a FadeBox to onFadeNotify and a DreamSys to
 * onDreamSysNotify.
 * @param self   The object.
 * @param sender The notifying object.
 * @param event  The event code.
 */
void ObjM__OnNotify(ObjM *self, BasicClass *sender, s32 event);

/** @brief resetCounters (slot +0x040): does nothing. */
void ObjM__NoOpSlot40(void);

/**
 * @brief init (slot +0x044): makes ObjM__GetGridRecord the StageMap's chunk
 * callback, keeps the DreamSys, runs TimedTask's init and adds the DreamSys
 * as a child.
 * @param self     The object.
 * @param args     The building DayTask's init args; `lightRig` is its StageMap.
 * @param dreamSys The player's DreamSys.
 */
void ObjM__AttachTarget(ObjM *self, IntermediateBaseInitArgs *args, struct DreamSys *dreamSys);

/**
 * @brief The StageMap's chunk callback: a chunk's file record on this stage.
 * @param self The object.
 * @param cell The chunk's linear index, or negative to use `x` and `y`.
 * @param x    The chunk column, when `cell` is negative.
 * @param y    The chunk row, when `cell` is negative.
 * @return The chunk's file record.
 */
struct CdFileEntry *ObjM__GetGridRecord(ObjM *self, s32 cell, s32 x, s32 y);

/**
 * @brief deinit (slot +0x048): removes the DreamSys child, then TimedTask's
 * deinit.
 * @param self The object.
 */
void ObjM__DetachTarget(ObjM *self);

/**
 * @brief onInit (slot +0x04C): starts the stage's BGM, starts loading the
 * day's TIM block, points the viewport at the DreamSys, registers the
 * scene's StyleConfig, sets the StageMap's bounds, the move mode, the grid
 * span and the hit-height gate, and enters OBJM_STATE_LINK_DYNAMIC.
 * @param self       The object.
 * @param gridSpan   The StageMap's grid span; 0 for DEFAULT_GRID_SPAN.
 * @param style      A StyleConfig to use instead of the registered one, or NULL.
 * @param initOption Kept in `initOption`; nothing reads it.
 */
void ObjM__InitStyleAndWorld(ObjM *self, s32 gridSpan, struct StyleConfig *style, s32 initOption);

/**
 * @brief onDeinit (slot +0x050): exits the scene style, releases the dream's
 * auxiliary entities and the style layer, and stops the BGM.
 * @param self The object.
 */
void ObjM__TeardownStyle(ObjM *self);

/**
 * @brief onDrawSystemEvent (slot +0x054): on DRAWSYSTEM_EVENT_VSYNC polls
 * the TIM block load.
 * @param self   The object.
 * @param sender The DrawSystem (unused).
 * @param event  The event code.
 */
void ObjM__OnDrawSystemEvent(ObjM *self, void *sender, s32 event);

/**
 * @brief Finishes the TIM block load and starts the session. Loaded, the
 * block's CLUT rows fade to a styleConfig colour; loaded or failed, it is
 * released and the scene set up (a failure also adds 30 to the DreamSys's
 * time limit). With nothing pending and the StageMap idle, the style session
 * starts.
 * @param self The object.
 * @param src  Always self->timBlockSrc.
 */
void ObjM__PollTimBlockLoad(ObjM *self, struct TimBlockSrc *src);

/**
 * @brief onPadEvent (slot +0x058), in session only: Start pressed toggles
 * the pause, Select held and released sets and clears the close-ready flag,
 * Triangle pressed runs closeAndNotifyNewGame.
 * @param self   The object.
 * @param sender The Pad (unused).
 * @param code   The pad event code (PAD_EVENT_* + PAD_BUTTON_*).
 */
void ObjM__DispatchPadEvent(ObjM *self, void *sender, s32 code);

/**
 * @brief update (slot +0x05C), in session only: counts the frame and runs
 * advancePauseSetup while the pause overlay is up, else tickStyle.
 * @param self The object.
 */
void ObjM__Update(ObjM *self);

/**
 * @brief togglePause (slot +0x074): tears the pause overlay down if it is
 * up, else starts building it.
 * @param self The object.
 */
void ObjM__TogglePause(ObjM *self);

/** @brief onTimedOut (slot +0x07C): does nothing. */
void ObjM__NoOpSlot7C(void);

/**
 * @brief setupSceneStyle (slot +0x080): sets the viewport's projection from
 * the screen width and reattaches it to the DreamSys, hands the world to the
 * dream's auxiliary entities, adds the StageMap as a child and gives it the
 * style's lights, the stage's grid, the DreamSys, the grid span and the
 * accepted class ids.
 * @param self The object.
 */
void ObjM__SetupSceneStyle(ObjM *self);

/**
 * @brief exitSceneStyle (slot +0x084): tears down the pause overlay, blocks
 * the DreamSys's movement and detaches it, detaches the viewport and removes the StageMap
 * child.
 * @param self The object.
 */
void ObjM__ExitSceneStyle(ObjM *self);

/**
 * @brief enterStyleSession (slot +0x088): sets `inSession`, resets the
 * DreamSys's link state, enables the StageMap, gives the viewport the
 * style's fog and colours, and fades the scene in (in the flashback colour
 * during a flashback).
 * @param self The object.
 */
void ObjM__EnterStyleSession(ObjM *self);

/**
 * @brief tickStyle (slot +0x08C): runs the style layer's per-frame tick on
 * the StageMap's target cell.
 * @param self The object.
 */
void ObjM__TickStyle(ObjM *self);

/**
 * @brief onDreamSysNotify (slot +0x090). While IDLE, each DreamSys link code
 * runs its enter slot (DREAMSYS_LINK_DAY_START none); otherwise any code
 * from 9 up clears the DreamSys's state.
 * @param self   The object.
 * @param sender The DreamSys.
 * @param code   The DreamSys's code (DREAMSYS_TIME_UP .. DREAMSYS_LINK_TELEPORT).
 */
void ObjM__OnDreamSysNotify(ObjM *self, BasicClass *sender, s32 code);

/**
 * @brief enterTimeUp (slot +0x094): enters OBJM_STATE_TIME_UP and fades up,
 * in black during a flashback; otherwise the frame count and stage pick
 * black, red, white or no fade at all (an immediate notify).
 * @param self The object.
 */
void ObjM__EnterTimeUp(ObjM *self);

/**
 * @brief enterLinkDynamic (slot +0x098): with no current stage, a wall link
 * instead; otherwise enters OBJM_STATE_LINK_DYNAMIC, fades up in the dream's
 * colour and blocks the DreamSys's movement.
 * @param self The object.
 */
void ObjM__EnterLinkDynamic(ObjM *self);

/**
 * @brief enterLinkWall (slot +0x09C): enters OBJM_STATE_LINK_WALL, fades up
 * slowly in the dream's colour and blocks the DreamSys's movement.
 * @param self The object.
 */
void ObjM__EnterLinkWall(ObjM *self);

/**
 * @brief enterLinkFlashback (slot +0x0A0): enters OBJM_STATE_LINK_FLASHBACK,
 * fades up in the flashback's colour and blocks the DreamSys's movement.
 * @param self The object.
 */
void ObjM__EnterLinkFlashback(ObjM *self);

/**
 * @brief enterLinkTunnel (slot +0x0A4): enters OBJM_STATE_LINK_TUNNEL, fades
 * up in black and forces the DreamSys's movement.
 * @param self The object.
 */
void ObjM__EnterLinkTunnel(ObjM *self);

/**
 * @brief enterLinkStageTimer (slot +0x0A8): enters
 * OBJM_STATE_LINK_STAGE_TIMER, fades up in black and sets the DreamSys
 * drifting under a held move override.
 * @param self The object.
 */
void ObjM__EnterLinkStageTimer(ObjM *self);

/**
 * @brief notifyLinkTeleport (slot +0x0AC): notifies the parents with
 * OBJM_NOTIFY_LINK_TELEPORT.
 * @param self The object.
 */
void ObjM__NotifyLinkTeleport(ObjM *self);

/**
 * @brief Fades the viewport's FadeBox up, driven by the FrameClock.
 * @param self     The object.
 * @param channels The fade colour (a DreamColors value).
 * @param fadeMode The FadeBox's fade mode.
 * @param step     The fade step; 0 keeps the box's own.
 * @param addChild Non-zero to add the FadeBox as a child first.
 */
void ObjM__StartFadeUp(ObjM *self, s32 channels, s32 fadeMode, s32 step, s32 addChild);

/**
 * @brief onFadeNotify (slot +0x0B0). Fade down done: removes the FadeBox,
 * frees the DreamSys and returns to IDLE. Fade up done: removes it, sets the
 * viewport's clear colour to the fade's and notifies the state (a
 * stage-timer link stops drifting and turns into TIME_UP first).
 * @param self   The object.
 * @param sender The FadeBox.
 * @param event  FADEBOX_EVENT_FADE_DOWN_DONE or FADEBOX_EVENT_FADE_UP_DONE.
 */
void ObjM__OnFadeNotify(ObjM *self, struct FadeBox *sender, s32 event);

/**
 * @brief onStageMapNotify (slot +0x0B4): on STAGEMAP_EVENT_SLOT_DATA_READY
 * runs checkAuxTrigger.
 * @param self   The object.
 * @param sender The StageMap.
 * @param event  The event code.
 */
void ObjM__OnStageMapNotify(ObjM *self, BasicClass *sender, s32 event);

/**
 * @brief checkAuxTrigger (slot +0x0B8): hands the StageMap's last event
 * slot's data block to TryDreamAuxTrigger with the slot's chunk and the day;
 * the slot keeps what that returns as `heldObj`, and the block is released
 * when it returns nothing.
 * @param self The object.
 * @return 0 when an object was made, 1 when the block was released.
 */
s32 ObjM__CheckAuxTrigger(ObjM *self);

/** @brief Slot +0x0BC: does nothing; never called. */
void ObjM__NoOpSlotBC(void);

/**
 * @brief updateCloseReadyFlag (slot +0x0C0): sets `closeReady` while the
 * pause overlay is up and the scene is IDLE.
 * @param self The object.
 */
void ObjM__UpdateCloseReadyFlag(ObjM *self);

/**
 * @brief clearCloseReadyFlag (slot +0x0C4): clears `closeReady`.
 * @param self The object.
 */
void ObjM__ClearCloseReadyFlag(ObjM *self);

/**
 * @brief closeAndNotifyNewGame (slot +0x0C8): when close-ready, tears down
 * the pause overlay and notifies OBJM_NOTIFY_CLOSE_NEW_GAME.
 * @param self The object.
 */
void ObjM__CloseAndNotifyNewGame(ObjM *self);

/**
 * @brief closeAndNotify (slot +0x0CC): when close-ready, tears down the
 * pause overlay and notifies OBJM_NOTIFY_CLOSE.
 * @param self The object.
 */
void ObjM__CloseAndNotify(ObjM *self);

/**
 * @brief advancePauseSetup (slot +0x0D0), called each update while the
 * overlay is up. The first call builds the red "Pause" row under the
 * StageMap; the fourth after it hides the viewport and pauses the
 * FrameClock, the BGM and the sound.
 * @param self The object.
 */
void ObjM__AdvancePauseSetup(ObjM *self);

/**
 * @brief teardownPauseOverlay (slot +0x0D4): releases the "Pause" row if it
 * was built, resumes the sound, the BGM and the FrameClock, shows the
 * viewport and clears the pause step.
 * @param self The object.
 */
void ObjM__TeardownPauseOverlay(ObjM *self);

#endif
