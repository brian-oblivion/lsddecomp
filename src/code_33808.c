/*
 * code_33808 -- GAME code carved from the head of psyq_33808 on 2026-09-25
 * (FINISHING-PLAN revision 18). 0x33808..0x36654 (vram
 * 0x80043008..0x80045E54). It was counted as Psy-Q SDK by segment name;
 * tools/gameinsdk.py measured it as game (a call into game code, a method-
 * table entry beside game methods, or contiguity with those, and no Sony
 * fingerprint). What it holds: the methods of eleven tables (D_8006F0B8,
 * D_8006F13C, D_8006F1C4, D_8006F240, D_8006F2C4, D_8006F384, D_8006F40C,
 * D_8006F498, D_8006F514, D_8006F590, D_8006F614) and nine slots of
 * D_8006D430 (Class6D430, the data-source interface): from +0x07C that table
 * lists `Get...Methods` getters (4-word `return &table` stubs) and nine of
 * them are here, one per class. The methods call Lock/UnlockActiveDataSource
 * and GetClass6B5CCMethods. libpress starts right after, at DecDCTReset
 * (now psyq_36654).
 *
 * Nothing here is matched yet: every function is fresh track-1 ground.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043008);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043068);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800431A8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043200);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800434DC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043538);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004355C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800435D0);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043648);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043830);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043840);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800438B0);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043954);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800439EC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043B18);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043B3C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043B58);
void func_80043B70(void) {
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043B78);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043B88);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043BE8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043C60);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043CB8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043DFC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043E74);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043E84);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043EE4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043F78);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043FB0);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043FE4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004416C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800441A4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800441B4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044220);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044294);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044380);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004441C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004464C);
void func_80044674(void) {
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004467C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004468C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800446FC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800447B4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044808);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044858);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800448F8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004497C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800449B8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800449FC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044A0C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044A7C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044B04);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044B58);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044B88);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044C58);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044C90);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044CC4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044CD4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044D40);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044DC8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044E10);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044E64);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044F20);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044F30);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044F90);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004500C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045060);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800450B4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800451A8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800451B8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045228);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800452AC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800452FC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800453DC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045428);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045438);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800454C4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800455D4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004564C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004575C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800457C0);
/* A class with an s32 at +0x50, set to 1 / -1 by the two setters below;
 * the class is not yet identified (neither setter sits in a method table). */
typedef struct Obj33808_50 {
    u8 pad0[0x50];
    s32 unk50;
} Obj33808_50;

void func_800458AC(Obj33808_50 *self) {
    self->unk50 = 1;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800458B8);
void func_8004593C(Obj33808_50 *self) {
    self->unk50 = -1;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045948);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045A38);
void func_80045AC8(void) {
}
void func_80045AD0(void) {
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045AD8);
void func_80045BC0(void) {
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045BC8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045C94);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045CFC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045DE0);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045E18);
/* D_8006F614 +0x06C: stores its argument at +0x68. */
typedef struct Obj6F614 {
    u8 pad0[0x68];
    s32 unk68;
} Obj6F614;

void func_80045E3C(Obj6F614 *self, s32 value) {
    self->unk68 = value;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045E44);
