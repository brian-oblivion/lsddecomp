#include "common.h"
#include "code_4cd08.h"

#if 0
/* STALL snapshot -- see docs/match-reports/func_8005C508.md. Best reached:
 * 40/56 words (0x4CD08-0x4CDE8). Everything matches byte-for-byte except one
 * extra retail instruction materializing &D_80088D28[i] as a full absolute
 * address (lui+addiu) before adding the running byte offset, where every
 * source shape tried here folds %lo(D_80088D28) into the store's own
 * displacement instead, making the function 4 bytes short and shifting
 * everything after it. If reused, this needs
 * const char D_8001186C[] = "ETC\\SYMSPY.MOM";
 * const char D_8001187C[] = "ETC\\SYMDOG.MOM";
 * defined ahead of it (see the report for why).
 */
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
#endif

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005C508);

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

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005C76C);

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

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005CAB4);

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005CBC8);

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005CD58);

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005CDA8);

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005CDF8);

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005CF34);
