/* First slice of the 365-function class_3bb8c block -- 20 functions,
 * 0x3BB8C..0x3CD88. The remainder is `class_3bb8c_b` and is still a
 * monolithic asm segment.
 *
 * Carve notes for whoever takes the NEXT slice: this block holds all 13 of
 * the game's PSX BIOS trampolines (`jr $t2` with the vector in $t2 and the
 * call number in $t1) and 38 switch jump tables. None of either landed in
 * THIS slice -- verified, not assumed -- which is why it needs no attached
 * rodata slot and no `hasm` segment. The next slice will hit both, and both
 * have to be dispositioned at carve time rather than discovered by a runner
 * that has already spent its attempt budget. See Gate 2 in
 * docs/PARALLEL-RUNS.md.
 */
#include "common.h"
#include "class_3bb8c.h"

s32 func_8004B38C(Obj866E8 *self, void *arg1, Unk6CObj *arg2, Descriptor10 *arg3) {
    s32 stackBuf[3];
    s32 ret;

    self->unk6C = arg2;
    self->unkBC = *arg3;
    ret = func_8004B44C(arg1, stackBuf, self->unk68, &self->unk54, arg3);
    return self->methods->slotF8(self, ret, stackBuf, &D_80086904);
}

s32 func_8004B418(Obj866E8 *self, void *arg1, void *arg2) {
    s32 outBuf[3];

    return func_8004B44C(arg1, outBuf, self->unk68, &self->unk54, arg2);
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004B44C);

void func_8004B570(Obj866E8 *self) {
    self->unk70 = 1;
}

void func_8004B57C(Obj866E8 *self) {
    self->methods->slotC0(self);
    self->unk70 = 0;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004B5BC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004B700);

s32 func_8004B930(Obj866E8 *self, s32 val, s32 flag) {
    Unk68Struct *u;
    s32 divisor;
    s32 unk4;
    s32 count;
    s32 flags;
    s32 i;

    u = self->unk68;
    divisor = u->divisor;
    unk4 = u->unk4;
    count = u->count;
    if (unk4 == 0) {
        flags = (val < divisor) ? 3 : 0;
        if (val >= divisor * (count - 1)) {
            flags |= 0x60;
        }
        if (val % divisor == 0) {
            flags |= flag ? 0x25 : 4;
        }
        if ((val + 1) % divisor != 0) {
            return ~flags;
        }
        flags |= flag ? 0x10 : 0x52;
        return ~flags;
    } else {
        flags = -1;
        for (i = 0; i < count; i++) {
            flags <<= 1;
        }
        return ~flags;
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004BA40);

#if 0
/* STALL snapshot -- see docs/match-reports/func_8004BB3C.md. 90/105 words
 * with CORRECT total length: every instruction in the loop body and the
 * epilogue matches retail one-for-one, including BOTH `addiu sN,sN,0xc`
 * walker increments. The residue is a whole-function $s3<->$s4 identity
 * swap plus the prologue scheduling that follows from it -- a
 * register-identity stall, which project rule 6 forbids fixing with a
 * register pin. The two-differently-BASED-walker shape below is what
 * recovered the previously-missing second increment; do not go back to a
 * single indexed base.
 *
 * ROUND 19 (bravo): re-verified per the head's mid-round drift-check
 * broadcast. Rebuilt exactly as below: 90/105, ZERO drift, compiled
 * length 0x1A4 (105 words) matching retail's own `.s` header exactly.
 * The claim in this report ("correct total length, no insertions or
 * deletions") is CONFIRMED accurate, unlike two other reports' claims
 * the same broadcast flagged as wrong. Not re-attempted further this
 * round -- the residue matches this round's independently-confirmed
 * "declaration order is inert" finding for this exact class.
 */
void func_8004BB3C(Obj866E8 *self, SetupEntry866E8 *arr1, s32 count) {
    SetupEntry866E8 *ep = arr1;
    s32 i;
    Elem *e;
    SetupSub866E8 *sp = (SetupSub866E8 *)((u8 *)arr1 + 4);

    for (i = 0; i < count; i++) {
        e = self->methods->slot118(self, sp->id);
        self->methods->slot88(self, 6, e, i);
        if (ep->ptr0 != 0) {
            if (e->unk4->unk2C != 0) {
                self->methods->slot108(self, e);
            }
            e->unk4->unk30 = sp->rate;
            e->unk4->methods->slot78(e->unk4, ep->ptr0);
            e->flag = 1;
            self->unk1B0 = 1;
        } else {
            if (e->unk4->unk2C != 0) {
                self->methods->slot108(self, e);
            }
            if (e->unk4->unk2A != 0) {
                e->unk4->methods->slot74(e->unk4);
                e->flag = 0;
            }
        }
        ep++;
        sp++;
    }
    self->unk1B4 = func_8004BCE0(self);
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004BB3C);

s32 func_8004BCE0(Obj866E8 *self) {
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < 7; i++) {
        if (self->arr[i].flag != 0) {
            count++;
        }
    }
    return count;
}

void func_8004BD14(Obj866E8 *self, void *arg1, s32 mode) {
    s32 i;
    Elem *e;
    s32 curMode;

    if (mode != 2) {
        return;
    }
    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk2E != 0) {
            e->unk4->unk2E = 0;
            self->methods->slot88(self, 7, e, i);
        }
        curMode = self->unk1B0;
        if (curMode == 1 && e->flag != 0) {
            if (e->unk4->unk2C != 0) {
                self->methods->slot104(self, e);
                e->unk4->unk2C = 2;
                e->flag = 0;
                if (--self->unk1B4 == 0) {
                    self->unk1B4 = 0;
                    self->unk1B0 = 0;
                    self->unk1B8 = curMode;
                }
            } else if (e->unk4->unk2A == 0) {
                e->flag = 0;
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004BE54);

void func_8004C0AC(Obj866E8 *self, Elem *entry) {
    EntryChildObj **p;
    EntryChildObj **end;

    if (entry->unk4->unk30 >= 0) {
        entry->unk4->methods->slot7C(entry->unk4, entry);
        end = entry->unk10 + 0x19A;
        for (p = entry->unk10; p < end; p++) {
            (*p)->unk10 |= 0x80000000;
            (*p)->unk20 = 0;
            (*p)->unk18 = 0;
        }
    }
}

Descriptor10 *func_8004C158(Obj866E8 *self, Descriptor10Ext *arg1, void **out) {
    void *v1;

    v1 = (u8 *)self->unk6C->unk14 + 0x18;
    if (out != 0) {
        *out = v1;
    }
    if (arg1 != 0) {
        if (self->methods->slot110(self, arg1, v1) != 0) {
            return 0;
        }
    }
    return &self->unkBC;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004C1C0);

void func_8004C368(Obj866E8 *self, u8 *out, s32 val) {
    out[0] = val % self->unk68->divisor;
    out[1] = val / self->unk68->divisor;
}

Unk1BCObj *func_8004C3F0(Obj866E8 *self, u8 *out) {
    func_8004C368(self, out, self->unk1BC->unk4->unk30);
    return self->unk1BC;
}

Elem *func_8004C434(Obj866E8 *self, s32 key) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk32 == key) {
            return e;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004C470);
