/*
 * code_322b4 -- GAME code carved from psyq_322b4 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x322B4..0x330F4 (vram 0x80041AB4..0x800428F4). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: methods of D_8006EC74,
 * D_8006ED4C, gSpriteMethods, D_8006EF50, D_8006EFAC, D_8006EB90, D_8006EED8,
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
#include "Sprite.h"

/* Local view of a D_8006EF50 (class id 0x5) object: only the three words its
 * +0x048..+0x058 accessors touch. */
typedef struct D_8006EF50Methods D_8006EF50Methods;
typedef struct D_8006EF50Obj {
    D_8006EF50Methods *methods; /* +0x000 */
    u8 pad04[0xC - 0x4];
    s32 unkC;  /* +0x00C, read by func_8004264C */
    s32 unk10; /* +0x010, set to 1 by func_80042658, cleared by func_80042664, read by func_8004266C */
    s32 unk14; /* +0x014, set to 1 by func_80042678, cleared by func_800425D8 */
    s32 unk18; /* +0x018, cleared by func_800425D8 */
} D_8006EF50Obj;
struct D_8006EF50Methods {
    u8 pad00[0x30];
    void (*notifyParents)(D_8006EF50Obj *self, s32 event); /* +0x030 = func_80042550 */
    u8 pad34[0x40 - 0x34];
    void (*reset)(D_8006EF50Obj *self, s32 a1); /* +0x040 = func_800425D8 */
};

/* Local view of a D_8006EC74 object: func_80041C28 reads the byte at +0xA8. */
typedef struct D_8006EC74Obj {
    u8 pad00[0xA8];
    u8 unkA8;
} D_8006EC74Obj;

/* Local view of a D_8006EFAC/D_800866E8 object: a child array at +0x44. */
typedef struct ChildArrayObj_322b4 {
    u8 pad00[0x44];
    void *children[1];
} ChildArrayObj_322b4;

/* The method tables the getters below return. */
extern s32 D_8006EC74[];
extern s32 D_8006ED4C[];
extern s32 D_8006EED8[];
extern s32 D_8006EF50[];
extern s32 D_8006EFAC[];

/* Local view of a D_8006EED8 (class id 0xB03) object: func_800423E4 sets +0x2C. */
typedef struct D_8006EED8Obj {
    u8 pad00[0x2C];
    s32 unk2C;
} D_8006EED8Obj;

/* Local view of a D_8006EC74/D_8006ED4C object (subclasses of Sprite,
 * include/Sprite.h) as their round-82 methods see it. attribute and u,v are
 * Sprite's sprite.attribute/u/v; this view is the subclasses' to unify. */
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

/* The cell origin func_80041C4C copies: {0, 0, 8, 8}. */
extern SpriteRect D_8006ED40;

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
void *func_800423F0(void);

void *func_80041C3C(void);
typedef struct CellCtorMethods_322b4 {
    u8 pad00[0x8];
    void *(*ctor)(void *self, void *texture, u8 cell); /* +0x008 = func_80041B20 */
} CellCtorMethods_322b4;

typedef struct CtorArg1Methods_322b4 {
    u8 pad00[0x8];
    void (*ctor)(void *self, s32 arg); /* +0x008 */
} CtorArg1Methods_322b4;
void *func_800428E4(void);
void func_80041C4C(SpriteRect *dst, u32 cell);

/* Allocate and construct a D_8006EC74 object (0xAC bytes): one cell. */
void *func_80041AB4(void *texture, u8 cell) {
    void *obj = BMemPMgrAlloc(0xAC);

    if (obj != NULL) {
        ((CellCtorMethods_322b4 *)func_80041C3C())->ctor(obj, texture, cell);
        return obj;
    }
    return NULL;
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041B20);
/* D_8006EC74 slot +0x040 (reset): re-select the cell through slot +0x0C4. */
void func_80041BAC(SpriteView_322b4 *self, u8 cell) {
    self->methods->setCell(self, cell);
}
/* D_8006EC74 slot +0x0C4: store the cell index and point u,v at its 8x8 cell. */
void func_80041BDC(SpriteView_322b4 *self, u8 cell) {
    SpriteRect r;

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
void func_80041C4C(SpriteRect *dst, u32 cell) {
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
INCLUDE_ASM("asm/nonmatchings/code_322b4", New_Sprite);
INCLUDE_ASM("asm/nonmatchings/code_322b4", Sprite__Sprite);
/* gSpriteMethods slot +0x040 (reset): bind the texture and cell, rebuild the GsSPRITE. */
void Sprite__Reset(Sprite *self, void *texture, s32 abr, SpriteRect *rect) {
    self->image = (struct GsIMAGE *)((u8 *)texture + 0x2C);
    self->rect = *rect;
    InitGsSprite(&self->sprite, abr, rect, self->image);
    self->unk58 = 0;
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", InitGsSprite);
INCLUDE_ASM("asm/nonmatchings/code_322b4", Sprite__UpdateRotation);
/* Sprite classes slot +0x060: display on/off (attribute bit 31, inverted). */
s32 Sprite__SetDisplay(Sprite *self, s32 a1) {
    return GetSetBitField(&self->sprite.attribute, 0x1F, 1, a1 == 0) == 0;
}
/* Sprite classes slot +0x064: attribute bit 30. */
s32 Sprite__SetSemiTrans(Sprite *self, s32 a1) {
    return GetSetBitField(&self->sprite.attribute, 0x1E, 1, a1 != 0);
}
/* Sprite classes slot +0x068: attribute bits 28..29. */
s32 Sprite__SetSemiTransRate(Sprite *self, s32 a1) {
    return GetSetBitField(&self->sprite.attribute, 0x1C, 2, a1);
}
/* D_8006EB90 and D_8006EC74 slot +0x098 (update): empty override. */
void Sprite__Update(Sprite *self, void *sender, s32 event) {
}
/* Slot +0x0B8 of D_8006EC74, D_8006ED4C, gSpriteMethods and D_800879C4 (the
 * sprite classes): copy three bytes into the embedded GsSPRITE's r,g,b. */
void Sprite__SetColor(Sprite *self, SpriteRgb *rgb) {
    self->sprite.rgb = *rgb;
}
/* Returns the gSpriteMethods method table. */
SpriteMethods *GetSpriteMethods(void) {
    return &gSpriteMethods;
}
/* Allocate and construct a D_8006EED8 object (0x30 bytes). */
void *func_800422CC(s32 arg) {
    void *obj = BMemPMgrAlloc(0x30);

    if (obj != NULL) {
        ((CtorArg1Methods_322b4 *)func_800423F0())->ctor(obj, arg);
        return obj;
    }
    return NULL;
}
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
/* Allocate and construct a D_8006EF50 object (0x1C bytes). */
void *func_80042400(void) {
    void *obj = BMemPMgrAlloc(0x1C);

    if (obj != NULL) {
        ((Slot08Methods_322b4 *)func_80042684())->init(obj);
        return obj;
    }
    return NULL;
}
/* D_8006EF50 slot +0x008 (ctor): the BasicClass ctor, install the table, reset(0). */
void func_80042450(D_8006EF50Obj *self) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = func_80042684();
    self->methods->reset(self, 0);
}
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
/* D_8006EF50 slot +0x044: notify event 4 if unk14, else 3 if unk10, else
 * count unkC up and notify 2. */
void func_800425EC(D_8006EF50Obj *self) {
    s32 event;

    if (self->unk14 != 0) {
        event = 4;
    } else if (self->unk10 != 0) {
        event = 3;
    } else {
        self->unkC++;
        event = 2;
    }
    self->methods->notifyParents(self, event);
}
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
/* Allocate and construct a D_8006EFAC object (0x54 bytes). */
void *func_80042694(void) {
    void *obj = BMemPMgrAlloc(0x54);

    if (obj != NULL) {
        ((Slot08Methods_322b4 *)func_800428E4())->init(obj);
        return obj;
    }
    return NULL;
}
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
