/* Second slice of the 365-function class_3bb8c block, 0x3CD88..0x3DA54 --
 * same class (Obj866E8, D_800866E8) as class_3bb8c.c's first slice, split
 * only for parallel runners, so this unit reuses that unit's header
 * (same convention as Entity.c/Entity_b.c). */
#include "common.h"
#include "class_3bb8c.h"

s32 func_8004C588(Obj866E8 *self, s32 key) {
    s32 result;
    s32 i;
    Elem *e;

    result = 0;
    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk32 == key) {
            result = i;
            break;
        }
    }
    return result;
}

s32 func_8004C5D0(Obj866E8 *self, s32 key) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk30 == key && e->unk4->unk2C != 0) {
            return i;
        }
    }
    return -1;
}

/* Forward declarations: all three are defined later in this file (in ROM
 * order, after func_8004C620), but func_8004C620 calls them before their
 * own definitions appear. Signatures are typed from the registers loaded
 * at each call site, per this unit's established convention for calling a
 * same-unit function whose body is still INCLUDE_ASM. */
extern void func_8004CE24(Obj866E8 *self, s32 arg1);
extern void func_8004CC74(Obj866E8 *self);
extern void func_8004C6A8(Obj866E8 *self, s32 arg1, s32 arg2);

void func_8004C620(Obj866E8 *self) {
    s32 idx;

    if (self->unk1B8 == 0) {
        return;
    }
    idx = self->unk78 * 2;
    func_8004CE24(self, 0);
    if (self->unk68->unk4 == 0) {
        func_8004C6A8(self, idx, self->unk7A);
    } else {
        func_8004CC74(self);
    }
    func_8004CE24(self, 1);
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004C6A8);

/* STALLED at 45/109 words -- see docs/match-reports/func_8004C93C.md for
 * the full round-19 analysis. Two real CFG/scheduling fixes closed most of
 * the gap (a hoisted "flag=0" default matching the project's established
 * "default value in the guarding branch's delay slot" idiom; a
 * deliberately-duplicated `slot0 = &self->slots8C[0];` on both arms of the
 * h6<0 test, held apart with a bare `__asm__("")` scheduling barrier to
 * stop the compiler tail-merging the two identical stores back into one).
 * What remains is a clean register-identity rotation across the whole
 * function (self/h4/h6/slot0/span all permuted, matching set of registers)
 * -- restored here per project convention. */
#if 0
void func_8004C93C(Obj866E8 *self) {
    s32 flag;
    s32 h4;
    s32 width;
    s32 height;
    s32 quadrant;
    s32 h6;
    GridSlot866E8 *slot0;
    s32 span;
    s32 count;
    GridSlot866E8 *slot1;

    flag = 0;
    h4 = self->unk7C;
    width = self->unk80;
    height = self->unk84;
    quadrant = 3;
    if (h4 < 0) {
        h4 += 0x14;
        flag = 1;
        quadrant = 2;
    }
    h6 = self->unk7E;
    if (h6 < 0) {
        if (flag == 0) {
            h6 += 0x14;
            if (h4 < 0xA) {
                h4 += 0xA;
                quadrant = 0;
            } else {
                h4 -= 0xA;
                quadrant = 1;
            }
        } else {
            h4 -= 0xA;
            quadrant = 0;
        }
        slot0 = &self->slots8C[0];
    } else {
        __asm__("");
        slot0 = &self->slots8C[0];
    }
    slot0->elemIdx = self->methods->slot120(self, quadrant);
    slot0->h4 = (h4 >= 0) ? h4 : 0;
    slot0->h6 = h6;
    span = h4 + width;
    if (span >= 0x15) {
        span -= 0x14;
        slot0->h8 = width - span;
        count = func_8004CAF0(self, slot0, 0, quadrant, h4, h6, width, height);
        count += 1;
        slot1 = &self->slots8C[count];
        slot1->elemIdx = self->methods->slot120(self, quadrant + 1);
        slot1->h4 = 0;
        slot1->h6 = self->slots8C[0].h6;
        slot1->h8 = span;
        slot1->hA = self->slots8C[0].hA;
    } else {
        slot0->h8 = width;
        count = func_8004CAF0(self, slot0, 0, quadrant, h4, h6, width, height);
    }
    count += 1;
    self->unk88 = count;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004C93C);

/* STALLED at 55/97 words -- see docs/match-reports/func_8004CAF0.md for the
 * full round-19 analysis. Frame size, callee-saved register SET and CFG
 * shape all now match retail exactly (round 9's "frame off by 8 bytes,
 * reconstruction problem" diagnosis is SUPERSEDED); the residue is a clean
 * 3-register rotation (self/slot/a third reused value among $s0/$s1/$s3)
 * confirmed inert to declaration reordering, consistent with this
 * project's established register-identity-rotation class. Preserved here
 * per project convention rather than only in the report. */
#if 0
s32 func_8004CAF0(Obj866E8 *self, GridSlot866E8 *slot, s32 count, s32 baseIdx, s32 p5, s32 p6, s32 p7, s32 p8) {
    s32 hSpan;
    s32 hSpan2;
    s32 h4;
    s32 nextArg;

    if (p6 + p8 >= 21) {
        hSpan = (p6 + p8) - 20;
        hSpan2 = hSpan;
        slot->hA = p8 - hSpan;
        count = count + 1;
        slot = &self->slots8C[count];

        if (p5 < 10) {
            nextArg = baseIdx + 2;
            slot->elemIdx = self->methods->slot120(self, nextArg);
            __asm__("");
            h4 = p5 + 10;
        } else {
            nextArg = baseIdx + 3;
            slot->elemIdx = self->methods->slot120(self, nextArg);
            __asm__("");
            h4 = p5 - 10;
        }
        slot->h4 = h4;
        slot->hA = hSpan2;

        hSpan2 = slot->h4 + p7;
        slot->h6 = 0;
        if (hSpan2 >= 21) {
            count = count + 1;
            slot->h8 = (p7 + 20) - hSpan2;
            slot = &self->slots8C[count];
            slot->elemIdx = self->methods->slot120(self, nextArg + 1);
            slot->h4 = 0;
            slot->h6 = 0;
            slot->h8 = hSpan2 - 20;
            slot->hA = hSpan;
            return count;
        }
        slot->h8 = p7;
        return count;
    }
    slot->hA = p8;
    return count;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004CAF0);

/* Forward declaration: defined later in this file (in ROM order, after
 * func_8004CC74), and EXCLUDED from this round's targets (documented
 * STALL, see docs/match-reports/func_8004CD38.md) -- calling into it
 * while it is still INCLUDE_ASM is fine, per this unit's established
 * convention. Signature per that report. */
extern s32 func_8004CD38(Bounds866E8_3bb8c_b *bounds, s8 *point);

/* Forward declaration: defined later in this file (in ROM order, after
 * func_8004CD38), but func_8004CC74 calls it before its own definition
 * appears. */
extern s32 func_8004CDA4(Obj866E8 *self, s32 unused, s32 key, s32 arg3);

void func_8004CC74(Obj866E8 *self) {
    s32 junk;
    CC74QueryBuf buf;

    self->methods->slot10C(self, &buf, 0);
    self->unk88 = 0;
    self->unk88 = func_8004CDA4(self, junk, 0, buf.count);
    if (func_8004CD38(self->unk1DC, buf.point) != 0) {
        if (buf.count + 1 < self->unk68->count) {
            self->unk88 = func_8004CDA4(self, junk, self->unk88, buf.count + 1);
        }
    }
    if (buf.count - 1 >= 0) {
        self->unk88 = func_8004CDA4(self, junk, self->unk88, buf.count - 1);
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004CD38);

s32 func_8004CDA4(Obj866E8 *self, s32 unused, s32 key, s32 arg3) {
    Unk54Struct *slot;

    slot = (Unk54Struct *) ((u8 *) self + 0x8C + key * sizeof(Unk54Struct));
    *slot = D_80086990;
    slot->unk0 = self->methods->slot124(self, arg3);
    return key + 1;
}

void func_8004CE24(Obj866E8 *self, s32 setBit) {
    s32 i;
    s32 j;
    s32 k;
    GridSlot866E8 *slot;
    Elem *e;
    EntryChildObj **cell;
    EntryChildObj *next;

    slot = self->slots8C;
    for (i = 0; i < self->unk88; slot++, i++) {
        e = &self->arr[slot->elemIdx];
        if (e->unk4->unk2C == 0) {
            continue;
        }
        cell = e->unk10 + slot->h4 + slot->h6 * 20;
        for (j = 0; j < slot->hA; j++) {
            for (k = 0; k < slot->h8; k++) {
                if (setBit != 0) {
                    (*cell)->unk10 &= 0x7FFFFFFF;
                } else {
                    (*cell)->unk10 |= 0x80000000;
                }
                next = (*cell)->unk38;
                while (next != 0) {
                    if (setBit != 0) {
                        next->unk10 &= 0x7FFFFFFF;
                    } else {
                        next->unk10 |= 0x80000000;
                    }
                    next = next->unk38;
                }
                cell++;
            }
            cell += 20 - slot->h8;
        }
    }
}

void *func_8004CFA8(Obj866E8 *self) {
    return &self->unk1CC;
}

void func_8004CFB0(Obj866E8 *self, Bounds866E8_3bb8c_b *arg1) {
    self->unk1DC = arg1;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004CFB8);

/* Forward declaration: defined later in this file (after func_8004D028 in
 * ROM-address order), but passed to func_8004D140 as a function-pointer
 * argument before its own definition appears. */
void func_8004D0D0(Obj866E8 *self, EntryChildObj *item);

void func_8004D028(Obj866E8 *self) {
    if (self->unk1E0 > 0) {
        func_8004D140(self, func_8004D0D0, 0);
        self->unk1E0 -= 1;
        if (self->unk1E0 == 0) {
            self->unk1E0 = -1;
        }
    }
}

/* Forward declaration: defined later in this file (after func_8004D088 in
 * ROM-address order), but passed to func_8004D140 as a function-pointer
 * argument before its own definition appears. */
void func_8004D108(Obj866E8 *self, EntryChildObj *item);

void func_8004D088(Obj866E8 *self) {
    if (self->unk1E0 != 0) {
        func_8004D140(self, func_8004D108, 0);
        self->unk1E0 = 0;
    }
}

void func_8004D0D0(Obj866E8 *self, EntryChildObj *item) {
    item->methods->slot48(item, 0, self->unk1E4);
}

void func_8004D108(Obj866E8 *self, EntryChildObj *item) {
    item->methods->slot48(item, 1, D_800869CC);
}

void func_8004D140(Obj866E8 *self, void (*arg1)(Obj866E8 *self, EntryChildObj *item), void (*arg2)(Obj866E8 *self, Elem *item)) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (arg2 != 0) {
            arg2(self, e);
        }
        func_8004D1D0(self, arg1, e);
    }
}

void func_8004D1D0(Obj866E8 *self, void (*callback)(Obj866E8 *self, EntryChildObj *item), Elem *item) {
    EntryChildObj **p;
    EntryChildObj **end;

    end = item->unk10 + (0x668 / 4);
    p = item->unk10;
    for (; p < end; p++) {
        callback(self, *p);
    }
}

Obj866E8Methods *func_8004D244(void) {
    return &D_800866E8;
}
