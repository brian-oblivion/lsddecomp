#include "common.h"
#include "code_d294.h"

void func_8001E58C(Class6B5CCObj *self, Class6B5CCSub44 *dst, s16 *src) {
    u8 buf[0x20];

    self->methods->slot84(self, buf, 0);
    dst->unk0 = src[0];
    dst->unk4 = src[1];
    dst->unk8 = src[2];
    func_8001EE98(dst, dst, 1, buf);
}

void func_8001E600(Class6B5CCObj *self, s32 *dst, s32 *src) {
    u8 buf[0x20];
    s32 *table;

    self->methods->slot84(self, buf, 0);
    func_8001EE98(dst, src, 1, buf);

    table = self->unkC != 0 ? self->unk14->unk38 : 0;
    dst[0] = dst[0] + table[0];

    table = self->unkC != 0 ? self->unk14->unk38 : 0;
    dst[1] = dst[1] + table[1];

    table = self->unkC != 0 ? self->unk14->unk38 : 0;
    dst[2] = dst[2] + table[2];
}

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001E6F8);

void func_8001E770(Class6B5CCObj *self, GenericObj_d294 *other) {
    self->unk20 = other;
    self->unk18 = other->unk10;
    GsLinkObject4((u8 *)((GenericObj_d294 *)self->unk20)->unkC + 0xC, &self->unk10, 0);
}

void func_8001E7B0(Class6B5CCObj *self) {
    self->unk18 = 0;
    self->unk20 = 0;
}

extern s32 func_8001F8B8(void *arg0, void *arg1, void *arg2, s32 arg3, void *arg4, s16 *arg5);
extern void func_8001EA8C(s32 *dest, s16 *b, s16 *a);

/* STALL -- see docs/match-reports/func_8001E7BC.md. First real C attempt
 * this round (echo): 29/180 words in-range, WITH address drift -- not
 * yet a trustworthy score, but the overall block shape (guards, backup
 * copy, accumulation loop, both func_8001F8B8 calls) is present and
 * roughly in the right place; residue includes at least a register
 * identity difference in the backup-copy/loop section and likely
 * buffer-layout details not yet nailed down. Restored to INCLUDE_ASM
 * per project rule. */
#if 0
s32 func_8001E7BC(Class6B5CCObj *self, s32 *arg1, s32 *arg2) {
    s32 *table;
    s16 buf18[4];
    s16 delta[4];
    s16 buf28[4];
    s16 buf30[4];
    Class6B5CCSub14 *node;
    UnkOwner_d294 *cur;

    if (self->unk20 == NULL) {
        return 0;
    }
    if ((s32)self->unk10 < 0 && self->unkC != NULL) {
        node = self->unk14;
        if ((u8 *)node + 0x38 != NULL) {
            node->unk38[0] = node->unk18;
            node->unk38[1] = node->unk1C;
            node->unk38[2] = node->unk20;

            cur = self->unkC;
            if (cur != NULL) {
                do {
                    table = self->unkC != 0 ? self->unk14->unk38 : 0;
                    table[0] = table[0] + cur->unk14->unk18;

                    table = self->unkC != 0 ? self->unk14->unk38 : 0;
                    table[1] = table[1] + cur->unk14->unk1C;

                    table = self->unkC != 0 ? self->unk14->unk38 : 0;
                    table[2] = table[2] + cur->unk14->unk20;

                    cur = cur->next;
                } while (cur != NULL);
            }
        }
    }

    table = self->unkC != 0 ? self->unk14->unk38 : 0;
    delta[0] = (u16)arg2[0] - (u16)table[0];
    delta[1] = (u16)arg2[1] - (u16)table[1];
    delta[2] = (u16)arg2[2] - (u16)table[2];

    self->methods->slotA4(self, 0, buf18, delta, 1);

    delta[0] = buf18[0];
    delta[2] = buf18[2];
    delta[1] = (u16)buf18[1] - 0x400;
    if (!func_8001F8B8(self->unk20, buf30, buf28, 0, buf18, delta)) {
        delta[1] = (u16)buf18[1] + 0x400;
        if (!func_8001F8B8(self->unk20, buf30, buf28, 0, buf18, delta)) {
            return 0;
        }
    }
    func_8001EA8C(arg1, buf18, buf28);
    return 1;
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001E7BC);

void func_8001EA8C(s32 *dest, s16 *b, s16 *a) {
    dest[0] = a[0] - b[0];
    dest[1] = a[1] - b[1];
    dest[2] = a[2] - b[2];
}

void func_8001EACC(Class6B5CCObj *self, Class6B5CCObj *target, s32 arg2, s32 arg3, void *arg4) {
    s32 *pos;
    s32 *table;
    s32 dx;
    s32 dz;
    WholeFrac_d294 out[3];

    pos = &self->unk14->unk18;
    table = target->unkC != 0 ? target->unk14->unk38 : 0;

    if (table[0] != pos[0]) {
        dx = table[0] - pos[0];
        dz = table[2] - pos[2];
        out[1].whole = ratan2(dx, dz);
    } else {
        dz = table[2] - pos[2];
        out[1].whole = ratan2(1, dz);
    }

    if (table[2] != pos[2]) {
        dz = table[2] - pos[2];
        dx = table[1] - pos[1];
        out[0].whole = ratan2(dz, dx);
    } else {
        dx = table[1] - pos[1];
        out[0].whole = ratan2(1, dx);
    }

    out[0].whole = (out[0].whole + 0x400) * 360 / 4096;
    out[1].whole = out[1].whole * 360 / 4096;

    out[2].whole = 0;
    out[2].frac = 1;
    out[1].frac = 1;
    out[0].frac = 1;
    if (arg2 != 0) {
        out[0].whole = 0;
    }
    if (arg3 == 0) {
        out[1].whole = out[1].whole + 0xB4;
    }

    self->methods->slot44(self, 1, out);
    if (arg4 != 0) {
        self->methods->slot44(self, 0, arg4);
    }
}

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001EC84);

s32 func_8001ECFC(BoundsBox_d294 *box, Vec3S16_d294 *point) {
    s32 flags;

    flags = 0;
    if (box->hi.x < point->x) {
        flags = 8;
    } else if (point->x < box->lo.x) {
        flags = 4;
    }
    if (box->hi.y < point->y) {
        flags |= 2;
    } else if (point->y < box->lo.y) {
        flags |= 1;
    }
    if (box->hi.z < point->z) {
        flags |= 0x20;
    } else if (point->z < box->lo.z) {
        flags |= 0x10;
    }
    return flags;
}

u32 func_8001EDAC(u32 *word, s32 shift, s32 width, u32 value) {
    u32 mask;
    s32 i;
    u32 old;

    mask = 1;
    for (i = 0; i < width; i++) {
        mask <<= 1;
    }
    mask -= 1;
    mask <<= shift;
    old = (*word & mask) >> shift;
    *word = *word & ~mask;
    *word = *word | (value << shift);
    return old;
}

/* Element type for func_8001EE04's own per-record copy: 6 bytes, a 32-bit
 * value (as two s16 halves, never accessed as a native s32 -- keeping the
 * struct's OWN members all s16 is what gives it alignment 2, the same
 * "all-s8/s16 struct -> alignment 2 -> whole-struct copy compiles to
 * unaligned lwl/lwr + swl/swr" idiom documented in
 * DECOMPILATION_LEARNINGS for Vec3S16_d294/FlashbackRotation, confirmed
 * here by an isolated toolchain reproducer) plus a trailing s16. */
typedef struct Rec6_d294 {
    s16 w0;
    s16 w1;
    s16 h;
} Rec6_d294;

extern void ApplyMatrixSV(void *out, void *buf, void *src);

void func_8001EE04(void *src, void *dest, s32 count, void *out) {
    u8 *end;

    end = (u8 *)src + count * 6;
    while ((u8 *)src < end) {
        Rec6_d294 buf;

        buf = *(Rec6_d294 *)dest;
        ApplyMatrixSV(out, &buf, src);
        dest = (u8 *)dest + 6;
        src = (u8 *)src + 6;
    }
}

void func_8001EE98(void *a, void *b, s32 count, void *fixed) {
    u8 *end;

    end = (u8 *)a + count * 0xC;
    while ((u8 *)a < end) {
        ApplyMatrixLV(fixed, b, a);
        a = (u8 *)a + 0xC;
        b = (u8 *)b + 0xC;
    }
    if (0) {
        ApplyMatrixLV(fixed, b, a, 0, 0, 0);
    }
}

s32 func_8001EF14(s32 *a, s32 range, s32 *b) {
    s32 i;

    for (i = 0; i < 3; i++, a++, b++) {
        if (*b < *a - range) {
            return 0;
        }
        if (*a + range < *b) {
            return 0;
        }
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001EF60);
