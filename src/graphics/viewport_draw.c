/*
 * viewport_draw.c -- Viewport__DrawNode, the Viewport's drawNode slot
 * (+0x0A0 of gViewportMethods, inherited by gNodeGuardedViewportMethods,
 * include/viewport.h): it draws a SceneNode and its SceneNode children into
 * the Viewport's current ordering table, picking a libgs sort call by the
 * node's class. The rest of the Viewport class is in src/app/task.c.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libetc.h>
#include "basic_class.h"
#include "scene_node.h"
#include "grid_cell.h"
#include "bg_layer.h"
#include "box_fill.h"
#include "sprite.h"
#include "screen_sprite.h"
#include "viewport.h"
#include "tmd_renderer.h"

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

/*
 * Draws `node` into self->ot[self->otIndex], after first drawing, in list
 * order, every child that is a SceneNode and whose parent is `node`. A
 * GridCell whose GsDOFF bit is set is skipped, children and all. A node
 * whose coord2 is dirty (flg == 0) first rebuilds its matrix from its
 * GsCOORD2PARAM's rotate and scale, and marks each child it draws dirty.
 * The node's class picks the path:
 *  - BgLayer: its GsBG to GsSortBg, at the OT's last tag;
 *  - BoxFill (FadeBox too): its GsBOXF at posX/posY, percent of the
 *    half-screen while `relative` is set and pixels otherwise, to
 *    GsSortBoxFill at its own pri;
 *  - ScreenSprite: its GsSPRITE at screenPos, percent of the half-screen
 *    from the centre, plus its pivot, to GsSortSprite at tag 0;
 *  - any other Sprite, in world space: projected by the local-screen matrix
 *    and the Viewport's projH; dropped unless its depth is 1..0xFFFF and
 *    past nearZ, x/y clamped to SPRITE_POS_LIMIT, OT tag (depth - nearZ) /
 *    zDiv;
 *  - anything else: its GsDOBJ2 under GsGetLws's matrices to SortTmdObject,
 *    when it has a TMD. Casts to Sony's types mark the libgs calls.
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
