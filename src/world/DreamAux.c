/*
 * DreamAux -- the dream's aux entities: one resident Entity kept near the
 * player, and the chunk triggers that spawn Entities as the StageMap loads
 * chunks.
 *
 * Lifecycle. DayTask's ctor calls InitDreamAux, which clears every trigger
 * record's latch and loads the resident entity's ModelData (ETC\SYMSPY.MOM);
 * its finalize calls ReleaseDreamAuxModels. ObjM's scene setup calls
 * SetDreamAuxWorld with the stage, the StageMap, the player DreamSys, the
 * VabStreamObj and the FrameClock every Entity here is built and attached
 * with, and it builds the resident Entity; ObjM's teardown calls
 * ReleaseDreamAuxEntities. SetTeleportsEnabled turns DreamSys's instant
 * teleporters on for stages 3 and 11 and off elsewhere; EnableTeleportsForKind
 * turns them on when a trigger spawns mood row 11, 56, 78 or 93.
 *
 * Triggers. When the StageMap has loaded a chunk's data block, ObjM passes it
 * to TryDreamAuxTrigger with the chunk's coordinates and the day. The chunk's
 * DreamAuxTriggerEntry (LookupDreamAuxTrigger, over the stage's table) names
 * up to three TriggerRecords; on a day its dayParity allows,
 * FireDreamAuxTriggerEntries builds a TriggerWorld over the data block (its
 * models) and ProcessDreamAuxTriggerRecord spawns, for each record whose
 * condition holds (CheckDreamAuxTriggerCondition: the day, the dream colour
 * or the style variant), one Entity per spawn index, placed in the chunk's
 * cell (SpawnDreamAuxTriggerEntity). The TriggerWorld goes back to ObjM,
 * which keeps it with the chunk. When the parity rules the day out, on a
 * stage other than 0 and an even day, one time in 12 the resident entity is
 * moved next to the player instead (PlaceDreamAuxEntityByPlayer).
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "Entity.h"
#include "DreamAux.h"
#include "SceneNode.h"
#include "ModelData.h"
#include "TriggerWorld.h"
#include "DreamSys.h"
#include "StageMap.h"

extern s8 sSpecialColors[];

const char gMomPathSymSpy[] = "ETC\\SYMSPY.MOM";
const char gMomPathSymDog[] = "ETC\\SYMDOG.MOM";

void InitDreamAux(void) {
    ResourceRequest req;
    u32 i;
    s32 record;

    for (i = 0; i < ARRAY_COUNT(sDreamAuxGroupRecords); i++) {
        for (record = 0; record < sDreamAuxGroupCounts[i]; record++) {
            sDreamAuxGroupRecords[i][record].triggered = 0;
        }
    }

    ResourceRequest__Set(&req, 0, (char *)gMomPathSymSpy, 1);

    for (i = 0; i < ARRAY_COUNT(sDreamAuxSlots); i++) {
        sDreamAuxSlots[i].model = New_ModelData(&req.src);
        req.src.name = (char *)gMomPathSymDog;
    }
}

void ReleaseDreamAuxModels(void) {
    DreamAuxSlot *slot = sDreamAuxSlots;
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sDreamAuxSlots); i++) {
        ModelData *model = slot->model;

        if (model != NULL) {
            slot->model = model->methods->release(model);
        }
        slot++;
    }
}

/* What SetDreamAuxWorld installs: the stage, its StageMap (the parent every
 * Entity here attaches to), the player (each Entity's peer), the sound bank
 * each Entity is built with and the FrameClock each attaches as its
 * companion. */
extern s32 gDreamAuxStage;
extern StageMap *gDreamAuxStageMap;
extern DreamSys *gDreamAuxWorld;
extern struct VabStreamObj *gDreamAuxSound;
extern struct FrameClock *sDreamAuxFrameClock;

void SetTeleportsEnabled(s32 stage);

void SetDreamAuxWorld(s32 stage, StageMap *stageMap, DreamSys *world, struct VabStreamObj *sound,
                      struct FrameClock *frameClock) {
    DreamAuxSlot *slot = sDreamAuxSlots;
    u32 i;

    gDreamAuxStage = stage;
    gDreamAuxStageMap = stageMap;
    gDreamAuxWorld = world;
    gDreamAuxSound = sound;
    sDreamAuxFrameClock = frameClock;

    for (i = 0; i < ARRAY_COUNT(sDreamAuxSlots); i++) {
        s32 desc[4]; /* New_Entity's descriptor: word +0x00C the ModelData */
        desc[3] = (s32)slot->model;
        slot->entity = New_Entity(i + DREAM_AUX_FIRST_MOOD, desc, gDreamAuxSound);
        slot++;
    }
    SetTeleportsEnabled(stage);
}

extern void SetInstantTeleportersEnabled(bool value);

void SetTeleportsEnabled(s32 stage) {
    SetInstantTeleportersEnabled(stage == 11 || stage == 3);
}

/* Mood rows 11, 56, 78 and 93 turn the instant teleporters on. */
void EnableTeleportsForKind(s32 moodIndex) {
    if (moodIndex == 78) {
        goto call;
    }
    if (moodIndex < 79) {
        if (moodIndex == 11) {
            goto call;
        }
        if (moodIndex == 56) {
            goto call;
        }
        return;
    }
    if (moodIndex != 93) {
        return;
    }
call:
    SetInstantTeleportersEnabled(1);
}

void ReleaseDreamAuxEntities(void) {
    u32 i;
    DreamAuxSlot *slot;

    /* MATCHING: assignments, not initializers, order the two spills */
    i = 0;
    slot = gDreamAuxSlots2;

    for (; i < ARRAY_COUNT(gDreamAuxSlots2); i++) {
        Entity *entity = (Entity *)slot->model; /* one word in: the entity */

        if (entity != NULL) {
            slot->model = entity->methods->release(entity);
        }
        slot++;
    }
}

extern s32 rand(void);

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
        if (gDreamAuxStage != 0 && rand() % 12 == 0 && (day & 1) == 0) {
            PlaceDreamAuxEntityByPlayer(sDreamAuxSlots);
        }
    }
    return 0;
}

DreamAuxTriggerEntry *RemapTriggerForDreamColor(DreamAuxTriggerEntry *trigger, s32 index);

DreamAuxTriggerEntry *LookupDreamAuxTrigger(s16 *chunkKey) {
    s32 stage = gDreamAuxStage;
    s32 count = gDreamAuxTriggerCounts[stage];
    DreamAuxTriggerEntry *trigger = gDreamAuxTriggerEntries[stage];
    s32 i;

    for (i = 0; i < count; i++) {
        if (*chunkKey == trigger->key) {
            return RemapTriggerForDreamColor(trigger, i);
        }
        trigger++;
    }
    return NULL;
}

/* On stage 4, a red dream swaps the stage's trigger 16 for trigger 21. */
DreamAuxTriggerEntry *RemapTriggerForDreamColor(DreamAuxTriggerEntry *trigger, s32 index) {
    s32 stage = gDreamAuxStage;

    if (stage == 4 && index == 16) {
        DreamSys *player = gDreamAuxWorld;
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
        TriggerRecord *records = sDreamAuxGroupRecords[gDreamAuxStage];
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

/* Spawns `record`'s Entities if its condition holds, and follows a mood-2
 * record's chain. True only when an Entity could not be made. */
bool ProcessDreamAuxTriggerRecord(s32 day, DreamAuxTriggerEntry *trigger, TriggerRecord *record,
                                  TriggerWorld *world) {
    s8 *spawn;
    s8 *end;
    ModelData *model;
    s32 desc[4]; /* New_Entity's descriptor: word +0x00C the ModelData */

    if (!CheckDreamAuxTriggerCondition(day, record)) {
        goto fail;
    }

    EnableTeleportsForKind(record->moodIndex);

    spawn = record->spawnIndices;
    end = record->spawnIndices + ARRAY_COUNT(record->spawnIndices);
    model = world->methods->getModelData(world, record->modelIndex);
    desc[3] = (s32)model;

    if (model == NULL) {
        goto skip;
    }

    while (spawn < end) {
        if (*spawn == -1) {
            break;
        }
        if (SpawnDreamAuxTriggerEntity(record->moodIndex, desc, trigger, (u8)*spawn)) {
            return true;
        }
        spawn++;
    }

skip:
    if (record->moodIndex == 2) {
        return ProcessDreamAuxTriggerRecord(day, trigger, record + 7, world);
    }

fail:
    return false;
}

/* Tests `record`'s condition against `day` (enum TriggerCondition) and
 * latches `triggered` when it passes. */
bool CheckDreamAuxTriggerCondition(s32 day, TriggerRecord *record) {
    s8 condition = record->condition;
    s32 id;

    if (condition == TRIGGER_COND_ALWAYS) {
        goto success;
    }

    if (condition < 0) {
        if (record->triggered == 0) {
            goto negate;
        }
        return false;
    }
    /* MATCHING: this arm sits before `negate:` and jumps, as retail's does */
    id = condition;
    goto have_idx;

negate:
    id = ~condition + 1; /* MATCHING: nor + addiu; -condition is one negu */

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

success:
    record->triggered = 1;
    return true;
}

/* Whether the player's dream colour is sSpecialColors' entry for trigger
 * condition `condition` (10..17). */
bool IsCurrentDreamColor(s32 condition) {
    DreamSys *player = gDreamAuxWorld;
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

/* One placement: the cell (column, row) inside the chunk, a yaw from
 * gDreamAuxSpawnRotations and an offset inside the cell from
 * sDreamAuxPosTable. */
typedef struct {
    u16 cell;
    s8 rotationIndex;
    s8 offsetIndex;
} DreamAuxSpawnInfo;

extern DreamAuxSpawnInfo gDreamAuxSpawnInfo[];

/* MATCHING: x and y are one struct so the copy is one lwl/lwr pair */
typedef struct {
    s16 x;
    s16 y;
} DreamAuxPosXY;

typedef struct {
    DreamAuxPosXY xy;
    s16 z;
} DreamAuxPos6;

extern DreamAuxPos6 sDreamAuxPosTable[];

/* Ratio16 degree triples: yaw 0, -90, +90 and 180. */
extern Ratio16 gDreamAuxSpawnRotations[][3];

/* Makes an Entity of mood row `moodIndex`, turns it and attaches it at
 * placement `spawnIndex` of `trigger`'s chunk. True when New_Entity failed. */
bool SpawnDreamAuxTriggerEntity(s32 moodIndex, void *desc, DreamAuxTriggerEntry *trigger, s32 spawnIndex) {
    Entity *entity = New_Entity(moodIndex, desc, gDreamAuxSound);

    if (entity != NULL) {
        DreamAuxSpawnInfo *spawn;

        struct { /* StageMap.h's Descriptor10, as computeCellOffsets reads it */
            u16 chunk;
            u16 cell;
            DreamAuxPos6 offset;
        } cellDesc;

        s32 worldPos[4];

        cellDesc.chunk = trigger->key;
        spawn = &gDreamAuxSpawnInfo[spawnIndex];
        cellDesc.cell = spawn->cell;
        cellDesc.offset = sDreamAuxPosTable[spawn->offsetIndex];

        gDreamAuxStageMap->methods->computeCellOffsets(gDreamAuxStageMap, worldPos, &cellDesc);
        entity->methods->updateRotation(entity, 1, gDreamAuxSpawnRotations[spawn->rotationIndex]);
        ((TodActorAttachToParentFn)entity->methods->attachToParent)(
            (TodActor *)entity, (TodActor *)gDreamAuxWorld, sDreamAuxFrameClock,
            (void *)gDreamAuxStageMap, worldPos);
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
        SceneNode__LocalOffsetToWorldPos((SceneNode *)gDreamAuxWorld, worldPos, slot->pos, 0);
        ((TodActorAttachToParentFn)slot->entity->methods->attachToParent)(
            (TodActor *)slot->entity, (TodActor *)gDreamAuxWorld, sDreamAuxFrameClock,
            (void *)gDreamAuxStageMap, worldPos);
        SceneNode__FaceTarget((SceneNode *)slot->entity, (SceneNode *)gDreamAuxWorld, 1, 0, NULL);
    }
}
