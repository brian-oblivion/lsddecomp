/*
 * code_2864 -- Viewport__DrawNode, the scene-graph walk that draws one node
 * and its drawable children into the Viewport's current ordering table
 * (vram 0x80012064..0x80012768).
 *
 * It is slot +0x0A0 (drawNode) of gViewportMethods, inherited unchanged by
 * gNodeGuardedViewportMethods (include/Viewport.h); `self` is the Viewport.
 * The node is read through DrawNode below, a view of a scene node whose
 * class-id low byte picks what it is: 0x54 a GsBG, 0x64 a GsBOXF placed in
 * percent of the half-screen, 0x144 a screen-space GsSPRITE, 0x44 a sprite
 * projected from its GsCOORDINATE2's world position, and anything else a
 * GsDOBJ2 model sorted by SortTmdObject (code_8220_b.c), the game's
 * replacement for GsSortObject4. Before drawing, a node whose coord2 is dirty
 * (flg == 0) rebuilds its matrix from GsCOORD2PARAM's rotate and scale and
 * marks its children dirty; children (class-id low nibble 4, parent == node)
 * are drawn first, so the order in the OT is children before parent.
 *
 * Types are Sony's (libgte.h, libgs.h). Viewport.h's ViewportOt is a local
 * view of GsOT, hence the (GsOT *) casts at the sort calls.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "BasicClass.h"
#include "Viewport.h"

/* A drawable scene node: a BasicClass whose +0x00C is its parent and whose
 * class-id low byte picks the draw path in Viewport__DrawNode (0x54 background,
 * 0x64 box fill, 0x44 sprite -- 0x144 screen-space, otherwise world-space --
 * and anything else a GsDOBJ2 model). */
typedef struct DrawNode DrawNode;

struct DrawNode {
    BASICCLASS_FIELDS(BasicClassMethods);
    DrawNode *parent; /* +0x00C */
    GsDOBJ2 obj; /* +0x010 */
    u8 pad20[0x44 - 0x20];

    union {
        GsBG bg; /* tag 0x54, +0x044 */

        struct {     /* tag 0x64 */
            u16 pri; /* +0x044 */
            u8 pad46[2];
            s32 relative; /* +0x048 */
            u8 pad4C[4];
            s32 x;           /* +0x050 */
            s32 y;           /* +0x054 */
            GsBOXF box; /* +0x058 */
        } boxf;

        struct { /* tag 0x44 */
            u8 pad44[0x64 - 0x44];
            GsSPRITE sprite; /* +0x064 */
            u8 pad88[0xA0 - 0x88];
            s32 ratioX; /* +0x0A0 */
            s32 ratioY; /* +0x0A4 */
        } spr;
    } u;
};

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
void Viewport__DrawNode(Viewport *self, DrawNode *node) {
    MATRIX lsBuf;
    MATRIX lwBuf;
    MATRIX *ls;
    MATRIX *lw;
    DrawNode *child;
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
    if ((u8)node->methods->header == 0x24 && (s32)node->obj.attribute < 0) {
        return;
    }

    c = node->obj.coord2;
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
                child->obj.coord2->flg = 0;
            }
            Viewport__DrawNode(self, child);
        }
    } while (cursor != NULL);

    tag = node->methods->header;
    if ((tag & 0xFF) == 0x54) {
        GsSortBg(&node->u.bg, (GsOT *)self->ot[self->otIndex], (1 << self->otLength) - 1);
    } else if ((tag & 0xFF) == 0x64) {
        DrawNode *b = node;
        if (b->u.boxf.relative) {
            b->u.boxf.box.x = ((self->screenSize.width >> 1) * b->u.boxf.x) / 100;
            b->u.boxf.box.y = ((self->screenSize.height >> 1) * b->u.boxf.y) / 100;
        } else {
            b->u.boxf.box.x = b->u.boxf.x;
            b->u.boxf.box.y = b->u.boxf.y;
        }
        GsSortBoxFill(&b->u.boxf.box, (GsOT *)self->ot[self->otIndex], b->u.boxf.pri);
    } else if ((tag & 0xFF) != 0x44) {
        GsGetLws(node->obj.coord2, lw, ls);
        GsSetLightMatrix(lw);
        GsSetLsMatrix(ls);
        if (node->obj.tmd != NULL) {
            SortTmdObject(&node->obj, (GsOT *)self->ot[self->otIndex], 14 - self->otLength, (void *)0x1F800000);
        }
    } else if ((tag & 0xFFF) == 0x144) {
        DrawNode *n = node;
        GsSPRITE *sp = &n->u.spr.sprite;
        s32 *size = &self->screenSize.width;
        sp->x = (n->u.spr.ratioX != 0) ? ((size[0] >> 1) * 100) / (10000 / n->u.spr.ratioX) : 0;
        sp->y = (n->u.spr.ratioY != 0) ? ((size[1] >> 1) * 100) / (10000 / n->u.spr.ratioY) : 0;
        sp->x += sp->mx;
        sp->y += sp->my;
        GsSortSprite(sp, (GsOT *)self->ot[self->otIndex], 0);
    } else {
        VECTOR pos;
        SVECTOR scr; /* never used; reserves retail's 8 unused frame bytes */
        DrawNode *n;

        GsGetLs(node->obj.coord2, ls);
        if (ls->t[2] < 1 || ls->t[2] > 0xFFFF) {
            return;
        }
        if (node->parent->parent != NULL) {
            ApplyMatrixToLVArray(&pos, &node->obj.coord2->coord.t, 1, ls);
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
            n = node;
            if ((pos.vx < 0 ? ~pos.vx + 1 : pos.vx) <= 0x200) {
                n->u.spr.sprite.x = pos.vx;
            } else {
                n->u.spr.sprite.x = 0x200;
            }
            if ((pos.vy < 0 ? ~pos.vy + 1 : pos.vy) <= 0x200) {
                n->u.spr.sprite.y = pos.vy;
            } else {
                n->u.spr.sprite.y = 0x200;
            }
            GsSortSprite(&n->u.spr.sprite, (GsOT *)self->ot[self->otIndex], pos.vz);
        }
    }
}
