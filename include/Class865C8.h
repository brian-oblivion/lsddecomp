#ifndef CLASS865C8_H
#define CLASS865C8_H

#include "TimedTask.h"

/*
 * Class865C8 -- class id 0x1F230, method table gClass865C8Methods:
 * TimedTask's subclass (its ctor calls TimedTask__TimedTask first).
 * Methods in src/class_39e08.c (New_Class865C8 through
 * GetClass865C8Methods). No class derives from it. The object is 0x50 bytes
 * (New_Class865C8); its own fields run from TimedTask's 0x38.
 *
 * The game builds one, in GameApplication__PollStatusObj (src/code_1677c.c):
 * New_Class865C8(the task's IntermediateBaseInitArgs, the DreamSys, a flag),
 * init, release, and a switch on init's result, TimedTask::result (2 and 3
 * are values OnObjMNotify sets). What its methods do, measured:
 *  - the ctor passes GetSoundEffectDir() as the base's soundBankPath, loads
 *    "ETC\ETC.TIM" (etcTim), "ETC\DREAMER.TMD" (dreamerTmd) and the
 *    week's BGM (bgm, New_WBgm(PickWeeklyGroup(0), NULL, 1)), fills the init
 *    args' +0x008..+0x010 with a FrameClock, a StageMap and a NodeGuardedViewport
 *    viewport, keeps the DreamSys as a child and hands it `sound` and etcTim;
 *  - init hands the DreamSys the init args' children and viewport, onInit
 *    sets up the viewport and attaches the DreamSys to it (phase 1);
 *  - onTag1Notify's event 2 (Class865C8__AdvancePhase) runs the DreamSys's
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
 * (Class865C8__Init, self only; PollStatusObj calls it through
 * Class865C8InitFn and passes self alone) and onInit (Class865C8__OnInit,
 * self only; IntermediateBase__Init calls it with (0, 0, 0)). The slots keep
 * IntermediateBase's types.
 */

typedef struct Class865C8 Class865C8;
typedef struct Class865C8Methods Class865C8Methods;

struct DreamSys;
struct WBgm;
struct TimImage;
struct LinkResource;
struct ObjM; /* include/ObjM.h */

/* TimedTask's slots, then this class's own. Overridden: ctor, finalize,
 * onNotify, resetCounters (Class865C8__ResetPhase), init, deinit, onInit,
 * onDeinit, onTag1Notify (Class865C8__AdvancePhase) and onState4. */
/* clang-format off */
#define CLASS865C8_SLOTS(Self, CtorParams)                                                         \
    TIMEDTASK_SLOTS(Self, CtorParams);                                                            \
    /* +0x080 */ void (*onDreamSysNotify)(Self *self, BasicClass *sender, s32 event); /* Class865C8__OnDreamSysNotify, empty; onNotify's 0x1F34 (DreamSys) sender */ \
    /* +0x084 */ void (*onObjMNotify)(Self *self, BasicClass *sender, s32 event)      /* Class865C8__OnObjMNotify; onNotify's 0x2F230 (gObjMMethods) sender */
/* clang-format on */

/* clang-format off */
#define CLASS865C8_FIELDS(Methods)                                                                 \
    TIMEDTASK_FIELDS(Methods);                                                                    \
    /* +0x038 */ struct DreamSys *dreamSys; /* the ctor's; a child; init/deinit hand it the init args' objects */ \
    /* +0x03C */ s32 phase;                 /* 0 ResetPhase, 1 OnInit, 2 StartObjM, 3 OnObjMNotify; not IntermediateBase::state */ \
    /* +0x040 */ struct WBgm *bgm;          /* New_WBgm(PickWeeklyGroup(0), NULL, 1); New_ObjM's 2nd argument; finalize releases it */ \
    /* +0x044 */ struct TimImage *etcTim;   /* New_TimImage("ETC\ETC.TIM"), uploaded and its buffer freed; DreamSys +0x114; New_ObjM's 3rd */ \
    /* +0x048 */ struct LinkResource *dreamerTmd;    /* New_LinkResource("ETC\DREAMER.TMD"); New_ObjM's 4th; finalize releases it */ \
    /* +0x04C */ struct ObjM *objM          /* StartObjM's New_ObjM(...); a child; released by AdvancePhase/OnObjMNotify */
/* clang-format on */

struct Class865C8Methods {
    CLASS865C8_SLOTS(Class865C8, (Class865C8 * self, IntermediateBaseInitArgs *initArgs,
                                  struct DreamSys *dreamSys, s32 arg3));
};

struct Class865C8 {
    CLASS865C8_FIELDS(Class865C8Methods);
}; /* 0x50 bytes: New_Class865C8 */

extern Class865C8Methods gClass865C8Methods;
extern Class865C8Methods *GetClass865C8Methods(void); /* returns &gClass865C8Methods */

/* init's occupant takes self alone (see the banner). */
typedef s32 (*Class865C8InitFn)(Class865C8 *self);

/* The class's own methods, in address order. */
Class865C8 *New_Class865C8(IntermediateBaseInitArgs *initArgs, struct DreamSys *dreamSys,
                           s32 arg3); /* BMemPMgrAlloc(0x50), then ctor */
void Class865C8__Class865C8(Class865C8 *self, IntermediateBaseInitArgs *initArgs,
                            struct DreamSys *dreamSys, s32 arg3);
void Class865C8__Finalize(Class865C8 *self);
void Class865C8__OnNotify(Class865C8 *self, BasicClass *sender, s32 event);
void Class865C8__ResetPhase(Class865C8 *self);
s32 Class865C8__Init(Class865C8 *self);
void Class865C8__Deinit(Class865C8 *self);
void Class865C8__OnInit(Class865C8 *self);
void Class865C8__OnDeinit(Class865C8 *self);
void Class865C8__AdvancePhase(Class865C8 *self, BasicClass *sender, s32 event);
void Class865C8__StartObjM(Class865C8 *self, s32 stage);
void Class865C8__OnState4(void);         /* +0x07C; empty, reads no argument */
void Class865C8__OnDreamSysNotify(void); /* +0x080; empty, reads no argument */
void Class865C8__OnObjMNotify(Class865C8 *self, BasicClass *sender, s32 event);

#endif
