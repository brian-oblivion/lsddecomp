/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * class_3bb8c_o -- functions 54..73 of the 113-function `class_3bb8c_n`
 * remainder, 0x475F0..0x47CC4 (vram 0x80056DF0..0x800574C4).  Carved round 17
 * (2026-09-04); `class_3bb8c_n` keeps its name for the 54 functions in front
 * of this slice and `class_3bb8c_q` is the 19-function tail behind
 * `class_3bb8c_p`.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 19 of the 20 clean.
 *
 * func_80056F5C: was gp_rel. MATCHED round 44, 34/34, first build.
 *
 * Owns NO switch jump table -- zero `jtbl_` references in the slice -- so no
 * rodata sub-slot is attached to this unit.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM addresses,
 * not at class boundaries, and round 15 measured three of five such slices
 * spanning two or more vtables.  Identify each class with tools/classtable.py
 * rather than assuming the unit has one.  A class that spans a carve boundary
 * is also the normal reason two units name the same table -- see the
 * multiple-independent-local-views convention in CLAUDE.md before deciding
 * whether your view of one belongs in include/class_3bb8c.h or here.
 *
 * Confirmed round 17 (runner bravo): the slice really does span (at least)
 * two classes.
 *
 *  - LinkOwnerObj__ReleaseLinks/LinkOwnerObj__RandomizeLinks/LinkOwnerObj__ReleaseLinksB operate on a DIFFERENT,
 *    larger object (fields observed at +0x84/+0x88, an inline 5-element
 *    `BasicClass *` array) that is unrelated to the class below -- no
 *    shared header claims it, so it is kept purely local to this file
 *    (`LinkOwnerObj`/`LinkElemObj`).
 *  - func_80057044 onward implement the SAME shared intermediate base
 *    class already known from two independent angles: `code_55dd4.h`'s
 *    `D800878D4Methods` (Class65650's own reading, resolved via its own
 *    getter `func_80057C84`) and `DreamSys.h`'s `vtable_DreamSys` (whose
 *    +0x010/+0x014/+0x0B8/+0x0BC slots already name func_800570B4/
 *    func_80057130/func_80057384/func_800573A8 as the shared occupants).
 *    This unit is where those functions are actually DEFINED, so it earns
 *    its own local view (`BaseObjO`/`BaseObjOMethods`) rather than
 *    extending either sibling header -- neither is this unit's to edit,
 *    and the ctor's own dispatch through `func_8001E57C()` needs a
 *    non-void, checkable return that `class_3bb8c.h`'s existing
 *    `BaseCtorTableB_3bb8c_c` (ctor typed `void`) cannot provide (see
 *    that header's own note on `func_8001E57C`'s per-call-site typing).
 *  - `func_80056F4C` is this class's SIBLING table's own getter (returns
 *    `&D_800876FC`, exactly analogous to `func_80057C84`/`func_80066818`
 *    already documented in code_55dd4.h) -- `D_800876FC` shares this same
 *    class's +0x010/+0x014/+0x018/+0x088/+0x09C/+0x0B8/+0x0BC/+0x0C0/+0x0C4
 *    slots with the functions below (confirmed with
 *    `tools/classtable.py D_800876FC`), so it is typed `BaseObjOMethods *`.
 */
#include "common.h"

void Noop(void) {
}

/* ------------------------------------------------------------------ *
 * Group 1: LinkOwnerObj__ReleaseLinks / LinkOwnerObj__RandomizeLinks / LinkOwnerObj__ReleaseLinksB.
 * Self is some larger object with an inline 5-element `BasicClass *`
 * array at +0x084.  LinkOwnerObj__ReleaseLinks/LinkOwnerObj__ReleaseLinksB release the whole array
 * (ReleaseBasicClassArray, already established elsewhere as
 * `void ReleaseBasicClassArray(BasicClass **array, s32 count)` in code_8220_b.c --
 * kept generic `void **` here per this project's per-unit convention for
 * that symbol, e.g. code_2cc8c.h's own looser reading).  LinkOwnerObj__RandomizeLinks
 * walks array indices [1..4] (self+0x88 .. self+0x94), which is exactly
 * inside the same 5-element array, and for each element calls its own
 * vtable slot +0x048 with a random Vec3-ish table entry, then sets the
 * element's own +0x084 field to a random "angle" value
 * (`(rand() % 360) << 12`, a degrees->fixed-point conversion).
 * ------------------------------------------------------------------ */

extern void ReleaseBasicClassArray(void **array, s32 count);
extern s32 rand(void);

typedef struct Vec3O {
    s32 x, y, z;
} Vec3O;

/* A rand()-indexed table of 6 Vec3-shaped entries, passed to each link
 * element's own slot48. */
extern Vec3O D_8008788C[];

typedef struct LinkElemObj LinkElemObj;
typedef struct LinkElemMethods {
    u8 pad0[0x48];
    void (*slot48)(LinkElemObj *self, s32 arg1, Vec3O *arg2); /* +0x048 */
} LinkElemMethods;
struct LinkElemObj {
    LinkElemMethods *methods; /* +0x000 */
    u8 pad4[0x80];             /* +0x004 .. +0x083, unknown */
    s32 unk84;                   /* +0x084, a random "angle" set by LinkOwnerObj__RandomizeLinks */
};

typedef struct LinkOwnerObj {
    u8 pad0[0x84];              /* +0x000 .. +0x083, unknown */
    LinkElemObj *arr84[5];        /* +0x084 .. +0x097 */
} LinkOwnerObj;

void LinkOwnerObj__ReleaseLinks(LinkOwnerObj *this) {
    ReleaseBasicClassArray((void **)this->arr84, 5);
}

extern void func_80056D18(void *arg0, s32 arg1, s32 arg2, s32 arg3);

void LinkOwnerObj__func_56e1c(void *this) {
    func_80056D18(this, 0, 0, 0);
}

void LinkOwnerObj__RandomizeLinks(LinkOwnerObj *this) {
    LinkElemObj **p = &this->arr84[1];
    s32 i;

    for (i = 0; i < 4; i++, p++) {
        u32 r = rand();

        (*p)->methods->slot48(*p, 1, &D_8008788C[r % 6]);
        (*p)->unk84 = (rand() % 360) << 12;
    }
}

void LinkOwnerObj__ReleaseLinksB(LinkOwnerObj *this) {
    ReleaseBasicClassArray((void **)this->arr84, 5);
}

/* ------------------------------------------------------------------ *
 * Group 2: func_80056F4C onward -- the shared intermediate base class,
 * see the file banner.  BaseObjO/BaseObjOMethods is THIS unit's own view
 * (established here, not copied from either sibling header).
 * ------------------------------------------------------------------ */

typedef struct BaseObjO BaseObjO;
typedef struct BaseObjOMethods BaseObjOMethods;

/* arg passed to slot10/slot14 (the link/unlink pair): only its own vtable
 * HEADER WORD (masked, not just the tag byte) is read, to classify it as
 * one of two companion-object kinds. */
typedef struct TagWordMethodsO {
    s32 header; /* +0x000 */
} TagWordMethodsO;
typedef struct TagWordObjO {
    TagWordMethodsO *methods;
} TagWordObjO;

/* A different tag check (func_80057320's arg1, and self->unk28 in
 * func_800571F8): only the vtable header's LOW BYTE is read. */
typedef struct TagByteObjO TagByteObjO;
typedef struct TagByteMethodsO {
    u8 tag;                              /* +0x000 */
    u8 pad1[0xE8 - 0x1];                   /* +0x001 .. +0x0E7, unknown */
    void (*slotE8)(TagByteObjO *self);       /* +0x0E8 */
} TagByteMethodsO;
struct TagByteObjO {
    TagByteMethodsO *methods;
};

/* Scratch buffer func_800571F8 builds on its own stack and forwards to
 * slot8C/slot90/func_8001F66C.  Retail's own frame layout requires it to
 * be 0x38 bytes (sp+0x10 .. sp+0x47, with the saved registers starting at
 * sp+0x48) -- a plain `Vec3O` (0xC bytes) undersizes the frame and
 * shifts everything after this function.  Field layout beyond that is
 * unestablished. */
typedef struct Buf38O {
    u8 raw[0x38];
} Buf38O;

/* self->unk14's pointee (func_800573CC): +0x000 is a word cleared after
 * the vector at +0x018 is written/accumulated into. */
typedef struct Unk14ObjO {
    s32 unk0;      /* +0x000 */
    u8 pad4[0x14];   /* +0x004 .. +0x017, unknown */
    Vec3O vec18;      /* +0x018 .. +0x023 */
} Unk14ObjO;

/* The FIXED base table returned by func_8001E57C(), which -- MEASURED
 * elsewhere (class_3bb8c.h, code_d294.h) -- takes no real arguments and
 * always returns the same global regardless of what garbage is in $a0 at
 * the call site.  This unit's own reading needs slots +0x008 (ctor,
 * checked against NULL -- so unlike class_3bb8c.h's existing
 * `BaseCtorTableB_3bb8c_c` this one is NOT void), +0x010, +0x014, +0x018
 * and +0x088. */
typedef struct FixedBaseTable {
    u8 pad0[0x8];                                     /* +0x000 .. +0x007 */
    void *(*ctor)(void *self);                          /* +0x008 */
    u8 pad0C[0x10 - 0xC];                                 /* +0x00C .. +0x00F */
    void (*slot10)(BaseObjO *self, TagWordObjO *arg);       /* +0x010 */
    void (*slot14)(BaseObjO *self, TagWordObjO *arg);        /* +0x014 */
    void (*slot18)(BaseObjO *self);                             /* +0x018 */
    u8 pad1C[0x88 - 0x1C];                                        /* +0x01C .. +0x087 */
    void (*slot88)(BaseObjO *self, s32 arg1);                       /* +0x088 */
} FixedBaseTable;

extern FixedBaseTable *func_8001E57C(void);

struct BaseObjOMethods {
    s32 header;                                       /* +0x000 */
    void *unk04;                                         /* +0x004 */
    BaseObjO *(*ctor)(BaseObjO *self);                     /* +0x008 func_80057044 (this unit) */
    void (*dtor)(BaseObjO *self);                            /* +0x00C */
    void (*slot10)(BaseObjO *self, TagWordObjO *arg);          /* +0x010 func_800570B4 (this unit) */
    void (*slot14)(BaseObjO *self, TagWordObjO *arg);            /* +0x014 func_80057130 (this unit) */
    void (*slot18)(BaseObjO *self);                                 /* +0x018 func_800571A8 (this unit) */
    u8 pad1C[0x40 - 0x1C];                                            /* +0x01C .. +0x03F */
    void (*slot40)(BaseObjO *self);                                     /* +0x040, called by func_80057044's own ctor; occupant outside this unit */
    u8 pad44[0x80 - 0x44];                                                /* +0x044 .. +0x07F */
    void *(*slot80)(BaseObjO *self, s32 arg1);                              /* +0x080, called by func_80056F5C (this unit); occupant outside this unit (D_800876FC's slot80 is func_8001D4AC, a BasicClass-range function) */
    u8 pad84[0x8C - 0x84];                                                /* +0x084 .. +0x08B */
    void (*slot8C)(BaseObjO *self, Buf38O *arg1);                            /* +0x08C, called by func_800571F8 */
    void (*slot90)(BaseObjO *self, Buf38O *arg1, s32 arg2);                    /* +0x090, called by func_800571F8 */
    u8 pad94[0xBC - 0x94];                                                       /* +0x094 .. +0x0BB */
    void (*slotBC)(BaseObjO *self, Vec3O *arg1);                                  /* +0x0BC func_800573A8 (this unit), called by func_80057444 */
};

struct BaseObjO {
    BaseObjOMethods *methods; /* +0x000 */
    u8 pad4[0x10];              /* +0x004 .. +0x013, unknown */
    Unk14ObjO *unk14;             /* +0x014 */
    u8 pad18[0x8];                  /* +0x018 .. +0x01F, unknown */
    void *unk20;                      /* +0x020, checked non-NULL and passed to func_8001F3A4 */
    u8 pad24[0x4];                      /* +0x024 .. +0x027, unknown */
    TagByteObjO *unk28;                   /* +0x028 */
    u8 pad2C[0x44 - 0x2C];                  /* +0x02C .. +0x043, unknown */
    s32 unk44;                                /* +0x044 */
    s16 unk48;                                  /* +0x048 */
    u8 pad4A[0x2];                                /* +0x04A .. +0x04B, unknown */
    TagWordObjO *unk4C;                             /* +0x04C, companion-object pointer #1 (header&0xFFF==0x114) */
    TagWordObjO *unk50;                               /* +0x050, companion-object pointer #2 (header&0xF==5) */
    s32 unk54;                                          /* +0x054 */
};

extern BaseObjOMethods D_800876FC;

BaseObjOMethods *func_80056F4C(void) {
    return &D_800876FC;
}

extern s32 D_8008ACA4;
extern s32 D_8008ACA8;
extern s32 D_8008ACAC;
extern s32 D_8008AB98[3];
extern s32 D_8008AB94;

extern void func_80020510(void *arg0, void *arg1);

void func_80056F5C(s32 arg0, BaseObjO *self, s32 arg2, s32 arg3) {
    s32 i;
    void *ret;

    D_8008ACA4 = (s32) self;
    D_8008ACA8 = arg2;
    D_8008ACAC = arg3;
    i = 0;
    do {
        ret = self->methods->slot80(self, D_8008AB98[i]);
        func_80020510(ret, &D_8008AB94);
        i++;
    } while (i < 2);
}

extern void *func_80017B34(s32 size);
extern void *func_80017CFC(void *ptr);
extern BaseObjOMethods *func_80057C84(void);

void *func_80056FE4(void) {
    BaseObjO *self = func_80017B34(0x58);

    if (self != NULL) {
        if (func_80057C84()->ctor(self) != NULL) {
            return self;
        }
        func_80017CFC(self);
        return NULL;
    }
    return NULL;
}

BaseObjO *func_80057044(BaseObjO *self) {
    if (func_8001E57C()->ctor(self) == NULL) {
        goto fail;
    }
    self->methods = func_80057C84();
    self->unk44 = 0;
    self->unk4C = NULL;
    self->unk50 = NULL;
    self->methods->slot40(self);
    return self;
fail:
    return NULL;
}

void func_800570B4(BaseObjO *self, TagWordObjO *arg) {
    s32 tag;

    func_8001E57C()->slot10(self, arg);
    tag = arg->methods->header;
    if ((tag & 0xFFF) == 0x114) {
        self->unk4C = arg;
    } else if ((tag & 0xF) == 5) {
        self->unk50 = arg;
    }
}

void func_80057130(BaseObjO *self, TagWordObjO *arg) {
    s32 tag = arg->methods->header;

    if ((tag & 0xFFF) == 0x114) {
        self->unk4C = NULL;
    } else if ((tag & 0xF) == 5) {
        self->unk50 = NULL;
    }
    func_8001E57C()->slot14(self, arg);
}

void func_800571A8(BaseObjO *self) {
    self->unk4C = NULL;
    self->unk50 = NULL;
    func_8001E57C()->slot18(self);
}

void func_800571E8(BaseObjO *self) {
    self->unk48 = 0x12C;
    self->unk54 = 0;
}

extern s32 func_8001F3A4(void *arg0);
extern void func_8001F66C(Buf38O *out, s32 arg1, s32 arg2, s32 arg3);

void func_800571F8(BaseObjO *self, s32 arg1) {
    func_8001E57C()->slot88(self, arg1);
    /* Written as two nested guards, not a combined `arg1 >= 5 && arg1 < 9`
     * range test -- the combined form optimizes into a single unsigned
     * `(arg1-5) < 4` comparison, which is not what retail does (two
     * separate `slti`s). */
    if (arg1 < 9) {
        if (arg1 >= 5) {
            Buf38O buf;

            if (self->unk20 != NULL && func_8001F3A4(self->unk20)) {
                self->methods->slot8C(self, &buf);
                if (arg1 != 5) {
                    s16 h = self->unk48;
                    s32 isSeven = (arg1 == 7);
                    s32 nonneg = (h >= 0);
                    s32 adjusted;

                    /* `goto`, not `if/else`, to match retail's actual
                     * branch shape (see the match report). */
                    if (h < 0) {
                        goto negative;
                    }
                    adjusted = h + self->unk54;
                    goto joinAdjust;
                negative:
                    adjusted = h - self->unk54;
                joinAdjust:
                    func_8001F66C(&buf, isSeven, nonneg, adjusted);
                }
                self->methods->slot90(self, &buf, arg1);
                if (self->unk28 != NULL) {
                    if (self->unk28->methods->tag == 0x34) {
                        self->unk28->methods->slotE8(self->unk28);
                    }
                }
            }
        }
    }
}

typedef struct DispatchObjO DispatchObjO;
typedef struct DispatchObjOMethods {
    u8 pad0[0xDC];                        /* +0x000 .. +0x0DB, unknown */
    void (*slotDC)(DispatchObjO *self);      /* +0x0DC */
    void (*slotE0)(DispatchObjO *self);        /* +0x0E0 */
} DispatchObjOMethods;
struct DispatchObjO {
    DispatchObjOMethods *methods;
};

void func_80057320(DispatchObjO *self, TagByteObjO *arg1) {
    if (arg1->methods->tag == 0x34) {
        self->methods->slotDC(self);
    } else if (arg1->methods->tag == 0x24) {
        self->methods->slotE0(self);
    }
}

extern void func_800573CC(BaseObjO *self, s32 flag, Vec3O *v);

void func_80057384(BaseObjO *self, Vec3O *arg1) {
    func_800573CC(self, 1, arg1);
}

void func_800573A8(BaseObjO *self, Vec3O *arg1) {
    func_800573CC(self, 0, arg1);
}

void func_800573CC(BaseObjO *self, s32 flag, Vec3O *v) {
    BaseObjO *t = self;
    Unk14ObjO *u = t->unk14;

    if (flag) {
        u->vec18 = *v;
    } else {
        u->vec18.x += v->x;
        u->vec18.y += v->y;
        u->vec18.z += v->z;
    }
    t->unk14->unk0 = 0;
}

extern void Class6B5CC__RotateLocalVector(BaseObjO *self, Vec3O *dst, s16 *src);

void func_80057444(BaseObjO *self, s16 *arg1) {
    Vec3O buf;

    Class6B5CC__RotateLocalVector(self, &buf, arg1);
    self->methods->slotBC(self, &buf);
}

extern s32 D_8008ABA8;
extern void func_80057534(BaseObjO *self, void *arg0, s32 arg1, s32 arg2, s32 arg3);

void func_8005748C(BaseObjO *self, s32 arg1, s32 arg2) {
    func_80057534(self, &D_8008ABA8, arg1, arg2, 6);
}
