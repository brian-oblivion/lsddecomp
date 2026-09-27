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
 * (code_8220_b.c), the game's replacement for GsSortObject4. A GridCell
 * (0x24) whose GsDOFF bit is set is skipped outright.
 *
 * Before drawing, a node whose coord2 is dirty (flg == 0) rebuilds its matrix
 * from GsCOORD2PARAM's rotate and scale and marks its children dirty;
 * children (class-id low nibble 4, parent == node) are drawn first, so the
 * order in the OT is children before parent.
 *
 * Types are Sony's (libgte.h, libgs.h) and the classes' own. SceneNode's
 * coord2 is the parked SceneNodeSub14 view of GsCOORDINATE2, the subclasses
 * spell their GsBG/GsBOXF/GsSPRITE field by field: hence the casts to
 * Sony's types at the libgs calls. The OT is Viewport's own GsOT.
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
/* code_8220_b.c */
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
 *  - `b`/`n` are separate copies of `node` (retail's move a3/a1/a0,s2);
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
    GsCOORDINATE2 *c;
    s16 *m;
    s16 *end;
    u32 *sc;
    u32 tag;

    dirty = 0;
    ls = &lsBuf;
    lw = &lwBuf;
    if ((u8)node->methods->header == 0x24 && (s32)node->attribute < 0) {
        return;
    }

    c = (GsCOORDINATE2 *)node->coord2;
    if (c->flg == 0) {
        m = &c->coord.m[0][0];
        sc = (u32 *)&c->param->scale; /* MATCHING: read unsigned (srl) */
        dirty = 1;
        end = &c->coord.m[3][0];
        RotMatrix(&c->param->rotate, &c->coord);
        while (m < end) {
            *m = (*m * *sc++) >> 12;
            m++;
            *m = (*m * *sc++) >> 12;
            m++;
            *m = (*m * *sc++) >> 12;
            m++;
            sc -= 3;
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

    tag = node->methods->header;
    if ((tag & 0xFF) == 0x54) {
        GsSortBg((GsBG *)&((BgLayer *)node)->bgAttribute, self->ot[self->otIndex],
                 (1 << self->otLength) - 1);
    } else if ((tag & 0xFF) == 0x64) {
        BoxFill *b = (BoxFill *)node;
        if (b->relative) {
            b->boxX = ((self->screenSize.width >> 1) * b->posX) / 100;
            b->boxY = ((self->screenSize.height >> 1) * b->posY) / 100;
        } else {
            b->boxX = b->posX;
            b->boxY = b->posY;
        }
        GsSortBoxFill((GsBOXF *)&b->boxAttribute, self->ot[self->otIndex], b->pri);
    } else if ((tag & 0xFF) != 0x44) {
        GsGetLws((GsCOORDINATE2 *)node->coord2, lw, ls);
        GsSetLightMatrix(lw);
        GsSetLsMatrix(ls);
        if (node->tmd != 0) {
            SortTmdObject((GsDOBJ2 *)&node->attribute, self->ot[self->otIndex], 14 - self->otLength,
                          (void *)0x1F800000);
        }
    } else if ((tag & 0xFFF) == 0x144) {
        ScreenSprite *n = (ScreenSprite *)node;
        GsSPRITE *sp = (GsSPRITE *)&n->sprite;
        s32 *size = &self->screenSize.width;
        sp->x = (n->screenPos.x != 0) ? ((size[0] >> 1) * 100) / (10000 / n->screenPos.x) : 0;
        sp->y = (n->screenPos.y != 0) ? ((size[1] >> 1) * 100) / (10000 / n->screenPos.y) : 0;
        sp->x += sp->mx;
        sp->y += sp->my;
        GsSortSprite(sp, self->ot[self->otIndex], 0);
    } else {
        VECTOR pos;
        SVECTOR scr; /* never used; reserves retail's 8 unused frame bytes */
        Sprite *n;

        GsGetLs((GsCOORDINATE2 *)node->coord2, ls);
        if (ls->t[2] < 1 || ls->t[2] > 0xFFFF) {
            return;
        }
        if (node->parent->parent != NULL) {
            ApplyMatrixToLVArray(&pos, &node->coord2->tx, 1, ls);
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
            n = (Sprite *)node;
            if ((pos.vx < 0 ? ~pos.vx + 1 : pos.vx) <= 0x200) {
                n->sprite.x = pos.vx;
            } else {
                n->sprite.x = 0x200;
            }
            if ((pos.vy < 0 ? ~pos.vy + 1 : pos.vy) <= 0x200) {
                n->sprite.y = pos.vy;
            } else {
                n->sprite.y = 0x200;
            }
            GsSortSprite((GsSPRITE *)&n->sprite, self->ot[self->otIndex], pos.vz);
        }
    }
}
