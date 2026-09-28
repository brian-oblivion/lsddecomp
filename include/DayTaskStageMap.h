#ifndef DAYTASKSTAGEMAP_H
#define DAYTASKSTAGEMAP_H

#include "common.h"
#include "DayTask.h"

/*
 * Declarations src/world/DayTaskStageMap.c's DayTask, TimedTask and
 * RegisterRecordTableFiles use (the .c's banner says what it holds): the
 * functions they call that no header it includes declares. DayTask and
 * TimedTask themselves are include/DayTask.h and include/TimedTask.h.
 */

/* Defined in src/world/DayTaskStageMap.c, after DayTask's methods; GameApplicationFileResource.c
 * declares it too. */
extern s32 RegisterRecordTableFiles(s32 all);

#endif
