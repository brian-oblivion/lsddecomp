#include "common.h"
#include "code_171e0.h"

void *func_800269E0(void) {
    return D_8006D3C8;
}

void *func_800269F0(UnkFlagsObj_171e0 *this) {
    this->unk20 = 0;
    this->methods->dtor(this);
    func_80018390()->dtor(this);
    func_80017CFC(this);
    return NULL;
}

void func_80026A50(UnkFlagsObj_171e0 *this) {
    func_80018390()->ctor(this);
    this->methods = (UnkFlagsObjMethods_171e0 *) func_80026C9C();
    this->unk0C = 0;
    this->unk10 = NULL;
    this->unk14 = 0;
    this->unk20 = 0;
    this->unk22 = 0;
    this->unknown_value_0x24 = 0;
    this->unk28 = 0;
    this->unk2A = 0;
}

void *func_80026AB4(UnkFlagsObj_171e0 *this) {
    this->methods->slot48(this);
    return this->methods->slot5C(this);
}

void func_80026B08(UnkFlagsObj_171e0 *this, s32 arg1) {
    s32 savedUnk0C;
    s32 size;
    void *newRes;

    if (this->unk10 != NULL) {
        return;
    }
    savedUnk0C = this->unk0C;
    this->unk0C = 0;
    this->methods->slot44(this, arg1, 1, 0);
    size = this->methods->slot4C(this, 0, 2);
    newRes = func_80017B34(size);
    if (newRes != NULL) {
        this->methods->slot4C(this, 0, 0);
        this->methods->slot54(this, newRes, size);
        this->methods->slot48(this);
        this->unk10 = newRes;
        this->unk14 = size;
        this->unk0C = savedUnk0C;
    } else {
        func_80017CFC(NULL);
        this->methods->slot48(this);
    }
}

void func_80026C20(UnkFlagsObj_171e0 *this) {
    if (this->unk10 == NULL) {
        return;
    }
    if (this->unk14 == 0) {
        return;
    }
    if (this->unk20 != 0) {
        return;
    }
    func_80017CFC(this->unk10);
    this->unk10 = NULL;
}

void func_80026C80(void) {
}

void func_80026C88(UnkFlagsObj_171e0 *this) {
    this->unknown_value_0x24 |= 1;
}

void *func_80026C9C(void) {
    return D_8006D430;
}

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026CAC);

Vec3_171e0 *func_80026CE8(Vec3_171e0 *this, s32 x, s32 y, s32 z) {
    this->x = x;
    this->y = y;
    this->z = z;
    return this;
}

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026CFC);

void func_80026D88(UnkFlagsObj_171e0 *dst, UnkFlagsObj_171e0 *src) {
    dst->unk40 = src->unk40;
    dst->unk44 = src->unk44;
    dst->unk48 = src->unk48;
    dst->unk4C = src->unk4C;
    dst->unk50 = src->unk50;
    dst->unk54 = src->unk54;
    dst->unk58 = src->unk58;
    dst->unk68 = src->unk68;
    dst->unk6C = src->unk6C;
    dst->unk70 = src->unk70;
    dst->unk74 = src->unk74;
}

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026E0C);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026E38);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026E64);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026E98);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026ECC);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026F00);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026F34);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026FAC);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026FE8);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80027024);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_800270AC);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_800270B8);

char *func_800270C4(char *dest, char *arg1, char *arg2, char *arg3) {
    dest[0] = '\0';
    if (arg2 != NULL) {
        strcat(dest, arg2);
    }
    strcat(dest, arg1);
    strcat(dest, arg3);
    return dest;
}

/* Not the textbook libc `strcat`. It carries a guard textbook `strcat` has
 * no reason to: after taking both lengths it bails if the two strings' END
 * pointers coincide. That, plus returning NULL rather than `dest` on a NULL
 * input, reads as Psy-Q-library defensive code rather than game code.
 *
 * Two source shapes here are load-bearing and both look like free choices:
 *
 *  - `while (*dest++) {} dest--;` -- the POST-increment scan, which walks
 *    unconditionally and backs up at the merge. The pre-test spelling
 *    `while (*dest) dest++;` is different code: retail's guard branch
 *    targets the `addiu` fixup, which only the post-increment form emits.
 *    Worth 25 words.
 *  - `return dest;` on the NULL-dest path, NOT `return NULL`. The two are
 *    the same value -- dest IS null there -- but returning `dest` USES it,
 *    which keeps it live and produces retail's `move a0,s1` in the first
 *    call's delay slot. Spelling it `NULL` costs exactly that instruction.
 *    The other two exits genuinely do return NULL and share one tail.
 *
 * See docs/match-reports/strcat.md. */
char *strcat(char *dest, char *src) {
    char *origDest;

    if (dest == NULL) {
        return dest;
    }
    if (src == NULL) {
        goto fail;
    }
    if ((dest + strlen(dest)) == (src + strlen(src))) {
        goto fail;
    }
    origDest = dest;
    while (*dest++) {
    }
    dest--;
    while ((*dest++ = *src++) != 0) {
    }
    return origDest;
fail:
    return NULL;
}
