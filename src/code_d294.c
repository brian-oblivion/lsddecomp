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
    self->unk14->super = 0;
    self->methods->reset(self);
    return self;
}

void Class6B5CC__Finalize(Class6B5CCObj *self) {
    Class6B5CCSub14 *sub;

    self->methods->detachFromParent(self);
    self->methods->detachAttachedChildren(self);
    self->methods->slot5C(self, 0);
    sub = self->unk14;
    func_80017CFC(sub->unk44);
    func_80017CFC(self->unk14);
    Get_vtable_BasicClass()->finalize(self);
}

void Class6B5CC__AddChild(Class6B5CCObj *self, GenericObj_d294 *other) {
    Get_vtable_BasicClass()->addChild(self, other);
    if ((other->methods->header & 0xF) == 9) {
        Class6B5CC__LinkModel(self, other);
    }
}

void Class6B5CC__RemoveChild(Class6B5CCObj *self, GenericObj_d294 *other) {
    if ((other->methods->header & 0xF) == 9) {
        Class6B5CC__UnlinkModel(self);
    }
    Get_vtable_BasicClass()->removeChild(self, other);
}

void Class6B5CC__RemoveAllChildren(Class6B5CCObj *self) {
    Class6B5CC__UnlinkModel(self);
    Get_vtable_BasicClass()->removeAllChildren(self);
}

void Class6B5CC__OnNotify(Class6B5CCObj *self, GenericObj_d294 *other, s32 arg2) {
    s32 tag;

    Get_vtable_BasicClass()->onNotify(self, other, arg2);
    tag = other->methods->header & 0xF;
    if (tag == 2) {
        self->methods->slot94(self, other, arg2);
    } else if (tag == 5) {
        self->methods->slot98(self, other, arg2);
    } else if (tag == 4) {
        self->methods->dispatchLinkCommand(self, other, arg2);
    }
}

void Class6B5CC__Reset(Class6B5CCObj *self) {
    self->tick = 0;
    self->unk10 = 0;
    GsInitCoordinate2(0, self->unk14);
    self->methods->updateRotation(self, 1, ROTATION_ZERO);
    self->methods->updateScale(self, 1, SCALE_ONE);
    self->unk14->flg = 1;
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
    self->unk14->flg = 0;
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
    self->unk14->flg = 0;
}

Class6B5CCObj *Class6B5CC__AttachToParent(Class6B5CCObj *self, UnkOwner_d294 *obj, Vec3_d294 *vec) {
    Class6B5CCSub14 *sub;

    if (self->unkC == NULL) {
        self->unkC = obj;
        sub = self->unk14;
        sub->super = obj->unk14;
        obj->methods->addChild(obj, self);
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
        self->unk14->flg = 0;
    }
    return self;
}

Class6B5CCObj *Class6B5CC__DetachFromParent(Class6B5CCObj *self) {
    UnkOwner_d294 *owner;

    owner = self->unkC;
    if (owner != NULL) {
        owner->methods->removeChild(owner, self);
        self->unk14->super = 0;
        self->unkC = NULL;
    }
    return self;
}

void Class6B5CC__DetachAttachedChildren(Class6B5CCObj *self) {
    GenericObj_d294 *entry = NULL;
    s32 cont;

    do {
        self->methods->getNextAttachedChild(self, &entry, &cont);
        if (entry != NULL) {
            entry->methods->detachFromParent(entry);
        }
    } while (cont);
}

void Class6B5CC__GetNextAttachedChild(Class6B5CCObj *self, GenericObj_d294 **entry, GenericObj_d294 **cursor) {
    s32 tag;

    tag = 4;
    do {
        if (*entry == NULL) {
            *cursor = self->children;
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

void Class6B5CC__func_1d33c(void) {
}

s32 Class6B5CC__SetDisplay(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 0x1F, 1, a1 == 0) == 0;
}

u32 Class6B5CC__SetSemiTrans(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 0x1E, 1, a1 != 0);
}

u32 Class6B5CC__SetSemiTransRate(Class6B5CCObj *self, u32 a1) {
    return GetSetBitField(&self->unk10, 0x1C, 2, a1);
}

u32 Class6B5CC__SetLighting(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 6, 1, a1 == 0);
}

u32 Class6B5CC__SetLightMode(Class6B5CCObj *self, u32 a1) {
    return GetSetBitField(&self->unk10, 3, 3, a1);
}
