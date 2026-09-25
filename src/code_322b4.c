/*
 * code_322b4 -- GAME code carved from psyq_322b4 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x322B4..0x330F4 (vram 0x80041AB4..0x800428F4). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: methods of D_8006EC74,
 * D_8006ED4C, D_8006EE1C, D_8006EF50, D_8006EFAC, D_8006EB90, D_8006EED8,
 * D_800879C4 and D_800866E8, calling GetClass6B5CCMethods, GetSetBitField and
 * the BasicClass framework. Owns jtbl_80011290 (attached rodata sub-slot
 * 0x1A90).
 *
 * Round 82 matched the one- to eight-word bodies (getters, accessors, empty
 * overrides), then a third batch of 11- to 20-word bodies (the sprite
 * attribute-bit setters, cell selection, finalize chains, the D_8006EF50
 * allocator); the remaining bodies are still INCLUDE_ASM.
 */
#include "common.h"
#include "Class6B5CC.h"

/* Local view of a D_8006EF50 (class id 0x5) object: only the three words its
 * +0x048..+0x058 accessors touch. */
typedef struct D_8006EF50Obj {
    u8 pad00[0xC];
    s32 unkC;  /* +0x00C, read by func_8004264C */
    s32 unk10; /* +0x010, set to 1 by func_80042658, cleared by func_80042664, read by func_8004266C */
    s32 unk14; /* +0x014, set to 1 by func_80042678, cleared by func_800425D8 */
    s32 unk18; /* +0x018, cleared by func_800425D8 */
} D_8006EF50Obj;

/* Local view of a D_8006EC74 object: func_80041C28 reads the byte at +0xA8. */
typedef struct D_8006EC74Obj {
    u8 pad00[0xA8];
    u8 unkA8;
} D_8006EC74Obj;

/* Local view of a sprite object (D_8006EC74/ED4C/EE1C/879C4): the embedded
 * GsSPRITE's r,g,b bytes at +0x78..+0x7A. */
typedef struct Rgb_322b4 {
    s8 r, g, b;
} Rgb_322b4;
typedef struct SpriteObj_322b4 {
    u8 pad00[0x78];
    Rgb_322b4 rgb;
} SpriteObj_322b4;

/* Local view of a D_8006EFAC/D_800866E8 object: a child array at +0x44. */
typedef struct ChildArrayObj_322b4 {
    u8 pad00[0x44];
    void *children[1];
} ChildArrayObj_322b4;

/* The method tables the getters below return. */
extern s32 D_8006EC74[];
extern s32 D_8006ED4C[];
extern s32 D_8006EE1C[];
extern s32 D_8006EED8[];
extern s32 D_8006EF50[];
extern s32 D_8006EFAC[];

/* Local view of a D_8006EED8 (class id 0xB03) object: func_800423E4 sets +0x2C. */
typedef struct D_8006EED8Obj {
    u8 pad00[0x2C];
    s32 unk2C;
} D_8006EED8Obj;

/* Local view of a sprite-class object (D_8006EC74/ED4C/EE1C/879C4/EB90) as
 * the round-82 batch-3 methods see it: an embedded GsSPRITE at +0x064 whose
 * attribute word is the packed flags GetSetBitField edits, and whose u,v
 * bytes sit at +0x072/+0x073. */
typedef struct SpriteMethods_322b4 SpriteMethods_322b4;
typedef struct Pair_322b4 {
    s32 a, b;
} Pair_322b4;
typedef struct SpriteView_322b4 {
    SpriteMethods_322b4 *methods; /* +0x000 */
    u8 pad04[0xC - 0x4];
    s32 unkC;                     /* +0x00C, tested by func_80041E2C */
    u8 pad10[0x64 - 0x10];
    u32 attribute;                /* +0x064, GsSPRITE.attribute */
    u8 pad68[0x72 - 0x68];
    u8 u;                         /* +0x072, GsSPRITE.u */
    u8 v;                         /* +0x073, GsSPRITE.v */
    u8 pad74[0xA0 - 0x74];
    Pair_322b4 unkA0;             /* +0x0A0, set by func_80041E2C */
    u8 unkA8;                     /* +0x0A8, the cell index func_80041BDC stores */
} SpriteView_322b4;
struct SpriteMethods_322b4 {
    u8 pad00[0xC4];
    void (*setCell)(SpriteView_322b4 *self, u8 cell); /* +0x0C4 = func_80041BDC */
};

/* The 12-byte record func_80041C4C copies from D_8006ED40 = {0, 0, 8, 8}. */
typedef struct CellRect_322b4 {
    u16 u, v;
    s32 w, h;
} CellRect_322b4;
extern CellRect_322b4 D_8006ED40;

extern u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value);
extern void *BMemPMgrAlloc(s32 size);

typedef struct Slot0CMethods_322b4 {
    u8 pad00[0xC];
    void (*slot0C)(void *self); /* +0x00C */
} Slot0CMethods_322b4;
extern Slot0CMethods_322b4 *GetActiveDataSourceMethods(void);

typedef struct Slot08Methods_322b4 {
    u8 pad00[0x8];
    void (*init)(void *self); /* +0x008 */
} Slot08Methods_322b4;
void *func_80042684(void);
void func_80041C4C(CellRect_322b4 *dst, u32 cell);

INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041AB4);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041B20);
/* D_8006EC74 slot +0x040 (reset): re-select the cell through slot +0x0C4. */
void func_80041BAC(SpriteView_322b4 *self, u8 cell) {
    self->methods->setCell(self, cell);
}
/* D_8006EC74 slot +0x0C4: store the cell index and point u,v at its 8x8 cell. */
void func_80041BDC(SpriteView_322b4 *self, u8 cell) {
    CellRect_322b4 r;

    self->unkA8 = cell;
    func_80041C4C(&r, cell);
    self->u = r.u;
    self->v = r.v;
}
/* D_8006EC74 slot +0x0C8: read the byte at +0x0A8. */
u8 func_80041C28(D_8006EC74Obj *self) {
    u8 pad[16]; /* unused: it is what gives retail its 0x10-byte frame */

    return self->unkA8;
}
/* Returns the D_8006EC74 method table. */
void *func_80041C3C(void) {
    return D_8006EC74;
}
/* Cell index -> 8x8 rect in a 32-wide grid, offset from D_8006ED40. */
void func_80041C4C(CellRect_322b4 *dst, u32 cell) {
    *dst = D_8006ED40;
    cell &= 0xFF;
    dst->u += (cell & 0x1F) * 8;
    dst->v += (cell >> 5) * 8;
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041C9C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041D18);
/* D_8006ED4C slot +0x040 (reset): empty override. */
void func_80041DA4(Class6B5CC *self) {
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041DAC);
/* D_8006EC74 and D_8006ED4C slot +0x0BC. */
void func_80041E2C(SpriteView_322b4 *self, Pair_322b4 *src) {
    if (self->unkC != 0) {
        self->unkA0 = *src;
    }
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041E58);
/* Returns the D_8006ED4C method table. */
void *func_80041ED8(void) {
    return D_8006ED4C;
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041EE8);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041F88);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_8004202C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_8004208C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042170);
/* Sprite classes slot +0x060: display on/off (attribute bit 31, inverted). */
s32 func_8004220C(SpriteView_322b4 *self, s32 a1) {
    return GetSetBitField(&self->attribute, 0x1F, 1, a1 == 0) == 0;
}
/* Sprite classes slot +0x064: attribute bit 30. */
s32 func_8004223C(SpriteView_322b4 *self, s32 a1) {
    return GetSetBitField(&self->attribute, 0x1E, 1, a1 != 0);
}
/* Sprite classes slot +0x068: attribute bits 28..29. */
s32 func_80042268(SpriteView_322b4 *self, s32 a1) {
    return GetSetBitField(&self->attribute, 0x1C, 2, a1);
}
/* D_8006EB90 and D_8006EC74 slot +0x098 (update): empty override. */
void func_80042294(Class6B5CC *self, void *sender, s32 event) {
}
/* Slot +0x0B8 of D_8006EC74, D_8006ED4C, D_8006EE1C and D_800879C4 (the
 * sprite classes): copy three bytes into the embedded GsSPRITE's r,g,b. */
void func_8004229C(SpriteObj_322b4 *self, Rgb_322b4 *rgb) {
    self->rgb = *rgb;
}
/* Returns the D_8006EE1C method table. */
void *func_800422BC(void) {
    return D_8006EE1C;
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800422CC);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_8004232C);
/* D_8006EED8 slot +0x00C (finalize): clear +0x2C, then the base finalize. */
void func_800423A8(D_8006EED8Obj *self) {
    self->unk2C = 0;
    GetActiveDataSourceMethods()->slot0C(self);
}
/* D_8006EED8 slot +0x064. */
void func_800423E4(D_8006EED8Obj *self) {
    self->unk2C = 1;
}
/* Returns the D_8006EED8 method table. */
void *func_800423F0(void) {
    return D_8006EED8;
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042400);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042450);
/* D_8006EF50 slot +0x00C (finalize): the BasicClass finalize. */
void func_800424A8(BasicClass *self) {
    Get_vtable_BasicClass()->finalize(self);
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800424E0);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042550);
/* D_8006EF50 slot +0x040 (reset). */
void func_800425D8(D_8006EF50Obj *self, s32 a1) {
    self->unkC = a1;
    self->unk14 = 0;
    self->unk10 = 0;
    self->unk18 = 0;
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800425EC);
/* D_8006EF50 slot +0x048. */
s32 func_8004264C(D_8006EF50Obj *self) {
    return self->unkC;
}
/* D_8006EF50 slot +0x04C. */
void func_80042658(D_8006EF50Obj *self) {
    self->unk10 = 1;
}
/* D_8006EF50 slot +0x050. */
void func_80042664(D_8006EF50Obj *self) {
    self->unk10 = 0;
}
/* D_8006EF50 slot +0x054. */
s32 func_8004266C(D_8006EF50Obj *self) {
    return self->unk10;
}
/* D_8006EF50 slot +0x058. */
void func_80042678(D_8006EF50Obj *self) {
    self->unk14 = 1;
}
/* Returns the D_8006EF50 method table. */
void *func_80042684(void) {
    return D_8006EF50;
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042694);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800426E4);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042790);
/* D_8006EFAC slot +0x040 (reset): mark the coordinate for recompute. */
void func_80042814(Class6B5CC *self) {
    self->coord2->flg = 0;
}
/* D_8006EFAC slot +0x09C (dispatchLinkCommand): empty override. */
void func_80042820(Class6B5CC *self, void *sender, s32 event) {
}
/* D_8006EFAC and D_800866E8 slot +0x0B8 (getChild). */
void *func_80042828(ChildArrayObj_322b4 *self, s32 index) {
    return self->children[index];
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_8004283C);
/* Returns the D_8006EFAC method table. */
void *func_800428E4(void) {
    return D_8006EFAC;
}
