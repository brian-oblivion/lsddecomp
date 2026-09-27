/*
 * ViewportDraw -- Viewport__DrawNode: draws one SceneNode into the Viewport's
 * current ordering table, after first drawing each of its SceneNode children
 * the same way.
 *
 * It is slot +0x0A0 (drawNode) of gViewportMethods, inherited unchanged by
 * gNodeGuardedViewportMethods (include/Viewport.h). The node's class id
 * picks the draw path, and each path reads the node as the class it tests
 * for:
 *  - a BgLayer: its GsBG to GsSortBg, at the OT's last tag;
 *  - a BoxFill (FadeBox too): its GsBOXF placed from posX/posY, in percent
 *    of the half-screen while `relative` is set and in pixels otherwise, to
 *    GsSortBoxFill at the object's own pri;
 *  - a ScreenSprite (and its subclasses): its GsSPRITE placed from
 *    screenPos, percent of the half-screen from the centre, plus its pivot
 *    (mx, my), to GsSortSprite at tag 0;
 *  - any other Sprite: a world-space sprite. Its coord2's position goes
 *    through the local-screen matrix and a perspective divide by the
 *    Viewport's projH. It is dropped unless its depth is in 1..0xFFFF (the
 *    GTE's 16-bit screen z) and past nearZ, its x/y are clamped to
 *    SPRITE_POS_LIMIT, and its depth past nearZ over zDiv is its OT tag;
 *  - anything else: the node's own GsDOBJ2 (+0x010), under GsGetLws's light
 *    and local-screen matrices, to SortTmdObject (TmdRenderer.c), the
 *    game's replacement for GsSortObject4, when it has a TMD.
 * A GridCell whose GsDOFF bit is set is skipped, children and all.
 *
 * Before drawing, a node whose coord2 is dirty (flg == 0) rebuilds its
 * matrix from its GsCOORD2PARAM's rotate and scale and marks each child it
 * draws dirty in turn.
 *
 * The subclasses spell their GsBG/GsBOXF/GsSPRITE field by field, hence the
 * casts to Sony's types at the libgs calls.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libetc.h>
#include "BasicClass.h"
#include "SceneNode.h"
#include "GridCell.h"
#include "BgLayer.h"
#include "BoxFill.h"
#include "Sprite.h"
#include "ScreenSprite.h"
#include "Viewport.h"

/* The bits of avsz3's OTZ that one OT spans: SortTmdObject files a face at
 * otBase[otz >> (OTZ_BITS - otLength)], so 1 << otLength tags cover OTZ
 * 0..(1 << 14) - 1; Sony's samples pass GsSortObject4 the same
 * 14 - OT_LENGTH. */
#define OTZ_BITS 14

/* A world-space sprite whose projected x or y is further than this from the
 * screen centre, on either side, is placed at +SPRITE_POS_LIMIT on that
 * axis: past the right or bottom edge even of a 640x480 display, whose
 * half-size is 320x240. */
#define SPRITE_POS_LIMIT 512

/* code_d294_c.c. include/code_d294.h declares it the same way; this unit
 * does not include that header. */
extern void ApplyMatrixToLVArray(void *dst, void *src, s32 count, void *m);
/* TmdRenderer.c */
extern void SortTmdObject(GsDOBJ2 *obj, GsOT *ot, s32 otShift, void *scratch);

/*
 * Draws `node` into self->ot[self->otIndex], after first drawing, in list
 * order, every child that is a SceneNode and whose parent is `node`.
 */
void Viewport__DrawNode(Viewport *self, SceneNode *node) {
    MATRIX lsBuf;
    MATRIX lwBuf;
    MATRIX *ls;
    MATRIX *lw;
    SceneNode *child;
    BasicClassListNode *cursor;
    s32 dirty;
    GsCOORDINATE2 *coord2;
    s16 *elem;
    s16 *end;
    u32 *scale;
    u32 classId;

    dirty = 0;
    /* MATCHING: explicit pointers; retail keeps both addresses in s-registers */
    ls = &lsBuf;
    lw = &lwBuf;
    if ((u8)node->methods->header == GRIDCELL_CLASS_ID && (s32)node->attribute < 0) { /* GsDOFF, bit 31 */
        return;
    }

    coord2 = node->coord2;
    if (coord2->flg == 0) {
        elem = &coord2->coord.m[0][0];
        scale = (u32 *)&coord2->param->scale; /* MATCHING: read unsigned (srl) */
        dirty = 1;
        end = &coord2->coord.m[3][0];
        RotMatrix(&coord2->param->rotate, &coord2->coord);
        while (elem < end) {
            *elem = (*elem * *scale++) >> FIX12_SHIFT;
            elem++;
            *elem = (*elem * *scale++) >> FIX12_SHIFT;
            elem++;
            *elem = (*elem * *scale++) >> FIX12_SHIFT;
            elem++;
            scale -= 3;
        }
    }

    child = NULL;
    do {
        for (;;) {
            if (child == NULL) {
                cursor = node->children;
            }
            GetNextBasicClass((BasicClass **)&child, &cursor);
            if (child != NULL && (child->methods->header & CLASS_ID_ROOT_MASK) == SCENENODE_CLASS_ID &&
                child->parent == node) {
                break;
            }
            if (cursor == NULL) {
                child = NULL;
                break;
            }
        }
        if (child != NULL) {
            if (dirty) {
                child->coord2->flg = 0;
            }
            Viewport__DrawNode(self, child);
        }
    } while (cursor != NULL);

    classId = node->methods->header;
    if ((classId & 0xFF) == BGLAYER_CLASS_ID) {
        GsSortBg((GsBG *)&((BgLayer *)node)->bgAttribute, self->ot[self->otIndex],
                 (1 << self->otLength) - 1);
    } else if ((classId & 0xFF) == BOXFILL_CLASS_ID) {
        BoxFill *box = (BoxFill *)node; /* MATCHING: a copy of node, not a cast at each use */
        if (box->relative) {
            box->boxX = ((self->screenSize.width >> 1) * box->posX) / 100;
            box->boxY = ((self->screenSize.height >> 1) * box->posY) / 100;
        } else {
            box->boxX = box->posX;
            box->boxY = box->posY;
        }
        GsSortBoxFill((GsBOXF *)&box->boxAttribute, self->ot[self->otIndex], box->pri);
    } else if ((classId & 0xFF) != SPRITE_CLASS_ID) {
        GsGetLws(node->coord2, lw, ls);
        GsSetLightMatrix(lw);
        GsSetLsMatrix(ls);
        if (node->tmd != 0) {
            SortTmdObject((GsDOBJ2 *)&node->attribute, self->ot[self->otIndex],
                          OTZ_BITS - self->otLength, getScratchAddr(0));
        }
    } else if ((classId & 0xFFF) == SCREENSPRITE_CLASS_ID) {
        /* MATCHING: a copy of node; each ternary is one store */
        ScreenSprite *screenSprite = (ScreenSprite *)node;
        GsSPRITE *gsSprite = (GsSPRITE *)&screenSprite->sprite;
        s32 *screen = &self->screenSize.width;
        gsSprite->x = (screenSprite->screenPos.x != 0)
                          ? ((screen[0] >> 1) * 100) / (10000 / screenSprite->screenPos.x)
                          : 0;
        gsSprite->y = (screenSprite->screenPos.y != 0)
                          ? ((screen[1] >> 1) * 100) / (10000 / screenSprite->screenPos.y)
                          : 0;
        gsSprite->x += gsSprite->mx;
        gsSprite->y += gsSprite->my;
        GsSortSprite(gsSprite, self->ot[self->otIndex], 0);
    } else {
        /* MATCHING: declared here, not at the top, for the frame layout; scr is
         * never used and reserves 8 frame bytes retail leaves untouched */
        VECTOR pos;
        SVECTOR scr;
        Sprite *worldSprite;

        GsGetLs(node->coord2, ls);
        if (ls->t[2] < 1 || ls->t[2] > 0xFFFF) {
            return;
        }
        if (node->parent->parent != NULL) {
            ApplyMatrixToLVArray(&pos, node->coord2->coord.t, 1, ls);
            pos.vx += lsBuf.t[0];
            pos.vy += lsBuf.t[1];
            pos.vz += lsBuf.t[2];
        } else {
            pos.vx = lsBuf.t[0];
            pos.vy = lsBuf.t[1];
            pos.vz = lsBuf.t[2];
        }
        if (pos.vz > self->nearZ) {
            pos.vx = (pos.vx * self->projH) / pos.vz;
            pos.vy = (pos.vy * self->projH) / pos.vz;
            pos.vz = (pos.vz - self->nearZ) / self->zDiv;
            worldSprite = (Sprite *)node;
            /* MATCHING: ~v + 1, not -v (nor/addiu, not negu) */
            if ((pos.vx < 0 ? ~pos.vx + 1 : pos.vx) <= SPRITE_POS_LIMIT) {
                worldSprite->sprite.x = pos.vx;
            } else {
                worldSprite->sprite.x = SPRITE_POS_LIMIT;
            }
            if ((pos.vy < 0 ? ~pos.vy + 1 : pos.vy) <= SPRITE_POS_LIMIT) {
                worldSprite->sprite.y = pos.vy;
            } else {
                worldSprite->sprite.y = SPRITE_POS_LIMIT;
            }
            GsSortSprite((GsSPRITE *)&worldSprite->sprite, self->ot[self->otIndex], pos.vz);
        }
    }
}
