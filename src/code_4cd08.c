#include "common.h"
#include "Entity.h"
#include "code_4cd08.h"
#include "SceneNode.h"
#include "ModelData.h"
#include "TriggerWorld.h"
#include "DreamSys.h"
#include "Class866E8.h"

const char gMomPathSymSpy[] = "ETC\\SYMSPY.MOM";
const char gMomPathSymDog[] = "ETC\\SYMDOG.MOM";

void InitDreamAux(void) {
    DreamAuxLoadReq req;
    u32 i;
    s32 j;

    for (i = 0; i < 14; i++) {
        for (j = 0; j < gDreamAuxGroupCounts[i]; j++) {
            gDreamAuxGroupRecords[i][j].flag = 0;
        }
    }

    SetVec3(&req, 0, gMomPathSymSpy, 1);

    for (i = 0; i < 1; i++) {
        gDreamAuxSlots[i].obj = New_ModelData((struct Src6F240 *)&req);
        req.name = gMomPathSymDog;
    }
}

void TickDreamAuxSlots(void) {
    DreamAuxSlot *slot = gDreamAuxSlots;
    u32 done;

    for (done = 0; done < 1; done++) {
        DreamAuxObj *obj = slot->obj;

        if (obj != NULL) {
            DreamAuxTickFn tick = (DreamAuxTickFn)obj->vtable[1];
            slot->obj = tick(obj);
        }
        slot++;
    }
}

extern s32 gDreamAuxStage;
extern Class866E8 *D_8008ABFC; /* the grid manager: SetDreamAuxWorld's a1; Entity__AttachToParent keeps it as the entity's grid */
extern DreamSys *gDreamAuxWorld; /* the player DreamSys: class_3bb8c_l passes its `target` */
extern s32 D_8008AC04;
extern s32 D_8008AC08;

void SetTeleportsEnabled(s32 triggerType);

void SetDreamAuxWorld(s32 a0, s32 a1, DreamSys *world, s32 a3, s32 a4) {
    DreamAuxSlot *slot = gDreamAuxSlots;
    u32 i;

    gDreamAuxStage = a0;
    D_8008ABFC = (Class866E8 *)a1;
    gDreamAuxWorld = world;
    D_8008AC04 = a3;
    D_8008AC08 = a4;

    for (i = 0; i < 1; i++) {
        s32 buf[4];
        buf[3] = (s32)slot->obj;
        slot->entity = New_Entity(i + 0x62, buf, (void *)D_8008AC04);
        slot++;
    }
    SetTeleportsEnabled(a0);
}

extern void SetInstantTeleportersEnabled(bool value);

void SetTeleportsEnabled(s32 triggerType) {
    SetInstantTeleportersEnabled(triggerType == 0xB || triggerType == 3);
}

void EnableTeleportsForKind(s32 kind) {
    if (kind == 0x4E) {
        goto call;
    }
    if (kind < 0x4F) {
        if (kind == 0xB) {
            goto call;
        }
        if (kind == 0x38) {
            goto call;
        }
        return;
    }
    if (kind != 0x5D) {
        return;
    }
call:
    SetInstantTeleportersEnabled(1);
}

void TickDreamAuxSlots2(void) {
    u32 done;
    DreamAuxSlot *slot;

    done = 0;
    slot = gDreamAuxSlots2;

    for (; done < 1; done++) {
        DreamAuxObj *obj = slot->obj;

        if (obj != NULL) {
            DreamAuxTickFn tick = (DreamAuxTickFn)obj->vtable[1];
            slot->obj = tick(obj);
        }
        slot++;
    }
}

extern s32 rand(void);

s32 LookupDreamAuxTrigger(s16 *a0);
bool CheckTriggerParity(s32 coordParity, s8 *entry);
s32 FireDreamAuxTriggerEntries(s32 a0, s8 *a1, s32 a2);
void DespawnDreamAuxEntity(DreamAuxSlot *a0);

s32 TryDreamAuxTrigger(s32 a0, s16 *a1, s32 a2) {
    s32 record = LookupDreamAuxTrigger(a1);

    if (record != 0) {
        if (CheckTriggerParity(a2, (s8 *)record)) {
            return FireDreamAuxTriggerEntries(a2, (s8 *)record, a0);
        }
        if (gDreamAuxStage != 0 && rand() % 12 == 0 && (a2 & 1) == 0) {
            DespawnDreamAuxEntity(gDreamAuxSlots);
        }
    }
    return 0;
}

s32 AdjustDreamAuxTriggerOffset(s32 a0, s32 a1);

s32 LookupDreamAuxTrigger(s16 *a0) {
    s32 idx = gDreamAuxStage;
    s32 count = gDreamAuxTriggerCounts[idx];
    DreamAuxTriggerEntry *entry = gDreamAuxTriggerEntries[idx];
    s32 i;

    for (i = 0; i < count; i++) {
        if (*a0 == entry->key) {
            return AdjustDreamAuxTriggerOffset((s32)entry, i);
        }
        entry++;
    }
    return 0;
}

s32 AdjustDreamAuxTriggerOffset(s32 a0, s32 a1) {
    s32 val = gDreamAuxStage;

    if (val == 4 && a1 == 0x10) {
        DreamSys *w = gDreamAuxWorld;
        s32 result = w->methods->getDreamColor(w);

        if (result == val) {
            a0 += 0x1E;
        }
    }
    return a0;
}

/* True when `entry`'s side/parity byte (offset 0x2) disagrees with
 * `coordParity`'s own parity. `entry` is a candidate spawn/link record from
 * one of this unit's stage tables (see LookupDreamAuxTrigger); its layout beyond this
 * one byte is not yet known here, so it is addressed by byte offset rather
 * than through a named struct. A parity byte of 0 means "no side constraint",
 * hence the early `true`. */
bool CheckTriggerParity(s32 coordParity, s8 *entry) {
    bool result = true;

    if (entry[2] != 0) {
        coordParity = coordParity % 2 + 1;
        result = entry[2] != coordParity;
    }
    return result;
}

bool ProcessDreamAuxTriggerRecord(s32 value, void *ctx, TriggerRecord *record, TriggerWorld *world);

s32 FireDreamAuxTriggerEntries(s32 a0, s8 *a1, s32 a2) {
    s32 ctxArg[4];
    TriggerWorld *world;

    ctxArg[0] = a2;
    world = New_TriggerWorld((struct Src6F240 *)ctxArg);

    if (world != NULL) {
        DreamAuxGroupRecord *base = gDreamAuxGroupRecords[gDreamAuxStage];
        s8 *p = a1 + 3;
        s8 *end = a1 + 6;

        while (p < end) {
            s8 entry = *p;

            if (entry == -1) {
                break;
            }
            ProcessDreamAuxTriggerRecord(a0, a1, (TriggerRecord *)((u8 *)base + entry * 8), world);
            p++;
        }
        return (s32)world;
    }
    return 0;
}

bool ProcessDreamAuxTriggerRecord(s32 value, void *ctx, TriggerRecord *record, TriggerWorld *world) {
    s8 *p;
    s8 *end;
    void *callResult;
    s32 scratch[4];

    if (!CheckDreamAuxTriggerCondition(value, record)) {
        goto fail;
    }

    EnableTeleportsForKind(record->kind);

    p = record->entries;
    end = record->entries + 4;
    callResult = world->methods->getModelData(world, record->parity);
    scratch[3] = (s32)callResult;

    if (callResult == NULL) {
        goto skip;
    }

    while (p < end) {
        if (*p == -1) {
            break;
        }
        if (SpawnDreamAuxTriggerEntity(record->kind, scratch, ctx, (u8)*p)) {
            return true;
        }
        p++;
    }

skip:
    if (record->kind == 2) {
        return ProcessDreamAuxTriggerRecord(value, ctx, record + 1, world);
    }

fail:
    return false;
}

/* CheckDreamAuxTriggerCondition -- MATCHED round 25.  The last word came from BASIC-BLOCK
 * ORDER, not from the expression shapes.  Retail lays the `sel >= 0` arm
 * out BETWEEN the `return false` path and the `~sel + 1` tail, so it needs
 * an explicit `j` over the join; the obvious spelling
 * (`if (sel < 0) { ...; idx = ~sel + 1; goto have_idx; } idx = sel;`) lets
 * the `sel >= 0` arm fall through into the join instead and is one word
 * short forever.  Writing the inner test as `if (triggered == 0) goto negate;
 * return false;`, with the `idx = sel; goto have_idx;` block placed
 * textually BEFORE the `negate:` label, reproduces retail's block order
 * exactly.  See docs/match-reports/CheckDreamAuxTriggerCondition.md.
 */

bool CheckDreamAuxTriggerCondition(s32 value, TriggerRecord *record) {
    s8 sel = record->sel;
    s32 idx;

    if (sel == 1) {
        goto success;
    }

    if (sel < 0) {
        if (record->triggered == 0) {
            goto negate;
        }
        return false;
    }
    idx = sel;
    goto have_idx;

negate:
    idx = ~sel + 1;

have_idx:

    switch (idx - 2) {
        case 0:
        case 1:
        case 2:
            if (!MatchesDreamAuxProgression(value, idx - 1)) {
                return false;
            }
            break;
        case 3:
            if (value % 3 != 0) {
                return false;
            }
            break;
        case 4:
            if (value % 3 == 0) {
                return false;
            }
            break;
        case 5:
            if (!IsStyleVariantEven()) {
                return false;
            }
            break;
        case 6:
        case 7:
            if (value % 3 != idx - 7) {
                return false;
            }
            break;
        case 18:
            if ((value & 1) != 0) {
                return false;
            }
            break;
        case 19:
            if ((value & 1) == 0) {
                return false;
            }
            break;
        default:
            if (idx >= 10) {
                if (!CheckDreamAuxWorldState(idx)) {
                    return false;
                }
            }
            break;
    }

success:
    record->triggered = 1;
    return true;
}

/* Compares gDreamAuxWorld's getDreamColor (DreamSys +0x200) against a per-idx signed byte from D_80088D16. */
bool CheckDreamAuxWorldState(s32 idx) {
    DreamSys *w = gDreamAuxWorld;
    s32 val = D_80088D16[idx];
    s32 result = w->methods->getDreamColor(w);

    return val == result;
}

bool MatchesDreamAuxProgression(s32 a0, s32 a1) {
    s32 target = (a0 - 1) / 30 + 1;
    s32 i;

    for (i = 0; i < 4; i++) {
        if (target == a1) {
            return true;
        }
        a1 += 3;
    }
    return false;
}

/* A 4-byte record indexed by `entry` (this function's own last parameter):
 * a u16 followed by two signed bytes. `val2` indexes D_80088F18 (stride
 * 0xC, element type undiscovered -- only its address is ever taken here)
 * and `posIndex` indexes gDreamAuxPosTable (stride 6, see DreamAuxPos6
 * below; named round 63 -- confirmed by this struct's only reader). */
typedef struct {
    u16 val0;
    s8 val2;
    s8 posIndex;
} DreamAuxSpawnInfo;

extern DreamAuxSpawnInfo gDreamAuxSpawnInfo[];

/* A 6-byte position record: a 4-byte (x,y) pair copied as ONE unaligned
 * whole-struct assignment (the idiom CLAUDE.md documents: an all-s8/s16
 * struct at alignment 2 compiles a whole-struct copy to lwl/lwr), plus a
 * separate z half-word. Indexed by DreamAuxSpawnInfo.posIndex. */
typedef struct {
    s16 x;
    s16 y;
} DreamAuxPosXY;

typedef struct {
    DreamAuxPosXY xy;
    s16 z;
} DreamAuxPos6;

extern DreamAuxPos6 gDreamAuxPosTable[];
extern u8 D_80088F18[];

bool SpawnDreamAuxTriggerEntity(s32 kind, void *out, void *ctx, s32 entry) {
    Entity *entity = New_Entity(kind, out, (void *)D_8008AC04);

    if (entity != NULL) {
        DreamAuxSpawnInfo *rec;

        struct {
            u16 ctxVal;
            u16 recordVal0;
            DreamAuxPos6 pos;
        } coords;

        s32 outBuf[4];

        coords.ctxVal = *(u16 *)ctx;
        rec = &gDreamAuxSpawnInfo[entry];
        coords.recordVal0 = rec->val0;
        coords.pos = gDreamAuxPosTable[rec->posIndex];

        D_8008ABFC->methods->computeCellOffsets(D_8008ABFC, outBuf, &coords);
        entity->methods->updateRotation(entity, 1, D_80088F18 + rec->val2 * 12);
        ((Class65650AttachToParentFn)entity->methods->attachToParent)(
            (Class65650 *)entity, (Class65650 *)gDreamAuxWorld, (void *)D_8008AC08,
            (void *)D_8008ABFC, outBuf);
        return false;
    }
    return true;
}

void DespawnDreamAuxEntity(DreamAuxSlot *a0) {
    if (a0->entity != NULL) {
        s32 localPos[3];

        a0->entity->methods->detachFromParent(a0->entity);
        SceneNode__LocalOffsetToWorldPos((SceneNode *)gDreamAuxWorld, localPos, a0->pos, 0);
        ((Class65650AttachToParentFn)a0->entity->methods->attachToParent)(
            (Class65650 *)a0->entity, (Class65650 *)gDreamAuxWorld, (void *)D_8008AC08,
            (void *)D_8008ABFC, localPos);
        SceneNode__FaceTarget((SceneNode *)a0->entity, (SceneNode *)gDreamAuxWorld, 1, 0, 0);
    }
}
