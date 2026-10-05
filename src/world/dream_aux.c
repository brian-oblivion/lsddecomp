/*
 * dream_aux.c -- the dream's auxiliary entities: one resident Entity kept
 * near the player, and the chunk triggers that spawn Entities as the
 * StageMap loads chunks. include/dream_aux.h describes the lifecycle and
 * the trigger flow; this file holds the tables' walkers in the order the
 * flow runs them: load and release, the world, the lookup, the day and
 * condition tests, and the spawn.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "entity.h"
#include "dream_aux.h"
#include "scene_node.h"
#include "model_data.h"
#include "trigger_world.h"
#include "dream_sys.h"
#include "stage_map.h"
#include <rand.h>

/** @brief One placement: the cell (column, row) inside the chunk, a yaw
 * from sDreamAuxSpawnRotations and an offset inside the cell from
 * sDreamAuxPosTable. */
typedef struct {
    u16 cell;         /**< the cell's key inside the chunk */
    s8 rotationIndex; /**< the yaw, an index into sDreamAuxSpawnRotations */
    s8 offsetIndex;   /**< the offset in the cell, an index into sDreamAuxPosTable */
} DreamAuxSpawnInfo;

/* A trigger entry's chunk, a StageChunk (column, then row) read as one s16. */
#define CHUNK_KEY(col, row) ((row) << 8 | (col))

/* dream_aux.c's data, in address order. */
#include "dream_aux_tables.inc"

/* The two ModelData files InitDreamAux can load. With one slot, only
 * SYMSPY.MOM is ever requested. */
const char sMomPathSymSpy[] = "ETC\\SYMSPY.MOM";
const char sMomPathSymDog[] = "ETC\\SYMDOG.MOM";

void InitDreamAux(void) {
    ResourceRequest req;
    u32 i; /* MATCHING: unsigned; a signed counter compiles a signed loop test */
    s32 record;

    for (i = 0; i < ARRAY_COUNT(sDreamAuxGroupRecords); i++) {
        for (record = 0; record < sDreamAuxGroupCounts[i]; record++) {
            sDreamAuxGroupRecords[i][record].triggered = 0;
        }
    }

    ResourceRequest__Set(&req, 0, (char *)sMomPathSymSpy, 1);

    for (i = 0; i < ARRAY_COUNT(sDreamAuxSlots); i++) {
        sDreamAuxSlots[i].model = New_ModelData(&req.src);
        req.src.name = (char *)sMomPathSymDog;
    }
}

void ReleaseDreamAuxModels(void) {
    DreamAuxSlot *slot = sDreamAuxSlots;
    u32 i; /* MATCHING: unsigned; a signed counter compiles a signed loop test */

    for (i = 0; i < ARRAY_COUNT(sDreamAuxSlots); i++) {
        ModelData *model = slot->model;

        if (model != NULL) {
            slot->model = model->methods->release(model);
        }
        slot++;
    }
}

/* DreamAux's small data, what SetDreamAuxWorld installs: the stage, its
 * StageMap (the parent every Entity here attaches to), the player (each
 * Entity's peer), the sound bank each Entity is built with and the
 * FrameClock each attaches as its companion. No stage (-1) and NULL until
 * then. */
static s32 sDreamAuxStage SDATA = -1;
static StageMap *sDreamAuxStageMap SDATA = NULL;
static DreamSys *sDreamAuxWorld SDATA = NULL;
static struct VabStreamObj *sDreamAuxSound SDATA = NULL;
static struct FrameClock *sDreamAuxFrameClock SDATA = NULL;

void SetTeleportsEnabled(s32 stage);

void SetDreamAuxWorld(s32 stage, StageMap *stageMap, DreamSys *world, struct VabStreamObj *sound,
                      struct FrameClock *frameClock) {
    DreamAuxSlot *slot = sDreamAuxSlots;
    u32 i; /* MATCHING: unsigned; a signed counter compiles a signed loop test */

    sDreamAuxStage = stage;
    sDreamAuxStageMap = stageMap;
    sDreamAuxWorld = world;
    sDreamAuxSound = sound;
    sDreamAuxFrameClock = frameClock;

    for (i = 0; i < ARRAY_COUNT(sDreamAuxSlots); i++) {
        s32 desc[4]; /* New_Entity's descriptor: word +0x00C the ModelData */
        desc[3] = (s32)slot->model;
        slot->entity = New_Entity(i + DREAM_AUX_FIRST_MOOD, desc, sDreamAuxSound);
        slot++;
    }
    SetTeleportsEnabled(stage);
}

void SetTeleportsEnabled(s32 stage) {
    SetInstantTeleportersEnabled(stage == STAGE_SUN_FACES_HEAVE || stage == STAGE_NATURAL_WORLD);
}

/* Mood rows 11, 56, 78 and 93 turn the instant teleporters on. */
void EnableTeleportsForKind(s32 moodIndex) {
    switch (moodIndex) {
        case 11:
        case 56:
        case 78:
        case 93:
            SetInstantTeleportersEnabled(1);
            break;
    }
}

void ReleaseDreamAuxEntities(void) {
    u32 i; /* MATCHING: unsigned; a signed counter compiles a signed loop test */
    DreamAuxSlot *slot;

    slot = sDreamAuxSlots;
    for (i = 0; i < ARRAY_COUNT(sDreamAuxSlots); i++) {
        Entity *entity = slot->entity;

        if (entity != NULL) {
            slot->entity = entity->methods->release(entity);
        }
        slot++;
    }
}

DreamAuxTriggerEntry *LookupDreamAuxTrigger(s16 *chunkKey);
bool CheckTriggerDayParity(s32 day, DreamAuxTriggerEntry *trigger);
TriggerWorld *FireDreamAuxTriggerEntries(s32 day, DreamAuxTriggerEntry *trigger, s32 data);
void PlaceDreamAuxEntityByPlayer(DreamAuxSlot *slot);

s32 TryDreamAuxTrigger(s32 data, s16 *chunkKey, s32 day) {
    DreamAuxTriggerEntry *trigger = LookupDreamAuxTrigger(chunkKey);

    if (trigger != NULL) {
        if (CheckTriggerDayParity(day, trigger)) {
            return (s32)FireDreamAuxTriggerEntries(day, trigger, data);
        }
        if (sDreamAuxStage != STAGE_BRIGHT_MOON_COTTAGE && rand() % 12 == 0 && (day & 1) == 0) {
            PlaceDreamAuxEntityByPlayer(sDreamAuxSlots);
        }
    }
    return 0;
}

DreamAuxTriggerEntry *RemapTriggerForDreamColor(DreamAuxTriggerEntry *trigger, s32 index);

DreamAuxTriggerEntry *LookupDreamAuxTrigger(s16 *chunkKey) {
    s32 stage = sDreamAuxStage;
    s32 count = sDreamAuxTriggerCounts[stage];
    DreamAuxTriggerEntry *trigger = sDreamAuxTriggerEntries[stage];
    s32 i;

    for (i = 0; i < count; i++) {
        if (*chunkKey == trigger->key) {
            return RemapTriggerForDreamColor(trigger, i);
        }
        trigger++;
    }
    return NULL;
}

/* In Happy Town, a red dream swaps the stage's trigger 16 for trigger 21. */
DreamAuxTriggerEntry *RemapTriggerForDreamColor(DreamAuxTriggerEntry *trigger, s32 index) {
    s32 stage = sDreamAuxStage;

    if (stage == STAGE_HAPPY_TOWN && index == 16) {
        DreamSys *player = sDreamAuxWorld;
        s32 color = player->methods->getDreamColor(player);

        if (color == DREAM_COLOR_RED) {
            trigger += 5;
        }
    }
    return trigger;
}

/* Whether `trigger` may fire on `day` (see DreamAuxTriggerEntry.dayParity). */
bool CheckTriggerDayParity(s32 day, DreamAuxTriggerEntry *trigger) {
    bool result = true;

    if (trigger->dayParity != 0) {
        day = day % 2 + 1;
        result = trigger->dayParity != day;
    }
    return result;
}

bool ProcessDreamAuxTriggerRecord(s32 day, DreamAuxTriggerEntry *trigger, TriggerRecord *record,
                                  TriggerWorld *world);

TriggerWorld *FireDreamAuxTriggerEntries(s32 day, DreamAuxTriggerEntry *trigger, s32 data) {
    /* MATCHING: only src.buffer is set, but a bare ResourceSource shrinks
     * the frame by 8. */
    ResourceRequest req;
    TriggerWorld *world;

    req.src.buffer = (void *)data;
    world = New_TriggerWorld(&req.src);

    if (world != NULL) {
        TriggerRecord *records = sDreamAuxGroupRecords[sDreamAuxStage];
        s8 *next = trigger->recordIndices;
        s8 *end = trigger->recordIndices + ARRAY_COUNT(trigger->recordIndices);

        while (next < end) {
            s8 index = *next;

            if (index == -1) {
                break;
            }
            ProcessDreamAuxTriggerRecord(day, trigger, &records[index], world);
            next++;
        }
        return world;
    }
    return NULL;
}

/* Spawns `record`'s Entities if its condition holds, and follows a
 * TRIGGER_CHAIN_MOOD_ROW record's chain. True only when an Entity could not be made. */
bool ProcessDreamAuxTriggerRecord(s32 day, DreamAuxTriggerEntry *trigger, TriggerRecord *record,
                                  TriggerWorld *world) {
    s8 *spawn;
    s8 *end;
    ModelData *model;
    s32 desc[4]; /* New_Entity's descriptor: word +0x00C the ModelData */

    if (!CheckDreamAuxTriggerCondition(day, record)) {
        return false;
    }

    EnableTeleportsForKind(record->moodIndex);

    spawn = record->spawnIndices;
    end = record->spawnIndices + ARRAY_COUNT(record->spawnIndices);
    model = world->methods->getModelData(world, record->modelIndex);
    desc[3] = (s32)model;

    if (model != NULL) {
        while (spawn < end) {
            if (*spawn == -1) {
                break;
            }
            if (SpawnDreamAuxTriggerEntity(record->moodIndex, desc, trigger, (u8)*spawn)) {
                return true;
            }
            spawn++;
        }
    }

    if (record->moodIndex != TRIGGER_CHAIN_MOOD_ROW) {
        return false;
    }
    return ProcessDreamAuxTriggerRecord(day, trigger, record + TRIGGER_CHAIN_STRIDE, world);
}

/* Tests `record`'s condition against `day` (enum TriggerCondition) and
 * latches `triggered` when it passes. */
bool CheckDreamAuxTriggerCondition(s32 day, TriggerRecord *record) {
    s8 condition = record->condition;
    s32 id;

    if (condition == TRIGGER_COND_ALWAYS) {
        record->triggered = 1;
        return true;
    }

    /* MATCHING: gotos; retail lays the negating arm out after the plain one, which no if/else gives */
    if (condition < 0) {
        if (record->triggered == 0) {
            goto negate;
        }
        return false;
    }
    id = condition;
    goto have_idx;

negate:
    id = ~condition + 1; /* MATCHING: not -condition; retail complements and adds one */

have_idx:

    switch (id) {
        case TRIGGER_COND_PERIOD_PHASE_1:
        case TRIGGER_COND_PERIOD_PHASE_2:
        case TRIGGER_COND_PERIOD_PHASE_3:
            if (!IsDayInPeriodPhase(day, id - 1)) {
                return false;
            }
            break;
        case TRIGGER_COND_DAY_MOD3_IS_0:
            if (day % 3 != 0) {
                return false;
            }
            break;
        case TRIGGER_COND_DAY_MOD3_NOT_0:
            if (day % 3 == 0) {
                return false;
            }
            break;
        case TRIGGER_COND_STYLE_VARIANT_EVEN:
            if (!IsStyleVariantEven()) {
                return false;
            }
            break;
        case TRIGGER_COND_DAY_MOD3_IS_1:
        case TRIGGER_COND_DAY_MOD3_IS_2:
            if (day % 3 != id - 7) {
                return false;
            }
            break;
        case TRIGGER_COND_EVEN_DAY:
            if ((day & 1) != 0) {
                return false;
            }
            break;
        case TRIGGER_COND_ODD_DAY:
            if ((day & 1) == 0) {
                return false;
            }
            break;
        default:
            if (id >= TRIGGER_COND_DREAM_COLOR_FIRST) {
                if (!IsCurrentDreamColor(id)) {
                    return false;
                }
            }
            break;
    }

    record->triggered = 1;
    return true;
}

/* Whether the player's dream colour is sSpecialColors' entry for trigger
 * condition `condition` (10..17). */
bool IsCurrentDreamColor(s32 condition) {
    DreamSys *player = sDreamAuxWorld;
    s32 color = sSpecialColors[condition - TRIGGER_COND_DREAM_COLOR_FIRST];
    s32 current = player->methods->getDreamColor(player);

    return color == current;
}

/* The length of the periods IsDayInPeriodPhase counts, in days. */
#define DREAM_PERIOD_DAYS 30

/* Whether `day`'s 30-day period, counted from 1, is phase, phase + 3,
 * phase + 6 or phase + 9: with phase 1..3, every third period of the 12. */
bool IsDayInPeriodPhase(s32 day, s32 phase) {
    s32 period = (day - 1) / DREAM_PERIOD_DAYS + 1;
    s32 i;

    for (i = 0; i < 4; i++) {
        if (period == phase) {
            return true;
        }
        phase += 3;
    }
    return false;
}

/* Makes an Entity of mood row `moodIndex`, turns it and attaches it at
 * placement `spawnIndex` of `trigger`'s chunk. True when New_Entity failed. */
bool SpawnDreamAuxTriggerEntity(s32 moodIndex, void *desc, DreamAuxTriggerEntry *trigger, s32 spawnIndex) {
    Entity *entity = New_Entity(moodIndex, desc, sDreamAuxSound);

    if (entity != NULL) {
        DreamAuxSpawnInfo *spawn;

        CellKeyDesc cellDesc;

        s32 worldPos[4];

        cellDesc.key.chunk = trigger->key;
        spawn = &sDreamAuxSpawnInfo[spawnIndex];
        cellDesc.key.cell = spawn->cell;
        cellDesc.offset = sDreamAuxPosTable[spawn->offsetIndex];

        sDreamAuxStageMap->methods->computeCellOffsets(sDreamAuxStageMap, worldPos, &cellDesc);
        entity->methods->updateRotation(entity, 1, sDreamAuxSpawnRotations[spawn->rotationIndex]);
        ((TodActorAttachToParentFn)entity->methods->attachToParent)(
            (TodActor *)entity, (TodActor *)sDreamAuxWorld, sDreamAuxFrameClock,
            (void *)sDreamAuxStageMap, worldPos);
        return false;
    }
    return true;
}

/* Re-attaches `slot`'s entity at its offset from the player, facing the
 * player. */
void PlaceDreamAuxEntityByPlayer(DreamAuxSlot *slot) {
    if (slot->entity != NULL) {
        s32 worldPos[3];

        slot->entity->methods->detachFromParent(slot->entity);
        SceneNode__LocalOffsetToWorldPos((SceneNode *)sDreamAuxWorld, worldPos, slot->pos, 0);
        ((TodActorAttachToParentFn)slot->entity->methods->attachToParent)(
            (TodActor *)slot->entity, (TodActor *)sDreamAuxWorld, sDreamAuxFrameClock,
            (void *)sDreamAuxStageMap, worldPos);
        SceneNode__FaceTarget((SceneNode *)slot->entity, (SceneNode *)sDreamAuxWorld, 1, 0, NULL);
    }
}
