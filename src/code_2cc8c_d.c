#include "common.h"
#include "code_2cc8c.h"

/* Forwards to the inherited BasicClass slot38, then dispatches self's OWN
 * slot94 or slot98 depending on arg1's dynamic class tag (5 or 1
 * respectively, per its header nibble -- same tag idiom as func_8003E770,
 * round 13). Neither dispatch happens for any other tag. */
void func_8003E8B8(Unk18Obj *self, GenericObj *arg1, s32 arg2) {
    s32 tag;

    func_80018390()->slot38(self, arg1, arg2);

    tag = arg1->methods->header & 0xF;
    if (tag == 5) {
        self->methods->slot94(self, arg1, arg2);
    } else if (tag == 1) {
        self->methods->slot98(self, arg1, arg2);
    }
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003E968);

void func_8003EA0C(Unk18Obj *self, Pair32_d294 *pair) {
    self->unk34 = *pair;
}

void func_8003EA24(Unk18Obj *self, s32 a1) {
    self->unk3C = a1;
}

/* Only writes unk44 the first time (guarded by the unk70 latch). */
void func_8003EA2C(Unk18Obj *self, s32 a1) {
    if (self->unk70 == 0) {
        self->unk44 = a1;
    }
}

/* Same guard as func_8003EA2C, writes unk48 instead. */
void func_8003EA48(Unk18Obj *self, s32 a1) {
    if (self->unk70 == 0) {
        self->unk48 = a1;
    }
}

void func_8003EA64(Unk18Obj *self, s32 a1) {
    self->unk40 = a1;
}

void func_8003EA6C(void) {
}

void func_8003EA74(void) {
}

void func_8003EA7C(Unk18Obj *self, s32 a1) {
    self->unk54 = a1;
}

void func_8003EA84(Unk18Obj *self, SByte3_d294 *src) {
    self->unk58 = *src;
}

void func_8003EAA4(Unk18Obj *self, SByte3_d294 *src) {
    self->unk5B = *src;
}

void func_8003EAC4(Unk18Obj *self, s32 a1) {
    self->unk60 = a1;
}

/* GsSetRefView2 is Sony's (`libgs/gs_131.o`, linked from the SDK object).
   Declared LOCALLY rather than in include/code_2cc8c.h, which six units
   include: the real `LIBGS.H` prototype for this name will collide there.
   This is only the shape THIS unit's call sites use -- the real one takes a
   GsRVIEW2*. */
extern void GsSetRefView2(void *arg0);

/* One-time init, guarded by self->unk10: registers `a1` as a child (via
 * the inherited BasicClass "addChild" slot10), dispatches slot78/slot7C
 * with a2/a3, dispatches slot80 with arg5 (or a default, D_8008A8F4, when
 * arg5 is NULL), then hands &self->unk14 to GsSetRefView2. Does nothing at
 * all once self->unk10 is already set. */
void func_8003EACC(Unk18Obj *self, void *a1, void *a2, void *a3, void *arg5) {
    Unk18ObjMethods *m = self->methods;

    if (self->unk10 != NULL) {
        return;
    }
    m->slot10(self, a1);
    m->slot78(self, a2);
    m->slot7C(self, a3);
    m->slot80(self, arg5 != NULL ? arg5 : D_8008A8F4);
    GsSetRefView2(&self->unk14);
}

/* Teardown counterpart to func_8003EACC's init: removes self->unk10 as a
 * child (inherited BasicClass "removeChild") if it was ever set. */
void func_8003EB84(Unk18Obj *self) {
    if (self->unk10 != NULL) {
        self->methods->slot14(self, self->unk10);
    }
}

/* Copies a1 wholesale into self->unk14, but only when self->unk10 is set. */
void func_8003EBC4(Unk18Obj *self, Vec3_2cc8c *a1) {
    if (self->unk10 != NULL) {
        self->unk14 = *a1;
    }
}

/* Sibling of func_8003EBC4: copies a1 wholesale into self->unk20, guarded
 * by the same self->unk10 flag. */
void func_8003EBF8(Unk18Obj *self, Vec3_2cc8c *a1) {
    if (self->unk10 != NULL) {
        self->unk20 = *a1;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EC2C);

void func_8003ECC0(void) {
}

void func_8003ECC8(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003ECD0);

/* Teardown counterpart to func_8003ECD0's init. */
void func_8003EDF4(Unk18Obj *self) {
    if (self->unk70 != 0) {
        func_80021114(0);
        func_80017CFC((void *)self->unk78);
        self->unk70 = 0;
    }
}

/* Increments self->unk90 unconditionally, and additionally dispatches
 * self->methods->slot9C when arg2 is 2 or 3. */
void func_8003EE40(Unk18Obj *self, GenericObj *arg1, s32 arg2) {
    self->unk90 = self->unk90 + 1;
    if (arg2 == 2 || arg2 == 3) {
        self->methods->slot9C(self);
    }
}

void func_8003EE88(Unk18Obj *self, GenericObj *arg1, s32 arg2) {
    if (arg2 == 2) {
        self->methods->slotA4(self);
    }
}

/* Per-frame update, guarded by self->unk70 (only runs once func_8003ECD0's
 * init has succeeded). Notifies slotA0 if self->unk10->unkC is set,
 * updates three sub-objects (unk40/unk4C/unk54), conditionally re-notifies
 * a PsyQ helper when unk54 is 1 or 3, resets self->unk30's pointee,
 * recomputes self->unk98 from the (unk50-unk4C)/(1<<unk3C) division,
 * forwards the current unk74-indexed slot to two more helpers, dispatches
 * slotA0 again with self->unkAC, and finally -- if self->unk10 is set --
 * walks it to its list tail and dispatches slotA0 a third time with that
 * tail. */
/* Psy-Q's GTE far-colour register writer (libgte/reg03, linked from Sony's
 * own SDK object). LOCAL to this unit, not code_2cc8c.h -- see the note on
 * SetGeomScreen below. This call site reads self->unk5B's own three bytes
 * UNSIGNED (`lbu`, not `lb`) even though func_8003EAA4 writes them as signed
 * bytes; the disagreement is kept as a local cast rather than a retype of the
 * field. Sony's own argument type is `long` for each. */
extern void SetFarColor(u8 a0, u8 a1, u8 a2);

void func_8003EEC0(Unk18Obj *self) {
    s32 idx;
    Unk18Obj *tail;

    if (self->unk70 == 0) {
        return;
    }

    if (self->unk10->unkC != NULL) {
        ((void (*)(Unk18Obj *, GenericObj *))self->methods->slotA0)(self, self->unk10);
    }

    func_8003F28C((Unk18Obj *)self->unk40);
    func_8003FB0C(self->unk4C);
    func_8003FC70(self->unk54);

    if (self->unk54 == 1 || self->unk54 == 3) {
        u8 *rawBytes = (u8 *)&self->unk5B;
        SetFarColor(rawBytes[0], rawBytes[1], rawBytes[2]);
        func_8003FD4C(self->unk60, self->unk40);
    }

    GsSetRefView2(&self->unk14);
    *(s32 *)self->unk30 = 0;

    self->unk98 = (u32)(self->unk50 - self->unk4C) / (u32)(1 << self->unk3C) + 1;

    idx = self->unk74;
    func_8003FBE4(*(s32 *)((u8 *)self + 0x88 + idx * 4));

    idx = self->unk74;
    func_8003FC18(0, 0, *(s32 *)((u8 *)self + 0x78 + idx * 4));

    ((void (*)(Unk18Obj *, void *))self->methods->slotA0)(self, self->unkAC);

    if (self->unk10 != NULL) {
        tail = func_8003F25C((Unk18Obj *)self->unk10);
        ((void (*)(Unk18Obj *, Unk18Obj *))self->methods->slotA0)(self, tail);
    }
}

/* Recomputes self->unk74 from self->unkC->methods->slot54, optionally
 * resets the graphics context and re-notifies self->unkC->methods->slot50
 * (once, or twice more if unk74 is still 0), forwards the current
 * unk74-indexed slot to two rendering helpers, then finally collapses
 * self->unk74 to a plain boolean (1 if it was 0, else 0). */
void func_8003F04C(Unk18Obj *self) {
    s32 idx;
    u8 *rawBytes;

    if (self->unk70 == 0) {
        return;
    }

    self->unk74 = self->unkC->methods->slot54(self->unkC);
    if (self->unkB8 == 0) {
        goto tail_check;
    }

    ResetGraph(1);
    self->unkC->methods->slot50(self->unkC);

    if (self->unkB4 != 0) {
        if (self->unk74 == 0) {
            self->unkC->methods->slot50(self->unkC);
        }
    }

    idx = self->unk74;
    rawBytes = (u8 *)&self->unk58;
    func_80023DA0(rawBytes[0], rawBytes[1], rawBytes[2],
                  *(s32 *)((u8 *)self + 0x78 + idx * 4));

    idx = self->unk74;
    func_8003FBF4(*(s32 *)((u8 *)self + 0x78 + idx * 4));

    if (self->unkB4 != 0 && self->unk74 == 0) {
        self->unkC->methods->slot50(self->unkC);
    }

tail_check:
    self->unk74 = (self->unk74 == 0);
}

/* Only runs when self->unk10 is NULL: releases the current self->unkB0 (if
 * any) via its own slot4, then -- if arg1 is non-NULL -- installs arg1 as
 * the new self->unkB0 and notifies it (slot4C) with self->unkAC and the
 * shared D_8008A904 constant. */
void func_8003F1A8(Unk18Obj *self, SubHandleObj *arg1) {
    if (self->unk10 != NULL) {
        return;
    }

    if (self->unkB0 != NULL) {
        self->unkB0->methods->slot4(self->unkB0);
    }

    self->unkB0 = arg1;
    if (arg1 != NULL) {
        arg1->methods->slot4C(arg1, self->unkAC, D_8008A904);
    }
}

SubHandleObj *func_8003F230(Unk18Obj *self) {
    return self->unkB0;
}

void func_8003F23C(Unk18Obj *self, s32 a1) {
    self->unkB4 = a1;
}

void func_8003F244(Unk18Obj *self, s32 a1) {
    self->unkB8 = a1;
}

/* Plain no-arg getter for Unk18Obj's own vtable. */
Unk18ObjMethods *func_8003F24C(void) {
    return &D_8006E8E4;
}

/* Walks self->unkC repeatedly while non-NULL, finding the tail of a
 * singly-linked list rooted at self, threaded through unkC. Returns self
 * itself unchanged if self->unkC is already NULL. */
Unk18Obj *func_8003F25C(Unk18Obj *self) {
    while (self->unkC != NULL) {
        self = (Unk18Obj *)self->unkC;
    }
    return self;
}

/* Psy-Q's GTE far-colour and geometric-screen-distance register writers
 * (libgte/reg03, linked from Sony's own SDK object). Declared LOCAL to this
 * unit rather than in code_2cc8c.h, which nine units include, since they
 * belong to another translation unit (CLAUDE.md's header-contention rule).
 * Sony types SetGeomScreen's argument `long`; func_8003F28C forwards `self`
 * unexamined and ignores the return, so the local view keeps that call
 * site's own shape -- ABI-identical either way. */
extern void SetGeomScreen(Unk18Obj *self);

void func_8003F28C(Unk18Obj *self) {
    SetGeomScreen(self);
}
