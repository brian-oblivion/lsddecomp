#include "common.h"
#include "class_3ac78.h"

void func_8004A478(Class86668 *self, s32 arg1)
{
    Class866E8 *sub = self->unk34;

    if (sub != NULL) {
        sub->methods->slot80(sub, arg1, 0x7F, 0x7F);
    }
}

Class86668Methods *func_8004A4B8(void)
{
    return &D_80086668;
}

Class866E8 *func_8004A4C8(s32 arg1, s32 arg2)
{
    Class866E8 *self;

    self = func_80017B34(0x1E8);
    if (self != NULL) {
        func_8004D244()->ctor(self, arg1, arg2);
        return self;
    }
    return NULL;
}

/*
 * func_8004A534's own helpers -- all still-uncarved elsewhere, typed
 * purely from this call site's own register usage.
 */
typedef struct BaseCtorTable_3ac78 BaseCtorTable_3ac78;
struct BaseCtorTable_3ac78 {
    u8 pad0[0x8];
    void (*ctor)(void *self); /* +0x008, standard "further-base ctor first" slot */
    void (*dtor)(void *self); /* +0x00C, func_8004A7C0: standard "further-base dtor" slot, mirroring ctor */
};

extern BaseCtorTable_3ac78 *func_800428E4(void);
extern UnkSlotChildObj_3ac78 *func_80048894(void);
extern UnkSlotListObj_3ac78 *new_class_6d940(s32 arg1);
extern GenericObject *func_8004D38C(void);
extern s32 func_80020C5C(void);
extern void func_80017CFC(void *arg1);
extern Vec3_3ac78 D_8008682C;

void func_8004A534(Class866E8 *self, Vec3_3ac78 *arg1, s32 arg2)
{
    s32 i;
    UnkSlotEntry_3ac78 *entry;
    GenericObject *obj;
    Class866E8 **cellp;
    u8 *p;
    u8 *end;
    s32 buf[3];

    func_800428E4()->ctor(self);
    self->methods = func_8004D244();

    if (arg1 != NULL) {
        self->unk54 = *arg1;
    } else {
        self->unk54 = D_8008682C;
    }

    self->unk1B0 = 0;
    self->unk1B4 = 0;
    self->unk1B8 = 0;
    self->unk70 = 0;
    self->unk6C = 0;
    self->unkE8 = 0;
    self->unk1E0 = 0;

    for (i = 0; i < 7; i++) {
        entry = &self->unkEC[i];

        entry->unk4 = func_80048894();
        entry->unk4->unk20 = (entry->unk4->unk10 != 0);
        entry->unk4->unk32 = i;
        entry->unk4->methods->slot88(entry->unk4, arg2);

        entry->unk14 = 0;
        entry->unk18 = 0;
        entry->unk2 = i;
        entry->unk0 = 0;

        entry->unk8 = new_class_6d940(0);
        entry->unkC = func_8004D38C();
        entry->unkC->methods->slot4C(entry->unkC, self, &self->unk54);

        entry->unk10 = (Class866E8 **)func_80017B34(0x668);
        if (entry->unk10 == NULL) {
            return;
        }

        buf[0] = 0x400;
        buf[1] = 0;
        buf[2] = 0x400;

        cellp = entry->unk10;
        end = (u8 *)cellp + 0x668;
        p = (u8 *)cellp;
        while (p < end) {
            obj = func_8004D38C();
            *(GenericObject **)p = obj;
            obj->methods->slot4C(obj, entry->unkC, buf);

            buf[0] += 0x800;
            if (buf[0] > 0xA400) {
                buf[0] = 0x400;
                buf[2] += 0x800;
            }

            obj = *(GenericObject **)p;
            obj->methods->slot70(obj, 1);
            obj = *(GenericObject **)p;
            p += 4;
            obj->unk10 |= 0x80000000;
        }
    }

    self->methods->slot10(self, func_80020C5C());
    self->methods->slot40(self);
}

void func_8004A7C0(Class866E8 *self)
{
    s32 i;
    UnkSlotEntry_3ac78 *entry;
    GenericObject *obj;
    Class866E8 **cellp;
    u8 *p;
    u8 *end;

    self->methods->slot14(self, (void *)func_80020C5C());

    for (i = 0; i < 7; i++) {
        entry = &self->unkEC[i];
        self->methods->slot88(self, 6, entry, i);

        if (entry->unk4 != NULL) {
            entry->unk4->methods->unk04(entry->unk4);
        }

        if (entry->unk8 != NULL) {
            if (entry->unk8->unk2C != NULL) {
                entry->unk8->unk2C->methods->unk04(entry->unk8->unk2C);
            }
            entry->unk8 = (UnkSlotListObj_3ac78 *)entry->unk8->methods->unk04(entry->unk8);
        }

        if (entry->unkC != NULL) {
            entry->unkC->methods->unk04(entry->unkC);
        }

        cellp = entry->unk10;
        end = (u8 *)cellp + 0x668;
        p = (u8 *)cellp;
        while (p < end) {
            obj = *(GenericObject **)p;
            if (obj != NULL) {
                obj->methods->unk04(obj);
            }
            p += 4;
        }

        func_80017CFC(entry->unk10);
    }

    func_800428E4()->dtor(self);
}

/* MEASURED, round 9: func_8001E57C TAKES NO ARGUMENTS -- its body is
 * `lui/addiu %hi/%lo(D_8006B5CC); jr $ra` and it reads neither $a0 nor $a1
 * (asm/code_d294.s). The two args below are what THIS call site passes, not
 * the callee's signature; include/class_3bb8c.h passes ONE to the same symbol
 * and is equally byte-exact. Retail's source called one zero-argument getter
 * with different argument counts from different files, which is what C89 does
 * with no prototype in scope. Do not reconcile the two declarations. */
extern void *func_8001E57C(Class866E8 *self, s32 arg1);

void func_8004A984(Class866E8 *self, GenericObject *arg1, s32 arg2)
{
    void (*fn)(Class866E8 *self, GenericObject *arg1, s32 arg2);

    fn = *(void (**)(Class866E8 *, GenericObject *, s32))
        ((u8 *)func_8001E57C(self, (s32)arg1) + 0x38);
    fn(self, arg1, arg2);

    if ((arg1->methods->header & 0xF) == 1) {
        self->methods->slot100(self, arg1, arg2);
    }
}

extern s32 D_8008A980;

void func_8004AA10(Class866E8 *self)
{
    self->unk68 = NULL;
    self->unkE8 = 0;
    self->unk88 = 0;
    self->methods->slotDC(self, D_8008A980);
    self->unk1CC = -1;
    self->unk1D0 = -1;
    self->unk1D4 = -1;
    self->unk1D8 = -1;
}

void func_8004AA6C(Class866E8 *self, s32 arg1, UnkListObj_3ac78 *arg2)
{
    void (*fn)(Class866E8 *self, s32 arg1);

    fn = *(void (**)(Class866E8 *, s32))((u8 *)func_8001E57C(self, arg1) + 0x88);
    fn(self, arg1);

    if (arg1 == 6)
        goto handle6;
    if (arg1 == 7)
        goto merge;
    return;

handle6:
    if (arg2->unk14 != NULL) {
        arg2->unk14 = arg2->unk14->methods->unk04(arg2->unk14);
    }

merge:
    self->unk1BC = arg2;
    self->methods->slot30(self, arg1);
}

void func_8004AB24(Class866E8 *self)
{
    if (self->unk70) {
        self->methods->slotF4(self);
        self->methods->slot13C(self);
    }
}

void func_8004AB88(Class866E8 *self, GenericObject *other, s32 count)
{
    if ((u8)other->methods->header == 0x34) {
        self->methods->slotD0(self, other, count);
    }
}

/* STALL -- see docs/match-reports/func_8004ABD0.md. Best reached: 9/74
 * words in-range, correct size, no address drift. Residue is a
 * scheduling-only "the offset increment keeps landing in the wrong
 * delay slot" issue. Restored to INCLUDE_ASM per project rule. */
#if 0
void func_8004ABD0(Class866E8 *self)
{
    s32 i;
    s32 offset;
    UnkSlotEntry_3ac78 *entry;

    offset = 0xEC;
    for (i = 0; i < 7; i++) {
        GenericObject *check;

        entry = (UnkSlotEntry_3ac78 *)((u8 *)self + offset);
        entry->unk4->methods->slot74(entry->unk4);
        entry->unk0 = 0;
        self->methods->slot108(self, entry);
        check = entry->unk8->unk2C;
        offset += 0x1C;
        if (check != NULL) {
            entry->unk8->unk2C = check->methods->unk04(check);
        }
        self->methods->slot88(self, 6, entry, i);
        entry->unk4->methods->slot84(entry->unk4);
    }

    self->unk1B8 = 0;
    self->unk1B4 = 0;
    self->methods->slot140(self);
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004ABD0);

void func_8004ACF8(Class866E8 *self, s32 count, s32 arg2, s32 arg3)
{
    s32 i;
    UnkChildObj_3ac78 *child;

    for (i = 0; i < count; i++) {
        child = self->methods->slotB8(self, i);
        child->methods->slot44(child, 1, arg3);
        arg3 += 3;
        child->methods->slot48(child, 1, arg2);
        arg2 += 6;
    }
}

void func_8004ADC4(Class866E8 *self, s32 arg1, s32 arg2)
{
    self->unk60 = arg1;
    self->unk64 = arg2;
}

void func_8004ADD0(Class866E8 *self, s32 arg1)
{
    self->unkE8 = arg1;
}

void func_8004ADD8(Class866E8 *self, void *list, s32 count)
{
    s32 *p;
    u8 unused[24];

    switch (count) {
    case 2:
    case 3:
    case 5:
    case 6:
    case 7:
    case 8:
        break;
    default:
        return;
    }

    p = (s32 *)self->unkE8;
    if (p == NULL)
        return;
    if (*p == 0)
        return;

    do {
        if (*p == ((GenericObject *)list)->methods->header) {
            self->methods->slot12C(self, list, count);
        }
        p++;
    } while (*p != 0);
}

extern void func_8004AFE0(Class866E8 *self, UnkArgObj_3ac78 *arg1, s32 arg2);
extern void func_8004B030(Class866E8 *self, UnkArgObj_3ac78 *arg1, s32 arg2);
extern void func_8004B100(Class866E8 *self, UnkListObj_3ac78 *arg1, s32 arg2);

void func_8004AEA4(Class866E8 *self, UnkListObj_3ac78 *arg1, s32 arg2)
{
    s32 gateArg;
    HistoryBlock_3ac78 saved;
    UnkArgObj_3ac78 buf;
    s32 savedUnk88;

    if (arg1->unk0C != 0) {
        gateArg = (s32)((u8 *)arg1->unk14 + 0x38);
    } else {
        gateArg = 0;
    }

    if (self->methods->slot110(self, &buf, gateArg) != 0) {
        return;
    }

    savedUnk88 = self->unk88;
    saved = self->unk8C;

    if (self->unk68->unk4 == 0) {
        func_8004AFE0(self, &buf, 3);
    } else {
        func_8004B030(self, &buf, 3);
    }

    func_8004B100(self, arg1, arg2);

    self->unk88 = savedUnk88;
    self->unk8C = saved;
}

extern void func_8004C93C(Class866E8 *self);

void func_8004AFE0(Class866E8 *self, UnkArgObj_3ac78 *arg1, s32 arg2)
{
    s16 t;

    self->unk7C = arg1->unk2 - 1;
    t = arg1->unk3 - 1;
    self->unk80 = arg2;
    self->unk84 = arg2;
    self->unk7E = t;
    func_8004C93C(self);
}

/* STALL -- see docs/match-reports/func_8004B030.md. Best reached: 22/52
 * words in-range, correct size, no address drift (round 47, echo --
 * up from 19/52, via a permuter-found lead translated and oracle-
 * verified). Restored to INCLUDE_ASM per project rule. Field names below
 * use the CURRENT HistoryEntry_3ac78 layout (round 19, echo -- was
 * unk90/92/94/96 in the report's own preserved body, before
 * func_8004B100's field-shape correction). */
#if 0
void func_8004B030(Class866E8 *self, UnkArgObj_3ac78 *arg1, s32 count)
{
    s8 b2;
    s8 b3;
    s32 raw3;
    s16 col;
    s16 row;
    s16 width;
    s16 height;

    b2 = arg1->unk2;
    b3 = arg1->unk3;
    raw3 = b3;
    width = (height = count);

    if (b2 != 0) {
        col = b2 - 1;
    } else {
        width = count - 1;
    }
    if (b2 == 0x13) {
        width -= 1;
    }

    if (raw3 != 0) {
        row = raw3 - 1;
    } else {
        row = b3;
        height = count - 1;
    }
    if (raw3 == 0x13) {
        height -= 1;
    }

    self->unk88 = 1;
    self->unk8C.e[0].elemIdx = self->methods->slot124(self, arg1->unk28);
    self->unk8C.e[0].col = col;
    self->unk8C.e[0].row = row;
    self->unk8C.e[0].width = width;
    self->unk8C.e[0].height = height;
    col = b2;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004B030);

extern void func_8004B2D4(Class866E8 *self, UnkListObj_3ac78 *arg1, s32 arg2);

/* STALL, round 2026-09-02 (runner delta); re-verified round 19 (echo):
 * best reached 95/117, see docs/match-reports/func_8004B100.md for the
 * preserved near-miss body and the residue analysis (a single
 * instruction-scheduling swap at the inner loop's tail -- correct
 * branch/register shape everywhere else). */
#if 0
void func_8004B100(Class866E8 *self, UnkListObj_3ac78 *arg1, s32 arg2)
{
    s32 i;
    s32 row;
    s32 col;
    HistoryEntry_3ac78 *entry;
    UnkSlotEntry_3ac78 *slot;
    Class866E8 **cell;
    Class866E8 *obj;

    entry = self->unk8C.e;
    for (i = 0; i < self->unk88; i++, entry++) {
        slot = &self->unkEC[entry->elemIdx];
        if (slot->unk4->unk2C != 0) {
            cell = (slot->unk10 + entry->col) + entry->row * 20;
            for (row = 0; row < entry->height; row++) {
                for (col = 0; col < entry->width; col++, cell++) {
                    self->unk1C0 = self->unkBC;
                    self->unk1C2 = entry->col + col;
                    self->unk1C3 = entry->row + row;
                    func_8004B2D4(*cell, arg1, arg2);
                    for (obj = (*cell)->unk38; obj != NULL; obj = obj->unk38) {
                        func_8004B2D4(obj, arg1, arg2);
                    }
                }
                cell += 20 - entry->width;
            }
        }
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004B100);

/* Widened this round (func_8004B100) from a single-param signature to
 * accept two more, unused, forwarded params: func_8004B100's own call
 * sites explicitly set up $a1/$a2 before every call here (unlike
 * func_8001E57C's "leftover, already-there" args -- these are real,
 * explicit `move` instructions), so the call itself needs a matching
 * 3-param prototype to compile. Confirmed harmless to THIS function's own
 * already-matched body: neither extra param is read, and GCC does not
 * reserve stack space for unused trailing integer/pointer args on this
 * target, so the definition's own bytes are unaffected (reverified
 * 18/18 after the widening). */
void func_8004B2D4(Class866E8 *self, UnkListObj_3ac78 *arg1, s32 arg2)
{
    if (self != NULL && (self->flags36 & 0x80)) {
        self->methods->slot38(self);
    }
}

void *func_8004B31C(Class866E8 *self)
{
    return &self->unk1C0;
}

void func_8004B324(void) {
}

void func_8004B32C(Class866E8 *self, s32 arg1)
{
    self->unk74 = arg1;
    self->unk7A = (s16)(arg1 >> 11);
    self->unk78 = (s16)(arg1 >> 12);
}

void func_8004B344(Class866E8 *self, UnkPtr68Obj_3ac78 *arg1)
{
    self->methods->slot40(self);
    self->unk68 = arg1;
}
