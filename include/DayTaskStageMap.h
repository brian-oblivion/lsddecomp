#ifndef DAYTASKSTAGEMAP_H
#define DAYTASKSTAGEMAP_H

#include "common.h"
#include "DayTask.h"

/*
 * Declarations src/world/DayTaskStageMap.c's DayTask, TimedTask and
 * RegisterRecordTableFiles use (the .c's banner says what it holds): the
 * functions they call that no header it includes declares, and the call-site
 * view of the one object it reaches without its class's header. DayTask and
 * TimedTask themselves are include/DayTask.h and include/TimedTask.h.
 */

/* The init args' drawSystem as DayTask__OnInit calls it: its +0x07C (the
 * DrawSystem's getDims) returns the size it hands the viewport's
 * setScreenSize. */
typedef struct SubObjE SubObjE;

typedef struct SubObjEMethods {
    u8 pad00[0x7C];
    struct ViewportSize *(*getDims)(SubObjE *self, s32 out); /* DrawSystem__GetDims; its out is NULL here */
} SubObjEMethods;

struct SubObjE {
    SubObjEMethods *methods;
};

/* The object allocator every New_<Class> calls. */
extern void *BMemPMgrAlloc(s32 size);

/* src/world/DreamAux.c; DayTask's finalize calls it after releasing its
 * resources. */
extern void ReleaseDreamAuxModels(void);

/* src/cd/GameFiles.c: the "SND\\SE" sound bank path, which DayTask's ctor
 * passes as TimedTask's soundBankPath. */
extern char *GetSoundEffectDir(s32 unused); /* arity-ok: MATCHING: the definition takes no parameter; this dead argument is the `move a0,zero` before DayTask__DayTask's call */

/* src/world/DreamAux.c; DayTask's ctor calls it, and its finalize
 * ReleaseDreamAuxModels. */
extern void InitDreamAux(void);

/* src/cd/GameFiles.c: one of the seven sSoundBankPaths words, each a VAB
 * path string ("SND\\AMBIENT" ... "SND\\STANDERD"), which DayTask's ctor
 * passes to New_WBgm as its vabPath. */
extern s32 PickSoundBank(s32 unused);

/* Defined in src/world/DayTaskStageMap.c, after DayTask's methods; GameApplicationFileResource.c
 * declares it too. */
extern s32 RegisterRecordTableFiles(s32 all);

/* src/app/GameApplicationFileResource.c (a void function); GameApplicationFileResource.c declares it the same
 * way. */
extern s32 SetActiveDataSourceDriverMode(s32 async, s32 mode2, s32 useVSyncCallback);

#endif
