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
 * class_3bb8c_j -- fourth carved slice of the class_3bb8c block
 * (0x41F84..0x429D4, vram 0x80051784..0x800521D4), 20 functions.
 * Carved round 15 out of the 193-function class_3bb8c_j remainder.
 *
 * Blocker profile (head's Gate 1 three-grep screen at carve time):
 *   func_800518F4  gp_rel     -- MATCHED round 45 (41/41 words)
 *   func_80051998  gp_rel     -- MATCHED round 45 (45/45 words)
 * The other 18 are clean. This unit owns NO switch jump table.
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 *
 * TWO classes share this unit's address range, discovered this round:
 *  - The first four functions (func_80051784/func_800517EC/func_80051814/
 *    func_80051858), plus the two gp_rel-blocked siblings, are Obj866E8
 *    methods (the SAME class already established across class_3bb8c_b/c/
 *    etc.) -- they touch offsets 0x10/0x14/0x18/0x1C/0x20/0x48, all
 *    previously-unnamed padding, and dispatch through the SAME
 *    Obj866E8Methods table (slotA4/slotA8, also newly named). See
 *    include/class_3bb8c.h's Obj866E8/Obj866E8Methods for the additive
 *    edits.
 *  - Everything from func_80051A4C on is a SEPARATE, much smaller sibling
 *    class (alloc size 0x54, vtable D_80086ED0 -- see func_80051A4C, a
 *    plain address-of getter, and func_80051A5C, its New_X allocator).
 *    Named `Class86ED0` here, LOCAL to this unit (not added to the shared
 *    header -- nothing else references it yet). Its base class IS
 *    BasicClass (include/code_8220.h): slots 0x0C/0x10/0x14/0x18 line up
 *    exactly with BasicClassMethods' finalize/addChild/removeChild/
 *    removeAllChildren. Reached via Get_vtable_BasicClass(), which class_3bb8c.h
 *    ALREADY declares (class_3bb8c_f's own local view, `BasicMethods866E8F`)
 *    -- this round additively named those four slots on THAT existing
 *    type rather than adding a second, incompatible local declaration of
 *    the same function (which would conflict in this translation unit).
 */
#include "common.h"
#include "class_3bb8c.h"

void func_80051784(Obj866E8 *self)
{
    s32 count;

    if (self->unk48) {
        count = self->unk1C - 1;
        self->unk1C = count;
        if (count > 0) {
            self->methods->slotA8(self, self->unk18, count, 1);
        } else {
            self->unk1C = self->unk14;
        }
    }
}

void func_800517EC(Obj866E8 *self)
{
    if (self->unk48) {
        self->unk20 ^= 1;
    }
}

void func_80051814(Obj866E8 *self)
{
    if (self->unk48) {
        self->unk1C = 0;
        self->methods->slotA8(self, self->unk18, 0, 1);
    }
}

void func_80051858(Obj866E8 *self)
{
    s32 i;

    if (self->unk48) {
        self->unk1C = 0;
        i = self->unk10 - 1;
        if (i >= 0) {
            do {
                self->unk18 = i;
                self->methods->slotA8(self, i, self->unk1C, 0);
                i--;
            } while (i >= 0);
        }
        self->methods->slotA4(self, self->unk18, 1);
    }
}

/* round 45 -- func_800518F4's own opaque handle at self->unk40, dispatched
 * only through this unit; not the same type as class_3bb8c_i's
 * ChildObj86ED0 despite the coincidental D_8008AADC overlap (that unit
 * reads D_8008AADC's ADDRESS as an opaque arg2, this one reads its VALUE
 * arithmetically -- independent local readings of the same global, per
 * the project's convention). */
typedef struct Unk40Obj866E8 Unk40Obj866E8;
typedef struct Unk40Obj866E8Methods Unk40Obj866E8Methods;
struct Unk40Obj866E8Methods {
    u8 pad000[0x0BC];
    void (*slotBC)(Unk40Obj866E8 *self, void *arg1); /* +0x0BC */
};
struct Unk40Obj866E8 {
    Unk40Obj866E8Methods *methods; /* +0x000 */
};

/* func_800518F4's own stack-local argument to slotBC -- a 2-word block
 * (D_8008AADC-derived value at +0x0, the D_8008AAE0 constant at +0x4). */
typedef struct {
    s32 unk0;
    s32 unk4;
} SlotBCArg866E8_3bb8c_j;

extern s32 D_8008AADC; /* VALUE-of here (round 45): see the type comment above */
extern s32 D_8008AAE0; /* VALUE-of, round 45's func_800518F4 only */

void func_800518F4(Obj866E8 *self, s32 arg1, s32 arg2)
{
    SlotBCArg866E8_3bb8c_j local;
    Unk40Obj866E8 *obj;

    if (self->unk48) {
        local.unk4 = D_8008AAE0;
        local.unk0 = arg1 * 7 + D_8008AADC;
        obj = (Unk40Obj866E8 *)self->unk40;
        obj->methods->slotBC(obj, &local);
        self->unk18 = arg1;
        if (arg2) {
            self->methods->slot60(self, 0);
        }
    }
}

/* round 45 -- func_80051998's own opaque handle at self->unk44, dispatched
 * only through this unit. */
typedef struct Unk44Obj866E8 Unk44Obj866E8;
typedef struct Unk44Obj866E8Methods Unk44Obj866E8Methods;
struct Unk44Obj866E8Methods {
    u8 pad000[0x0C4];
    void (*slotC4)(Unk44Obj866E8 *self, s32 arg1, s32 arg2); /* +0x0C4 */
};
struct Unk44Obj866E8 {
    Unk44Obj866E8Methods *methods; /* +0x000 */
};

/* VALUE-of `%gp_rel`, round 45's func_80051998 only -- a byte lookup table
 * (ROM image initialises it to D_800115D0, still-uncarved rodata). */
extern u8 *D_8008AAE4;

void func_80051998(Obj866E8 *self, s32 arg1, s32 arg2, s32 arg3)
{
    Unk44Obj866E8 *obj;

    if (self->unk48) {
        self->unk28[arg1] = D_8008AAE4[arg2];
        obj = (Unk44Obj866E8 *)self->unk44;
        obj->methods->slotC4(obj, D_8008AAE4[arg2], arg1);
        self->unk18 = arg1;
        self->unk1C = arg2;
        if (arg3) {
            self->methods->slot60(self, 0);
        }
    }
}

/*
 * Class86ED0 -- a small BasicClass-derived sibling class, LOCAL to this
 * unit (see the file header comment for why this is not added to the
 * shared class_3bb8c.h). Alloc size 0x54 (func_80051A5C). Vtable
 * D_80086ED0 (func_80051A4C, a plain address-of getter).
 *
 * unk34/unk38 are single-slot caches for the most recently added child of
 * two distinguished "tag" kinds (established from func_80051D1C/
 * func_80051DA0: a child object's own `*(s32*)(*(void**)child) & 0xF`
 * selects unk34 for tag 2, unk38 for tag 5), layered on top of the
 * INHERITED BasicClass generic children list (added/removed via
 * Get_vtable_BasicClass()'s addChild/removeChild in the same two functions).
 */
typedef struct Class86ED0Methods Class86ED0Methods;
typedef struct Class86ED0 Class86ED0;

/*
 * Class86ED0's own opaque "handle" object (self->unk50's pointee, built by
 * func_80051F24 via BuildFileName/func_8003B39C/func_80041C9C: a
 * "CARD\\<name>.TIM" path is built and loaded, as in class_3bb8c_i's
 * func_80050F98). Only the three slots this unit's own
 * functions dispatch through are named.
 */
typedef struct Class86ED0Handle Class86ED0Handle;
typedef struct Class86ED0HandleMethods Class86ED0HandleMethods;
struct Class86ED0HandleMethods {
    u8 pad000[0x004];
    void *(*slot4)(Class86ED0Handle *self);                          /* +0x004, func_800520A0/func_80051F24 */
    u8 pad008[0x04C - 0x008];
    void *(*slot4C)(Class86ED0Handle *self, void *arg1, void *arg2); /* +0x04C, func_80051F24 */
    u8 pad050[0x078 - 0x050];
    void (*slot78)(Class86ED0Handle *self);                          /* +0x078, func_80051F24 */
};
struct Class86ED0Handle {
    Class86ED0HandleMethods *methods; /* +0x000 */
};

/*
 * Class86ED0's own vtable. `ctor` at +0x008 is the standard New_X
 * constructor slot -- func_80052B60() (a real function, defined in the
 * sibling unit class_3bb8c_k, still INCLUDE_ASM there) returns THIS EXACT
 * pointer type: func_80051A5C calls `func_80052B60()->ctor(...)` to reach
 * it, and func_80051AC8 (that very ctor occupant) separately does
 * `self->methods = func_80052B60();` -- both compile against the same
 * declared return type, which is why this is not split into a separate
 * "ctor table" type the way class_3bb8c_c's BaseCtorTable_3bb8c_c is for
 * an unrelated base class.
 */
struct Class86ED0Methods {
    u8 pad000[0x008];
    void (*ctor)(Class86ED0 *self, void *arg0, s32 arg1); /* +0x008, func_80051AC8 (occupant); func_80051A5C's call site */
    u8 pad00C[0x010 - 0x00C];
    /* Called by func_80052110 at two arities: (self,arg1,arg2,arg3) at its
     * first call site (a straight passthrough of that function's own
     * params) and (self,arg2) at its second -- see that function's report. */
    void (*slot10)(Class86ED0 *self, void *arg1, s32 arg2, s32 arg3); /* +0x010, func_80052110 */
    void (*slot14)(Class86ED0 *self, void *arg1);          /* +0x014, func_8005217C */
    u8 pad018[0x040 - 0x018];
    void (*slot40)(Class86ED0 *self);                        /* +0x040, func_80051AC8 tail */
    u8 pad044[0x058 - 0x044];
    void (*slot58)(Class86ED0 *self, void *arg1, s32 arg2);    /* +0x058, func_80051E64 (tag==5) */
    void (*slot5C)(Class86ED0 *self, void *arg1, s32 arg2);     /* +0x05C, func_80051E64 (tag==2) */
    u8 pad060[0x08C - 0x060];
    void (*slot8C)(Class86ED0 *self, void *arg1, Class86ED0Handle *arg2, s32 arg3, s32 arg4, s32 arg5); /* +0x08C, func_80051F24 */
    void (*slot90)(Class86ED0 *self);                          /* +0x090, func_800520A0 */
};

struct Class86ED0 {
    Class86ED0Methods *methods;    /* +0x000 */
    u8 pad04[0x0C - 0x04];
    s32 unkC;                       /* +0x00C, func_80051AC8: set to its own arg2 (a mode: 0 or 1) */
    s32 unk10;                       /* +0x010, func_80051C84/func_80051AC8: element count for unk18[]/unk1C[] */
    s32 unk14;                        /* +0x014, func_80051AC8: a running MAX over the per-entry lengths computed in its fill loop */
    void **unk18;                      /* +0x018, func_80051C84/func_80051AC8: array of unk10 individually-allocated buffers */
    s32 *unk1C;                         /* +0x01C, func_80051C84 (freed as one block)/func_80051AC8 (array of unk10 per-entry lengths) */
    s32 unk20;                           /* +0x020, func_80051F14 */
    s32 unk24;                            /* +0x024, func_80051F14 */
    s32 unk28;                             /* +0x028, func_80051F14 */
    s32 unk2C;                               /* +0x02C, func_80052110: cleared to 0 */
    u8 pad30[0x34 - 0x30];
    void *unk34;                            /* +0x034, "tag==2" registered-child cache */
    void *unk38;                             /* +0x038, "tag==5" registered-child cache */
    s32 unk3C;                                /* +0x03C, func_8005217C (cleared)/func_80052110 (set from its own arg3) */
    u8 pad40[0x50 - 0x40];
    Class86ED0Handle *unk50;                  /* +0x050 */
};

extern Class86ED0Methods D_80086ED0;

Class86ED0Methods *func_80051A4C(void)
{
    return &D_80086ED0;
}

/*
 * New_Class86ED0. func_80017B34/func_80017CFC already declared for the
 * Obj866E8 group above are the same generic pool allocator/free pair --
 * not redeclared here.
 *
 * BasicClass's own method table getter is ALREADY declared in the shared
 * class_3bb8c.h (`Get_vtable_BasicClass`/`BasicMethods866E8F`, class_3bb8c_f's
 * local view -- reused here rather than redeclared, since a second
 * incompatible extern for the same function in one translation unit is a
 * conflicting-types error). This round additively named its own
 * ctor/finalize/addChild/removeChild/removeAllChildren slots (+0x008/
 * +0x00C/+0x010/+0x014/+0x018) in that header, matching
 * include/code_8220.h's canonical BasicClassMethods layout exactly.
 */
extern void *func_80017B34(s32 size);
extern void *func_80017CFC(void *ptr);

extern Class86ED0Methods *func_80052B60(void);

void *func_80051A5C(void *arg0, s32 arg1)
{
    Class86ED0 *self = func_80017B34(0x54);

    if (self == NULL) {
        goto fail;
    }
    func_80052B60()->ctor(self, arg0, arg1);
    return self;
fail:
    return NULL;
}

/*
 * Class86ED0's own ctor (the func_80052B60()->ctor occupant, matched via
 * its own address -- classtable-style resolution doesn't apply here since
 * this vtable is a LOCAL, no `tools/classtable.py` slot list exists for
 * it; the identity comes from func_80051A5C's call site + this function's
 * own signature matching it exactly).
 *
 * arg1 is a NUL-terminated array of string pointers; arg2 is a mode flag
 * (0 or 1) also stashed into self->unkC. First pass counts entries; then
 * allocates two parallel self->unk10-length arrays (self->unk18: one
 * individually-allocated buffer per entry; self->unk1C: one s32 length
 * per entry, computed by strlen -- halved when arg2==1, via the
 * standard truncating-division-by-2 idiom). Each buffer is filled either
 * via DecodeFullWidthSjis (arg2==1) or strcpy (otherwise), and self->unk14
 * tracks the running max of the computed lengths.
 */
extern s32 strlen(void *arg0);
extern void DecodeFullWidthSjis(void *dst, void *src);
extern char *strcpy(char *dest, char *src);

#ifdef NON_MATCHING
/* NON_MATCHING: 6/107 words (length exact, 107/107 -- objdump confirms).
 * Residue: register identity, not instruction count -- retail keeps arg1
 * live in one register for the whole function and uses a separate cursor
 * register for both the counting and filling loops, while this body's
 * allocation rotates self/arg1/count/cursor/index differently, cascading
 * from partway through the body onward (docs/match-reports/func_80051AC8.md).
 * Hand-derived; reviewed rounds 9, 13, 19 -- round 19 additionally fixed a
 * real sign/unsigned-promotion bug in the halving idiom (`sra` vs `srl`)
 * and matched two more of retail's scheduling choices, neither of which
 * moved the word count since it is dominated by the register rotation. */
void func_80051AC8(Class86ED0 *self, void **arg1, s32 arg2)
{
    void **p;
    s32 count;
    s32 index;
    s32 len;

    Get_vtable_BasicClass()->ctor(self);
    self->methods = func_80052B60();

    count = 0;
    for (p = arg1; *p != NULL; p++) {
        count++;
    }

    self->unk10 = count;
    self->unk18 = func_80017B34(count * 4);
    p = arg1;
    self->unk1C = func_80017B34(self->unk10 * 4);

    if (count > 0) {
        self->unk14 = 0;
        for (index = 0; index < self->unk10; index++) {
            len = strlen(*p);
            if (arg2 == 1) {
                len = (s32)(len + ((u32)len >> 31)) >> 1;
            }
            self->unk1C[index] = len;
            self->unk18[index] = func_80017B34(len + 4);
            if (arg2 == 1) {
                DecodeFullWidthSjis(self->unk18[index], *p);
            } else {
                strcpy(self->unk18[index], *p);
            }
            if (self->unk14 < len) {
                self->unk14 = len;
            }
            p++;
        }
    }

    self->unkC = arg2;
    func_80051C74(self);
    self->methods->slot40(self);
}
#else
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051AC8);
#endif

void func_80051C74(Class86ED0 *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk50 = NULL;
}

void func_80051C84(Class86ED0 *self)
{
    s32 i;

    for (i = 0; i < self->unk10; i++) {
        func_80017CFC(self->unk18[i]);
    }
    func_80017CFC(self->unk1C);
    func_80017CFC(self->unk18);
    Get_vtable_BasicClass()->finalize(self);
}

void func_80051D1C(Class86ED0 *self, void *arg1)
{
    s32 tag;

    if (arg1) {
        Get_vtable_BasicClass()->addChild(self, arg1);
        tag = **(s32 **)arg1 & 0xF;
        if (tag == 2) {
            self->unk34 = arg1;
        } else if (tag == 5) {
            self->unk38 = arg1;
        }
    }
}

void func_80051DA0(Class86ED0 *self, void *arg1)
{
    s32 tag;

    if (arg1) {
        tag = **(s32 **)arg1 & 0xF;
        if (tag == 2) {
            self->unk34 = NULL;
        } else if (tag == 5) {
            self->unk38 = NULL;
        }
        Get_vtable_BasicClass()->removeChild(self, arg1);
    }
}

void func_80051E20(Class86ED0 *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk50 = NULL;
    Get_vtable_BasicClass()->removeAllChildren(self);
}

void func_80051E64(Class86ED0 *self, void *arg1, s32 arg2)
{
    s32 tag;

    Get_vtable_BasicClass()->slot38(self, arg1, arg2);
    tag = **(s32 **)arg1 & 0xF;
    if (tag == 2) {
        self->methods->slot5C(self, arg1, arg2);
    } else if (tag == 5) {
        self->methods->slot58(self, arg1, arg2);
    }
}

void func_80051F14(Class86ED0 *self)
{
    self->unk20 = 0;
    self->unk24 = 0;
    self->unk28 = 0;
}

extern char *BuildFileName(char *dest, const char *arg1, const char *arg2, const char *arg3);
extern Class86ED0Handle *func_8003B39C(char *path);
extern Class86ED0Handle *func_80041C9C(Class86ED0Handle *arg0, void *arg1, s32 arg2);
extern const char D_8008AB14[]; /* "SELECT" */
extern const char D_8008AB1C[]; /* "CARD\\" */
extern const char D_8008AB24[]; /* ".TIM" */
extern s32 D_80087028;
extern s32 D_8008AAF8;
extern const char D_800116E4[]; /* "FONTICON" */

/*
 * Two handle variables, not one: handle1 and handle2 are disjoint live
 * ranges, and merging them into one `h` gives the rotation filed as the
 * round-18/19 stall (75/95, both addresses and the handle swapped among
 * $s0-$s2). Same shape as class_3bb8c_i's func_80050F98.
 */
void func_80051F24(Class86ED0 *self, void *arg1)
{
    char path[0x20];
    const char *dir;
    const char *ext;
    Class86ED0Handle *handle1;
    Class86ED0Handle *handle2;

    if (arg1 == NULL) {
        return;
    }
    if (self->unk50 != NULL) {
        return;
    }

    dir = D_8008AB1C;
    ext = D_8008AB24;

    handle1 = func_8003B39C(BuildFileName(path, D_8008AB14, dir, ext));
    handle1->methods->slot78(handle1);
    self->unk50 = func_80041C9C(handle1, &D_80087028, 0);
    handle1->methods->slot4(handle1);
    self->unk50->methods->slot4C(self->unk50, arg1, &D_8008AAF8);

    handle2 = func_8003B39C(BuildFileName(path, D_800116E4, dir, ext));
    handle2->methods->slot78(handle2);
    self->methods->slot8C(self, arg1, handle2, self->unk20, self->unk24, self->unk28);
    handle2->methods->slot4(handle2);
}

void func_800520A0(Class86ED0 *self)
{
    if (self->unk50) {
        self->methods->slot90(self);
        self->unk50 = self->unk50->methods->slot4(self->unk50);
    }
}

void func_80052110(Class86ED0 *self, void *arg1, s32 arg2, s32 arg3)
{
    typedef void (*Slot10NarrowFn)(Class86ED0 *self, s32 arg1);
    void (*fn)(Class86ED0 *self, void *arg1, s32 arg2, s32 arg3);
    s32 zero;

    zero = 0;
    fn = self->methods->slot10;
    do {
        fn(self, arg1, arg2, arg3);
        ((Slot10NarrowFn)self->methods->slot10)(self, arg2);
        self->unk3C = arg3;
        self->unk2C = zero;
    } while (0);
}

void func_8005217C(Class86ED0 *self)
{
    self->methods->slot14(self, self->unk34);
    self->methods->slot14(self, self->unk38);
    self->unk3C = 0;
}
