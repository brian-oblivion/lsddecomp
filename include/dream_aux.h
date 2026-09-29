#ifndef DREAM_AUX_H
#define DREAM_AUX_H

#include "common.h"

/**
 * @file dream_aux.h
 * @brief The dream's auxiliary entities: one resident Entity kept near the
 *        player, and the chunk triggers that spawn Entities as the StageMap
 *        loads chunks.
 *
 * Defined in src/world/dream_aux.c, except IsStyleVariantEven
 * (src/world/style_layer.c). DayTask (src/world/dream_day.c) and ObjM
 * (src/world/objm.c) are its clients.
 *
 * Lifecycle: DayTask's ctor calls InitDreamAux, which clears every trigger
 * record's latch and loads the resident entity's ModelData
 * (ETC\\SYMSPY.MOM); its finalize calls ReleaseDreamAuxModels. ObjM's scene
 * setup calls SetDreamAuxWorld with the stage, the StageMap, the player
 * DreamSys, the VabStreamObj and the FrameClock every Entity here is built
 * and attached with, which also builds the resident Entity; ObjM's teardown
 * calls ReleaseDreamAuxEntities. The instant teleporters are on for stages 3
 * and 11 and off elsewhere, and a trigger that spawns mood row 11, 56, 78 or
 * 93 turns them on (EnableTeleportsForKind).
 *
 * Triggers: when the StageMap has loaded a chunk's data block, ObjM passes
 * it to TryDreamAuxTrigger with the chunk's coordinates and the day. The
 * chunk's DreamAuxTriggerEntry names up to three TriggerRecords; on a day
 * its dayParity allows, a TriggerWorld is built over the data block (its
 * models), and for each record whose condition holds
 * (CheckDreamAuxTriggerCondition: the day, the dream colour or the style
 * variant) one Entity per spawn index is placed in the chunk's cell
 * (SpawnDreamAuxTriggerEntity). The TriggerWorld goes back to ObjM, which
 * keeps it with the chunk. When the parity rules the day out, on a stage
 * other than 0 and an even day, one time in 12 the resident entity is moved
 * next to the player instead.
 */

/** The stages the per-stage trigger and record tables cover. */
#define DREAM_AUX_STAGE_COUNT 14

/** Slot i's resident entity uses mood row DREAM_AUX_FIRST_MOOD + i. */
#define DREAM_AUX_FIRST_MOOD 98

/**
 * @brief The resident auxiliary entity. There is one slot (sDreamAuxSlots).
 */
typedef struct DreamAuxSlot {
    struct ModelData *model; /**< ETC\\SYMSPY.MOM, loaded by InitDreamAux. */
    struct Entity *entity;   /**< Built over `model` by SetDreamAuxWorld. */
    s32 pos[3]; /**< The offset from the player the entity is moved to: {0, -200, 8000}. */
} DreamAuxSlot;

/**
 * @brief One chunk trigger of a stage's table (sDreamAuxTriggerEntries).
 */
typedef struct DreamAuxTriggerEntry {
    s16 key;             /**< The chunk's MapChunk (column, then row) read as one s16. */
    s8 dayParity;        /**< 0 any day, 1 odd days, 2 even days (CheckTriggerDayParity). */
    s8 recordIndices[3]; /**< Up to three records of the stage's TriggerRecord table; -1 ends the list. */
} DreamAuxTriggerEntry;

/**
 * @brief TriggerRecord::condition: CheckDreamAuxTriggerCondition's tests, by
 * id.
 *
 * A negative condition tests -condition and passes only while the record has
 * not triggered. Id 0 passes; 18, 19 and ids from 22 up reach the
 * dream-colour test and index sSpecialColors past its 8 entries.
 */
enum TriggerCondition {
    TRIGGER_COND_ALWAYS = 1,             /**< Always passes. */
    TRIGGER_COND_PERIOD_PHASE_1 = 2,     /**< 2..4: IsDayInPeriodPhase(day, id - 1). */
    TRIGGER_COND_PERIOD_PHASE_2 = 3,     /**< See TRIGGER_COND_PERIOD_PHASE_1. */
    TRIGGER_COND_PERIOD_PHASE_3 = 4,     /**< See TRIGGER_COND_PERIOD_PHASE_1. */
    TRIGGER_COND_DAY_MOD3_IS_0 = 5,      /**< day % 3 == 0. */
    TRIGGER_COND_DAY_MOD3_NOT_0 = 6,     /**< day % 3 != 0. */
    TRIGGER_COND_STYLE_VARIANT_EVEN = 7, /**< IsStyleVariantEven. */
    TRIGGER_COND_DAY_MOD3_IS_1 = 8,      /**< day % 3 == 1. */
    TRIGGER_COND_DAY_MOD3_IS_2 = 9,      /**< day % 3 == 2. */
    TRIGGER_COND_DREAM_COLOR_FIRST = 10, /**< 10..17: IsCurrentDreamColor(id). */
    TRIGGER_COND_EVEN_DAY = 20,          /**< An even day. */
    TRIGGER_COND_ODD_DAY = 21            /**< An odd day. */
};

/** A trigger record whose moodIndex is this chains on to the record
 * TRIGGER_CHAIN_STRIDE records later (ProcessDreamAuxTriggerRecord). */
#define TRIGGER_CHAIN_MOOD_ROW 2

/** How many records on a chained record's successor is. */
#define TRIGGER_CHAIN_STRIDE 7

/**
 * @brief One spawn record of a stage's table (sDreamAuxGroupRecords[stage]).
 * A record of mood row TRIGGER_CHAIN_MOOD_ROW chains on to another.
 */
typedef struct TriggerRecord {
    s8 triggered;  /**< Latched once `condition` has passed; InitDreamAux clears every latch. */
    s8 condition;  /**< enum TriggerCondition, negated to fire only once. */
    s8 modelIndex; /**< The ModelData of the chunk's TriggerWorld. */
    u8 moodIndex;  /**< The spawned Entities' mood row. */
    s8 spawnIndices[4]; /**< sDreamAuxSpawnInfo placements, one Entity each; -1 ends the list. */
} TriggerRecord;

/**
 * @brief Tests `record`'s condition against `day` and latches `triggered`
 * when it passes.
 * @param day    The dream day.
 * @param record The record to test.
 * @return Whether the condition holds.
 */
extern bool CheckDreamAuxTriggerCondition(s32 day, TriggerRecord *record);

/**
 * @brief Makes an Entity of mood row `moodIndex`, turns it and attaches it
 * at placement `spawnIndex` inside `trigger`'s chunk.
 * @param moodIndex  The Entity's mood row.
 * @param desc       New_Entity's descriptor, forwarded untouched; word
 *                   +0x00C holds the ModelData.
 * @param trigger    The chunk's trigger.
 * @param spawnIndex The sDreamAuxSpawnInfo placement.
 * @return True when New_Entity failed.
 */
extern bool SpawnDreamAuxTriggerEntity(s32 moodIndex, void *desc, DreamAuxTriggerEntry *trigger,
                                       s32 spawnIndex);

/**
 * @brief Turns the instant teleporters on when `moodIndex` is 11, 56, 78 or
 * 93.
 * @param moodIndex The spawned Entity's mood row.
 */
extern void EnableTeleportsForKind(s32 moodIndex);

/**
 * @brief Whether the style variant PickStyleFallbackConfig chose is even
 * (a stage with a fixed style config keeps variant -1, which is odd).
 * @return 1 for an even variant, else 0.
 */
extern s32 IsStyleVariantEven(void);

/**
 * @brief Whether the player's dream colour is sSpecialColors' entry for
 * trigger condition `condition`.
 * @param condition TRIGGER_COND_DREAM_COLOR_FIRST and up.
 * @return Whether the colours are equal.
 */
extern bool IsCurrentDreamColor(s32 condition);

/**
 * @brief Whether `day`'s 30-day period, counted from 1, is phase, phase + 3,
 * phase + 6 or phase + 9: with phase 1..3, every third period of the 12.
 * @param day   The dream day, from 1.
 * @param phase 1, 2 or 3.
 * @return Whether the day's period is in that phase.
 */
extern bool IsDayInPeriodPhase(s32 day, s32 phase);

struct StageMap;
struct DreamSys;
struct VabStreamObj;
struct FrameClock;

/**
 * @brief Clears every trigger record's latch and loads the resident entity's
 * ModelData. DayTask's ctor calls it.
 */
extern void InitDreamAux(void);

/**
 * @brief Releases the resident entity's ModelData. DayTask's finalize calls
 * it.
 */
extern void ReleaseDreamAuxModels(void);

/**
 * @brief Keeps what every auxiliary Entity is built and attached with, builds
 * the resident Entity and sets the instant teleporters for the stage.
 * ObjM__SetupSceneStyle calls it.
 * @param stage      The stage.
 * @param stageMap   The StageMap, every Entity's parent.
 * @param world      The player's DreamSys, every Entity's peer.
 * @param sound      The sound bank every Entity is built with.
 * @param frameClock The FrameClock every Entity attaches as its companion.
 */
extern void SetDreamAuxWorld(s32 stage, struct StageMap *stageMap, struct DreamSys *world,
                             struct VabStreamObj *sound, struct FrameClock *frameClock);

/**
 * @brief Releases the resident Entity. ObjM__TeardownStyle calls it.
 */
extern void ReleaseDreamAuxEntities(void);

/**
 * @brief Fires the trigger of a loaded chunk: builds a TriggerWorld over the
 * chunk's data block and spawns the Entities of every record whose condition
 * holds, on a day the trigger's parity allows; otherwise, now and then, moves
 * the resident entity next to the player.
 * @param data     The chunk's data block.
 * @param chunkKey The chunk's key (its column and row bytes).
 * @param day      The dream day.
 * @return The TriggerWorld, which the chunk keeps, or 0 when nothing fired.
 */
extern s32 TryDreamAuxTrigger(s32 data, s16 *chunkKey, s32 day);

#endif
