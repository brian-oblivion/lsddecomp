/*
 * class_39e08 -- the methods of DayTask and of most of its parent TimedTask,
 * with one free function between them.
 *
 * DayTask (include/DayTask.h), New_DayTask through GetDayTaskMethods: the
 * task GameApplication__PollStatusObj runs for one dream day. Its ctor loads
 * the day's shared resources (ETC\ETC.TIM, ETC\DREAMER.TMD, the week's
 * BGM) and fills the init args' viewport, frame clock and light rig; on each
 * DrawSystem VSync DayTask__AdvancePhase starts the day or replaces the
 * running ObjM, and DayTask__OnObjMNotify turns ObjM's states into the next
 * phase or the task's result.
 *
 * RegisterRecordTableFiles: registers gRecordTable's file entries with the
 * CD driver in at most two batches (DayTask's ctor, and the loader-task
 * callback in code_1677c.c).
 *
 * TimedTask (include/TimedTask.h), New_TimedTask through
 * TimedTask__SetTimeout: an IntermediateBase with a frame timeout, a sound
 * object and a result. Its last two functions, TimedTask__PlaySound and
 * GetTimedTaskMethods, open class_3ac78.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_39e08.h"
#include "VabStreamObj.h"
#include "NodeGuardedViewport.h"
#include "StageMap.h"
#include "WBgm.h"
#include "TimImage.h"
#include "FrameClock.h"
#include "DreamSys.h"
#include "LinkResource.h"
#include "ObjM.h"

/* The viewpoint and view-reference points DayTask__OnInit hands the
 * viewport's attachViewChild: (0, -1200, 0) and (0, -1200, 10000). */
extern LongVec3 sDayViewPoint;
extern LongVec3 sDayViewRef;

DayTask *New_DayTask(IntermediateBaseInitArgs *initArgs, DreamSys *dreamSys, s32 syncDriver) {
    DayTask *self;

    self = BMemPMgrAlloc(sizeof(DayTask));
    if (self != NULL) {
        GetDayTaskMethods()->ctor(self, initArgs, dreamSys, syncDriver);
        return self;
    }
    return NULL;
}

void DayTask__DayTask(DayTask *self, IntermediateBaseInitArgs *initArgs, DreamSys *dreamSys,
                      s32 syncDriver) {
    /* MATCHING: mode is never set, but a bare ResourceSource shrinks the
     * frame by 8. */
    ResourceRequest req;
    s32 vabPath;

    GetTimedTaskMethods()->ctor((TimedTask *)self, GetSoundEffectDir(0), 0);
    self->methods = GetDayTaskMethods();
    InitDreamAux();
    self->etcTim = New_TimImage((char *)sEtcTimPath);
    ((TimImageUploadFn)self->etcTim->methods->processBuffer)(self->etcTim);
    self->etcTim->methods->freeBuffer(self->etcTim);
    req.src.buffer = NULL;
    req.src.name = (char *)sDreamerTmdPath;
    self->dreamerTmd = New_LinkResource(&req.src);
    vabPath = PickWeeklyGroup(0);
    self->bgm = New_WBgm((char *)vabPath, NULL, 1);
    RegisterRecordTableFiles(1);
    SetActiveDataSourceDriverMode(syncDriver == 0, 1, 1);
    self->initArgs = initArgs;
    initArgs->viewport = (BasicClass *)New_NodeGuardedViewport();
    initArgs->frameClock = (BasicClass *)New_FrameClock();
    initArgs->lightRig = (BasicClass *)New_StageMap(NULL, 1);
    self->dreamSys = dreamSys;
    self->methods->addChild(self, (BasicClass *)dreamSys);
    dreamSys->methods->setSoundObj(dreamSys, (s32)self->sound);
    dreamSys->methods->setEtcTim(dreamSys, (s32)self->etcTim);
    self->methods->resetCounters(self);
}

void DayTask__Finalize(DayTask *self) {
    IntermediateBaseInitArgs *args = self->initArgs;
    BasicClass *obj;

    self->methods->removeChild(self, (BasicClass *)self->dreamSys);
    obj = args->lightRig;
    args->lightRig = obj->methods->release(obj);
    obj = args->frameClock;
    args->frameClock = obj->methods->release(obj);
    obj = args->viewport;
    args->viewport = obj->methods->release(obj);
    self->bgm->methods->release(self->bgm);
    self->dreamerTmd->methods->release(self->dreamerTmd);
    self->etcTim->methods->release(self->etcTim);
    TickDreamAuxSlots();
    GetTimedTaskMethods()->finalize((TimedTask *)self);
}

void DayTask__OnNotify(DayTask *self, BasicClass *sender, s32 event) {
    s32 tag;

    GetTimedTaskMethods()->onNotify((TimedTask *)self, sender, event);
    tag = sender->methods->header;
    if ((tag & 0xFFFF) == DREAMSYS_CLASS_ID) {
        self->methods->onDreamSysNotify(self, sender, event);
    } else if ((tag & 0xFFFFF) == OBJM_CLASS_ID) {
        self->methods->onObjMNotify(self, sender, event);
    }
}

void DayTask__ResetPhase(DayTask *self) {
    self->phase = DAYTASK_PHASE_IDLE;
}

s32 DayTask__Init(DayTask *self) {
    DreamSys *dreamSys = self->dreamSys;

    dreamSys->methods->addChild(dreamSys, self->initArgs->pad);
    dreamSys->methods->addChild(dreamSys, self->initArgs->frameClock);
    dreamSys->methods->setViewport(dreamSys, (Viewport *)self->initArgs->viewport);
    return GetTimedTaskMethods()->init((TimedTask *)self, self->initArgs, 0);
}

void DayTask__Deinit(DayTask *self) {
    DreamSys *dreamSys = self->dreamSys;

    GetTimedTaskMethods()->deinit((TimedTask *)self);
    dreamSys->methods->setViewport(dreamSys, 0);
    dreamSys->methods->removeChild(dreamSys, self->initArgs->pad);
    dreamSys->methods->removeChild(dreamSys, self->unk10);
}

void DayTask__OnInit(DayTask *self) {
    SubObjE *drawSystem;
    Viewport *vp;
    SceneNode *fadeBox;
    ViewportSize *size;

    drawSystem = (SubObjE *)self->initArgs->drawSystem;
    vp = (Viewport *)self->viewport;
    size = drawSystem->methods->getDims(drawSystem, 0);
    vp->methods->setScreenSize(vp, size);
    fadeBox = vp->methods->getFadeBox(vp);
    fadeBox->methods->setDisplay(fadeBox, 1);
    vp->methods->setUnk44(vp, 1200);
    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &sDayViewPoint, &sDayViewRef, 0);
    vp->methods->initOt(vp);
    self->phase = DAYTASK_PHASE_READY;
}

void DayTask__OnDeinit(DayTask *self) {
    Viewport *vp = (Viewport *)self->viewport;

    vp->methods->deinitOt(vp);
    vp->methods->detachViewChild(vp);
}

/* onTag1Notify: on each DrawSystem VSync, starts the day's first ObjM
 * (READY) or replaces the ObjM a link state ended (REPLACE_OBJM). A day
 * startDay refuses ends at once. */
void DayTask__AdvancePhase(DayTask *self, BasicClass *sender, s32 event) {
    s32 result;

    GetTimedTaskMethods()->onTag1Notify((TimedTask *)self, sender, event);
    if (event == DRAWSYSTEM_EVENT_VSYNC && self->phase != DAYTASK_PHASE_RUNNING) {
        switch (self->phase) {
            case DAYTASK_PHASE_READY:
                result = self->dreamSys->methods->startDay(self->dreamSys);
                if (result < 0) {
                    self->dreamSys->methods->endDay(self->dreamSys, 0);
                    self->result = DAYTASK_RESULT_CINEMATIC;
                    self->methods->setState(self, INTERMEDIATEBASE_STATE_STOP);
                    return;
                }
                DayTask__StartObjM(self, result);
                break;
            case DAYTASK_PHASE_RUNNING: /* MATCHING: unreachable (phase != RUNNING above), but removing it changes the switch's code */
                break;
            case DAYTASK_PHASE_REPLACE_OBJM:
                self->objM->methods->deinit(self->objM);
                self->objM->methods->release(self->objM);
                result = self->dreamSys->methods->getCurrentStage(self->dreamSys);
                DayTask__StartObjM(self, result);
                break;
        }
    }
}

void DayTask__StartObjM(DayTask *self, s32 stage) {
    self->objM = New_ObjM(self->sound, self->bgm, self->etcTim, self->dreamerTmd, stage);
    self->methods->addChild(self, (BasicClass *)self->objM);
    self->objM->methods->init(self->objM, self->initArgs, (s32)self->dreamSys);
    self->phase = DAYTASK_PHASE_RUNNING;
}

void DayTask__OnState4(void) {}

void DayTask__OnDreamSysNotify(void) {}

void DayTask__OnObjMNotify(DayTask *self, BasicClass *sender, s32 event) {
    CinematicCall cinematic;
    s32 result;

    switch (event) {
        case OBJM_STATE_TIME_UP:
            self->objM->methods->deinit(self->objM);
            self->objM->methods->release(self->objM);
            result = self->dreamSys->methods->endDay(self->dreamSys, 0);
            if (result == 0) {
                cinematic = self->dreamSys->methods->getCinematic(self->dreamSys);
                self->result = cinematic.entry < 0 ? DAYTASK_RESULT_ENDED : DAYTASK_RESULT_CINEMATIC;
            } else {
                self->result = DAYTASK_RESULT_CLOSED;
            }
            self->methods->setState(self, INTERMEDIATEBASE_STATE_STOP);
            break;
        case OBJM_STATE_LINK_DYNAMIC:
        case OBJM_STATE_LINK_WALL:
        case OBJM_STATE_LINK_FLASHBACK:
        case OBJM_STATE_LINK_TUNNEL:
        case OBJM_STATE_LINK_STAGE_TIMER:
            self->phase = DAYTASK_PHASE_REPLACE_OBJM;
            break;
        case OBJM_NOTIFY_CLOSE:
        case OBJM_NOTIFY_CLOSE_NEW_GAME:
            self->objM->methods->deinit(self->objM);
            self->objM->methods->release(self->objM);
            /* endDay(1) on CLOSE, endDay(2) (a new game) on CLOSE_NEW_GAME */
            self->dreamSys->methods->endDay(self->dreamSys, event != OBJM_NOTIFY_CLOSE ? 2 : 1);
            self->result = DAYTASK_RESULT_CLOSED;
            self->methods->setState(self, INTERMEDIATEBASE_STATE_STOP);
            break;
    }
}

DayTaskMethods *GetDayTaskMethods(void) {
    return &gDayTaskMethods;
}

/* src/code_39094.c: returns gRecordTable and writes its record count to
 * *out. */
extern void *GetRecordTable(s32 *out);
/* src/code_171e0.c: appends `count` records of `table` to the CD driver's
 * file table and resolves them; returns 0 to be retried, and 1 when the CD
 * driver is not the active data source. */
extern s32 RegisterFileTableEntries(void *table, s32 count);

/* How many times RegisterRecordTableFiles has run (a call with `all` set
 * counts as two), and how many records its first, half-table batch took. */
extern s32 sRecordRegisterCalls;
extern s32 sRecordFirstBatchCount;

/* Registers gRecordTable's records with the CD driver, retrying until it
 * accepts them. The first call registers the whole table when `all` is set,
 * else its first half; the second call registers the rest; any later call
 * registers nothing. */
s32 RegisterRecordTableFiles(s32 all) {
    s32 count;
    void *table;
    s32 prev;
    s32 result;

    table = GetRecordTable(&count);
    prev = sRecordRegisterCalls;
    sRecordRegisterCalls = prev + 1;

    switch (prev + 1) {
        case 1:
            if (all != 0) {
                sRecordRegisterCalls = prev + 2;
            } else {
                count = count / 2;
                sRecordFirstBatchCount = count;
            }
            break;
        case 2:
            count = count - sRecordFirstBatchCount;
            break;
        default:
            count = 0;
            break;
    }

    while ((result = RegisterFileTableEntries(table, count)) == 0) {
    }
    return result;
}

TimedTask *New_TimedTask(char *soundBankPath, BasicClass *sound) {
    TimedTask *self;

    self = BMemPMgrAlloc(sizeof(TimedTask));
    if (self != NULL) {
        GetTimedTaskMethods()->ctor(self, soundBankPath, sound);
        return self;
    }
    return NULL;
}

void TimedTask__TimedTask(TimedTask *self, char *soundBankPath, BasicClass *sound) {
    Get_vtable_IntermediateBase()->ctor((IntermediateBase *)self);
    self->methods = GetTimedTaskMethods();
    if (soundBankPath != NULL) {
        self->sound = (BasicClass *)New_VabStreamObj(soundBankPath);
    } else {
        self->sound = sound;
    }
    self->soundBankPath = soundBankPath;
    self->methods->resetCounters(self);
}

void TimedTask__Finalize(TimedTask *self) {
    if (self->soundBankPath != NULL) {
        self->sound->methods->release(self->sound);
    }
    Get_vtable_IntermediateBase()->finalize((IntermediateBase *)self);
}

void TimedTask__CancelTimeout(TimedTask *self) {
    self->methods->setTimeout(self, -1);
}

s32 TimedTask__Init(TimedTask *self, IntermediateBaseInitArgs *args, s32 mode) {
    self->result = 0;
    Get_vtable_IntermediateBase()->init((IntermediateBase *)self, args, mode);
    return self->result;
}

void TimedTask__Deinit(TimedTask *self) {
    Get_vtable_IntermediateBase()->deinit((IntermediateBase *)self);
}

void TimedTask__NoOpSlot58(void) {}

void TimedTask__CheckTimeout(TimedTask *self, BasicClass *sender, s32 event) {
    Get_vtable_IntermediateBase()->update((IntermediateBase *)self, sender, event);
    if ((u32)self->frameCounter > (u32)self->timeoutFrames) {
        self->methods->setState(self, TIMEDTASK_STATE_TIMED_OUT);
    }
}

void TimedTask__SetState(TimedTask *self, s32 state) {
    Get_vtable_IntermediateBase()->setState((IntermediateBase *)self, state);
    if (state == TIMEDTASK_STATE_TIMED_OUT) {
        self->result = TIMEDTASK_RESULT_TIMED_OUT;
        self->methods->onState4(self);
    }
}

void TimedTask__SetTimeout(TimedTask *self, s32 timeout) {
    self->timeoutFrames = (timeout < 0) ? timeout : timeout * TIMEDTASK_TIMEOUT_UNIT_FRAMES;
}
