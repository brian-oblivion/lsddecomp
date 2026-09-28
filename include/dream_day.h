#ifndef DREAM_DAY_H
#define DREAM_DAY_H

#include "common.h"
#include "day_task.h"

/*
 * Declarations src/world/dream_day.c's DayTask, TimedTask and
 * RegisterRecordTableFiles use (the .c's banner says what it holds): the
 * functions they call that no header it includes declares. DayTask and
 * TimedTask themselves are include/day_task.h and include/TimedTask.h.
 */

/* Defined in src/world/dream_day.c, after DayTask's methods; game_shell.c
 * declares it too. */
extern s32 RegisterRecordTableFiles(s32 all);

#endif
