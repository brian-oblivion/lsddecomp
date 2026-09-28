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

/* src/world/DreamAux.c; DayTask's finalize calls it after releasing its
 * resources. */

/* src/world/DreamAux.c; DayTask's ctor calls it, and its finalize
 * ReleaseDreamAuxModels. */

/* Defined in src/world/DayTaskStageMap.c, after DayTask's methods; GameApplicationFileResource.c
 * declares it too. */
extern s32 RegisterRecordTableFiles(s32 all);

#endif
