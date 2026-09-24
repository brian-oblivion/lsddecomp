/* 34 queued, all fresh. Offered to runners in round 2026-08-30-a; no longer
 * banked. No function in this unit touches a %gp_rel global, so none of it is
 * exposed to the gp-relative blocker (docs/research/gp-relative-blocker.md).
 */
#include "common.h"
#include "code_55dd4.h"

void *New_Class65650(void *arg1, void *arg2)
{
    Class65650 *self;
    Class65650Methods *vt;

    self = (Class65650 *)BMemPMgrAlloc(0x98);
    if (self == NULL) {
        return NULL;
    }
    vt = Get_vtable_Class65650();
    if (vt->ctor(self, arg1, arg2) != NULL) {
        return self;
    }
    BMemPMgrFree(self);
    return NULL;
}

Class65650 *Class65650__Class65650(Class65650 *self, void *arg1, void *arg2)
{
    D800878D4Methods *base;

    base = DreamSys__GetBaseMethods();
    if (base->ctor(self) == NULL) {
        return NULL;
    }
    self->methods = Get_vtable_Class65650();
    self->arg2 = arg2;
    self->unk5C = NULL;
    self->unk68 = NULL;
    self->unk70 = NULL;
    self->unk94 = 0;
    if (self->methods->slot_setup5C(self, arg1) != 0) {
        base = DreamSys__GetBaseMethods();
        base->dtor(self);
        return NULL;
    }
    self->methods->slot10(self, self->unk5C);
    self->methods->slot40(self);
    return self;
}

void Class65650__Destructor(Class65650 *self)
{
    self->methods->slot_teardown5C(self);
    DreamSys__GetBaseMethods()->dtor(self);
}

void Class65650__OnNotify(Class65650 *self, TagCheckArg *arg1, s32 arg2)
{
    D800878D4Methods *base;

    base = DreamSys__GetBaseMethods();
    base->slot38(self, arg1, arg2);
    if (arg1->tagged->tag == 0x5F03 && arg2 == 1 && self->unk60 == 0) {
        self->methods->slot04(self);
    }
}

void Class65650__InitDefaults(Class65650 *self)
{
    D800878D4Methods *base;

    base = DreamSys__GetBaseMethods();
    base->slot60(self, 0);
    self->methods->slotF0(self, 1);
    self->methods->slotE4(self, 0x12C);
    self->methods->slot114(self);
    self->methods->slot10C(self, 0x41);
    self->methods->slot130(self);
    self->methods->slot128(self, 0);
    if (self->unk68 != NULL) {
        Class6B5CC__LinkModel(self, self->unk68->unk20);
    }
}

void func_80065918(Class65650 *self, Class65650 *other, void *arg2, void *arg3, void *arg4)
{
    D800878D4Methods *base;

    if (self->unk0C == 0) {
        base = DreamSys__GetBaseMethods();
        base->slot4C(self, arg3, arg4);
        if (arg2 != NULL && self->unk50 == NULL) {
            self->methods->slot10(self, arg2);
        }
        self->methods->slot13C(self, other);
    }
}

void Class65650__DetachFromParent(Class65650 *self)
{
    if (self->unk0C != 0) {
        self->methods->slot140(self);
        if (self->unk50 != NULL) {
            self->methods->slot14(self, self->unk50);
        }
        DreamSys__GetBaseMethods()->slot50(self);
    }
}

void Class65650__SetDisplay(Class65650 *self, void *arg)
{
    Unk70ElemObj **p;
    s32 i;

    p = self->unk70;
    for (i = 0; i < self->unk6C; p++) {
        i++;
        (*p)->methods->slot60(*p, arg);
    }
}

void Class65650__SetLightMode(Class65650 *self, void *arg)
{
    Unk70ElemObj **p;
    s32 i;

    p = self->unk70;
    for (i = 0; i < self->unk6C; i++, p++) {
        (*p)->methods->slot70(*p, arg);
    }
    DreamSys__GetBaseMethods()->slot70(self, arg);
}

void Class65650__OnClass6EF50Notify(Class65650 *self, void *arg1, s32 val)
{
    if (val == 2) {
        self->methods->slot108(self);
    }
    if (val == 4) {
        self->methods->slot04(self);
    }
}

void func_80065BF4(Class65650 *self, s32 value)
{
    self->unk64 = value;
}

s32 func_80065BFC(Class65650 *self, void *arg1)
{
    if (self->unk5C != NULL) {
        return 0;
    }
    return func_80065C5C(self, arg1);
}

void func_80065C2C(Class65650 *self)
{
    if (self->unk5C != NULL) {
        func_80065CEC(self);
    }
}

s32 func_80065C5C(Class65650 *self, UnkArg1Obj *other)
{
    if (other->unk0C != NULL) {
        self->unk5C = other->unk0C;
        self->unk60 = 0;
    } else {
        self->unk5C = func_8004468C(other);
        self->unk60 = 1;
    }
    if (self->unk5C == NULL) {
        goto fail;
    }
    return self->methods->slot100(self);
fail:
    func_80065CEC(self);
    return 1;
}

void func_80065CEC(Class65650 *self)
{
    Unk5CObj *result;

    self->methods->slot_teardown70(self);
    if (self->unk60 != 0) {
        result = self->unk5C->methods->slot4(self->unk5C);
    } else {
        result = NULL;
    }
    self->unk5C = result;
}

s32 func_80065D64(Class65650 *self, s32 value)
{
    u8 *arr;
    s32 count;
    s32 i;
    u8 target;
    u8 unused[8];

    if (self->unk74 == NULL) {
        return -1;
    }
    arr = self->unk74;
    __asm__("");
    count = self->unk6C;
    if (count <= 0) {
        return -1;
    }
    i = 0;
    target = (u8)value;
    do {
        if (*arr == target) {
            return i;
        }
        i++;
        arr++;
    } while (i < count);
    return -1;
}

s32 func_80065DBC(Class65650 *self)
{
    if (self->unk70 != NULL) {
        return 0;
    }
    return func_80065E1C(self);
}

void func_80065DEC(Class65650 *self)
{
    if (self->unk70 != NULL) {
        func_80065F2C(self);
    }
}

s32 func_80065E1C(Class65650 *self)
{
    s32 buf[4];
    s32 count;
    s32 i;
    Unk70ElemObj **p;

    count = self->unk5C->methods->slot80(self->unk5C, NULL, buf) & 0xFF;
    self->unk70 = BMemPMgrAlloc(count * 4);
    if (self->unk70 == NULL) {
        goto alloc_fail;
    }
    self->unk74 = BMemPMgrAlloc(count);
    if (self->unk74 == NULL) {
        goto alloc_fail;
    }
    self->unk5C->methods->slot80(self->unk5C, self->unk74, buf);

    p = self->unk70;
    i = 0;
    self->unk6C = 0;
    if (count != 0) {
        do {
            if ((*p++ = New_BaseObjO()) == NULL) {
                goto fail;
            }
            self->unk6C++;
            i++;
        } while (i < count);
    }
    self->unk68 = self->unk70[buf[0]];
    return 0;

alloc_fail:
    self->unk74 = NULL;
fail:
    func_80065F2C(self);
    return 1;
}

void func_80065F2C(Class65650 *self)
{
    Unk70ElemObj **p;

    if (self->unk70 != NULL && self->unk74 != NULL) {
        p = self->unk70;
        while (self->unk6C-- > 0) {
            (*p)->methods->slot4(*p);
            p++;
        }
        self->unk68 = 0;
    }
    self->unk74 = BMemPMgrFree(self->unk74);
    self->unk70 = BMemPMgrFree(self->unk70);
}

void func_80065FD8(Class65650 *self)
{
    self->unk24 = self->unk24 + 1;
    if (self->unk8C != 0) {
        ((void (*)(void))self->unk78)();
    }
    if (self->unk90 != 0 && self->unk80 >= 2) {
        self->unk88 = self->methods->slot134(self, self->unk88, 0);
        self->unk84 = self->unk84 + 1;
        if (self->unk84 >= self->unk80) {
            self->unk84 = 0;
            self->unk88 = (u8 *)(*(GroupObj **)(self->unk5C->unk30->arr + 8 + self->unk7C * 4))->entry + 8;
        }
    }
    *self->unk14 = 0;
}

void func_800660BC(Class65650 *self, s32 value)
{
    switch ((u8)value) {
    case 0x41:
        self->unk78 = self->methods->slot118;
        break;
    case 0x42:
        self->unk78 = self->methods->slot11C;
        break;
    case 0x43:
        self->unk78 = self->methods->slot120;
        break;
    }
}

s32 func_8006613C(Class65650 *self)
{
    return self->unk8C = 1;
}

void func_80066148(Class65650 *self)
{
    self->unk8C = 0;
}

void func_80066150(Class65650 *self)
{
    self->methods->slotC4(self, -0x1E, 0);
    if (self->unk64 == 1 && self->unk68 != NULL) {
        self->unk68->methods->slot88(self->unk68, 6);
    }
}

void func_800661C4(void) {
}

void func_800661CC(void) {
}

void func_800661D4(Class65650 *self, void *arg1)
{
    UnkArg2Obj *obj;

    obj = self->arg2;
    if (obj != NULL) {
        obj->methods->slot80(obj, arg1, 0x6E, 0x6E);
    }
}

void func_80066214(Class65650 *self, s32 index)
{
    self->unk7C = index;
    self->unk80 = (*(GroupObj **)(self->unk5C->unk30->arr + 8 + index * 4))->entry->unk4;
    self->unk88 = (u8 *)(*(GroupObj **)(self->unk5C->unk30->arr + 8 + self->unk7C * 4))->entry + 8;
    self->unk84 = 0;
    self->methods->slot134(self, self->unk88, 0);
}

s32 func_800662A8(Class65650 *self)
{
    return self->unk90 = 1;
}

void func_800662B4(Class65650 *self)
{
    self->unk90 = 0;
}

void *func_800662BC(Class65650 *self, void *hdr, void *extra)
{
    s32 count;
    u32 i;

    count = *(u16 *)((u8 *)hdr + 2);
    hdr = (u8 *)hdr + 8;
    for (i = 0; i < count;) {
        i++;
        hdr = self->methods->slot138(self, hdr, extra);
    }
    return hdr;
}

void *func_80066340(Class65650 *self, void *acc, void *extra)
{
    u8 outbuf[4];
    void *s0;
    s32 idx;
    Unk70ElemObj *elem;
    Elem14Obj *e14;
    TimeTargetObj *t0;
    s32 i;

    s0 = self->unk5C->methods->slot84(self->unk5C, acc, &outbuf[0], &outbuf[1], &outbuf[2], &outbuf[3]);
    idx = func_80065D64(self, outbuf[0]);
    if (idx < 0) {
        goto end;
    }
    elem = self->unk70[idx];
    e14 = elem->unk14;
    e14->unk00 = 0;
    t0 = e14->unk44;

    switch (outbuf[1]) {
    case 0:
        elem->unk10 = (elem->unk10 & ((s32 *)s0)[0]) | ((s32 *)s0)[1];
        break;
    case 1: {
        if (outbuf[2] & 1) {
            if (outbuf[2] & 2) {
                s16 *p16 = t0->arr10;

                for (i = 0; i < 3; i++, p16++) {
                    s16 tmp;

                    tmp = *p16 + ((s32 *)s0)[i] / 360;
                    *p16 = tmp;
                    *p16 = tmp % 4096;
                }
                s0 = (u8 *)s0 + 0xC;
            }
            if (outbuf[2] & 4) {
                s32 *p32 = t0->arr00;

                for (i = 0; i < 3; i++, p32++) {
                    *p32 = (((s16 *)s0)[i] * *p32) / 4096;
                }
                s0 = (u8 *)s0 + 8;
            }
            if (!(outbuf[2] & 8)) {
                goto end;
            }
            {
                s32 *p32 = t0->arr18;

                for (i = 0; i < 3; i++, p32++) {
                    *p32 += ((s32 *)s0)[i];
                }
            }
        } else {
            if (outbuf[2] & 2) {
                s16 *p16 = t0->arr10;

                for (i = 0; i < 3; i++, p16++) {
                    *p16 = ((s32 *)s0)[i] / 360;
                }
                s0 = (u8 *)s0 + 0xC;
            }
            if (outbuf[2] & 4) {
                s32 *p32 = t0->arr00;

                for (i = 0; i < 3; i++, p32++) {
                    *p32 = ((s16 *)s0)[i];
                }
                s0 = (u8 *)s0 + 8;
            }
            if (!(outbuf[2] & 8)) {
                goto end;
            }
            {
                s32 *p32 = t0->arr18;

                for (i = 0; i < 3; i++, p32++) {
                    *p32 = ((s32 *)s0)[i];
                }
            }
        }
        {
            Elem14Obj *e14b;
            s32 v1, v2, v3;

            e14b = elem->unk14;
            v1 = t0->arr18[0];
            v2 = t0->arr18[1];
            v3 = t0->arr18[2];
            e14b->unk18 = v1;
            e14b->unk1C = v2;
            e14b->unk20 = v3;
            __asm__("");
        }
        break;
    }
    case 2: {
        u16 count;

        count = *(u16 *)s0;
        if (count != 0 && elem->unk20 == 0) {
            s32 v;

            v = self->unk5C->unk2C->methods->slot80(self->unk5C->unk2C, count - 1);
            Class6B5CC__LinkModel(elem, v);
        }
        break;
    }
    case 3: {
        s32 v1;

        v1 = *(s32 *)s0;
        if (v1 == 0 || v1 == 0xFFFF) {
            elem->methods->slot4C(elem, self, 0);
        } else {
            s32 idx2;

            idx2 = func_80065D64(self, *(u8 *)s0);
            elem->methods->slot4C(elem, self->unk70[idx2], 0);
        }
        break;
    }
    }

end:
    return (u8 *)acc + outbuf[3] * 4;
}

void func_80066748(Class65650 *self, Class65650 *other)
{
    if (other != NULL) {
        other->methods->slot10(other, self);
        self->methods->slot10(self, other);
        self->unk94 = other;
    }
}

void func_800667B0(Class65650 *self)
{
    Class65650 *other;

    other = self->unk94;
    if (other != NULL) {
        other->methods->slot14(other, self);
        self->methods->slot14(self, self->unk94);
        self->unk94 = NULL;
    }
}

Class65650Methods *Get_vtable_Class65650(void)
{
    return &gClass65650Methods;
}
