#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_80027C80);

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_80027D40);

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_80027D70);

/* D_8006D4E8's own method table, 29 slots per tools/classtable.py (header
 * 0x13 at +0x000, func_800269F0 at +0x004/own-slot, func_80027228 at
 * +0x008/ctor, func_80027274 at +0x00C/dtor, the 13 inherited BasicClass
 * slots at +0x010..+0x038, then own slots at +0x040..+0x074 -- func_80027C80
 * (+0x06C), func_80027D40 (+0x070) and func_80027D70 (+0x074), all three
 * queued later in this unit, are among them). This function is this class's
 * "get my own method table" accessor, the same convention func_800269E0
 * uses for D_8006D3C8 and func_80026C9C uses for D_8006D430 (see
 * include/code_171e0.h) -- just an address-of, not gp_rel since D_8006D4E8
 * lives in .data, not .sdata. */
extern s32 D_8006D4E8[];

s32 *func_80027E68(void)
{
    return D_8006D4E8;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_80027E78);

extern s32 D_8008A864;

s32 func_80027EC8(void)
{
    return D_8008A864;
}

extern s32 D_8008A870;

s32 func_80027ED4(void)
{
    return D_8008A870;
}

extern s32 D_8008A874;

s32 func_80027EE0(void)
{
    return D_8008A874;
}

extern s32 D_8008A878;

s32 func_80027EEC(void)
{
    return D_8008A878;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_80027EF8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_80027F18);

extern s32 D_8008A868;

void func_80027FD8(s32 a0)
{
    D_8008A868 = a0;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_80027FE4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_80027FF0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_80027FFC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_800280D0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_800280E0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_800280EC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_800281B0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_80028218);

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_80028280);

INCLUDE_ASM("asm/nonmatchings/code_179d8_q", func_800282AC);
