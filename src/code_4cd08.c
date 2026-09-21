#include "common.h"
#include "code_4cd08.h"

const char D_8001186C[] = "ETC\\SYMSPY.MOM";
const char D_8001187C[] = "ETC\\SYMDOG.MOM";

void InitDreamAux(void)
{
    DreamAuxLoadReq req;
    u32 i;
    s32 j;

    for (i = 0; i < 14; i++) {
        for (j = 0; j < D_80089A7C[i]; j++) {
            D_80089A44[i][j].flag = 0;
        }
    }

    SetVec3(&req, 0, D_8001186C, 1);

    for (i = 0; i < 1; i++) {
        D_80088D28[i].obj = func_8004468C(&req);
        req.name = D_8001187C;
    }
}

void TickDreamAuxSlots(void)
{
    DreamAuxSlot *slot = D_80088D28;
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

extern void *New_Entity(void *arg0, void *arg1, void *arg2);
extern s32 D_8008ABF8;
extern s32 D_8008ABFC;
extern s32 D_8008AC00;
extern s32 D_8008AC04;
extern s32 D_8008AC08;

void SetTeleportsEnabled(s32 triggerType);

void SetDreamAuxWorld(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4)
{
    DreamAuxSlot *slot = D_80088D28;
    u32 i;

    D_8008ABF8 = a0;
    D_8008ABFC = a1;
    D_8008AC00 = a2;
    D_8008AC04 = a3;
    D_8008AC08 = a4;

    for (i = 0; i < 1; i++) {
        s32 buf[4];
        buf[3] = (s32)slot->obj;
        slot->entity = New_Entity((void *)(i + 0x62), buf, (void *)D_8008AC04);
        slot++;
    }
    SetTeleportsEnabled(a0);
}

extern void func_8005BF68(bool value);

void SetTeleportsEnabled(s32 triggerType)
{
    func_8005BF68(triggerType == 0xB || triggerType == 3);
}

void EnableTeleportsForKind(s32 triggerType)
{
    if (triggerType == 0x4E) {
        goto call;
    }
    if (triggerType < 0x4F) {
        if (triggerType == 0xB) {
            goto call;
        }
        if (triggerType == 0x38) {
            goto call;
        }
        return;
    }
    if (triggerType != 0x5D) {
        return;
    }
call:
    func_8005BF68(1);
}

void TickDreamAuxSlots2(void)
{
    u32 done;
    DreamAuxSlot *slot;

    done = 0;
    slot = D_80088D2C;

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

s32 TryDreamAuxTrigger(s32 a0, s16 *a1, s32 a2)
{
    s32 record = LookupDreamAuxTrigger(a1);

    if (record != 0) {
        if (CheckTriggerParity(a2, (s8 *)record)) {
            return FireDreamAuxTriggerEntries(a2, (s8 *)record, a0);
        }
        if (D_8008ABF8 != 0 && rand() % 12 == 0 && (a2 & 1) == 0) {
            DespawnDreamAuxEntity(D_80088D28);
        }
    }
    return 0;
}

s32 AdjustDreamAuxTriggerOffset(s32 a0, s32 a1);

s32 LookupDreamAuxTrigger(s16 *a0)
{
    s32 idx = D_8008ABF8;
    s32 count = D_80089AC4[idx];
    DreamAuxTriggerEntry *entry = D_80089A8C[idx];
    s32 i;

    for (i = 0; i < count; i++) {
        if (*a0 == entry->key) {
            return AdjustDreamAuxTriggerOffset((s32)entry, i);
        }
        entry++;
    }
    return 0;
}

s32 AdjustDreamAuxTriggerOffset(s32 a0, s32 a1)
{
    s32 val = D_8008ABF8;

    if (val == 4 && a1 == 0x10) {
        TriggerWorld *w = (TriggerWorld *)D_8008AC00;
        s32 result = ((TriggerWorldFn80)w->vtable[0x80])(w);

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
bool CheckTriggerParity(s32 coordParity, s8 *entry)
{
    bool result = true;

    if (entry[2] != 0) {
        coordParity = coordParity % 2 + 1;
        result = entry[2] != coordParity;
    }
    return result;
}

extern TriggerWorld *func_80044A0C(s32 *ctx);
bool ProcessDreamAuxTriggerRecord(s32 value, void *ctx, TriggerRecord *record, TriggerWorld *world);

s32 FireDreamAuxTriggerEntries(s32 a0, s8 *a1, s32 a2)
{
    s32 ctxArg[4];
    TriggerWorld *world;

    ctxArg[0] = a2;
    world = func_80044A0C(ctxArg);

    if (world != NULL) {
        DreamAuxGroupRecord *base = D_80089A44[D_8008ABF8];
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

bool ProcessDreamAuxTriggerRecord(s32 value, void *ctx, TriggerRecord *record, TriggerWorld *world)
{
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
    callResult = ((TriggerWorldFn)world->vtable[0x22])(world, record->parity);
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
 * short forever.  Writing the inner test as `if (unk0 == 0) goto negate;
 * return false;`, with the `idx = sel; goto have_idx;` block placed
 * textually BEFORE the `negate:` label, reproduces retail's block order
 * exactly.  See docs/match-reports/CheckDreamAuxTriggerCondition.md.
 */

bool CheckDreamAuxTriggerCondition(s32 value, TriggerRecord *record)
{
    s8 sel = record->sel;
    s32 idx;

    if (sel == 1) {
        goto success;
    }

    if (sel < 0) {
        if (record->unk0 == 0) {
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
        if (!MatchesDreamAuxRange(value, idx - 1)) {
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
        if (!func_8005630C()) {
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
    record->unk0 = 1;
    return true;
}

/* Compares the vtable-slot-0x80 result of `*D_8008AC00` (TriggerWorldFn80,
 * include/code_4cd08.h) against a per-idx signed byte from D_80088D16. */
bool CheckDreamAuxWorldState(s32 idx)
{
    TriggerWorld *w = (TriggerWorld *)D_8008AC00;
    s32 val = D_80088D16[idx];
    s32 result = ((TriggerWorldFn80)w->vtable[0x80])(w);

    return val == result;
}

bool MatchesDreamAuxRange(s32 a0, s32 a1)
{
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
 * and `val3` indexes D_80088D3C (stride 6, see DreamAuxPos6 below). */
typedef struct {
    u16 val0;
    s8 val2;
    s8 val3;
} DreamAuxSpawnInfo;

extern DreamAuxSpawnInfo D_80088F48[];

/* A 6-byte position record: a 4-byte (x,y) pair copied as ONE unaligned
 * whole-struct assignment (the idiom CLAUDE.md documents: an all-s8/s16
 * struct at alignment 2 compiles a whole-struct copy to lwl/lwr), plus a
 * separate z half-word. Indexed by DreamAuxSpawnInfo.val3. */
typedef struct {
    s16 x;
    s16 y;
} DreamAuxPosXY;

typedef struct {
    DreamAuxPosXY xy;
    s16 z;
} DreamAuxPos6;

extern DreamAuxPos6 D_80088D3C[];
extern u8 D_80088F18[];

typedef void (*DreamAuxObjFn11)(DreamAuxObj *self, s32 arg1, void *arg2);
typedef void (*DreamAuxObjFn3A)(DreamAuxObj *self, void *arg1, void *arg2);

bool SpawnDreamAuxTriggerEntity(s32 kind, void *out, void *ctx, s32 entry)
{
    DreamAuxObj *entity = (DreamAuxObj *)New_Entity((void *)kind, out, (void *)D_8008AC04);

    if (entity != NULL) {
        DreamAuxSpawnInfo *rec;
        struct {
            u16 ctxVal;
            u16 recordVal0;
            DreamAuxPos6 pos;
        } coords;
        s32 outBuf[4];
        DreamAuxObj *obj;

        coords.ctxVal = *(u16 *)ctx;
        rec = &D_80088F48[entry];
        coords.recordVal0 = rec->val0;
        coords.pos = D_80088D3C[rec->val3];

        obj = (DreamAuxObj *)D_8008ABFC;
        ((DreamAuxObjFn3A)obj->vtable[0x3A])(obj, outBuf, &coords);
        ((DreamAuxObjFn11)entity->vtable[0x11])(entity, 1, D_80088F18 + rec->val2 * 12);
        ((DreamAuxObjFn13)entity->vtable[0x13])(entity, D_8008AC00, D_8008AC08, (void *)D_8008ABFC, outBuf);
        return false;
    }
    return true;
}

/* Local view of Class6B5CC__LocalOffsetToWorldPos/Class6B5CC__FaceTarget (both already matched in
 * code_d294_c.c, a different unit): their own headers type `self`/`target`
 * as this game's class-framework specifics (Class6B5CCObj*, Entity*), which
 * this unit has no reason to pull in for two calls. `Class6B5CC__LocalOffsetToWorldPos`'s own
 * disassembly at every known call site (see DreamSys.c) sets a 4th argument
 * register to 0 even though its 3-parameter C signature never reads it --
 * declared here to reproduce that register content, same as DreamSys.c's
 * own local prototype. */
extern void Class6B5CC__LocalOffsetToWorldPos(void *self, s32 *dst, s32 *src, s32 arg4); /* arity-ok: definition is 3-parameter, but arg4 is byte-load-bearing HERE -- retail emits `move a3,zero` at 0x8005CF7C */
extern void Class6B5CC__FaceTarget(void *self, void *target, s32 arg2, s32 arg3, void *arg4);

void DespawnDreamAuxEntity(DreamAuxSlot *a0)
{
    if (a0->entity != NULL) {
        s32 localPos[3];

        ((DreamAuxObjFn14)a0->entity->vtable[0x14])(a0->entity);
        Class6B5CC__LocalOffsetToWorldPos((void *)D_8008AC00, localPos, a0->pos, 0);
        ((DreamAuxObjFn13)a0->entity->vtable[0x13])(a0->entity, D_8008AC00, D_8008AC08, (void *)D_8008ABFC, localPos);
        Class6B5CC__FaceTarget(a0->entity, (void *)D_8008AC00, 1, 0, 0);
    }
}
