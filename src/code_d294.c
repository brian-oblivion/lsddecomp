/*
 * code_d294 -- 0xD294.., the first 20 methods of Class6B5CC (method table
 * gClass6B5CCMethods, class tag 4), a BasicClass subclass and the base of
 * every class table whose tag nibble is 4 (DreamSys, Entity, BaseObjO and
 * about a dozen more, per tools/classtable.py --scan). An instance embeds a libgs GsDOBJ2 at +0x10
 * (attribute, coord2, tmd) and owns its GsCOORDINATE2 and GsCOORD2PARAM.
 * This slice holds: New / ctor / Finalize; the BasicClass child-list
 * overrides, which link or unlink a tag-9 model child as it is added or
 * removed; OnNotify, which fans a notification out by the sender's tag;
 * Reset (identity transform); UpdateRotation / UpdateScale (set or
 * accumulate a ratio triple into the GsCOORD2PARAM); attach to and detach
 * from a parent's coordinate; and five setters over GsDOBJ2.attribute.
 * The class continues in code_d294_b (slots +0x074..+0x0B4) and its free
 * helpers in code_d294_c. All 20 functions are matched. Named round 71;
 * tiers and evidence in each function's match report.
 */
#include "common.h"
#include "code_d294.h"

/* The low nibble of a class table's header word is its class tag. */
#define CLASS_TAG_MASK   0xF
#define TAG_PAD          2   /* gPadMethods, PadMethods (include/Pad.h) */
#define TAG_CLASS6B5CC   4   /* this class and every subclass of it */
#define TAG_CLASS6EF50   5   /* D_8006EF50 */
#define TAG_CLASS6BEA0   9   /* D_8006BEA0: the object Class6B5CC__LinkModel links */

/* Bit positions in GsDOBJ2.attribute (self->unk10), include/psyq/LIBGS.H. */
#define ATTR_LIGHTMODE_SHIFT  3   /* GsFOG|GsMATE|GsLLMOD, 3 bits */
#define ATTR_LOFF_SHIFT       6   /* GsLOFF */
#define ATTR_ABR_SHIFT        28  /* GsAZERO..GsATHREE, 2 bits */
#define ATTR_ALON_SHIFT       30  /* GsALON */
#define ATTR_DOFF_SHIFT       31  /* GsDOFF */

Class6B5CCObj *New_Class6B5CC(void) {
    Class6B5CCObj *obj;

    obj = BMemPMgrAlloc(0x44);
    if (obj == NULL) {
        return NULL;
    }
    if (GetClass6B5CCMethods()->ctor(obj) != NULL) {
        return obj;
    }
    BMemPMgrFree(obj);
    return NULL;
}

void *Class6B5CC__Class6B5CC(Class6B5CCObj *self) {
    void *blockB;

    self->coord2 = BMemPMgrAlloc(0x50);
    if (self->coord2 == NULL) {
        return NULL;
    }
    blockB = BMemPMgrAlloc(0x28);
    self->coord2->param = blockB;
    if (blockB == NULL) {
        BMemPMgrFree(self->coord2);
        return NULL;
    }
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetClass6B5CCMethods();
    self->unk20 = 0;
    self->unk18 = 0;
    self->parent = NULL;
    self->coord2->super = 0;
    self->methods->reset(self);
    return self;
}

void Class6B5CC__Finalize(Class6B5CCObj *self) {
    Class6B5CCSub14 *sub;

    self->methods->detachFromParent(self);
    self->methods->detachAttachedChildren(self);
    self->methods->slot5C(self, 0);
    sub = self->coord2;
    BMemPMgrFree(sub->param);
    BMemPMgrFree(self->coord2);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void Class6B5CC__AddChild(Class6B5CCObj *self, GenericObj_d294 *other) {
    Get_vtable_BasicClass()->addChild((BasicClass *)self, (BasicClass *)other);
    if ((other->methods->header & CLASS_TAG_MASK) == TAG_CLASS6BEA0) {
        Class6B5CC__LinkModel(self, other);
    }
}

void Class6B5CC__RemoveChild(Class6B5CCObj *self, GenericObj_d294 *other) {
    if ((other->methods->header & CLASS_TAG_MASK) == TAG_CLASS6BEA0) {
        Class6B5CC__UnlinkModel(self);
    }
    Get_vtable_BasicClass()->removeChild((BasicClass *)self, (BasicClass *)other);
}

void Class6B5CC__RemoveAllChildren(Class6B5CCObj *self) {
    Class6B5CC__UnlinkModel(self);
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

void Class6B5CC__OnNotify(Class6B5CCObj *self, GenericObj_d294 *other, s32 arg2) {
    s32 tag;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, other, arg2);
    tag = other->methods->header & CLASS_TAG_MASK;
    if (tag == TAG_PAD) {
        self->methods->slot94(self, other, arg2);
    } else if (tag == TAG_CLASS6EF50) {
        self->methods->slot98(self, other, arg2);
    } else if (tag == TAG_CLASS6B5CC) {
        self->methods->dispatchLinkCommand(self, other, arg2);
    }
}

void Class6B5CC__Reset(Class6B5CCObj *self) {
    self->tick = 0;
    self->attribute = 0;
    GsInitCoordinate2(0, self->coord2);
    self->methods->updateRotation(self, 1, ROTATION_ZERO);
    self->methods->updateScale(self, 1, SCALE_ONE);
    self->coord2->flg = 1;
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
    dst = self->coord2->param;
    field = &dst->rotate.x;
    if (flag) {
        dst->rotate.x = vals[0];
        dst->rotate.y = vals[1];
        dst->rotate.z = vals[2];
    } else {
        s32 i;
        s16 *cur;

        for (i = 0; i < 3; i++) {
            cur = field;
            field++;
            *cur = (*cur + vals[i]) % 4096;
        }
    }
    self->coord2->flg = 0;
}

void Class6B5CC__UpdateScale(Class6B5CCObj *self, s32 flag, void *data) {
    s32 r0, r1, r2;
    Class6B5CCSub44 *dst;

    r0 = RatioToFixed12(data);
    r1 = RatioToFixed12((u8 *)data + 4);
    r2 = RatioToFixed12((u8 *)data + 8);
    dst = self->coord2->param;
    if (flag) {
        dst->scaleX = (s16)r0;
        dst->scaleY = (s16)r1;
        dst->scaleZ = (s16)r2;
    } else {
        dst->scaleX += (s16)r0;
        dst->scaleY += (s16)r1;
        dst->scaleZ += (s16)r2;
    }
    self->coord2->flg = 0;
}

Class6B5CCObj *Class6B5CC__AttachToParent(Class6B5CCObj *self, UnkOwner_d294 *obj, Vec3_d294 *vec) {
    Class6B5CCSub14 *sub;

    if (self->parent == NULL) {
        self->parent = obj;
        sub = self->coord2;
        sub->super = obj->coord2;
        obj->methods->addChild(obj, self);
        sub = self->coord2;
        if (vec != NULL) {
            sub->tx = vec->x;
            sub->ty = vec->y;
            sub->tz = vec->z;
        } else {
            sub->tx = 0;
            sub->ty = 0;
            sub->tz = 0;
        }
        self->coord2->flg = 0;
    }
    return self;
}

Class6B5CCObj *Class6B5CC__DetachFromParent(Class6B5CCObj *self) {
    UnkOwner_d294 *owner;

    owner = self->parent;
    if (owner != NULL) {
        owner->methods->removeChild(owner, self);
        self->coord2->super = 0;
        self->parent = NULL;
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

    tag = TAG_CLASS6B5CC;
    do {
        if (*entry == NULL) {
            *cursor = self->children;
        }
        GetNextBasicClass((BasicClass **)entry, (BasicClassListNode **)cursor);
        if (*entry != NULL) {
            if ((((*entry)->methods->header) & CLASS_TAG_MASK) == tag) {
                if ((*entry)->parent == self) {
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
    return GetSetBitField(&self->attribute, ATTR_DOFF_SHIFT, 1, a1 == 0) == 0;
}

u32 Class6B5CC__SetSemiTrans(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->attribute, ATTR_ALON_SHIFT, 1, a1 != 0);
}

u32 Class6B5CC__SetSemiTransRate(Class6B5CCObj *self, u32 a1) {
    return GetSetBitField(&self->attribute, ATTR_ABR_SHIFT, 2, a1);
}

u32 Class6B5CC__SetLighting(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->attribute, ATTR_LOFF_SHIFT, 1, a1 == 0);
}

u32 Class6B5CC__SetLightMode(Class6B5CCObj *self, u32 a1) {
    return GetSetBitField(&self->attribute, ATTR_LIGHTMODE_SHIFT, 3, a1);
}
