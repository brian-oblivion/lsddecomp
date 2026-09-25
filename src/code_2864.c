/*
 * code_2864 -- GAME code carved from psyq_2864 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2864..0x2F68 (vram 0x80012064..0x80012768). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: one 449-word function,
 * Unk18Obj__DrawNode, listed in D_8006E8E4 and gClass869D8Methods and calling
 * GetNextBasicClass and ApplyMatrixToLVArray.
 *
 * MATCHED round 81 (alpha; docs/match-reports/Unk18Obj__DrawNode.md). It is slot
 * +0x0A0 of Unk18Obj's table (tools/classtable.py D_8006E8E4): the view
 * draws one scene node and recurses into the node's children. Every type
 * below is a LOCAL view (DrawView is this function's reading of Unk18Obj,
 * whose shared view lives in include/code_2cc8c.h and is not touched here).
 */
#include "common.h"
#include "BasicClass.h"

/* Psy-Q LIBGTE.H / LIBGS.H shapes, declared locally. */
typedef struct { s16 m[3][3]; s32 t[3]; } MATRIX_2864;
typedef struct { s32 vx, vy, vz, pad; } VECTOR_2864;
typedef struct { s16 vx, vy, vz, pad; } SVECTOR_2864;

typedef struct {
    u32 scale[4];            /* +0x00 VECTOR scale, read as unsigned */
    SVECTOR_2864 rotate;     /* +0x10 */
    VECTOR_2864 trans;       /* +0x18 */
} GsCOORD2PARAM_2864;

typedef struct GsCOORDINATE2_2864 GsCOORDINATE2_2864;
struct GsCOORDINATE2_2864 {
    u32 flg;                          /* +0x00 */
    MATRIX_2864 coord;                /* +0x04 */
    MATRIX_2864 workm;                /* +0x24 */
    GsCOORD2PARAM_2864 *param;        /* +0x44 */
    GsCOORDINATE2_2864 *super;        /* +0x48 */
    GsCOORDINATE2_2864 *sub;          /* +0x4C */
};

typedef struct {
    u32 attribute;
    s16 x, y;
    u16 w, h;
    u16 tpage;
    u8 u, v;
    s16 cx, cy;
    u8 r, g, b;
    s16 mx, my;
    s16 scalex, scaley;
    s32 rotate;
} GsSPRITE_2864;

typedef struct {
    u32 attribute;
    s16 x, y;
    u16 w, h;
    u8 r, g, b;
} GsBOXF_2864;

typedef struct {
    u32 attribute;              /* +0x10 */
    GsCOORDINATE2_2864 *coord2; /* +0x14 */
    u32 *tmd;                   /* +0x18 */
    u32 id;                     /* +0x1C */
} GsDOBJ2_2864;

/* A drawable scene node: a BasicClass whose +0x00C is its parent and whose
 * class-id low byte picks the draw path in Unk18Obj__DrawNode (0x54 background,
 * 0x64 box fill, 0x44 sprite -- 0x144 screen-space, otherwise world-space --
 * and anything else a GsDOBJ2 model). */
typedef struct DrawNode DrawNode;
struct DrawNode {
    BASICCLASS_FIELDS(BasicClassMethods);
    DrawNode *parent;           /* +0x00C */
    GsDOBJ2_2864 obj;           /* +0x010 */
    u8 pad20[0x44 - 0x20];
    union {
        u8 bg[0x28];            /* tag 0x54: GsBG at +0x44 */
        struct {                /* tag 0x64 */
            u16 pri;            /* +0x044 */
            u8 pad46[2];
            s32 relative;       /* +0x048 */
            u8 pad4C[4];
            s32 x;              /* +0x050 */
            s32 y;              /* +0x054 */
            GsBOXF_2864 box;    /* +0x058 */
        } boxf;
        struct {                /* tag 0x44 */
            u8 pad44[0x64 - 0x44];
            GsSPRITE_2864 sprite; /* +0x064 */
            u8 pad88[0xA0 - 0x88];
            s32 ratioX;         /* +0x0A0 */
            s32 ratioY;         /* +0x0A4 */
        } spr;
    } u;
};

/* Unk18Obj__DrawNode's reading of its `self` (Unk18Obj). */
typedef struct {
    u8 pad0[0x34];
    s32 width;                  /* +0x34 */
    s32 height;                 /* +0x38 */
    s32 otLen;                  /* +0x3C */
    s32 projH;                  /* +0x40 */
    u8 pad44[0x4C - 0x44];
    s32 nearZ;                  /* +0x4C */
    u8 pad50[0x74 - 0x50];
    s32 buf;                    /* +0x74 */
    void *ot[2];                /* +0x78 */
    u8 pad80[0x98 - 0x80];
    s32 zDiv;                   /* +0x98 */
} DrawView;

extern void RotMatrix(SVECTOR_2864 *r, MATRIX_2864 *m);
extern void GsGetLs(GsCOORDINATE2_2864 *m, MATRIX_2864 *out);
extern void GsGetLws(GsCOORDINATE2_2864 *m, MATRIX_2864 *outw, MATRIX_2864 *outs);
extern void GsSetLightMatrix(MATRIX_2864 *mp);
extern void GsSetLsMatrix(MATRIX_2864 *mp);
extern void GsSortBg(void *bg, void *ot, u16 pri);
extern void GsSortBoxFill(GsBOXF_2864 *bp, void *ot, u16 pri);
extern void GsSortSprite(GsSPRITE_2864 *sp, void *ot, u16 pri);
extern void ApplyMatrixToLVArray(void *dst, void *src, s32 count, void *m);
extern void func_80018464(void *objIn, void *otSrc, s32 otShift, void *ctxIn);

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
void Unk18Obj__DrawNode(DrawView *self, DrawNode *node)
{
    MATRIX_2864 lsBuf;
    MATRIX_2864 lwBuf;
    MATRIX_2864 *ls;
    MATRIX_2864 *lw;
    DrawNode *child;
    BasicClassListNode *cursor;
    s32 dirty;
    GsCOORDINATE2_2864 *c;
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
        sc = c->param->scale;
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
            Unk18Obj__DrawNode(self, child);
        }
    } while (cursor != NULL);

    tag = node->methods->header;
    if ((tag & 0xFF) == 0x54) {
        GsSortBg(node->u.bg, self->ot[self->buf], (1 << self->otLen) - 1);
    } else if ((tag & 0xFF) == 0x64) {
        DrawNode *b = node;
        if (b->u.boxf.relative) {
            b->u.boxf.box.x = ((self->width >> 1) * b->u.boxf.x) / 100;
            b->u.boxf.box.y = ((self->height >> 1) * b->u.boxf.y) / 100;
        } else {
            b->u.boxf.box.x = b->u.boxf.x;
            b->u.boxf.box.y = b->u.boxf.y;
        }
        GsSortBoxFill(&b->u.boxf.box, self->ot[self->buf], b->u.boxf.pri);
    } else if ((tag & 0xFF) != 0x44) {
        GsGetLws(node->obj.coord2, lw, ls);
        GsSetLightMatrix(lw);
        GsSetLsMatrix(ls);
        if (node->obj.tmd != NULL) {
            func_80018464(&node->obj, self->ot[self->buf], 14 - self->otLen, (void *)0x1F800000);
        }
    } else if ((tag & 0xFFF) == 0x144) {
        DrawNode *n = node;
        GsSPRITE_2864 *sp = &n->u.spr.sprite;
        s32 *size = &self->width;
        sp->x = (n->u.spr.ratioX != 0) ? ((size[0] >> 1) * 100) / (10000 / n->u.spr.ratioX) : 0;
        sp->y = (n->u.spr.ratioY != 0) ? ((size[1] >> 1) * 100) / (10000 / n->u.spr.ratioY) : 0;
        sp->x += sp->mx;
        sp->y += sp->my;
        GsSortSprite(sp, self->ot[self->buf], 0);
    } else {
        VECTOR_2864 pos;
        SVECTOR_2864 scr; /* never used; reserves retail's 8 unused frame bytes */
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
            GsSortSprite(&n->u.spr.sprite, self->ot[self->buf], pos.vz);
        }
    }
}
