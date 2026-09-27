/*
 * code_2864 -- Viewport__DrawNode, the scene-graph walk that draws one node
 * and its drawable children into the Viewport's current ordering table
 * (vram 0x80012064..0x80012768).
 *
 * It is slot +0x0A0 (drawNode) of gViewportMethods, inherited unchanged by
 * gNodeGuardedViewportMethods (include/Viewport.h); `self` is the Viewport
 * and `node` a SceneNode (include/SceneNode.h). The class-id low byte picks
 * the draw path, and each path reads the node as the subclass that id names:
 * 0x54 a BgLayer (its GsBG at +0x044 to GsSortBg), 0x64 a BoxFill (its GsBOXF
 * placed in percent of the half-screen while `relative` is set), 0x144 a
 * ScreenSprite (its GsSPRITE placed from `screenPos`), any other 0x44 a
 * Sprite projected from its GsCOORDINATE2's world position, and anything
 * else the node's own GsDOBJ2 (+0x010) sorted by SortTmdObject
 * (TmdRenderer.c), the game's replacement for GsSortObject4. A GridCell
 * (0x24) whose GsDOFF bit is set is skipped outright.
 *
 * Before drawing, a node whose coord2 is dirty (flg == 0) rebuilds its matrix
 * from GsCOORD2PARAM's rotate and scale and marks its children dirty;
 * children (class-id low nibble 4, parent == node) are drawn first, so the
 * order in the OT is children before parent.
 *
 * Types are Sony's (libgte.h, libgs.h) and the classes' own. SceneNode's
 * coord2 is Sony's GsCOORDINATE2; the subclasses spell their
 * GsBG/GsBOXF/GsSPRITE field by field: hence the casts to Sony's types at
 * the libgs calls. The OT is Viewport's own GsOT.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "BasicClass.h"
#include "SceneNode.h"
#include "BgLayer.h"
#include "BoxFill.h"
#include "Sprite.h"
#include "ScreenSprite.h"
#include "Viewport.h"

/* code_d294_c.c (include/code_d294.h) */
extern void ApplyMatrixToLVArray(void *dst, void *src, s32 count, void *m);
/* TmdRenderer.c */
extern void SortTmdObject(GsDOBJ2 *obj, GsOT *ot, s32 otShift, void *scratch);

/*
 * Draw `node` into self's current ordering table, after first drawing every
 * child whose class-id low nibble is 4 and whose parent is `node`.
 *
 * Source-shape notes (all measured; see the match report):
 *  - `ls`/`lw` are explicit pointers to the two stack matrices: retail keeps
 *    &lsBuf/&lwBuf live in s5/s7 from the prologue on.
 *  - `pos` and the unused `scr` live in the world-space-sprite block: that
 *    puts them ABOVE child/cursor in the frame, and `scr` is the 8 bytes of
 *    frame retail reserves and never touches.
 *  - `box`/`screenSprite` are separate copies of `node` (retail's move a3/a1/a0,s2);
 *    the ratio ternaries are one store each (the second copy of the store
 *    is the delay-slot filler's); `~v + 1` is retail's nor/addiu negate.
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
    ls = &lsBuf;
    lw = &lwBuf;
    if ((u8)node->methods->header == 0x24 && (s32)node->attribute < 0) {
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
            *elem = (*elem * *scale++) >> 12;
            elem++;
            *elem = (*elem * *scale++) >> 12;
            elem++;
            *elem = (*elem * *scale++) >> 12;
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
            if (child != NULL && (child->methods->header & 0xF) == 4 && child->parent == node) {
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
    if ((classId & 0xFF) == 0x54) {
        GsSortBg((GsBG *)&((BgLayer *)node)->bgAttribute, self->ot[self->otIndex],
                 (1 << self->otLength) - 1);
    } else if ((classId & 0xFF) == 0x64) {
        BoxFill *box = (BoxFill *)node;
        if (box->relative) {
            box->boxX = ((self->screenSize.width >> 1) * box->posX) / 100;
            box->boxY = ((self->screenSize.height >> 1) * box->posY) / 100;
        } else {
            box->boxX = box->posX;
            box->boxY = box->posY;
        }
        GsSortBoxFill((GsBOXF *)&box->boxAttribute, self->ot[self->otIndex], box->pri);
    } else if ((classId & 0xFF) != 0x44) {
        GsGetLws(node->coord2, lw, ls);
        GsSetLightMatrix(lw);
        GsSetLsMatrix(ls);
        if (node->tmd != 0) {
            SortTmdObject((GsDOBJ2 *)&node->attribute, self->ot[self->otIndex], 14 - self->otLength,
                          (void *)0x1F800000);
        }
    } else if ((classId & 0xFFF) == 0x144) {
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
        VECTOR pos;
        SVECTOR scr; /* never used; reserves retail's 8 unused frame bytes */
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
            if ((pos.vx < 0 ? ~pos.vx + 1 : pos.vx) <= 0x200) {
                worldSprite->sprite.x = pos.vx;
            } else {
                worldSprite->sprite.x = 0x200;
            }
            if ((pos.vy < 0 ? ~pos.vy + 1 : pos.vy) <= 0x200) {
                worldSprite->sprite.y = pos.vy;
            } else {
                worldSprite->sprite.y = 0x200;
            }
            GsSortSprite((GsSPRITE *)&worldSprite->sprite, self->ot[self->otIndex], pos.vz);
        }
    }
}
