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

s32 func_8004F394(TaskObjF *self) {
    return func_8004F40C(self, func_80038F6C, 1);
}

s32 func_8004F3BC(TaskObjF *self) {
    return func_8004F40C(self, func_8003903C, 1);
}

s32 func_8004F3E4(TaskObjF *self) {
    return func_8004F40C(self, func_800390F4, 0);
}

s32 func_8004F40C(TaskObjF *self, s32 (*callback)(s32), s32 flag) {
    s32 i;
    s32 result;

    if (flag) {
        func_80024CE0();
    }
    for (i = 0; i < 4; i++) {
        result = callback(self->field14[i]);
        if (result == 0) {
            break;
        }
    }
    if (flag) {
        func_80024CF0();
    }
    return result;
}

s32 func_8004F4A4(TaskObjF *self) {
    return func_8004F4C8(self->field14, 4);
}

s32 func_8004F4C8(s32 *arr, s32 count) {
    s32 i;

    for (;;) {
        for (i = 0; i < count; i++) {
            if (func_800390F4(arr[i]) != 0) {
                return D_80086E78[i];
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F55C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F5DC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F638);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F704);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F784);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F810);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F8A4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F9D8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004FB04);
