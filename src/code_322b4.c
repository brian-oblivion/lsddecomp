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
    u8 pad04[0x8 - 0x4];
    BasicClassListNode *parentRefs; /* +0x008, BasicClass's */
    s32 unkC;  /* +0x00C, read by func_8004264C */
    s32 unk10; /* +0x010, set to 1 by func_80042658, cleared by func_80042664, read by func_8004266C */
    s32 unk14; /* +0x014, set to 1 by func_80042678, cleared by func_800425D8 */
    BasicClassListNode *unk18; /* +0x018, cleared by func_800425D8; a parentRefs cursor func_800424E0 steps past a removed parent */
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

/* Local view of a D_8006EFAC (class id 0x14) object: a Class6B5CC with
 * three FlatLightObj children at +0x44 and an ambient colour at +0x50. */
typedef struct D_8006EFACMethods D_8006EFACMethods;
typedef struct D_8006EFACObj {
    CLASS6B5CC_FIELDS(D_8006EFACMethods);
    BasicClass *lights[3]; /* +0x044, New_FlatLightObj(0..2) */
    SpriteRgb ambient;     /* +0x050, GsSetAmbient's colour >> 4 */
} D_8006EFACObj;
struct D_8006EFACMethods {
    CLASS6B5CC_SLOTS(D_8006EFACObj, (D_8006EFACObj *self));
    /* +0x0B8 */ BasicClass *(*getChild)(D_8006EFACObj *self, s32 index); /* func_80042828 */
};
extern BasicClass *New_FlatLightObj(s32 lightId);
extern void GsSetAmbient(long r, long g, long b);

/* libgs GsIMAGE (LIBGS.H), as code_2bb9c.c defines it; Sprite.h keeps only
 * the tag. InitGsSprite reads pmode, px/py and cx/cy. */
struct GsIMAGE {
    /* +0x00 */ u32 pmode;
    /* +0x04 */ s16 px;
    /* +0x06 */ s16 py;
    /* +0x08 */ u16 pw;
    /* +0x0A */ u16 ph;
    /* +0x0C */ u32 *pixel;
    /* +0x10 */ s16 cx;
    /* +0x12 */ s16 cy;
    /* +0x14 */ u16 cw;
    /* +0x16 */ u16 ch;
    /* +0x18 */ u32 *clut;
};
extern u16 GetTPage(s32 tp, s32 abr, s32 x, s32 y);

/* The method tables the getters below return. */
extern s32 D_8006EC74[];
extern s32 D_8006ED4C[];
extern s32 D_8006EED8[];
extern s32 D_8006EF50[];
extern s32 D_8006EFAC[];

/* Local view of a D_8006EED8 (class id 0xB03) object: func_800423E4 sets +0x2C. */
typedef struct D_8006EED8Methods D_8006EED8Methods;
typedef struct D_8006EED8Obj {
    D_8006EED8Methods *methods; /* +0x000 */
    u8 pad04[0x2C - 0x4];
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
    u8 pad00[0x40];
    void (*reset)(SpriteView_322b4 *self, u8 cell);            /* +0x040 = func_80041BAC (D_8006EC74) */
    u8 pad44[0xBC - 0x44];
    void (*slotBC)(SpriteView_322b4 *self, Pair_322b4 *src);   /* +0x0BC = func_80041E2C */
    u8 padC0[0xC4 - 0xC0];
    void (*setCell)(SpriteView_322b4 *self, u8 cell); /* +0x0C4 = func_80041BDC */
};

/* The zero offset func_80041DAC attaches with. */
extern Vec3_d294 D_8006EE10;
extern char *strcpy(char *dst, char *src);

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

void *func_80041ED8(void);
typedef struct CtorArg3Methods_322b4 {
    u8 pad00[0x8];
    void *(*ctor)(void *self, void *a1, void *a2, void *a3); /* +0x008 = func_80041D18 */
} CtorArg3Methods_322b4;

struct D_8006EED8Methods {
    u8 pad00[0x6C];
    void (*slot6C)(D_8006EED8Obj *self, char *name); /* +0x06C */
};

typedef struct Slot08Arg0Methods_322b4 {
    u8 pad00[0x8];
    void (*ctor)(void *self); /* +0x008 */
} Slot08Arg0Methods_322b4;

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
/* D_8006EC74 slot +0x008 (ctor): the D_8006ED4C ctor with cell 0x20's rect,
 * install the table, then reset to the caller's cell. */
void func_80041B20(SpriteView_322b4 *self, void *texture, u8 cell) {
    SpriteRect r;

    func_80041C4C(&r, 0x20);
    ((CtorArg3Methods_322b4 *)func_80041ED8())->ctor(self, texture, &r, NULL);
    self->methods = func_80041C3C();
    self->methods->reset(self, cell);
}
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
/* Allocate and construct a D_8006ED4C object (0xA8 bytes). */
void *func_80041C9C(void *a1, void *a2, void *a3) {
    void *obj = BMemPMgrAlloc(0xA8);

    if (obj != NULL) {
        ((CtorArg3Methods_322b4 *)func_80041ED8())->ctor(obj, a1, a2, a3);
        return obj;
    }
    return NULL;
}
/* D_8006ED4C slot +0x008 (ctor): the Sprite ctor with abr 0 and arg4 NULL,
 * install the table, then reset. */
void func_80041D18(Sprite *self, void *texture, SpriteRect *rect, s32 arg3) {
    GetSpriteMethods()->ctor(self, texture, 0, rect, NULL, arg3);
    self->methods = func_80041ED8();
    self->methods->reset(self);
}
/* D_8006ED4C slot +0x040 (reset): empty override. */
void func_80041DA4(Class6B5CC *self) {
}
/* D_8006EC74 and D_8006ED4C slot +0x04C (attachToParent): when not yet
 * attached, attach through Sprite's with a zero offset, then hand the
 * caller's third argument to slot +0x0BC. */
void func_80041DAC(SpriteView_322b4 *self, Class6B5CC *parent, Pair_322b4 *pos) {
    if (self->unkC == 0) {
        GetSpriteMethods()->attachToParent((Sprite *)self, parent, &D_8006EE10);
        self->methods->slotBC(self, pos);
    }
}
/* D_8006EC74 and D_8006ED4C slot +0x0BC. */
void func_80041E2C(SpriteView_322b4 *self, Pair_322b4 *src) {
    if (self->unkC != 0) {
        self->unkA0 = *src;
    }
}
/* D_8006EC74 and D_8006ED4C slot +0x0C0: when attached, move the sprite's
 * pivot: 0 centre, 1 left, 2 right, 3 top, 4 bottom. */
void func_80041E58(Sprite *self, u32 anchor) {
    if (self->parent != NULL) {
        switch (anchor) {
        case 0:
            self->sprite.mx = self->sprite.w >> 1;
            self->sprite.my = self->sprite.h >> 1;
            break;
        case 1:
            self->sprite.mx = 0;
            break;
        case 2:
            self->sprite.mx = self->sprite.w;
            break;
        case 3:
            self->sprite.my = 0;
            break;
        case 4:
            self->sprite.my = self->sprite.h;
            break;
        }
    }
}
/* Returns the D_8006ED4C method table. */
void *func_80041ED8(void) {
    return D_8006ED4C;
}
/* Allocate and construct a Sprite (0xA0 bytes). */
Sprite *New_Sprite(void *texture, s32 abr, SpriteRect *rect, void *arg3, s32 arg4) {
    Sprite *obj = BMemPMgrAlloc(0xA0);

    if (obj != NULL) {
        GetSpriteMethods()->ctor(obj, texture, abr, rect, arg3, arg4);
        return obj;
    }
    return NULL;
}
/* Sprite's reset as its ctor calls it: with all five ctor arguments (the
 * slot is Class6B5CC's, typed without them; Sprite.h, "Not settled"). */
typedef void *(*SpriteCtorReset_322b4)(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *arg4, s32 arg5);
/* gSpriteMethods slot +0x008 (ctor): the Class6B5CC ctor, install the table,
 * and hand every argument to reset. */
void *Sprite__Sprite(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *arg4, s32 arg5) {
    GetClass6B5CCMethods()->ctor((Class6B5CC *)self);
    self->methods = GetSpriteMethods();
    return ((SpriteCtorReset_322b4)self->methods->reset)(self, texture, abr, rect, arg4, arg5);
}
/* gSpriteMethods slot +0x040 (reset): bind the texture and cell, rebuild the GsSPRITE. */
void Sprite__Reset(Sprite *self, void *texture, s32 abr, SpriteRect *rect) {
    self->image = (struct GsIMAGE *)((u8 *)texture + 0x2C);
    self->rect = *rect;
    InitGsSprite(&self->sprite, abr, rect, self->image);
    self->unk58 = 0;
}
/* Fill a GsSPRITE from a texture image and a cell: colour mode and tpage
 * from the image, size and u,v from the cell, the pivot at its centre,
 * neutral colour, scale 1.0 and no rotation. */
/* Fill a GsSPRITE from a texture image and a cell: colour mode and tpage
 * from the image, size and u,v from the cell, the pivot at its centre,
 * neutral colour, scale 1.0 and no rotation. */
void InitGsSprite(SpriteGs *sprite, s32 abr, SpriteRect *rect, struct GsIMAGE *image) {
    s32 mode = image->pmode & 3;
    s32 grey = 0x80;

    sprite->attribute = mode << 24;
    sprite->x = 0;
    sprite->y = 0;
    sprite->w = rect->w;
    sprite->h = rect->h;
    sprite->mx = sprite->w >> 1;
    sprite->my = sprite->h >> 1;
    sprite->tpage = GetTPage(mode, abr, image->px, image->py);
    sprite->u = rect->u;
    sprite->v = rect->v;
    sprite->cx = image->cx;
    sprite->cy = image->cy;
    sprite->rgb.b = grey;
    sprite->rgb.g = grey;
    sprite->rgb.r = grey;
    sprite->rotate = 0;
    sprite->scalex = 0x1000;
    sprite->scaley = 0x1000;
}
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
/* D_8006EED8 slot +0x008 (ctor): the base ctor, install the table, clear
 * +0x2C, and pass a stack copy of the name to slot +0x06C. */
void func_8004232C(D_8006EED8Obj *self, char *name) {
    char buf[32];

    ((Slot08Arg0Methods_322b4 *)GetActiveDataSourceMethods())->ctor(self);
    self->methods = func_800423F0();
    self->unk2C = 0;
    if (name != NULL) {
        strcpy(buf, name);
        self->methods->slot6C(self, buf);
    }
}
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
/* D_8006EF50 slot +0x024 (removeParentRef): step the cursor past the parent
 * being removed, then the BasicClass removeParentRef. */
void func_800424E0(D_8006EF50Obj *self, BasicClass *parent) {
    if (self->unk18 != NULL && parent == self->unk18->value) {
        self->unk18 = self->unk18->next;
    }
    Get_vtable_BasicClass()->removeParentRef((BasicClass *)self, parent);
}
/* D_8006EF50 slot +0x030 (notifyParents): walk the parent refs with the
 * cursor at +0x018 (which removeParentRef keeps valid) and pass each the
 * event through its onNotify. */
void func_80042550(D_8006EF50Obj *self, s32 event) {
    BasicClass *parent;

    self->unk18 = self->parentRefs;
    for (GetNextBasicClass(&parent, &self->unk18); parent != NULL; GetNextBasicClass(&parent, &self->unk18)) {
        parent->methods->onNotify(parent, self, event);
    }
    self->unk18 = NULL;
}
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
/* D_8006EFAC slot +0x008 (ctor): the Class6B5CC ctor, install the table,
 * create and add the three flat lights, then reset. */
void func_800426E4(D_8006EFACObj *self) {
    s32 i;
    BasicClass **light;

    GetClass6B5CCMethods()->ctor((Class6B5CC *)self);
    self->methods = func_800428E4();
    for (i = 0, light = self->lights; i < 3; i++, light++) {
        *light = New_FlatLightObj(i);
        self->methods->addChild(self, *light);
    }
    self->methods->reset(self);
}
/* D_8006EFAC slot +0x00C (finalize): release the three lights, then the
 * Class6B5CC finalize. */
void func_80042790(D_8006EFACObj *self) {
    s32 i;
    BasicClass *light;

    for (i = 0; i < 3; i++) {
        light = self->methods->getChild(self, i);
        light->methods->release(light);
    }
    GetClass6B5CCMethods()->finalize((Class6B5CC *)self);
}
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
/* D_8006EFAC slot +0x0BC: set the ambient colour (swapping the old one out
 * into *rgb when asked) and hand it to GsSetAmbient. */
void func_8004283C(D_8006EFACObj *self, SpriteRgb *rgb, s32 swap) {
    SpriteRgb old;

    if (swap) {
        old = self->ambient;
        self->ambient = *rgb;
        *rgb = old;
    } else {
        self->ambient = *rgb;
    }
    GsSetAmbient((u8)self->ambient.r << 4, (u8)self->ambient.g << 4, (u8)self->ambient.b << 4);
}
/* Returns the D_8006EFAC method table. */
void *func_800428E4(void) {
    return D_8006EFAC;
}
