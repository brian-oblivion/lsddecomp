#include "common.h"
#include "code_2cc8c.h"

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003F2AC);

extern s32 func_8003F764(s32 *arr);
extern s32 func_8003F82C(s32 a0);

void func_8003F674(s32 *src, s32 *dst) {
    s32 maxAbs;
    s32 shift;

    maxAbs = func_8003F764(src);
    shift = func_8003F82C(maxAbs);
    if (shift >= 0x10) {
        shift -= 0xF;
        dst[0] = src[0] >> shift;
        dst[1] = src[1] >> shift;
        dst[2] = src[2] >> shift;
        dst[3] = src[3] >> shift;
        dst[4] = src[4] >> shift;
        dst[5] = src[5] >> shift;
    } else {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
        dst[4] = src[4];
        dst[5] = src[5];
    }
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003F764);

s32 func_8003F82C(s32 a0) {
    s32 count;

    count = 0;
    if (a0 <= 0) {
        return count;
    }
    do {
        a0 >>= 1;
        count++;
    } while (a0 > 0);
    return count;
}

typedef struct HistorySnapshot HistorySnapshot;
struct HistorySnapshot {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
};

typedef struct HistoryNode HistoryNode;
struct HistoryNode {
    s32 unk0;                  /* +0x000, an "owner" tag: 0 (free), a real
                                   owner value, or the D_80090B74 sentinel
                                   (latched/finalized) */
    HistorySnapshot cur;       /* +0x004, "live" values */
    HistorySnapshot backup;    /* +0x024, finalized/cached copy of `cur` */
    u8 pad44[0x048 - 0x044];
    HistoryNode *parent;       /* +0x048 */
};

extern HistoryNode *D_80090260[0x64];
extern HistoryNode *D_8009025C[];  /* splat's own symbol at D_80090260-4;
                                       indexed here starting at 1, never 0 */
extern s32 D_80090B74;
extern void func_80012AF8(HistorySnapshot *out, HistorySnapshot *a1);

void func_8003F848(HistoryNode *self, HistorySnapshot *out) {
    HistoryNode *cur;
    HistoryNode *parent;
    s32 i;
    s32 lastFree;
    HistoryNode *src;

    cur = self;
    i = 0;
    lastFree = 0x64;
    for (;;) {
        D_80090260[i] = cur;
        parent = cur->parent;
        if (parent == NULL) {
            if (cur->unk0 == D_80090B74 || cur->unk0 == 0) {
                cur->backup = cur->cur;
                *out = cur->backup;
                cur->unk0 = D_80090B74;
                break;
            }
            i = lastFree + 1;
            if (lastFree == 0x64) {
                i = 0;
                *out = D_80090260[0]->backup;
            } else {
                src = D_80090260[i];
                *out = src->backup;
            }
            break;
        }
        if (cur->unk0 == D_80090B74) {
            *out = cur->backup;
            break;
        }
        if (cur->unk0 == 0) {
            lastFree = i;
        }
        cur = parent;
        i++;
    }

    if (i > 0) {
        do {
            func_80012AF8(out, &D_8009025C[i]->cur);
            D_8009025C[i]->backup = *out;
            D_8009025C[i]->unk0 = D_80090B74;
            i--;
        } while (i > 0);
    }
}

extern void *D_800902E4;

void func_8003FB0C(void *a0) {
    D_800902E4 = a0;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003FB1C);

extern void *D_8008E794;

void func_8003FBE4(void *a0) {
    D_8008E794 = a0;
}

extern void func_80021678(void *arg0); /* asm/psyq_GsLinkObject4.s, Psy-Q
                                           library, not game code */

void func_8003FBF4(Class6E99CObj *self) {
    func_80021678(self->unk10);
}

extern void func_80021580(s32 stride, s32 mask); /* asm/psyq_GsLinkObject4.s,
                                                     Psy-Q library, not game
                                                     code */

void func_8003FC18(s32 a0, s32 a1, TexPageDesc *desc) {
    desc->width = a0 & 0xFFFF;
    desc->height = a1 & 0xFFFF;
    desc->size = (4 << desc->shift) + desc->stride - 4;
    func_80021580(desc->stride, 1 << desc->shift);
}

extern s32 D_800902E0;
extern void func_80012C20(const char *fmt, s32 arg1); /* asm/psyq_2258.s,
                                                           Psy-Q printf-like;
                                                           declared locally
                                                           with THIS call
                                                           site's own arity
                                                           (fmt + 1 vararg),
                                                           same convention as
                                                           code_8220.h's own
                                                           independent
                                                           extern for it */

extern const char D_80011194[];

void func_8003FC70(s32 mode) {
    if (mode == 1) {
        goto set;
    }
    if (mode < 2) {
        if (mode == 0) {
            goto zero;
        }
        goto err;
    }
    if (mode == 2) {
        goto set;
    }
    if (mode == 3) {
        goto set;
    }
    goto err;
zero:
    D_800902E0 = 0;
    return;
set:
    D_800902E0 = mode;
    return;
err:
    func_80012C20(D_80011194, mode);
}



#if 0
s16 *func_8003FCFC(s16 *src, s16 *dst) {
    s32 t1, t2, t3;

    t1 = src[0];
    dst[0] = t1;
    t2 = src[3];
    t1 = src[6];
    dst[1] = t2;
    t3 = src[1];
    dst[2] = t1;
    t2 = src[4];
    dst[3] = t3;
    t1 = src[7];
    dst[4] = t2;
    t3 = src[2];
    dst[5] = t1;
    t2 = src[5];
    dst[6] = t3;
    t1 = src[8];
    dst[7] = t2;
    dst[8] = t1;
    return dst;
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003FCFC);

extern void func_80024B9C(s32 a0);
extern void func_80024BA8(s32 a0);

void func_8003FD4C(s32 a0, s32 a1) {
    func_80024B9C((-(a0 * 5 * 64)) / a1);
    func_80024BA8(0x1400000);
}

Class6E99CObj *func_8003FDB0(void *a1, s32 a2, s32 a3) {
    Class6E99CObj *self;

    self = func_80017B34(0xA0);
    if (self != NULL) {
        func_800404C0()->ctor(self, a1, a2, a3);
        return self;
    }
    return NULL;
}

void func_8003FE2C(Class6E99CObj *self, void *a1, s32 a2, s32 a3) {
    ClassEAC0Methods *base;
    void *tableEntry;

    base = (ClassEAC0Methods *)func_800408BC();
    if (a2 != 0) {
        tableEntry = &D_8006EA90[a2 * 3];
    } else {
        tableEntry = D_8006EAA8;
    }
    base->ctor((ClassEAC0Obj *)self, a1, tableEntry, a3);
    self->methods = func_800404C0();
    self->methods->slot40(self, a2);
}

void func_8003FED8(Class6E99CObj *self, s32 a1) {
    self->unk70 = a1;
    self->unk6C = 0;
    self->unk74 = 0xA;
    self->unk78 = 0;
    self->unk7C = 0;
    self->methods->slot60(self, 0);
    self->methods->slot64(self, 0);
    self->unk98 = 0;
}

void func_8003FF44(Class6E99CObj *self, void *a1, s32 a2) {
    s32 old;

    if (a2 != 2) {
        return;
    }
    old = self->unk80;
    self->unk80 = old - 1;
    if (old > 0) {
        if (self->unk7C == 9) {
            return;
        }
        if (self->unk78 & 4) {
            self->unk64 += (u8)self->unk74;
        }
        if (self->unk78 & 2) {
            self->unk65 += (u8)self->unk74;
        }
        if (self->unk78 & 1) {
            self->unk66 += (u8)self->unk74;
        }
    } else {
        self->methods->slotE0(self, a1);
    }
}

void func_8004001C(Class6E99CObj *self, s32 a1) {
    self->unk74 = a1;
}

#if 0
void func_80040024(Class6E99CObj *self) {
    s32 idx;

    if (self->unk6C != 0) {
        return;
    }
    idx = self->methods->slotDC(self);
    self->methods->slotB8(self, 1, &D_8006EA90[idx * 3]);
    self->unk6C = 1;
    self->unk74 = -self->unk74;
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_80040024);

#if 0
void func_800400B0(Class6E99CObj *self, s32 a1, s32 a2) {
    s32 idx;

    if (self->unk6C != 0) {
        return;
    }
    idx = self->methods->slotDC(self);
    if (self->unk98 != 0) {
        self->unk80--;
    } else {
        self->methods->slotB8(self, 1, &D_8006EAA8[idx * 3]);
    }
    self->unk6C = 2;
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_800400B0);

s32 func_80040154(Class6E99CObj *self, s32 a1, s32 a2, s32 a3) {
    Class6E99CMethods *methods;
    s32 flag;
    s32 q1, q2;

    methods = self->methods;
    if (a2 < 0) {
        a2 = self->unk70;
    } else {
        self->unk70 = a2;
    }
    flag = 1;
    if (a2 != 0) {
        self->unk78 = a2;
    } else {
        flag = 2;
        self->unk78 = 0xF;
    }
    self->unk78 = a2;
    if (a2 == 0) {
        self->unk78 = 0xF;
    }
    q1 = 0x100 / self->unk74;
    self->unk7C = a3;
    self->unk80 = q1;
    if (self->unk98 != 0) {
        q2 = q1 / self->unk9C;
        self->unk80 = q1 - (s16)q2;
    }
    q2 = self->unk68;
    q2 = q2 / self->unk80;
    self->unk84 = q2;
    methods->slot10(self);
    methods->slot64(self, 1);
    methods->slot68(self, flag);
    methods->slot60(self, 1);
    return a2;
}

void func_800402F0(Class6E99CObj *self, void *a1) {
    Class6E99CMethods *methods;
    s32 mode;

    methods = self->methods;
    if (self->unk6C == 0) {
        return;
    }
    if (self->unk6C == 1) {
        mode = 5;
        if (self->unk98 == 0) {
            methods->slot60(self, 0);
            methods->slot64(self, 0);
        }
    } else {
        mode = 6;
        if (self->unk98 != 0) {
            if (self->unk78 == 0xF) {
                methods->slotB8(self, 1, D_8006EAA8);
            }
            methods->slot64(self, 0);
        }
    }
    methods->slot14(self, a1);
    if (self->unk74 < 0) {
        self->unk74 = -self->unk74;
    }
    self->unk6C = 0;
    methods->slot30(self, mode);
}

void *func_800403F8(Class6E99CObj *self) {
    if (self->unk78 == 0xF) {
        return D_8006EAA8;
    }
    return &D_8006EA90[self->unk78 * 3];
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8004042C);

void func_80040490(Class6E99CObj *self) {
    s32 t0, t1;

    t0 = self->unk90;
    t1 = self->unk94;
    self->unk50 = t0;
    self->unk54 = t1;
    __asm__("" ::: "memory");
    self->unk60 = self->unk88;
    self->unk62 = self->unk8C;
}

void func_800404B4(Class6E99CObj *self, s32 a1, s32 a2) {
    self->unk98 = a1;
    self->unk9C = a2;
}

Class6E99CMethods *func_800404C0(void) {
    return &D_8006E99C;
}

ClassEAC0Obj *func_800404D0(void *a0, void *a1, s32 a2) {
    ClassEAC0Obj *self;

    self = func_80017B34(0x6C);
    if (self != NULL) {
        ((ClassEAC0Methods *)func_800408BC())->ctor(self, a0, a1, a2);
        return self;
    }
    return NULL;
}

void func_8004054C(ClassEAC0Obj *self, SkipShort2 *a1, void *a2, s32 a3) {
    func_8001E57C()->ctor(self);
    self->methods = (ClassEAC0Methods *)func_800408BC();
    self->methods->slot40(self, a1, a2, a3);
}

void func_800405D0(ClassEAC0Obj *self, SkipShort2 *a1, void *a2, s32 a3) {
    ClassEAC0Methods *methods;

    self->unk44 = a3;
    self->unk48 = 1;
    self->unk4C = 0;
    self->unk58 = 0;
    self->unk5C = 0;
    self->unk5E = 0;
    self->unk60 = a1->x;
    self->unk62 = a1->y;
    methods = self->methods;
    if (a2 == NULL) {
        a2 = D_8008A924;
    }
    methods->slotB8(self, 1, a2);
    self->methods->slotCC(self, 0xD);
}
