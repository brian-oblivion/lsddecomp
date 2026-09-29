/**
 * @file dream_day.h
 * @brief RegisterRecordTableFiles, the record-table registration
 *        src/world/dream_day.c defines, for DayTask's ctor and the
 *        image-view callback in src/app/game_shell.c.
 *
 * Includes include/day_task.h, so an includer also gets DayTask and
 * TimedTask.
 */
#ifndef DREAM_DAY_H
#define DREAM_DAY_H

#include "common.h"
#include "day_task.h"

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
extern s32 RegisterRecordTableFiles(s32 all);

#endif
