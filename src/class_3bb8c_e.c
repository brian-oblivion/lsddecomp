#include "common.h"

/*
 * class_3bb8c_e (round 14): 19 functions carved from the same 305-function
 * class_3bb8c remainder segment as class_3bb8c_b/_c/_d/_f, but they operate
 * on a class UNRELATED to Obj866E8/D_800866E8 (include/class_3bb8c.h) --
 * none of these 19 functions read or write anything typed there, and no
 * function in _b/_c/_d/_f calls into this unit (checked: the only cross-unit
 * calls are FROM this unit INTO class_3bb8c_f's still-INCLUDE_ASM
 * func_8004F32C/func_8004F394/func_8004F3BC/func_8004F3E4/func_8004F40C/
 * func_8004F4A4, never the reverse). Kept entirely local to this .c file
 * rather than include/class_3bb8c.h, to carry zero collision risk with the
 * other two runners (charlie on _f, echo on _d) editing that shared header
 * this round.
 *
 * The object derives from the same BasicClass framework documented in
 * include/code_8220.h (base vtable fetched via a no-argument getter,
 * func_80018390(), with finalize/addChild/removeChild/removeAllChildren at
 * +0x00C/+0x010/+0x014/+0x018) -- this unit's own independent local view of
 * that same getter, per the project's established
 * multiple-independent-local-views convention (docs/DECOMPILATION_LEARNINGS.md).
 */
typedef struct BaseMethods3bb8cE BaseMethods3bb8cE;
struct BaseMethods3bb8cE {
    u8 pad000[0x00C];
    void (*finalize)(void *self);                 /* +0x00C, func_8004E40C */
    void (*addChild)(void *self, void *child);    /* +0x010, func_8004E444 */
    void (*removeChild)(void *self, void *child); /* +0x014, func_8004E4E8 */
    void (*removeAllChildren)(void *self);        /* +0x018, func_8004E588 */
};
extern BaseMethods3bb8cE *func_80018390(void);

/*
 * A typed child resource attached to a Node3bb8cE. Only the resource's own
 * header/type-tag word (methods->header, whose low byte(s) this unit tests
 * against literals 2/5/0x10/0x20) is read here -- func_8004E444/func_8004E4E8
 * never dereference the resource beyond that one word.
 */
typedef struct ResHeader3bb8cE ResHeader3bb8cE;
struct ResHeader3bb8cE {
    s32 header;   /* +0x000, low byte(s): type tag (2, 5, 0x10 or 0x20) */
};
typedef struct Res3bb8cE Res3bb8cE;
struct Res3bb8cE {
    ResHeader3bb8cE *methods;   /* +0x000 */
};

/*
 * The class itself. Offsets established purely from this unit's own 19
 * functions (see each field's comment). unk60/unk64/unk78/unk7C are four
 * typed resource slots, one per tag value func_8004E444/func_8004E4E8
 * dispatch on; unk68 is zeroed alongside them by func_8004E3F4/func_8004E588
 * but has no setter anywhere in this unit, so its pointee type is unproven.
 */
typedef struct Node3bb8cE Node3bb8cE;
struct Node3bb8cE {
    u8 pad00[0x00C];
    s32 unkC;              /* +0x00C, func_8004E5D4 sets it (caller value); func_8004E940 nonzero-tests it; func_8004EA38/func_8004ECCC forward it as func_8004F32C's arg1 */
    s32 unk10;             /* +0x010, func_8004E5D4: unkC << 4; func_8004E7D0/func_8004E890: a resource handle passed to func_80050B18/func_80050B08/func_80050B28 */
    u8 pad14[0x060 - 0x014];
    Res3bb8cE *unk60;      /* +0x060, tag 2 */
    Res3bb8cE *unk64;      /* +0x064, tag 5 */
    Res3bb8cE *unk68;      /* +0x068, zeroed only -- no setter in this unit */
    u8 pad6C[0x078 - 0x06C];
    Res3bb8cE *unk78;      /* +0x078, tag 0x10 */
    Res3bb8cE *unk7C;      /* +0x07C, tag 0x20 */
};

/* Uncarved helpers this unit calls into, all still INCLUDE_ASM in
 * class_3bb8c_f.c (extern for a function OUTSIDE this unit). Declared with
 * unspecified argument lists (K&R style, no prototype) where this unit's own
 * call sites disagree on arity -- same idiom already established for
 * strcpy/strcat in include/psyq/STRINGS.H -- rather than forcing one
 * prototype to fit every call site. */
extern void *func_8004F32C();
extern void func_8004F394(void *self);
extern void *func_8004F3BC(void *self);
extern void func_8004F3E4(void *self);
extern void func_8004F40C(void *self, void (*fn)(void), s32 arg2);
extern s32 func_8004F4A4(void *self);

/* PSX thread-table constant walked by func_8004E5E4 (4 entries, one per
 * OpenTh-style thread it starts). Address-only-derived walk (lui/addiu then
 * plain lw at increasing offsets), never gp-relative, so unaffected by the
 * project's gp_rel blocker. */
extern s32 D_80086E78[4];

/* Two format-string-like globals selected by func_8004E940 on self->unkC's
 * truth value; passed opaquely (never dereferenced in this unit). */
extern s32 D_8008AA9C;
extern s32 D_8008AAA4;
/* Third such constant, passed as func_8004F32C's 3rd argument by
 * func_8004ECCC only. */
extern s32 D_8008AAAC;

extern void *func_80017B34(s32 size);
extern void *func_80017CFC(void *ptr);
extern char *strcpy(char *dest, char *src);
extern char *strcat(char *dest, char *src);

void func_8004E3F4(Node3bb8cE *self)
{
    self->unk60 = NULL;
    self->unk64 = NULL;
    self->unk68 = NULL;
    self->unk78 = NULL;
    self->unk7C = NULL;
}

void func_8004E40C(Node3bb8cE *self)
{
    func_80018390()->finalize(self);
}

void func_8004E444(Node3bb8cE *self, Res3bb8cE *res)
{
    s32 tag;

    if (res == NULL) {
        return;
    }
    func_80018390()->addChild(self, res);
    tag = res->methods->header;
    if ((tag & 0xF) == 2) {
        self->unk60 = res;
        return;
    }
    if ((tag & 0xF) == 5) {
        self->unk64 = res;
        return;
    }
    if ((tag & 0xFF) == 0x10) {
        self->unk78 = res;
        return;
    }
    if ((tag & 0xFF) == 0x20) {
        self->unk7C = res;
    }
}

void func_8004E4E8(Node3bb8cE *self, Res3bb8cE *res)
{
    s32 tag;

    if (res == NULL) {
        return;
    }
    tag = res->methods->header;
    if ((tag & 0xF) == 2) {
        self->unk60 = NULL;
    } else if ((tag & 0xF) == 5) {
        self->unk64 = NULL;
    } else if ((tag & 0xFF) == 0x10) {
        self->unk78 = NULL;
    } else if ((tag & 0xFF) == 0x20) {
        self->unk7C = NULL;
    }
    func_80018390()->removeChild(self, res);
}

void func_8004E588(Node3bb8cE *self)
{
    self->unk60 = NULL;
    self->unk64 = NULL;
    self->unk68 = NULL;
    self->unk78 = NULL;
    self->unk7C = NULL;
    func_80018390()->removeAllChildren(self);
}

void func_8004E5D4(Node3bb8cE *self, s32 val)
{
    self->unkC = val;
    self->unk10 = val << 4;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_e", func_8004E5E4);

extern void func_8003902C(void);

s32 func_8004E678(Node3bb8cE *self)
{
    func_8004F3BC(self);
    func_8004F40C(self, func_8003902C, 1);
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_e", func_8004E6B8);

extern s32 func_8004E7D0(Node3bb8cE *self, s32 *p1, s32 *p2);
extern s32 func_8004E890(Node3bb8cE *self, s32 *p1, s32 *p2);

void func_8004E77C(Node3bb8cE *self, s32 *p1, s32 *p2, s32 *p3)
{
    if (func_8004E7D0(self, p1, p2) != 0) {
        func_8004E890(self, p1, p3);
    }
}

extern s32 func_80050B18(s32 arg0);
extern s32 func_80050B28(s32 arg0);

s32 func_8004E7D0(Node3bb8cE *self, s32 *p1, s32 *p2)
{
    s32 status;
    s32 code;

    status = 1;
    *p2 = *p1 = 0;
    func_8004F3E4(self);
    while (func_80050B18(self->unk10) == 0)
        ;
    code = func_8004F4A4(self);
    if (code == 0x100) {
        status = 0;
    } else if (code == 0x8000) {
        status = 0;
        *p1 = 1;
    } else if (code == 0x2000) {
        *p2 = 1;
        func_80050B28(self->unk10);
    }
    return status;
}

extern s32 func_80050B08(s32 arg0);

s32 func_8004E890(Node3bb8cE *self, s32 *p1, s32 *p2)
{
    s32 status;
    s32 code;

    status = 1;
    *p2 = (*p1 = 0, status);
    func_8004F3E4(self);
    while (func_80050B08(self->unk10) == 0)
        ;
    code = func_8004F4A4(self);
    if (code == 0x100) {
        status = 0;
    } else if (code == 0x8000) {
        status = 0;
        *p1 = 1;
    } else if (code == 0x2000) {
        *p2 = 0;
    }
    return status;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_e", func_8004E940);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_e", func_8004E9AC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_e", func_8004EA38);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_e", func_8004EADC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_e", func_8004EB88);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_e", func_8004EC5C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_e", func_8004ECCC);
