#ifndef DAYTASK_H
#define DAYTASK_H

#include "TimedTask.h"

/*
 * DayTask -- class id 0x1F230, method table gDayTaskMethods. A TimedTask
 * that runs one day of the dream: it brackets a DreamSys startDay/endDay
 * pair and, in between, runs the day's play as ObjM children (one per
 * stage), and init returns how the day ended. The object is 0x50 bytes; its
 * own fields run from TimedTask's 0x38. No class derives from it. Methods:
 * src/class_39e08.c, New_DayTask through GetDayTaskMethods.
 *
 * Who creates it. Application__RunMainLoop (src/code_2b78c.c) calls
 * GameApplication__RunDayTask (src/GameApplicationFileResource.c) when the GraphRoom poll
 * returns 2, and that builds one with New_DayTask(the application's
 * IntermediateBaseInitArgs, its DreamSys, config->unk04), runs its init to
 * completion and releases it. init's return is TimedTask::result:
 *   1 or 2 -- ObjM's event 4 and endDay(0) returned 0: 2 when the
 *             DreamSys's getCinematic then has an entry (RunDayTask
 *             plays it), else 1; also 2 when startDay refused the day
 *             (returned < 0; a special day, by DreamSys's reading);
 *   3      -- event 4 in a flashback session (endDay returned nonzero), or
 *             ObjM's close codes 0xC/0xD; RunDayTask then sets
 *             skipGraphRoomPoll.
 *
 * Lifecycle, by phase (`phase`):
 *  - ctor: TimedTask's with GetSoundEffectDir() as soundBankPath; loads
 *    "ETC\ETC.TIM" (etcTim, uploaded and its buffer freed),
 *    "ETC\DREAMER.TMD" (dreamerTmd) and the week's BGM (bgm); fills the
 *    init args' frameClock, lightRig (a StageMap) and viewport (a
 *    NodeGuardedViewport); adopts the DreamSys as a child and hands it
 *    `sound` and etcTim. finalize releases all of it.
 *  - init hands the DreamSys the pad, the FrameClock and the viewport;
 *    onInit sizes the viewport, shows its fade box and attaches the
 *    DreamSys as its view child (phase 1).
 *  - onTag1Notify event 2 (DayTask__AdvancePhase): in phase 1 runs
 *    startDay and StartObjM on the stage it returns; in phase 3 releases
 *    the old ObjM and StartObjM on getCurrentStage. StartObjM builds the
 *    ObjM from sound, bgm, etcTim, dreamerTmd and the stage, adopts it and
 *    inits it with the DreamSys (phase 2).
 *  - onObjMNotify (+0x084; onNotify routes a 0x2F230 sender here, a 0x1F34
 *    DreamSys to the empty +0x080): events 5..8 and 0xA set phase 3, so
 *    the next AdvancePhase replaces the ObjM with one on the DreamSys's
 *    current stage; events 4, 0xC and 0xD release the ObjM, call endDay
 *    (0, 1 or 2 respectively; 2 is DreamSys's new-game reset), set
 *    `result` and setState(3).
 *
 * Two overrides take fewer arguments than their slots and the slots keep
 * IntermediateBase's types: init (DayTask__Init, self only; RunDayTask
 * calls it through DayTaskInitFn) and onInit (DayTask__OnInit, self only;
 * IntermediateBase__Init calls it with (0, 0, 0)).
 */

typedef struct DayTask DayTask;
typedef struct DayTaskMethods DayTaskMethods;

struct DreamSys;
struct WBgm;
struct TimImage;
struct LinkResource;
struct ObjM; /* include/ObjM.h */

/* DayTask::phase: which ObjM step DayTask__AdvancePhase takes on the next
 * DrawSystem VSync. */
enum DayTaskPhase {
    DAYTASK_PHASE_IDLE = 0,        /* resetCounters (DayTask__ResetPhase) */
    DAYTASK_PHASE_READY = 1,       /* onInit: the next VSync runs startDay */
    DAYTASK_PHASE_RUNNING = 2,     /* StartObjM: an ObjM is running */
    DAYTASK_PHASE_REPLACE_OBJM = 3 /* an ObjM link state: the next VSync replaces the ObjM */
};

/* TimedTask::result as a DayTask sets it: what its init returns to
 * GameApplication__RunDayTask. */
enum DayTaskResult {
    DAYTASK_RESULT_ENDED = 1,     /* ObjM's TIME_UP, endDay(0) returned 0, no cinematic entry */
    DAYTASK_RESULT_CINEMATIC = 2, /* as ENDED with a cinematic entry, or startDay refused the day;
                                     RunDayTask starts the cinematic stream */
    DAYTASK_RESULT_CLOSED = 3     /* TIME_UP with endDay nonzero, or an ObjM close; RunDayTask
                                     sets skipGraphRoomPoll */
};

/* TimedTask's slots, then this class's own. Overridden: ctor, finalize,
 * onNotify, resetCounters (DayTask__ResetPhase), init, deinit, onInit,
 * onDeinit, onTag1Notify (DayTask__AdvancePhase) and onState4. */
/* clang-format off */
#define DAYTASK_SLOTS(Self, CtorParams)                                                         \
    TIMEDTASK_SLOTS(Self, CtorParams);                                                            \
    /* +0x080 */ void (*onDreamSysNotify)(Self *self, BasicClass *sender, s32 event); /* DayTask__OnDreamSysNotify, empty; onNotify's 0x1F34 (DreamSys) sender */ \
    /* +0x084 */ void (*onObjMNotify)(Self *self, BasicClass *sender, s32 event)      /* DayTask__OnObjMNotify; onNotify's 0x2F230 (gObjMMethods) sender */
/* clang-format on */

/* clang-format off */
#define DAYTASK_FIELDS(Methods)                                                                 \
    TIMEDTASK_FIELDS(Methods);                                                                    \
    /* +0x038 */ struct DreamSys *dreamSys; /* the ctor's; a child; init/deinit hand it the init args' objects */ \
    /* +0x03C */ s32 phase;                 /* 0 ResetPhase, 1 OnInit, 2 StartObjM, 3 OnObjMNotify; not IntermediateBase::state */ \
    /* +0x040 */ struct WBgm *bgm;          /* New_WBgm(PickSoundBank(0), NULL, 1); New_ObjM's 2nd argument; finalize releases it */ \
    /* +0x044 */ struct TimImage *etcTim;   /* New_TimImage("ETC\ETC.TIM"), uploaded and its buffer freed; DreamSys +0x114; New_ObjM's 3rd */ \
    /* +0x048 */ struct LinkResource *dreamerTmd;    /* New_LinkResource("ETC\DREAMER.TMD"); New_ObjM's 4th; finalize releases it */ \
    /* +0x04C */ struct ObjM *objM          /* StartObjM's New_ObjM(...); a child; released by AdvancePhase/OnObjMNotify */
/* clang-format on */

struct DayTaskMethods {
    DAYTASK_SLOTS(DayTask, (DayTask * self, IntermediateBaseInitArgs *initArgs,
                            struct DreamSys *dreamSys, s32 syncDriver));
};

struct DayTask {
    DAYTASK_FIELDS(DayTaskMethods);
}; /* 0x50 bytes: New_DayTask */

extern DayTaskMethods gDayTaskMethods;
extern DayTaskMethods *GetDayTaskMethods(void); /* returns &gDayTaskMethods */

/* init's occupant takes self alone (see the banner). */
typedef s32 (*DayTaskInitFn)(DayTask *self);

/* The class's own methods, in address order. */
DayTask *New_DayTask(IntermediateBaseInitArgs *initArgs, struct DreamSys *dreamSys,
                     s32 syncDriver); /* BMemPMgrAlloc(0x50), then ctor */
void DayTask__DayTask(DayTask *self, IntermediateBaseInitArgs *initArgs, struct DreamSys *dreamSys,
                      s32 syncDriver);
void DayTask__Finalize(DayTask *self);
void DayTask__OnNotify(DayTask *self, BasicClass *sender, s32 event);
void DayTask__ResetPhase(DayTask *self);
s32 DayTask__Init(DayTask *self);
void DayTask__Deinit(DayTask *self);
void DayTask__OnInit(DayTask *self);
void DayTask__OnDeinit(DayTask *self);
void DayTask__AdvancePhase(DayTask *self, BasicClass *sender, s32 event);
void DayTask__StartObjM(DayTask *self, s32 stage);
void DayTask__OnState4(void);         /* +0x07C; empty, reads no argument */
void DayTask__OnDreamSysNotify(void); /* +0x080; empty, reads no argument */
void DayTask__OnObjMNotify(DayTask *self, BasicClass *sender, s32 event);

#endif
