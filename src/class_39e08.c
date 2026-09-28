/*
 * class_39e08 -- the methods of DayTask and of most of its parent TimedTask,
 * with one free function between them.
 *
 * DayTask (include/DayTask.h), New_DayTask through GetDayTaskMethods: the
 * task GameApplication__RunDayTask runs for one dream day. Its ctor loads
 * the day's shared resources (ETC\ETC.TIM, ETC\DREAMER.TMD, the week's
 * BGM) and fills the init args' viewport, frame clock and light rig; on each
 * DrawSystem VSync DayTask__AdvancePhase starts the day or replaces the
 * running ObjM, and DayTask__OnObjMNotify turns ObjM's states into the next
 * phase or the task's result.
 *
 * RegisterRecordTableFiles: registers gRecordTable's file entries with the
 * CD driver in at most two batches (DayTask's ctor, and the loader-task
 * callback in GameApplicationFileResource.c).
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
#include "Actor.h"
#include "LightRig.h"
#include "TimedTask.h"
#include "DrawSystem.h"
#include "PlacementGrid.h"
#include "LbdFile.h"
#include "GridCell.h"
#include "FlatLightObj.h"
#include "BMemPMgr.h"

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
    vabPath = PickSoundBank(0);
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
    ReleaseDreamAuxModels();
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
    vp->methods->setMaxPackets(vp, 1200);
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

/* src/GameFiles.c: returns gRecordTable and writes its record count to
 * *out. */
extern void *GetRecordTable(s32 *out);
/* src/GameApplicationFileResource.c: appends `count` records of `table` to the CD driver's
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

/* ---- merged from class_3ac78 ---- */

/*
 * class_3ac78 -- TimedTask's last two functions (TimedTask__PlaySound and
 * GetTimedTaskMethods, include/TimedTask.h; the rest are in class_39e08),
 * then the front third of StageMap (include/StageMap.h): the loaded part of
 * a stage's map, seven chunk slots each laid out as a lattice of GridCells.
 *
 * This third holds the object's life and its command path: the allocator
 * and ctor (seven slots, each an LbdFile, a placement list, a cellParent
 * GridCell attached at `origin` and STAGE_SLOT_CELLS cells STAGE_CELL_SIZE
 * apart, row stride STAGE_CHUNK_CELLS), Finalize, OnNotify, Reset,
 * OnSlotEvent, the per-tick update (UpdateIfEnabled: footprint tracking,
 * then the scale ramp), UnloadAllSlots, the setters ObjM configures it
 * through (SetChildParams, SetCallback, SetAcceptedTags, SetGridSpan,
 * SetConfig), and the path a command takes to the cells:
 * DispatchLinkCommand passes on an Actor sender's, ForwardAcceptedCommand
 * filters the sender against acceptedTags, ApplyToSenderFootprint turns
 * the sender's position into a 3 x 3 cell footprint (SetFootprintFromCell
 * or SetFootprintRect), and DispatchToRectCells hands the command to every
 * cell in it and every cell chained behind each (NotifyGridCell).
 * StageMap__NoOpSlotD8 is the empty +0x0D8 slot; nothing calls it.
 */

/* TimedTask::sound is BasicClass * (it may be the ctor's own argument); when
 * it is a New_VabStreamObj object, +0x080 is VabStreamObj__PlayTone. Plays
 * `tone` at full volume (127) throughout. */
void TimedTask__PlaySound(TimedTask *self, s32 tone) {
    VabStreamObj *sound = (VabStreamObj *)self->sound;

    if (sound != NULL) {
        sound->methods->playTone(sound, tone, 127, 127);
    }
}

TimedTaskMethods *GetTimedTaskMethods(void) {
    return &gTimedTaskMethods;
}

StageMap *New_StageMap(LongVec3 *origin, s32 autoLoad) {
    StageMap *self;

    self = BMemPMgrAlloc(sizeof(StageMap));
    if (self != NULL) {
        GetStageMapMethods()->ctor(self, origin, autoLoad);
        return self;
    }
    return NULL;
}

extern LongVec3 gDefaultOrigin;

void StageMap__StageMap(StageMap *self, LongVec3 *origin, s32 autoLoad) {
    s32 i;
    ChunkSlot *slot;
    GridCell *cell;
    GridCell **cells;
    GridCell **cursor;
    GridCell **end;
    LongVec3 pos;

    GetLightRigMethods()->ctor((LightRig *)self);
    self->methods = GetStageMapMethods();

    if (origin != NULL) {
        self->origin = *origin;
    } else {
        self->origin = gDefaultOrigin;
    }

    self->loadsPending = 0;
    self->pendingLoadCount = 0;
    self->chunksLoaded = 0;
    self->enabled = 0;
    self->target = NULL;
    self->acceptedTags = NULL;
    self->scaleRampTicks = 0;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];

        slot->loader = New_LbdFile();
        slot->loader->freeGuard = (slot->loader->buffer != NULL);
        slot->loader->elemKey = i;
        slot->loader->methods->setAutoLoadData(slot->loader, autoLoad);

        slot->heldObj = NULL;
        slot->unk18 = 0;
        slot->neighbour = i;
        slot->loadPending = 0;

        slot->placements = New_PlacementGrid(0);
        slot->cellParent = New_GridCell();
        slot->cellParent->methods->attachToParent(slot->cellParent, (SceneNode *)self, &self->origin);

        slot->cells = (GridCell **)BMemPMgrAlloc(STAGE_SLOT_CELLS * sizeof(GridCell *));
        if (slot->cells == NULL) {
            return;
        }

        pos.x = STAGE_CELL_SIZE / 2;
        pos.y = 0;
        pos.z = STAGE_CELL_SIZE / 2;

        /* MATCHING: `end` from `cells` before `cursor = cells`, or the load
         * of slot->cells no longer goes through $v0. */
        cells = slot->cells;
        end = cells + STAGE_SLOT_CELLS;
        cursor = cells;
        while (cursor < end) {
            cell = New_GridCell();
            *cursor = cell;
            cell->methods->attachToParent(cell, (SceneNode *)slot->cellParent, &pos);

            /* Cell centres, row by row; the test lets a row reach
             * STAGE_CHUNK_CELLS + 1 positions before it wraps. */
            pos.x += STAGE_CELL_SIZE;
            if (pos.x > STAGE_CHUNK_SIZE + STAGE_CELL_SIZE / 2) {
                pos.x = STAGE_CELL_SIZE / 2;
                pos.z += STAGE_CELL_SIZE;
            }

            /* MATCHING: each use reloads *cursor. */
            cell = *cursor;
            cell->methods->setLightMode(cell, 1); /* GsFOG */
            cell = *cursor;
            cursor++;
            cell->attribute |= GsDOFF;
        }
    }

    self->methods->addChild(self, (BasicClass *)GetDrawSystem());
    self->methods->reset(self);
}

void StageMap__Finalize(StageMap *self) {
    s32 i;
    ChunkSlot *slot;
    GridCell *cell;
    GridCell **cells;
    GridCell **cursor;
    GridCell **end;

    self->methods->removeChild(self, (BasicClass *)GetDrawSystem());

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(self, STAGEMAP_EVENT_SLOT_RELEASE,
                                                               slot, i);

        if (slot->loader != NULL) {
            slot->loader->methods->release(slot->loader);
        }

        if (slot->placements != NULL) {
            if (slot->placements->linkResource != NULL) {
                slot->placements->linkResource->methods->release(slot->placements->linkResource);
            }
            slot->placements = slot->placements->methods->release(slot->placements);
        }

        if (slot->cellParent != NULL) {
            slot->cellParent->methods->release(slot->cellParent);
        }

        cells = slot->cells;
        end = cells + STAGE_SLOT_CELLS;
        cursor = cells;
        while (cursor < end) {
            cell = *cursor;
            if (cell != NULL) {
                cell->methods->release(cell);
            }
            cursor++;
        }

        BMemPMgrFree(slot->cells);
    }

    GetLightRigMethods()->finalize((LightRig *)self);
}

/* A sender of DrawSystem's class (id nibble 0x1) goes on to onNotifyTag1. */
void StageMap__OnNotify(StageMap *self, BasicClass *sender, s32 command) {
    GetSceneNodeMethods()->onNotify((SceneNode *)self, sender, command);

    if ((sender->methods->header & CLASS_ID_ROOT_MASK) == DRAWSYSTEM_CLASS_ID) {
        self->methods->onNotifyTag1(self, sender, command);
    }
}

extern s32 gDefaultGridSpan;

void StageMap__Reset(StageMap *self) {
    self->config = NULL;
    self->acceptedTags = NULL;
    self->rectCount = 0;
    self->methods->setGridSpan(self, gDefaultGridSpan);
    self->unk1CC = -1;
    self->unk1D0 = -1;
    self->unk1D4 = -1;
    self->unk1D8 = -1;
}

/* Records the slot and passes a slot event on to the parents; a
 * STAGEMAP_EVENT_SLOT_RELEASE also releases the slot's heldObj. */
void StageMap__OnSlotEvent(StageMap *self, s32 command, ChunkSlot *slot) {
    GetSceneNodeMethods()->notifyWithHull((SceneNode *)self, command);

    /* MATCHING: two literal if+goto tests; an if/else-if chain inverts the
     * branches. */
    if (command == STAGEMAP_EVENT_SLOT_RELEASE)
        goto release;
    if (command == STAGEMAP_EVENT_SLOT_DATA_READY)
        goto record;
    return;

release:
    if (slot->heldObj != NULL) {
        slot->heldObj = slot->heldObj->methods->release(slot->heldObj);
    }

record:
    self->lastEventSlot = slot;
    self->methods->notifyParents(self, command);
}

void StageMap__UpdateIfEnabled(StageMap *self) {
    if (self->enabled) {
        self->methods->updateFootprintTracking(self);
        self->methods->stepScaleRamp(self);
    }
}

void StageMap__DispatchLinkCommand(StageMap *self, BasicClass *sender, s32 command) {
    if ((u8)sender->methods->header == ACTOR_CLASS_ID) {
        self->methods->forwardAcceptedCommand(self, sender, command);
    }
}

/* Cancel every slot's load, clear and release what it holds, then zero
 * the load counters and end the scale ramp. */
void StageMap__UnloadAllSlots(StageMap *self) {
    s32 i;
    ChunkSlot *slot;
    PlacementGrid *placements;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        slot->loader->methods->cancelRequests(slot->loader);
        slot->loadPending = 0;
        self->methods->clearSlotCells(self, slot);
        placements = slot->placements;
        if (placements->linkResource != NULL) {
            placements->linkResource =
                placements->linkResource->methods->release(placements->linkResource);
        }
        ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(self, STAGEMAP_EVENT_SLOT_RELEASE,
                                                               slot, i);
        slot->loader->methods->releaseDataBlock(slot->loader);
    }

    self->chunksLoaded = 0;
    self->pendingLoadCount = 0;
    self->methods->endScaleRamp(self);
}

void StageMap__SetChildParams(StageMap *self, s32 count, s32 dirs, s32 colors) {
    s32 i;
    FlatLightObj *light;

    for (i = 0; i < count; i++) {
        light = (FlatLightObj *)self->methods->getLight(self, i);
        light->methods->setColor(light, 1, (FlatLightColor *)colors);
        colors += sizeof(FlatLightColor);
        light->methods->setDirection(light, 1, (s16 *)dirs);
        dirs += 3 * sizeof(s16);
    }
}

void StageMap__SetCallback(StageMap *self, ChunkFileFn fn, void *ctx) {
    self->chunkFileFn = fn;
    self->chunkFileCtx = ctx;
}

void StageMap__SetAcceptedTags(StageMap *self, s32 *tags) {
    self->acceptedTags = tags;
}

void StageMap__ForwardAcceptedCommand(StageMap *self, void *sender, s32 command) {
    s32 *tag;
    u8 unused[24]; /* MATCHING: retail's frame is 24 bytes larger than the locals need */

    switch (command) {
        case 2:
        case 3:
        case 5:
        case 6:
        case 7:
        case 8:
            break;
        default:
            return;
    }

    tag = self->acceptedTags;
    if (tag == NULL)
        return;
    if (*tag == 0)
        return;

    do {
        if (*tag == ((BasicClass *)sender)->methods->header) {
            self->methods->applyToSenderFootprint(self, sender, command);
        }
        tag++;
    } while (*tag != 0);
}

void StageMap__ApplyToSenderFootprint(StageMap *self, SceneNode *sender, s32 command) {
    SplitLongVec3 *pos;
    CellRectSet savedRects;
    Descriptor10Ext desc;
    s32 savedRectCount;

    if (sender->parent != NULL) {
        pos = (SplitLongVec3 *)sender->coord2->workm.t;
    } else {
        pos = NULL;
    }

    if (self->methods->computeFootprintDescriptor(self, &desc, pos) != 0) {
        return;
    }

    savedRectCount = self->rectCount;
    savedRects = self->rects;

    if (self->config->isVertical == 0) {
        StageMap__SetFootprintFromCell(self, &desc, 3);
    } else {
        StageMap__SetFootprintRect(self, &desc, 3);
    }

    StageMap__DispatchToRectCells(self, sender, command);

    self->rectCount = savedRectCount;
    self->rects = savedRects;
}

void StageMap__SetFootprintFromCell(StageMap *self, Descriptor10Ext *desc, s32 span) {
    s16 row;

    self->footprintCol = desc->base.b2 - 1;
    row = desc->base.b3 - 1;
    self->footprintWidth = span;
    self->footprintHeight = span;
    self->footprintRow = row;
    StageMap__BuildFootprintRects(self);
}

/* Clamp a span x span footprint centred on desc's cell to the chunk's
 * STAGE_CHUNK_CELLS x STAGE_CHUNK_CELLS lattice: a cell on the low edge (0)
 * or the high edge loses one row/column there.
 * MATCHING: the edge tests read copies taken before the decrement, and the
 * height is `span` itself. */
void StageMap__SetFootprintRect(StageMap *self, Descriptor10Ext *desc, s32 span) {
    s32 col;
    s32 row;
    s32 width;
    s32 origCol;
    s32 origRow;

    width = span;
    col = desc->base.b2;
    row = desc->base.b3;
    origCol = col;
    origRow = row;

    if (col == 0) {
        width = span - 1;
    } else {
        col--;
    }
    if (origCol == STAGE_CHUNK_CELLS - 1) {
        width--;
    }

    if (origRow == 0) {
        span--;
    } else {
        row--;
    }
    if (origRow == STAGE_CHUNK_CELLS - 1) {
        span--;
    }

    self->rectCount = 1;
    self->rects.e[0].slotIndex = self->methods->findSlotIndexByChunk(self, desc->chunkIndex);
    self->rects.e[0].col = col;
    self->rects.e[0].row = row;
    self->rects.e[0].width = width;
    self->rects.e[0].height = span;
}

/* Notify every cell of every rectangle, and every object chained behind
 * each cell, with the cell's key in curCell while it is notified.
 * MATCHING: the comma increments go `rect++, i++` and `cell++, col++`. */
void StageMap__DispatchToRectCells(StageMap *self, SceneNode *sender, s32 command) {
    s32 i;
    s32 row;
    s32 col;
    CellRect *rect;
    ChunkSlot *slot;
    GridCell **cell;
    GridCell *chained;

    rect = self->rects.e;
    for (i = 0; i < self->rectCount; rect++, i++) {
        slot = &self->slots[rect->slotIndex];
        if (slot->loader->headerReady != 0) {
            cell = (slot->cells + rect->col) + rect->row * STAGE_CHUNK_CELLS;
            for (row = 0; row < rect->height; row++) {
                for (col = 0; col < rect->width; cell++, col++) {
                    /* MATCHING: b0/b1 as one halfword; two byte copies are two lb/sb */
                    *(u16 *)&self->curCell = *(u16 *)&self->targetCell;
                    self->curCell.b2 = rect->col + col;
                    self->curCell.b3 = rect->row + row;
                    NotifyGridCell(*cell, sender, command);
                    for (chained = (*cell)->nextInCell; chained != NULL; chained = chained->nextInCell) {
                        NotifyGridCell(chained, sender, command);
                    }
                }
                cell += STAGE_CHUNK_CELLS - rect->width;
            }
        }
    }
}

/* Hands the command to a cell flagged GRIDCELL_FLAG_TAKES_COMMANDS. sender
 * and command arrive as DispatchToRectCells' own. */
void NotifyGridCell(GridCell *cell, SceneNode *sender, s32 command) {
    if (cell != NULL && (cell->flags36 & GRIDCELL_FLAG_TAKES_COMMANDS)) {
        cell->methods->onNotify(cell, sender, command);
    }
}

Descriptor10 *StageMap__GetCurrentCellKey(StageMap *self) {
    return &self->curCell;
}

void StageMap__NoOpSlotD8(void) {}

void StageMap__SetGridSpan(StageMap *self, s32 span) {
    self->gridSpan = span;
    self->gridCells = (s16)(span >> STAGE_CELL_SHIFT);
    self->gridHalfCells = (s16)(span >> (STAGE_CELL_SHIFT + 1));
}

void StageMap__SetConfig(StageMap *self, StageGridDimensions *config) {
    self->methods->reset(self);
    self->config = config;
}
