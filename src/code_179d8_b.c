/*
 * code_179d8_b -- window [60..79] of the original 274-function code_179d8
 * monolith, 0x194E0..0x1A1BC (vram 0x80028CE0..0x800299BC).
 *
 * Carved round 16 by blocker DENSITY, not by "next": code_179d8 is 44%
 * blocked in aggregate but the blockers CLUSTER, so the aggregate says
 * nothing about any particular window. This one screened 16/20 clean.
 * The four blocked functions are already stubbed as match reports:
 *   func_80028CF8, func_80028D30, func_80029478  -- addiu_at
 *   func_800292F4                                -- nop_mflo_mfhi
 *
 * func_80029478 owns jtbl_800109F8, whose sub-slot of the 0xFD8 rodata
 * region is ATTACHED to this unit in the splat yaml. Leave that alone.
 *
 * This slice was cut at ROM-address boundaries, so it has no reason to
 * align with class boundaries -- expect it to span more than one class,
 * and identify each with tools/classtable.py rather than assuming one.
 *
 * Declarations: keep anything that encodes THIS unit's reading of a class
 * next to the code, in this file. Do not create a shared code_179d8*.h --
 * the sibling slices are staffed independently and a shared header is what
 * makes their merges collide.
 */
#include "common.h"

/* code_179d8_b -- this window's globals are plain scalar state (not a
 * classtable.py hit anywhere near D_8006D5FC..D_8006D8D8), most plausibly a
 * low-level serial/link driver: func_8002A378 and func_8002B304 (still
 * INCLUDE_ASM in the sibling monolith code_179d8_mid) poke raw-looking
 * control words (0x20943, 0x1323, 0x1325) into a block of globals at
 * D_8006D8C0..D_8006D934 that read like hardware/SIO register staging, not
 * object fields. Nothing here is a class method table; extern prototypes
 * below are this unit's own reading of that driver's entry points. */

extern s32 D_8006D5FC;
extern s32 D_8006D600;
extern s32 D_8006D604;
extern s32 D_8006D608;

/* Still INCLUDE_ASM in asm/code_179d8_mid.s -- not this unit's to carve. */
extern s32 func_800299BC(s32 arg0, s32 arg1);
extern s32 func_80029C40(s32 arg0, s32 arg1);
extern s32 func_8002A378(void *arg0);
extern s32 func_8002B304(s32 arg0, s32 arg1);
extern s32 func_80024D70(s32 arg0, s32 arg1); /* asm/psyq_GsLinkObject4.s */
extern s32 func_8002B198(s32 arg0);
extern s32 func_8002AEE0(s32 arg0, s32 arg1);

/* CD-ROM MSF (minute/second/sector-in-frame) timecode, all three fields
 * packed BCD. This unit's own local reading -- see func_800293F8. */
typedef struct {
    u8 minute;
    u8 second;
    u8 sector;
} MSF179D8;

s32 func_80028CE0(s32 arg0)
{
    s32 old = D_8006D608;
    D_8006D608 = arg0;
    return old;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028CF8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028D30);

s32 func_80028D68(s32 arg0, s32 arg1)
{
    return func_800299BC(arg0, arg1);
}

s32 func_80028D88(s32 arg0, s32 arg1)
{
    return func_80029C40(arg0, arg1);
}

s32 func_80028DA8(s32 arg0)
{
    s32 old = D_8006D5FC;
    D_8006D5FC = arg0;
    return old;
}

s32 func_80028DC0(s32 arg0)
{
    s32 old = D_8006D600;
    D_8006D600 = arg0;
    return old;
}

s32 func_80028DD8(s32 arg0)
{
    s32 old = D_8006D604;
    D_8006D604 = arg0;
    return old;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028DF0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028F38);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80029074);

s32 func_800291C8(void *arg0)
{
    func_8002A378(arg0);
    return 1;
}

s32 func_800291EC(s32 arg0, s32 arg1)
{
    return func_8002B304(arg0, arg1) == 0;
}

s32 func_80029210(s32 arg0)
{
    return func_80024D70(3, arg0);
}

s32 func_80029234(s32 arg0)
{
    return func_8002B198(arg0);
}

s32 func_80029254(s32 arg0, s32 arg1)
{
    return func_8002AEE0(arg0, arg1);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80029274);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_800292F4);

s32 func_800293F8(MSF179D8 *arg0)
{
    u8 minute;
    u8 second;
    s32 decodedMinute;
    u8 sector;
    s32 total;

    minute = arg0->minute;
    second = arg0->second;
    decodedMinute = (minute >> 4) * 10 + (minute & 0xF);
    total = decodedMinute;
    total = total * 60 + ((second >> 4) * 10 + (second & 0xF));
    sector = arg0->sector;
    return (total * 75 + ((sector >> 4) * 10 + (sector & 0xF))) - 150;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80029478);
