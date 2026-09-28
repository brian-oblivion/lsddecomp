/*
 * code_d294 -- SceneNode (include/SceneNode.h), part 1 of 3: slots +0x000
 * to +0x070. New, the ctor (allocates the GsCOORDINATE2 and GsCOORD2PARAM)
 * and Finalize; the BasicClass child-list overrides, which link or unlink a
 * TmdModel child as it is added or removed; OnNotify, which dispatches on
 * the sender's class id; Reset (identity transform); UpdateRotation and
 * UpdateScale (set or add three Ratio16s into the GsCOORD2PARAM); attach to
 * and detach from a parent's coordinate; and the first five setters over
 * GsDOBJ2.attribute. Part 2 is code_d294_b.c, part 3 code_d294_c.c.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_d294.h"
#include "Pad.h"
#include "FrameClock.h"

/* Bit positions in GsDOBJ2.attribute (self->attribute), include/psyq/libgs.h. */
#define ATTR_LIGHTMODE_SHIFT 3 /* GsFOG|GsMATE|GsLLMOD, 3 bits */
#define ATTR_LOFF_SHIFT 6      /* GsLOFF */
#define ATTR_ABR_SHIFT 28      /* GsAZERO..GsATHREE, 2 bits */
#define ATTR_ALON_SHIFT 30     /* GsALON */
#define ATTR_DOFF_SHIFT 31     /* GsDOFF */

/* UpdateRotation's divisor: its inputs are degrees, and a degree count in
 * 20.12 fixed point divided by 360 is the angle in 4096ths of a turn (ONE to
 * the turn), GsCOORD2PARAM.rotate's unit. */
#define DEGREES_PER_TURN 360

SceneNode *New_SceneNode(void) {
    SceneNode *obj;

    obj = BMemPMgrAlloc(sizeof(SceneNode));
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
    GsCOORD2PARAM *param;

    self->coord2 = BMemPMgrAlloc(sizeof(GsCOORDINATE2));
    if (self->coord2 == NULL) {
        return NULL;
    }
    param = BMemPMgrAlloc(sizeof(GsCOORD2PARAM));
    self->coord2->param = param;
    if (param == NULL) {
        BMemPMgrFree(self->coord2);
        return NULL;
    }
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetSceneNodeMethods();
    self->model = NULL;
    self->tmd = 0;
    self->parent = NULL;
    self->coord2->super = NULL;
    self->methods->reset(self);
    return self;
}

void SceneNode__Finalize(SceneNode *self) {
    GsCOORDINATE2 *coord2;

    self->methods->detachFromParent(self);
    self->methods->detachAttachedChildren(self);
    self->methods->slot5C(self, 0);
    coord2 = self->coord2;
    BMemPMgrFree(coord2->param);
    BMemPMgrFree(self->coord2);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void SceneNode__AddChild(SceneNode *self, BasicClass *child) {
    Get_vtable_BasicClass()->addChild((BasicClass *)self, child);
    if ((child->methods->header & CLASS_ID_ROOT_MASK) == TMDMODEL_CLASS_ID) {
        SceneNode__LinkModel(self, child);
    }
}

void SceneNode__RemoveChild(SceneNode *self, BasicClass *child) {
    if ((child->methods->header & CLASS_ID_ROOT_MASK) == TMDMODEL_CLASS_ID) {
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
    tag = sender->methods->header & CLASS_ID_ROOT_MASK;
    if (tag == PAD_CLASS_ID) {
        self->methods->onPadEvent(self, sender, event);
    } else if (tag == FRAMECLOCK_CLASS_ID) {
        self->methods->update(self, sender, event);
    } else if (tag == SCENENODE_CLASS_ID) {
        self->methods->dispatchLinkCommand(self, sender, event);
    }
}

void SceneNode__Reset(SceneNode *self) {
    self->tick = 0;
    self->attribute = 0;
    GsInitCoordinate2(NULL, self->coord2);
    self->methods->updateRotation(self, 1, ROTATION_ZERO);
    self->methods->updateScale(self, 1, SCALE_ONE);
    self->coord2->flg = 1;
}

void SceneNode__UpdateRotation(SceneNode *self, s32 set, void *table) {
    Ratio16 *ratios = table;
    s32 angles[3];
    GsCOORD2PARAM *param;
    s16 *next;

    angles[0] = RatioToFixed12(&ratios[0]);
    angles[1] = RatioToFixed12(&ratios[1]);
    angles[2] = RatioToFixed12(&ratios[2]);
    angles[0] /= DEGREES_PER_TURN;
    angles[1] /= DEGREES_PER_TURN;
    angles[2] /= DEGREES_PER_TURN;
    param = self->coord2->param;
    next = &param->rotate.vx;
    if (set) {
        param->rotate.vx = angles[0];
        param->rotate.vy = angles[1];
        param->rotate.vz = angles[2];
    } else {
        s32 i;
        s16 *cur;

        for (i = 0; i < 3; i++) {
            cur = next;
            next++;
            *cur = (*cur + angles[i]) % ONE;
        }
    }
    self->coord2->flg = 0;
}

void SceneNode__UpdateScale(SceneNode *self, s32 set, void *table) {
    Ratio16 *ratios = table;
    s32 sx, sy, sz;
    GsCOORD2PARAM *param;

    sx = RatioToFixed12(&ratios[0]);
    sy = RatioToFixed12(&ratios[1]);
    sz = RatioToFixed12(&ratios[2]);
    param = self->coord2->param;
    if (set) {
        param->scale.vx = (s16)sx;
        param->scale.vy = (s16)sy;
        param->scale.vz = (s16)sz;
    } else {
        param->scale.vx += (s16)sx;
        param->scale.vy += (s16)sy;
        param->scale.vz += (s16)sz;
    }
    self->coord2->flg = 0;
}

SceneNode *SceneNode__AttachToParent(SceneNode *self, SceneNode *parent, LongVec3 *offset) {
    GsCOORDINATE2 *coord2;

    if (self->parent == NULL) {
        self->parent = parent;
        coord2 = self->coord2;
        coord2->super = parent->coord2;
        parent->methods->addChild(parent, (BasicClass *)self);
        coord2 = self->coord2;
        if (offset != NULL) {
            coord2->coord.t[0] = offset->x;
            coord2->coord.t[1] = offset->y;
            coord2->coord.t[2] = offset->z;
        } else {
            coord2->coord.t[0] = 0;
            coord2->coord.t[1] = 0;
            coord2->coord.t[2] = 0;
        }
        self->coord2->flg = 0;
    }
    return self;
}

SceneNode *SceneNode__DetachFromParent(SceneNode *self) {
    SceneNode *parent;

    parent = self->parent;
    if (parent != NULL) {
        parent->methods->removeChild(parent, (BasicClass *)self);
        self->coord2->super = NULL;
        self->parent = NULL;
    }
    return self;
}

void SceneNode__DetachAttachedChildren(SceneNode *self) {
    SceneNode *child = NULL;
    BasicClassListNode *cursor;

    do {
        self->methods->getNextAttachedChild(self, &child, &cursor);
        if (child != NULL) {
            child->methods->detachFromParent(child);
        }
    } while (cursor);
}

void SceneNode__GetNextAttachedChild(SceneNode *self, SceneNode **child, BasicClassListNode **cursor) {
    s32 classId;

    classId = SCENENODE_CLASS_ID;
    do {
        if (*child == NULL) {
            *cursor = self->children;
        }
        GetNextBasicClass((BasicClass **)child, cursor);
        if (*child != NULL) {
            if ((((*child)->methods->header) & CLASS_ID_ROOT_MASK) == classId) {
                if ((*child)->parent == self) {
                    return;
                }
            }
        }
    } while (*cursor != NULL);
    *child = NULL;
}

void SceneNode__NoOpSlot5C(void) {}

s32 SceneNode__SetDisplay(SceneNode *self, s32 on) {
    return GetSetBitField(&self->attribute, ATTR_DOFF_SHIFT, 1, on == 0) == 0;
}

u32 SceneNode__SetSemiTrans(SceneNode *self, s32 on) {
    return GetSetBitField(&self->attribute, ATTR_ALON_SHIFT, 1, on != 0);
}

u32 SceneNode__SetSemiTransRate(SceneNode *self, u32 rate) {
    return GetSetBitField(&self->attribute, ATTR_ABR_SHIFT, 2, rate);
}

u32 SceneNode__SetLighting(SceneNode *self, s32 on) {
    return GetSetBitField(&self->attribute, ATTR_LOFF_SHIFT, 1, on == 0);
}

u32 SceneNode__SetLightMode(SceneNode *self, u32 mode) {
    return GetSetBitField(&self->attribute, ATTR_LIGHTMODE_SHIFT, 3, mode);
}
