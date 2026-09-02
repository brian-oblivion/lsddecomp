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

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004A4C8);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004A534);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004A7C0);

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

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004AA10);

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

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004B030);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004B100);

void func_8004B2D4(Class866E8 *self)
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
