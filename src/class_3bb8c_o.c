/*
 * class_3bb8c_o -- functions 54..73 of the 113-function `class_3bb8c_n`
 * remainder, 0x475F0..0x47CC4 (vram 0x80056DF0..0x800574C4).  Carved round 17
 * (2026-09-04); `class_3bb8c_n` keeps its name for the 54 functions in front
 * of this slice and `class_3bb8c_q` is the 19-function tail behind
 * `class_3bb8c_p`.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 19 of the 20 clean.
 *
 * BLOCKED, stub report already filed, do NOT spend attempts on it:
 *   gp_rel: func_80056F5C
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
 *  - func_80056DF8/func_80056E44/func_80056F28 operate on a DIFFERENT,
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

void func_80056DF0(void) {
}

/* ------------------------------------------------------------------ *
 * Group 1: func_80056DF8 / func_80056E44 / func_80056F28.
 * Self is some larger object with an inline 5-element `BasicClass *`
 * array at +0x084.  func_80056DF8/func_80056F28 release the whole array
 * (func_800183DC, already established elsewhere as
 * `void func_800183DC(BasicClass **array, s32 count)` in code_8220_b.c --
 * kept generic `void **` here per this project's per-unit convention for
 * that symbol, e.g. code_2cc8c.h's own looser reading).  func_80056E44
 * walks array indices [1..4] (self+0x88 .. self+0x94), which is exactly
 * inside the same 5-element array, and for each element calls its own
 * vtable slot +0x048 with a random Vec3-ish table entry, then sets the
 * element's own +0x084 field to a random "angle" value
 * (`(rand() % 360) << 12`, a degrees->fixed-point conversion).
 * ------------------------------------------------------------------ */

extern void func_800183DC(void **array, s32 count);
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
    s32 unk84;                   /* +0x084, a random "angle" set by func_80056E44 */
};

typedef struct LinkOwnerObj {
    u8 pad0[0x84];              /* +0x000 .. +0x083, unknown */
    LinkElemObj *arr84[5];        /* +0x084 .. +0x097 */
} LinkOwnerObj;

void func_80056DF8(LinkOwnerObj *this) {
    func_800183DC((void **)this->arr84, 5);
}

extern void func_80056D18(void *arg0, s32 arg1, s32 arg2, s32 arg3);

void func_80056E1C(void *this) {
    func_80056D18(this, 0, 0, 0);
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80056E44);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80056F28);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80056F4C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80056F5C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80056FE4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80057044);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_800570B4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80057130);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_800571A8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_800571E8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_800571F8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80057320);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80057384);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_800573A8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_800573CC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80057444);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_8005748C);
