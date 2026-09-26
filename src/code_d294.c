/*
 * code_d294 -- 0xD294.., the first 20 methods of SceneNode (method table
 * gSceneNodeMethods, class tag 4), a BasicClass subclass and the base of
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
#define CLASS_TAG_MASK 0xF
#define TAG_PAD 2        /* gPadMethods, PadMethods (include/Pad.h) */
#define TAG_SCENENODE 4 /* this class and every subclass of it */
#define TAG_CLASS6EF50 5 /* gFrameClockMethods */
#define TAG_TMDMODEL \
    9 /* gTmdModelMethods (include/TmdModel.h): the object SceneNode__LinkModel links */

/* Bit positions in GsDOBJ2.attribute (self->unk10), include/psyq/libgs.h. */
#define ATTR_LIGHTMODE_SHIFT 3 /* GsFOG|GsMATE|GsLLMOD, 3 bits */
#define ATTR_LOFF_SHIFT 6      /* GsLOFF */
#define ATTR_ABR_SHIFT 28      /* GsAZERO..GsATHREE, 2 bits */
#define ATTR_ALON_SHIFT 30     /* GsALON */
#define ATTR_DOFF_SHIFT 31     /* GsDOFF */

SceneNode *New_SceneNode(void) {
    SceneNode *obj;

    obj = BMemPMgrAlloc(0x44);
    if (obj == NULL) {
        return NULL;
    }
    if (GetSceneNodeMethods()->ctor(obj) != NULL) {
        return obj;
    }
    BMemPMgrFree(obj);
    return NULL;
}

void *SceneNode__SceneNode(SceneNode *self) {
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
    self->methods = GetSceneNodeMethods();
    self->model = 0;
    self->tmd = 0;
    self->parent = NULL;
    self->coord2->super = 0;
    self->methods->reset(self);
    return self;
}

void SceneNode__Finalize(SceneNode *self) {
    SceneNodeSub14 *sub;

    self->methods->detachFromParent(self);
    self->methods->detachAttachedChildren(self);
    self->methods->slot5C(self, 0);
    sub = self->coord2;
    BMemPMgrFree(sub->param);
    BMemPMgrFree(self->coord2);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void SceneNode__AddChild(SceneNode *self, BasicClass *child) {
    Get_vtable_BasicClass()->addChild((BasicClass *)self, child);
    if ((child->methods->header & CLASS_TAG_MASK) == TAG_TMDMODEL) {
        SceneNode__LinkModel(self, child);
    }
}

void SceneNode__RemoveChild(SceneNode *self, BasicClass *child) {
    if ((child->methods->header & CLASS_TAG_MASK) == TAG_TMDMODEL) {
        SceneNode__UnlinkModel(self);
    }
    Get_vtable_BasicClass()->removeChild((BasicClass *)self, child);
}

void SceneNode__RemoveAllChildren(SceneNode *self) {
    SceneNode__UnlinkModel(self);
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

void SceneNode__OnNotify(SceneNode *self, BasicClass *sender, s32 event) {
    s32 tag;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);
    tag = sender->methods->header & CLASS_TAG_MASK;
    if (tag == TAG_PAD) {
        self->methods->onPadEvent(self, sender, event);
    } else if (tag == TAG_CLASS6EF50) {
        self->methods->update(self, sender, event);
    } else if (tag == TAG_SCENENODE) {
        self->methods->dispatchLinkCommand(self, sender, event);
    }
}

void SceneNode__Reset(SceneNode *self) {
    self->tick = 0;
    self->attribute = 0;
    GsInitCoordinate2(0, self->coord2);
    self->methods->updateRotation(self, 1, ROTATION_ZERO);
    self->methods->updateScale(self, 1, SCALE_ONE);
    self->coord2->flg = 1;
}

void SceneNode__UpdateRotation(SceneNode *self, s32 flag, void *data) {
    s32 vals[3];
    SceneNodeSub44 *dst;
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

void SceneNode__UpdateScale(SceneNode *self, s32 flag, void *data) {
    s32 r0, r1, r2;
    SceneNodeSub44 *dst;

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

SceneNode *SceneNode__AttachToParent(SceneNode *self, SceneNode *obj, Vec3_d294 *vec) {
    SceneNodeSub14 *sub;

    if (self->parent == NULL) {
        self->parent = obj;
        sub = self->coord2;
        sub->super = obj->coord2;
        obj->methods->addChild(obj, (BasicClass *)self);
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

SceneNode *SceneNode__DetachFromParent(SceneNode *self) {
    SceneNode *owner;

    owner = self->parent;
    if (owner != NULL) {
        owner->methods->removeChild(owner, (BasicClass *)self);
        self->coord2->super = 0;
        self->parent = NULL;
    }
    return self;
}

void SceneNode__DetachAttachedChildren(SceneNode *self) {
    SceneNode *entry = NULL;
    BasicClassListNode *cursor;

    do {
        self->methods->getNextAttachedChild(self, &entry, &cursor);
        if (entry != NULL) {
            entry->methods->detachFromParent(entry);
        }
    } while (cursor);
}

void SceneNode__GetNextAttachedChild(SceneNode *self, SceneNode **entry, BasicClassListNode **cursor) {
    s32 tag;

    tag = TAG_SCENENODE;
    do {
        if (*entry == NULL) {
            *cursor = self->children;
        }
        GetNextBasicClass((BasicClass **)entry, cursor);
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

void SceneNode__func_1d33c(void) {}

s32 SceneNode__SetDisplay(SceneNode *self, s32 a1) {
    return GetSetBitField(&self->attribute, ATTR_DOFF_SHIFT, 1, a1 == 0) == 0;
}

u32 SceneNode__SetSemiTrans(SceneNode *self, s32 a1) {
    return GetSetBitField(&self->attribute, ATTR_ALON_SHIFT, 1, a1 != 0);
}

u32 SceneNode__SetSemiTransRate(SceneNode *self, u32 a1) {
    return GetSetBitField(&self->attribute, ATTR_ABR_SHIFT, 2, a1);
}

u32 SceneNode__SetLighting(SceneNode *self, s32 a1) {
    return GetSetBitField(&self->attribute, ATTR_LOFF_SHIFT, 1, a1 == 0);
}

u32 SceneNode__SetLightMode(SceneNode *self, u32 a1) {
    return GetSetBitField(&self->attribute, ATTR_LIGHTMODE_SHIFT, 3, a1);
}
