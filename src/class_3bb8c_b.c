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

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004C93C);

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
