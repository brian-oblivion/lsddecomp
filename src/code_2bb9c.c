/*
 * code_2bb9c -- GAME code carved from psyq_2bb9c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2BB9C..0x2BF70 (vram 0x8003B39C..0x8003B770). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: 12 methods of D_8006E558, a
 * Class6D430 (data-source) subclass calling GetActiveDataSourceMethods.
 *
 * Nothing here is matched yet: every function is fresh track-1 ground.
 */
#include "common.h"
#include "Class6D430.h"

/* Unit-local view of the D_8006E558 class: a Class6D430 data source whose
 * buffer holds a TIM image (func_8003B5F0 hands buffer+4, past the TIM id
 * word, to GsGetTimInfo). Its own fields past Class6D430's 0x2C bytes are
 * unread here except +0x048, which func_8003B3FC clears and func_8003B5E4
 * sets to 1. The methods pointer is typed as the base's table: slots past
 * +0x078 (this class's own +0x07C..+0x09C) are not reached through it here. */
typedef struct D_8006E558Obj {
    CLASS6D430_FIELDS(Class6D430Methods);
    /* +0x02C */ u8 pad2C[0x1C];
    /* +0x048 */ s32 unk48;
} D_8006E558Obj;

/* LIBGS.H: void GsGetTimInfo(unsigned long *im, GsIMAGE *tim); the GsIMAGE
 * is only passed through here, so it stays opaque. */
typedef struct GsIMAGE GsIMAGE;
void GsGetTimInfo(u32 *im, GsIMAGE *tim);

extern Class6D430Methods D_8006E558;

INCLUDE_ASM("asm/nonmatchings/code_2bb9c", func_8003B39C);
INCLUDE_ASM("asm/nonmatchings/code_2bb9c", func_8003B3FC);
INCLUDE_ASM("asm/nonmatchings/code_2bb9c", func_8003B470);
INCLUDE_ASM("asm/nonmatchings/code_2bb9c", func_8003B4A8);
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5AC(void) {
}
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5B4(void) {
}
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5BC(void) {
}
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5C4(void) {
}
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5CC(void) {
}
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5D4(void) {
}
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5DC(void) {
}
/* D_8006E558 +0x098. */
void func_8003B5E4(D_8006E558Obj *self) {
    self->unk48 = 1;
}
/* D_8006E558 +0x09C: describe the TIM held in the buffer. */
void func_8003B5F0(D_8006E558Obj *self, GsIMAGE *tim) {
    GsGetTimInfo((u32 *)self->buffer + 1, tim);
}
/* The class's table getter (called by func_8003B39C and func_8003B3FC). */
Class6D430Methods *func_8003B614(void) {
    return &D_8006E558;
}
INCLUDE_ASM("asm/nonmatchings/code_2bb9c", func_8003B624);
