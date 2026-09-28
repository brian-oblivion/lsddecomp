#ifndef DREAMAUX_H
#define DREAMAUX_H

#include "common.h"

/* The dream's aux entities and chunk triggers (src/world/DreamAux.c; its banner
 * describes the subsystem). Only that unit includes this header. */

/* The per-stage tables hold 14 pointers each (0x38 bytes between one
 * table's label and the next). */
#define DREAM_AUX_STAGE_COUNT 14

/* Slot i's entity uses mood row DREAM_AUX_FIRST_MOOD + i. */
#define DREAM_AUX_FIRST_MOOD 98

/* The resident aux entity. InitDreamAux loads `model` (ETC\SYMSPY.MOM) and
 * SetDreamAuxWorld builds `entity` over it (New_Entity's descriptor word
 * +0x00C). `pos` is the offset from the player PlaceDreamAuxEntityByPlayer
 * puts the entity at, {0, -200, 8000} in the image. There is one slot: the
 * position table follows it at +0x14. */
typedef struct DreamAuxSlot {
    struct ModelData *model;
    struct Entity *entity;
    s32 pos[3];
} DreamAuxSlot;

extern DreamAuxSlot sDreamAuxSlots[1];

/* sDreamAuxSlots one word in, so each element's `model` is that slot's
 * entity: ReleaseDreamAuxEntities walks it. */
extern DreamAuxSlot sDreamAuxSlots2[1];

/* One chunk trigger. `key` is the chunk's ChunkCoord (column, then row)
 * read as one s16. `dayParity` restricts the day: 0 any day, 1 odd days, 2
 * even days (CheckTriggerDayParity). `recordIndices` name up to three
 * records of the stage's TriggerRecord table, -1 ending the list early. */
typedef struct DreamAuxTriggerEntry {
    s16 key;
    s8 dayParity;
    s8 recordIndices[3];
} DreamAuxTriggerEntry;

extern s8 sDreamAuxTriggerCounts[DREAM_AUX_STAGE_COUNT];
extern DreamAuxTriggerEntry *sDreamAuxTriggerEntries[DREAM_AUX_STAGE_COUNT];

/* The two ModelData files InitDreamAux can load. With one slot, only
 * SYMSPY.MOM is ever requested. */
extern const char sMomPathSymSpy[];
extern const char sMomPathSymDog[];

/* TriggerRecord.condition: CheckDreamAuxTriggerCondition's tests, by id. A
 * negative condition tests -condition and passes only while the record has
 * not triggered. Id 0 passes; 18, 19 and ids from 22 up reach the
 * dream-colour test and index sSpecialColors past its 8 entries. */
enum TriggerCondition {
    TRIGGER_COND_ALWAYS = 1,
    TRIGGER_COND_PERIOD_PHASE_1 = 2, /* 2..4: IsDayInPeriodPhase(day, id - 1) */
    TRIGGER_COND_PERIOD_PHASE_2 = 3,
    TRIGGER_COND_PERIOD_PHASE_3 = 4,
    TRIGGER_COND_DAY_MOD3_IS_0 = 5,
    TRIGGER_COND_DAY_MOD3_NOT_0 = 6,
    TRIGGER_COND_STYLE_VARIANT_EVEN = 7, /* IsStyleVariantEven */
    TRIGGER_COND_DAY_MOD3_IS_1 = 8,      /* 8, 9: day % 3 == id - 7 */
    TRIGGER_COND_DAY_MOD3_IS_2 = 9,
    TRIGGER_COND_DREAM_COLOR_FIRST = 10, /* 10..17: IsCurrentDreamColor */
    TRIGGER_COND_EVEN_DAY = 20,
    TRIGGER_COND_ODD_DAY = 21
};

/* One spawn record of a stage's table (sDreamAuxGroupRecords[stage], 8-byte
 * stride). `triggered` latches once `condition` has passed; InitDreamAux
 * clears every latch. `modelIndex` picks the ModelData of the chunk's
 * TriggerWorld, and each of `spawnIndices` (-1 ends the list) one
 * sDreamAuxSpawnInfo placement for an Entity of mood row `moodIndex`. A
 * record whose moodIndex is 2 is followed by the one seven records on. */
typedef struct TriggerRecord {
    s8 triggered;
    s8 condition;
    s8 modelIndex;
    u8 moodIndex;
    s8 spawnIndices[4];
} TriggerRecord;

extern s8 sDreamAuxGroupCounts[DREAM_AUX_STAGE_COUNT];
extern TriggerRecord *sDreamAuxGroupRecords[DREAM_AUX_STAGE_COUNT];

extern bool CheckDreamAuxTriggerCondition(s32 day, TriggerRecord *record);
/* `desc` is New_Entity's descriptor, forwarded untouched; the caller has
 * put the ModelData in its word +0x00C. */
extern bool SpawnDreamAuxTriggerEntity(s32 moodIndex, void *desc, DreamAuxTriggerEntry *trigger,
                                       s32 spawnIndex);
extern void EnableTeleportsForKind(s32 moodIndex);
extern bool IsStyleVariantEven(void);
extern bool IsCurrentDreamColor(s32 condition);
extern bool IsDayInPeriodPhase(s32 day, s32 phase);

#endif
