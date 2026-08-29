#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005C508);

INCLUDE_ASM("asm/nonmatchings/code_4cd08", func_8005C5E8);

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
