#include "common.h"
#include "code_4cd08.h"

const char D_8001186C[] = "ETC\\SYMSPY.MOM";
const char D_8001187C[] = "ETC\\SYMDOG.MOM";

void func_8005C508(void)
{
    DreamAuxLoadReq req;
    u32 i;
    s32 j;

    for (i = 0; i < 14; i++) {
        for (j = 0; j < D_80089A7C[i]; j++) {
            D_80089A44[i][j].flag = 0;
        }
    }

    func_80026CE8(&req, 0, D_8001186C, 1);

    for (i = 0; i < 1; i++) {
        D_80088D28[i].obj = func_8004468C(&req);
        req.name = D_8001187C;
    }
}

void func_8005C5E8(void)
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

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005C650);

extern void func_8005BF68(bool value);

void SetTeleportsEnabled(s32 triggerType)
{
    func_8005BF68(triggerType == 0xB || triggerType == 3);
}

void func_8005C714(s32 triggerType)
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

void func_8005C76C(void)
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

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005C7D4);

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005C8AC);

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005C930);

/* True when `entry`'s side/parity byte (offset 0x2) disagrees with
 * `coordParity`'s own parity. `entry` is a candidate spawn/link record from
 * one of this unit's stage tables (see func_8005C8AC); its layout beyond this
 * one byte is not yet known here, so it is addressed by byte offset rather
 * than through a named struct. A parity byte of 0 means "no side constraint",
 * hence the early `true`. */
bool func_8005C9A4(s32 coordParity, s8 *entry)
{
    bool result = true;

    if (entry[2] != 0) {
        coordParity = coordParity % 2 + 1;
        result = entry[2] != coordParity;
    }
    return result;
}

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005C9DC);

bool func_8005CAB4(s32 value, void *ctx, TriggerRecord *record, TriggerWorld *world)
{
    s8 *p;
    s8 *end;
    void *callResult;
    s32 scratch[4];

    if (!func_8005CBC8(value, record)) {
        goto fail;
    }

    func_8005C714(record->kind);

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
        if (func_8005CDF8(record->kind, scratch, ctx, (u8)*p)) {
            return true;
        }
        p++;
    }

skip:
    if (record->kind == 2) {
        return func_8005CAB4(value, ctx, record + 1, world);
    }

fail:
    return false;
}

#if 0
/* STALL snapshot -- see docs/match-reports/func_8005CBC8.md.
 *
 * ROUND 24 (runner delta). The `case 6`/`case 7` merge below (dynamic
 * `value % 3 != idx - 7`, matching retail's SHARED jump-table target for
 * indices 6 and 7 -- confirmed in the .s: both `jtbl_8001188C` slots point
 * at `.L8005CCC8`) was a genuine, previously-undiscovered fix: without it
 * this body compiled 8 WORDS TOO LONG (108/100), not "1 word short" as
 * round 23 claimed -- round 23's figure was measured with `nearmiss`'s
 * blocker-clean signal trusted but the actual REBUILD never done. With the
 * merge, the function is a real, clean 99/100: every instruction in the
 * body matches retail except ONE preamble word (the redundant `j` over the
 * switch-index join, exactly as documented below), confirmed via `nm` on
 * the linked ELF (built function is 0x18C bytes vs retail's 0x190) and via
 * `asm-differ` realigned on the function's own instruction stream (no other
 * divergence anywhere in the body). Two permuter searches (~34.2k +
 * further iterations) found no zero; the one improving lead (reordering the
 * `idx = ~sel + 1` computation earlier) regresses to 98/100 under the real
 * oracle -- see the match report for the full derivation.
 */
bool func_8005CBC8(s32 value, TriggerRecord *record)
{
    s8 sel = record->sel;
    s32 idx;

    if (sel == 1) {
        goto success;
    }

    if (sel < 0) {
        if (record->unk0 != 0) {
            return false;
        }
        idx = ~sel + 1;
        goto have_idx;
    }
    idx = sel;

have_idx:

    switch (idx - 2) {
    case 0:
    case 1:
    case 2:
        if (!func_8005CDA8(value, idx - 1)) {
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
            if (!func_8005CD58(idx)) {
                return false;
            }
        }
        break;
    }

success:
    record->unk0 = 1;
    return true;
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005CBC8);

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005CD58);

bool func_8005CDA8(s32 a0, s32 a1)
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

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005CDF8);

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005CF34);
