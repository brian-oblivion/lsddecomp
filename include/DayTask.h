#ifndef DAYTASK_H
#define DAYTASK_H

#include "TimedTask.h"

/*
 * DayTask -- class id 0x1F230, method table gDayTaskMethods:
 * TimedTask's subclass (its ctor calls TimedTask__TimedTask first).
 * Methods in src/class_39e08.c (New_DayTask through
 * GetDayTaskMethods). No class derives from it. The object is 0x50 bytes
 * (New_DayTask); its own fields run from TimedTask's 0x38.
 *
 * The game builds one, in GameApplication__PollStatusObj (src/code_1677c.c):
 * New_DayTask(the task's IntermediateBaseInitArgs, the DreamSys, a flag),
 * init, release, and a switch on init's result, TimedTask::result (2 and 3
 * are values OnObjMNotify sets). What its methods do, measured:
 *  - the ctor passes GetSoundEffectDir() as the base's soundBankPath, loads
 *    "ETC\ETC.TIM" (etcTim), "ETC\DREAMER.TMD" (dreamerTmd) and the
 *    week's BGM (bgm, New_WBgm(PickWeeklyGroup(0), NULL, 1)), fills the init
 *    args' +0x008..+0x010 with a FrameClock, a StageMap and a NodeGuardedViewport
 *    viewport, keeps the DreamSys as a child and hands it `sound` and etcTim;
 *  - init hands the DreamSys the init args' children and viewport, onInit
 *    sets up the viewport and attaches the DreamSys to it (phase 1);
 *  - onTag1Notify's event 2 (DayTask__AdvancePhase) runs the DreamSys's
 *    startDay (or, from phase 3, releases the old objM and asks for the
 *    current stage) and StartObjM builds a New_ObjM (gObjMMethods, 0x2F230)
 *    from sound, bgm, etcTim and dreamerTmd, adds it as a child and inits it
 *    (phase 2);
 *  - onObjMNotify (+0x084, the notifications of a 0x2F230 sender) ends the
 *    day through the DreamSys on events 4, 0xC and 0xD, sets result and
 *    setState(3); events 5..8 and 0xA set phase 3.
 * That describes a day's loop but not enough to name it, so the name stays
 * the table's address.
 *
 * Two overrides take fewer arguments than their slots: init
 * (DayTask__Init, self only; PollStatusObj calls it through
 * DayTaskInitFn and passes self alone) and onInit (DayTask__OnInit,
 * self only; IntermediateBase__Init calls it with (0, 0, 0)). The slots keep
 * IntermediateBase's types.
 */

typedef struct DayTask DayTask;
typedef struct DayTaskMethods DayTaskMethods;

struct DreamSys;
struct WBgm;
struct TimImage;
struct LinkResource;
struct ObjM; /* include/ObjM.h */

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
    /* +0x040 */ struct WBgm *bgm;          /* New_WBgm(PickWeeklyGroup(0), NULL, 1); New_ObjM's 2nd argument; finalize releases it */ \
    /* +0x044 */ struct TimImage *etcTim;   /* New_TimImage("ETC\ETC.TIM"), uploaded and its buffer freed; DreamSys +0x114; New_ObjM's 3rd */ \
    /* +0x048 */ struct LinkResource *dreamerTmd;    /* New_LinkResource("ETC\DREAMER.TMD"); New_ObjM's 4th; finalize releases it */ \
    /* +0x04C */ struct ObjM *objM          /* StartObjM's New_ObjM(...); a child; released by AdvancePhase/OnObjMNotify */
/* clang-format on */

struct DayTaskMethods {
    DAYTASK_SLOTS(DayTask, (DayTask * self, IntermediateBaseInitArgs *initArgs,
                                  struct DreamSys *dreamSys, s32 arg3));
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
                           s32 arg3); /* BMemPMgrAlloc(0x50), then ctor */
void DayTask__DayTask(DayTask *self, IntermediateBaseInitArgs *initArgs,
                            struct DreamSys *dreamSys, s32 arg3);
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
