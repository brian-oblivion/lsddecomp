#include "common.h"
#include "code_d294.h"

Class6B5CCObj *New_Class6B5CC(void) {
    Class6B5CCObj *obj;

    obj = func_80017B34(0x44);
    if (obj == NULL) {
        return NULL;
    }
    if (GetClass6B5CCMethods()->ctor(obj) != NULL) {
        return obj;
    }
    func_80017CFC(obj);
    return NULL;
}

void *Class6B5CC__Class6B5CC(Class6B5CCObj *self) {
    void *blockB;

    self->unk14 = func_80017B34(0x50);
    if (self->unk14 == NULL) {
        return NULL;
    }
    blockB = func_80017B34(0x28);
    self->unk14->unk44 = blockB;
    if (blockB == NULL) {
        func_80017CFC(self->unk14);
        return NULL;
    }
    Get_vtable_BasicClass()->ctor(self);
    self->methods = GetClass6B5CCMethods();
    self->unk20 = 0;
    self->unk18 = 0;
    self->unkC = NULL;
    self->unk14->unk48 = 0;
    self->methods->slot40(self);
    return self;
}

void Class6B5CC__Finalize(Class6B5CCObj *self) {
    Class6B5CCSub14 *sub;

    self->methods->slot50(self);
    self->methods->slot54(self);
    self->methods->slot5C(self, 0);
    sub = self->unk14;
    func_80017CFC(sub->unk44);
    func_80017CFC(self->unk14);
    Get_vtable_BasicClass()->dtor(self);
}

void Class6B5CC__AddChild(Class6B5CCObj *self, GenericObj_d294 *other) {
    Get_vtable_BasicClass()->slot10(self, other);
    if ((other->methods->header & 0xF) == 9) {
        Class6B5CC__LinkModel(self, other);
    }
}

void Class6B5CC__RemoveChild(Class6B5CCObj *self, GenericObj_d294 *other) {
    if ((other->methods->header & 0xF) == 9) {
        Class6B5CC__UnlinkModel(self);
    }
    Get_vtable_BasicClass()->slot14(self, other);
}

void Class6B5CC__RemoveAllChildren(Class6B5CCObj *self) {
    Class6B5CC__UnlinkModel(self);
    Get_vtable_BasicClass()->slot18(self);
}

void Class6B5CC__OnNotify(Class6B5CCObj *self, GenericObj_d294 *other, s32 arg2) {
    s32 tag;

    Get_vtable_BasicClass()->slot38(self, other, arg2);
    tag = other->methods->header & 0xF;
    if (tag == 2) {
        self->methods->slot94(self, other, arg2);
    } else if (tag == 5) {
        self->methods->slot98(self, other, arg2);
    } else if (tag == 4) {
        self->methods->slot9C(self, other, arg2);
    }
}

void Class6B5CC__Reset(Class6B5CCObj *self) {
    self->unk24 = 0;
    self->unk10 = 0;
    GsInitCoordinate2(0, self->unk14);
    self->methods->updateRotation(self, 1, D_8006B684);
    self->methods->updateScale(self, 1, D_8006B690);
    self->unk14->unk0 = 1;
}

void Class6B5CC__UpdateRotation(Class6B5CCObj *self, s32 flag, void *data) {
    s32 vals[3];
    Class6B5CCSub44 *dst;
    s16 *field;

    vals[0] = RatioToFixed12(data);
    vals[1] = RatioToFixed12((u8 *)data + 4);
    vals[2] = RatioToFixed12((u8 *)data + 8);
    vals[0] /= 360;
    vals[1] /= 360;
    vals[2] /= 360;
    dst = self->unk14->unk44;
    field = &dst->vec.x;
    if (flag) {
        dst->vec.x = vals[0];
        dst->vec.y = vals[1];
        dst->vec.z = vals[2];
    } else {
        s32 i;
        s16 *cur;

        for (i = 0; i < 3; i++) {
            cur = field;
            field++;
            *cur = (*cur + vals[i]) % 4096;
        }
    }
    self->unk14->unk0 = 0;
}

void Class6B5CC__UpdateScale(Class6B5CCObj *self, s32 flag, void *data) {
    s32 r0, r1, r2;
    Class6B5CCSub44 *dst;

    r0 = RatioToFixed12(data);
    r1 = RatioToFixed12((u8 *)data + 4);
    r2 = RatioToFixed12((u8 *)data + 8);
    dst = self->unk14->unk44;
    if (flag) {
        dst->unk0 = (s16)r0;
        dst->unk4 = (s16)r1;
        dst->unk8 = (s16)r2;
    } else {
        dst->unk0 += (s16)r0;
        dst->unk4 += (s16)r1;
        dst->unk8 += (s16)r2;
    }
    self->unk14->unk0 = 0;
}

Class6B5CCObj *Class6B5CC__AttachToParent(Class6B5CCObj *self, UnkOwner_d294 *obj, Vec3_d294 *vec) {
    Class6B5CCSub14 *sub;

    if (self->unkC == NULL) {
        self->unkC = obj;
        sub = self->unk14;
        sub->unk48 = obj->unk14;
        obj->methods->slot10(obj, self);
        sub = self->unk14;
        if (vec != NULL) {
            sub->unk18 = vec->x;
            sub->unk1C = vec->y;
            sub->unk20 = vec->z;
        } else {
            sub->unk18 = 0;
            sub->unk1C = 0;
            sub->unk20 = 0;
        }
        self->unk14->unk0 = 0;
    }
    return self;
}

Class6B5CCObj *func_8001D1A4(Class6B5CCObj *self) {
    UnkOwner_d294 *owner;

    owner = self->unkC;
    if (owner != NULL) {
        owner->methods->slot14(owner, self);
        self->unk14->unk48 = 0;
        self->unkC = NULL;
    }
    return self;
}

void func_8001D204(Class6B5CCObj *self) {
    GenericObj_d294 *entry = NULL;
    s32 cont;

    do {
        self->methods->slot58(self, &entry, &cont);
        if (entry != NULL) {
            entry->methods->slot50(entry);
        }
    } while (cont);
}

void func_8001D280(Class6B5CCObj *self, GenericObj_d294 **entry, GenericObj_d294 **cursor) {
    s32 tag;

    tag = 4;
    do {
        if (*entry == NULL) {
            *cursor = self->unk4;
        }
        GetNextBasicClass(entry, cursor);
        if (*entry != NULL) {
            if ((((*entry)->methods->header) & 0xF) == tag) {
                if ((*entry)->unkC == self) {
                    return;
                }
            }
        }
    } while (*cursor != NULL);
    *entry = NULL;
}

void func_8001D33C(void) {
}

s32 func_8001D344(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 0x1F, 1, a1 == 0) == 0;
}

u32 func_8001D374(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 0x1E, 1, a1 != 0);
}

u32 func_8001D3A0(Class6B5CCObj *self, u32 a1) {
    return GetSetBitField(&self->unk10, 0x1C, 2, a1);
}

u32 func_8001D3CC(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 6, 1, a1 == 0);
}

u32 func_8001D3F8(Class6B5CCObj *self, u32 a1) {
    return GetSetBitField(&self->unk10, 3, 3, a1);
}
