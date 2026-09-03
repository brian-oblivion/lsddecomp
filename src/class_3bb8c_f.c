#include "common.h"
#include "class_3bb8c.h"

/* Forward declarations: these are defined later in this file (strict
 * ROM-address order), but func_8004F638/func_8004F8A4 (defined earlier)
 * call them. */
s32 func_8004F9D8(TaskObjF *self);
void func_8004F704(TaskObjF *self);
void func_8004F784(TaskObjF *self);
void func_8004F810(TaskObjF *self);
s32 func_8004F40C(TaskObjF *self, s32 (*callback)(s32), s32 flag);
s32 func_8004F4C8(s32 *arr, s32 count);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004ED40);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004EDC0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004EEA0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004EF6C);

char *func_8004F32C(DeviceName866E8 *dest, s32 selector, char *suffix) {
    DeviceName866E8 *src;

    if (selector) {
        src = &D_8008AA9C;
    } else {
        src = &D_8008AAA4;
    }
    *dest = *src;
    strcat((char *)dest, suffix);
    return (char *)dest;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F394);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F3BC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F3E4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F40C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F4A4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F4C8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F55C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F5DC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F638);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F704);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F784);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F810);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F8A4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F9D8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004FB04);
