/**
 * @file day_task.h
 * @brief DayTask, the task that runs one day of the dream, its method
 *        table, and the phases and results it moves through; and
 *        RegisterRecordTableFiles, the record-table registration its ctor
 *        and the image-view callback in src/app/game_application.c run.
 *
 * Application__RunMainLoop (src/app/application.c) calls
 * GameApplication__RunDayTask (src/app/game_application.c) when the GraphRoom poll
 * returns 2, and that builds one with New_DayTask(the application's
 * IntermediateBaseInitArgs, its DreamSys, config->dayTaskSyncDriver), runs
 * its init to completion, acts on the DayTaskResult init returns and
 * releases it.
 */
#ifndef DAY_TASK_H
#define DAY_TASK_H

#include "timed_task.h"

typedef struct DayTask DayTask;
typedef struct DayTaskMethods DayTaskMethods;

struct DreamSys;
struct WBgm;
struct TimImage;
struct LinkResource;
struct ObjM; /* include/objm.h */

/** DayTask::phase: which ObjM step DayTask__AdvancePhase takes on the next
 * DrawSystem VSync. */
enum DayTaskPhase {
    DAYTASK_PHASE_IDLE = 0,        /**< resetCounters (DayTask__ResetPhase) */
    DAYTASK_PHASE_READY = 1,       /**< onInit: the next VSync runs startDay */
    DAYTASK_PHASE_RUNNING = 2,     /**< StartObjM: an ObjM is running */
    DAYTASK_PHASE_REPLACE_OBJM = 3 /**< an ObjM link state: the next VSync replaces the ObjM */
};

/** TimedTask::result as a DayTask sets it: what its init returns to
 * GameApplication__RunDayTask. */
enum DayTaskResult {
    DAYTASK_RESULT_ENDED = 1,     /**< ObjM's TIME_UP, endDay(0) returned 0, no cinematic entry */
    DAYTASK_RESULT_CINEMATIC = 2, /**< as ENDED with a cinematic entry, or startDay refused the
                                       day; RunDayTask starts the cinematic stream */
    DAYTASK_RESULT_CLOSED = 3     /**< TIME_UP with endDay nonzero, or an ObjM close; RunDayTask
                                       sets skipGraphRoomPoll */
};

/**
 * TimedTask's slots, then DayTask's own. Overridden: ctor, finalize,
 * onNotify, resetCounters (DayTask__ResetPhase), init, deinit, onInit,
 * onDeinit, onDrawSystemEvent (DayTask__AdvancePhase) and onTimedOut.
 */
/* clang-format off */
#define DAYTASK_SLOTS(Self, CtorParams)                                                         \
    TIMEDTASK_SLOTS(Self, CtorParams);                                                            \
    /* +0x080 */ void (*onDreamSysNotify)(Self *self, BasicClass *sender, s32 event); /* DayTask__OnDreamSysNotify, empty; onNotify's 0x1F34 (DreamSys) sender */ \
    /* +0x084 */ void (*onObjMNotify)(Self *self, BasicClass *sender, s32 event)      /* DayTask__OnObjMNotify; onNotify's 0x2F230 (gObjMMethods) sender */
/* clang-format on */

/** TimedTask's fields, then DayTask's own, from +0x038. */
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

/** @brief DayTask's method table: DAYTASK_SLOTS with its ctor's parameters. */
struct DayTaskMethods {
    DAYTASK_SLOTS(DayTask, (DayTask * self, IntermediateBaseInitArgs *initArgs,
                            struct DreamSys *dreamSys, s32 syncDriver));
};

/**
 * @brief DayTask: a TimedTask that runs one day of the dream. It brackets a
 *        DreamSys startDay/endDay pair and, in between, runs the day's play
 *        as ObjM children, one per stage; init returns how the day ended
 *        (a DayTaskResult).
 *
 * Class id 0x1F230, table gDayTaskMethods, parent TimedTask
 * (include/timed_task.h); no class derives from it. Methods in
 * src/world/day_task.c. The object is 0x50 bytes (New_DayTask); its own
 * fields run from TimedTask's 0x38.
 *
 * Lifecycle, by `phase`:
 *  - ctor: TimedTask's with GetSoundEffectDir() as soundBankPath; loads
 *    "ETC\ETC.TIM" (etcTim, uploaded and its buffer freed),
 *    "ETC\DREAMER.TMD" (dreamerTmd) and the week's BGM (bgm); fills the
 *    init args' frameClock, lightRig (a StageMap) and viewport (a
 *    NodeGuardedViewport); adopts the DreamSys as a child and hands it
 *    `sound` and etcTim. finalize releases all of it.
 *  - init hands the DreamSys the pad, the FrameClock and the viewport;
 *    onInit sizes the viewport, shows its fade box and attaches the
 *    DreamSys as its view child (phase READY).
 *  - onDrawSystemEvent (DayTask__AdvancePhase), on each VSync: in READY runs
 *    startDay and starts an ObjM on the stage it returns; in REPLACE_OBJM
 *    releases the old ObjM and starts one on getCurrentStage. Either way the
 *    phase becomes RUNNING.
 *  - onObjMNotify: an ObjM link state sets REPLACE_OBJM; TIME_UP and the two
 *    close codes release the ObjM, call endDay (0, 1 or 2; 2 is DreamSys's
 *    new-game reset), set `result` and stop the task.
 *
 * Two overrides take fewer arguments than their slots, and the slots keep
 * IntermediateBase's types: init (DayTask__Init, self only;
 * GameApplication__RunDayTask calls it through DayTaskInitFn) and onInit
 * (DayTask__OnInit, self only; IntermediateBase__Init calls it with
 * (0, 0, 0)).
 */
struct DayTask {
    DAYTASK_FIELDS(DayTaskMethods);
};

/** DayTask's method table (class id 0x1F230). */
extern DayTaskMethods gDayTaskMethods;

/**
 * @brief The DayTask method table.
 * @return &gDayTaskMethods.
 */
extern DayTaskMethods *GetDayTaskMethods(void);

/** init's occupant, DayTask__Init, which takes self alone. */
typedef s32 (*DayTaskInitFn)(DayTask *self);

/**
 * @brief Allocates a DayTask (0x50 bytes) and runs its ctor through the
 *        table.
 * @param initArgs The application's init arguments; the ctor fills their
 *        viewport, frame clock and light rig.
 * @param dreamSys The dream's DreamSys, adopted as a child.
 * @param syncDriver Selects the data-source driver mode the ctor sets.
 * @return The new DayTask, or NULL when the allocation fails.
 */
DayTask *New_DayTask(IntermediateBaseInitArgs *initArgs, struct DreamSys *dreamSys, s32 syncDriver);

/**
 * @brief Constructor (slot +0x008): TimedTask's ctor on the sound-effect
 *        bank, then loads the day's shared resources (ETC\ETC.TIM,
 *        ETC\DREAMER.TMD, the week's BGM), registers the record-table files,
 *        builds the init args' viewport, frame clock and StageMap, adopts the
 *        DreamSys and hands it the sound object and etcTim.
 * @param self The task.
 * @param initArgs The application's init arguments, kept and filled.
 * @param dreamSys The dream's DreamSys.
 * @param syncDriver The ctor calls
 *        SetActiveDataSourceDriverMode(syncDriver == 0, 1, 1); what the
 *        driver's mode word means is not settled here.
 */
void DayTask__DayTask(DayTask *self, IntermediateBaseInitArgs *initArgs, struct DreamSys *dreamSys,
                      s32 syncDriver);

/**
 * @brief finalize (slot +0x00C): drops the DreamSys child, releases the init
 *        args' light rig, frame clock and viewport and the day's resources,
 *        then TimedTask's finalize.
 * @param self The task.
 */
void DayTask__Finalize(DayTask *self);

/**
 * @brief onNotify (slot +0x038): TimedTask's onNotify, then routes a DreamSys
 *        sender to onDreamSysNotify and an ObjM sender to onObjMNotify.
 * @param self The task.
 * @param sender The notifying object.
 * @param event Its event code.
 */
void DayTask__OnNotify(DayTask *self, BasicClass *sender, s32 event);

/**
 * @brief resetCounters (slot +0x040): phase = DAYTASK_PHASE_IDLE.
 * @param self The task.
 */
void DayTask__ResetPhase(DayTask *self);

/**
 * @brief init (slot +0x044): gives the DreamSys the pad and frame clock as
 *        children and the viewport, then runs TimedTask's init in mode 0.
 * @param self The task.
 * @return How the day ended (a DayTaskResult).
 */
s32 DayTask__Init(DayTask *self);

/**
 * @brief deinit (slot +0x048): TimedTask's deinit, then takes the viewport,
 *        pad and frame clock back from the DreamSys.
 * @param self The task.
 */
void DayTask__Deinit(DayTask *self);

/**
 * @brief onInit (slot +0x04C): sizes the viewport to the screen, shows its
 *        fade box, sets its packet budget, attaches the DreamSys as its view
 *        child and initialises its ordering tables (initOt); phase becomes
 *        DAYTASK_PHASE_READY.
 * @param self The task.
 */
void DayTask__OnInit(DayTask *self);

/**
 * @brief onDeinit (slot +0x050): the viewport's deinitOt, then detaches its
 *        view child.
 * @param self The task.
 */
void DayTask__OnDeinit(DayTask *self);

/**
 * @brief onDrawSystemEvent (slot +0x054): TimedTask's handler, then on a
 *        VSync starts the day's first ObjM (READY) or replaces the ObjM a
 *        link state ended (REPLACE_OBJM). A day startDay refuses ends the
 *        task at once with DAYTASK_RESULT_CINEMATIC.
 * @param self The task.
 * @param sender The DrawSystem.
 * @param event Its event code; only DRAWSYSTEM_EVENT_VSYNC advances.
 */
void DayTask__AdvancePhase(DayTask *self, BasicClass *sender, s32 event);

/**
 * @brief Builds an ObjM for a stage from the day's resources, adopts it, runs
 *        its init with the DreamSys and sets phase DAYTASK_PHASE_RUNNING.
 * @param self The task.
 * @param stage The stage to play.
 */
void DayTask__StartObjM(DayTask *self, s32 stage);

/**
 * @brief onTimedOut (slot +0x07C): empty.
 */
void DayTask__OnTimedOut(void);

/**
 * @brief onDreamSysNotify (slot +0x080): empty; a DreamSys notification
 *        needs no answer.
 */
void DayTask__OnDreamSysNotify(void);

/**
 * @brief onObjMNotify (slot +0x084): turns an ObjM's state into the next
 *        phase or the task's result. A link state sets
 *        DAYTASK_PHASE_REPLACE_OBJM; TIME_UP releases the ObjM, calls
 *        endDay(0) and sets ENDED or CINEMATIC (endDay 0, by whether the
 *        DreamSys has a cinematic entry) or CLOSED; the two close codes
 *        release the ObjM, call endDay(1) or endDay(2) (a new game) and set
 *        CLOSED. TIME_UP and the closes then stop the task.
 * @param self The task.
 * @param sender The ObjM.
 * @param event Its state or notify code (OBJM_STATE_*, OBJM_NOTIFY_*).
 */
void DayTask__OnObjMNotify(DayTask *self, BasicClass *sender, s32 event);

/**
 * @brief Registers the record table's sound-bank and stage records
 *        (GetRecordTable) with the CD driver, in at most two batches,
 *        retrying each until RegisterFileTableEntries accepts it.
 *
 * The first call takes the whole count when `all` is set (and counts as two
 * calls), else half of it; the second call takes the count the first left;
 * any later call registers nothing. Each batch is handed the table from its
 * first entry. DayTask's ctor calls it with 1,
 * GameApplication__RegisterFilesCallback (ShowImage's view callback) with 0.
 *
 * @param all Non-zero on the first call to register every record at once.
 * @return RegisterFileTableEntries' first non-zero result for the batch (1
 *         when the CD driver is not the active data source).
 */
s32 RegisterRecordTableFiles(s32 all);

#endif
