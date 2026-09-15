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

extern s32 D_8008A84C;
extern void *func_8002C438(void);
extern void *func_80027E68(void);

void *func_80026CAC(void) {
    if (D_8008A84C == 0x23) {
        return func_8002C438();
    } else {
        return func_80027E68();
    }
}

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

extern s32 D_8008A84C;
extern s32 func_800280D0(void);

void func_80026E0C(void) {
    if (D_8008A84C == 0x13) {
        func_800280D0();
    }
}

extern s32 func_800280E0(void);

void func_80026E38(void) {
    if (D_8008A84C == 0x13) {
        func_800280E0();
    }
}

extern s32 func_80027EC8(void);

s32 func_80026E64(void) {
    if (D_8008A84C == 0x13) {
        return func_80027EC8();
    }
    return 0;
}

extern s32 func_80027ED4(void);

s32 func_80026E98(void) {
    if (D_8008A84C == 0x13) {
        return func_80027ED4();
    }
    return 1;
}

extern s32 func_80027EE0(void);

s32 func_80026ECC(void) {
    if (D_8008A84C == 0x13) {
        return func_80027EE0();
    }
    return 0;
}

extern s32 func_80027EEC(void);

s32 func_80026F00(void) {
    if (D_8008A84C == 0x13) {
        return func_80027EEC();
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026F34);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026FAC);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026FE8);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80027024);

extern void *D_8008A854;

void func_800270AC(void *value)
{
	D_8008A854 = value;
}

void *func_800270B8(void)
{
	return D_8008A854;
}

char *func_800270C4(char *dest, char *arg1, char *arg2, char *arg3) {
    dest[0] = '\0';
    if (arg2 != NULL) {
        strcat(dest, arg2);
    }
    strcat(dest, arg1);
    strcat(dest, arg3);
    return dest;
}

/* ROUND 34: `strcat` (0x80027130, this unit's last function, 42 words) LEFT
 * THIS FILE. It is Sony's -- `libc2/strcat.o`, Psy-Q 3.3, 0xA8 of text
 * covering exactly it -- and the unit's segment now ends at 0x17930 with an
 * `o` entry after it. It had been matched as C since round 8, and the head
 * had noticed at the time that it reads as library code rather than game
 * code ("carries a guard textbook strcat has no reason to"); it was right,
 * and the reclassification is the correction CLAUDE.md asks for, not a
 * regression.
 *
 * The C body and the two load-bearing source shapes it turned on (the
 * post-increment scan, worth 25 words; `return dest` rather than
 * `return NULL` on the NULL-dest path, worth one) are preserved in full in
 * docs/match-reports/strcat.md. Nothing is lost by deleting them here.
 *
 * Callers in this unit (func_800270C4, just above) keep calling `strcat`
 * under that name -- the declaration in include/code_171e0.h still serves,
 * and now resolves to the linked object. */
