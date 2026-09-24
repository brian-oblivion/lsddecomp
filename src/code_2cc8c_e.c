#include "common.h"
#include "code_2cc8c.h"

/* ROUND 34: THIS UNIT LOST ITS FIRST EIGHT FUNCTIONS -- six of them to Sony,
 * two to files of their own -- and now begins at 0x305B0 / New_Class6E99C.
 * The segment it used to be is split three ways:
 *
 *   [c code_2cc8c_e0]  func_8003FB0C          game code, own file
 *   [o libgs/gs_123]   Gssub_make_matrix      was func_8003FB1C, matched C
 *   [c code_2cc8c_e1]  SetPacketBufCursor          game code, own file
 *   [o libgs/gs_111]   GsDrawOt               was func_8003FBF4, matched C
 *   [o libgs/gs_113]   GsClearOt              was func_8003FC18, matched C
 *   [o libgs/gs_108]   GsSetLightMode         was func_8003FC70, matched C
 *   [o libgte/fgo_00]  TransposeMatrix        was func_8003FCFC, a 20w stall
 *   [o libgte/fog_01]  SetFogNear             was func_8003FD4C, matched C
 *   [c code_2cc8c_e]   New_Class6E99C onward   <- this file
 *
 * THIS FILE KEEPS THE NAME deliberately: it holds the unit's remaining
 * INCLUDE_ASM stubs and its class, so every
 * `INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", ...)` path below and every
 * match report naming this unit stays valid. Only the two one-function heads
 * needed new names.
 *
 * NEITHER RODATA SLOT IS OURS ANY MORE. jtbl_80011108 (0x1908) went with
 * Gssub_make_matrix and D_80011194 (0x1994, "not supported light mode %d\n")
 * went with GsSetLightMode -- both are their own object's `.rdata` section
 * now. This unit needs no `.rodata` attach at all; if a future carve of it
 * hits Gate 2's `undefined reference to '.LXXXXXXXX'`, that is a NEW jump
 * table, not these.
 *
 * The six Sony bodies are gone from this file, not lost -- five matched C
 * bodies and one INCLUDE_ASM stub. Each one's match report is kept and
 * retitled CONVERTED, and carries its derivation verbatim.
 */

/*
 * WHAT THIS UNIT IS (round 61, track 3 naming pass). This file's own 17
 * functions are the bottom two links of a three-class chain rooted in
 * `code_d294.h`'s `Class6B5CCObj`: `Class6B5CCObj -> ClassEAC0Obj ->
 * Class6E99CObj` (`tools/classtable.py D_8006E99C --vs D_8006B58C`,
 * round 14; see `include/code_2cc8c.h`'s own header comment above
 * `struct ClassEAC0Obj` for the full derivation). Critically, `ClassEAC0Obj`
 * is THIS unit's own local view of the SAME table family that
 * `code_2cc8c_f` (bravo's unit) views as `Obj6EAC0` -- `ClassEAC0Methods`'s
 * `slotB8`/`slotCC` dispatch to bravo's own `Obj6EAC0__SetColor`/
 * `Obj6EAC0__SetMask` -- so `Class6E99CObj` is a further-derived subclass
 * of bravo's "on-screen text/digit display" class (round 54's own
 * working hypothesis for `Obj6EAC0`).
 *
 * `Class6E99CObj`'s own functions (`New_Class6E99C` onward) add a
 * start/stop pair over an indexed and a fixed color table
 * (`Class6E99C__StartFadeToIndex`/`Class6E99C__StartFadeDefault`/
 * `Class6E99C__Stop`), a per-tick `step`-driven RGB-channel accumulator
 * gated by a countdown (`Class6E99C__Update`), and a save/restore-of-one
 * position pair (`Class6E99C__PushPosition`/`Class6E99C__PopPosition`).
 * Read together this looks like a COLOR-FADE CONTROLLER layered on top of
 * bravo's digit display -- plausibly driving a transition when an
 * on-screen digit/counter's value or color changes -- but that reading is
 * this unit's own working hypothesis (tier B throughout), not confirmed
 * against any caller outside this file: nothing else in `src/*.c`
 * constructs or touches a `Class6E99CObj`/`ClassEAC0Obj` (only
 * `New_Class6E99C`/`New_ClassEAC0` themselves are called elsewhere, always
 * through a caller's own differently-typed local view -- see each
 * function's own match report). See each function's own `## Naming`
 * section for the specific evidence behind its name.
 */

Class6E99CObj *New_Class6E99C(void *a1, s32 a2, s32 a3) {
    Class6E99CObj *self;

    self = BMemPMgrAlloc(0xA0);
    if (self != NULL) {
        GetClass6E99CMethods()->ctor(self, a1, a2, a3);
        return self;
    }
    return NULL;
}

void Class6E99C__Class6E99C(Class6E99CObj *self, void *a1, s32 a2, s32 a3) {
    ClassEAC0Methods *base;
    void *tableEntry;

    base = (ClassEAC0Methods *)Obj6EAC0__GetBaseMethods();
    if (a2 != 0) {
        tableEntry = &D_8006EA90[a2 * 3];
    } else {
        tableEntry = D_8006EAA8;
    }
    base->ctor((ClassEAC0Obj *)self, a1, tableEntry, a3);
    self->methods = GetClass6E99CMethods();
    self->methods->finishConstruct(self, a2);
}

void Class6E99C__FinishConstruct(Class6E99CObj *self, s32 a1) {
    self->unk70 = a1;
    self->state = 0;
    self->step = 0xA;
    self->unk78 = 0;
    self->unk7C = 0;
    self->methods->slot60(self, 0);
    self->methods->slot64(self, 0);
    self->altMode = 0;
}

void Class6E99C__Update(Class6E99CObj *self, void *a1, s32 a2) {
    s32 old;

    if (a2 != 2) {
        return;
    }
    old = self->unk80;
    self->unk80 = old - 1;
    if (old > 0) {
        if (self->unk7C == 9) {
            return;
        }
        if (self->unk78 & 4) {
            self->unk64 += (u8)self->step;
        }
        if (self->unk78 & 2) {
            self->unk65 += (u8)self->step;
        }
        if (self->unk78 & 1) {
            self->unk66 += (u8)self->step;
        }
    } else {
        self->methods->stop(self, a1);
    }
}

void Class6E99C__SetStep(Class6E99CObj *self, s32 a1) {
    self->step = a1;
}

/* `configure`'s occupant, Class6E99C__Configure, reads all four argument
 * registers, and both StartFade* functions forward their own a1..a3 to it
 * untouched (no argument register is set before that jalr). The shared
 * Class6E99CMethods slot is declared `(self)` only, so the call goes through
 * this file-local view instead of retyping the shared header. Spelling the
 * forward is load-bearing: the `(self)`-only call compiles to the same
 * instructions in a different order (29/35; round 73). */
typedef s32 (*Configure6E99CFn)(Class6E99CObj *self, s32 a1, s32 a2, s32 a3);

void Class6E99C__StartFadeToIndex(Class6E99CObj *self, s32 a1, s32 a2, s32 a3) {
    s32 idx;

    if (self->state != 0) {
        return;
    }
    idx = ((Configure6E99CFn)self->methods->configure)(self, a1, a2, a3);
    self->methods->slotB8(self, 1, &D_8006EA90[idx * 3]);
    self->state = 1;
    self->step = -self->step;
}

void Class6E99C__StartFadeDefault(Class6E99CObj *self, s32 a1, s32 a2, s32 a3) {
    if (self->state != 0) {
        return;
    }
    a2 = ((Configure6E99CFn)self->methods->configure)(self, a1, a2, a3);
    if (self->altMode != 0) {
        self->unk80--;
    } else {
        self->methods->slotB8(self, 1, &D_8006EAA8[a2 * 3]);
    }
    self->state = 2;
}

s32 Class6E99C__Configure(Class6E99CObj *self, s32 a1, s32 a2, s32 a3) {
    Class6E99CMethods *methods;
    s32 flag;
    s32 q1, q2;

    methods = self->methods;
    if (a2 < 0) {
        a2 = self->unk70;
    } else {
        self->unk70 = a2;
    }
    flag = 1;
    if (a2 != 0) {
        self->unk78 = a2;
    } else {
        flag = 2;
        self->unk78 = 0xF;
    }
    self->unk78 = a2;
    if (a2 == 0) {
        self->unk78 = 0xF;
    }
    q1 = 0x100 / self->step;
    self->unk7C = a3;
    self->unk80 = q1;
    if (self->altMode != 0) {
        q2 = q1 / self->divisor;
        self->unk80 = q1 - (s16)q2;
    }
    q2 = self->unk68;
    q2 = q2 / self->unk80;
    self->unk84 = q2;
    methods->slot10(self);
    methods->slot64(self, 1);
    methods->slot68(self, flag);
    methods->slot60(self, 1);
    return a2;
}

void Class6E99C__Stop(Class6E99CObj *self, void *a1) {
    Class6E99CMethods *methods;
    s32 mode;

    methods = self->methods;
    if (self->state == 0) {
        return;
    }
    if (self->state == 1) {
        mode = 5;
        if (self->altMode == 0) {
            methods->slot60(self, 0);
            methods->slot64(self, 0);
        }
    } else {
        mode = 6;
        if (self->altMode != 0) {
            if (self->unk78 == 0xF) {
                methods->slotB8(self, 1, D_8006EAA8);
            }
            methods->slot64(self, 0);
        }
    }
    methods->slot14(self, a1);
    if (self->step < 0) {
        self->step = -self->step;
    }
    self->state = 0;
    methods->slot30(self, mode);
}

void *Class6E99C__GetColor(Class6E99CObj *self) {
    if (self->unk78 == 0xF) {
        return D_8006EAA8;
    }
    return &D_8006EA90[self->unk78 * 3];
}

/* Both s32 pairs are copied as whole structs. GCC 2.6.3's MIPS
 * `movstrsi_internal` clobbers $v0/$v1/$a0/$a1, so `self` and `a1`, live
 * across the first copy, cannot stay in their incoming registers: that is
 * retail's entry `move $a3,$a0` / delay-slot `move $t0,$a1` (round 73). */
void Class6E99C__PushPosition(Class6E99CObj *self, SkipShort2 *a1, Pair32E99C *a2) {
    if (self->unkC != 0) {
        self->unk88 = self->unk60;
        self->unk8C = self->unk62;
        *(Pair32E99C *)&self->unk90 = *(Pair32E99C *)&self->unk50;
        self->unk60 = a1->x;
        self->unk62 = a1->y;
        *(Pair32E99C *)&self->unk50 = *a2;
    }
}

void Class6E99C__PopPosition(Class6E99CObj *self) {
    s32 t0, t1;

    t0 = self->unk90;
    t1 = self->unk94;
    self->unk50 = t0;
    self->unk54 = t1;
    __asm__("" ::: "memory");
    self->unk60 = self->unk88;
    self->unk62 = self->unk8C;
}

void Class6E99C__SetDivisorMode(Class6E99CObj *self, s32 a1, s32 a2) {
    self->altMode = a1;
    self->divisor = a2;
}

Class6E99CMethods *GetClass6E99CMethods(void) {
    return &D_8006E99C;
}

ClassEAC0Obj *New_ClassEAC0(void *a0, void *a1, s32 a2) {
    ClassEAC0Obj *self;

    self = BMemPMgrAlloc(0x6C);
    if (self != NULL) {
        ((ClassEAC0Methods *)Obj6EAC0__GetBaseMethods())->ctor(self, a0, a1, a2);
        return self;
    }
    return NULL;
}

void ClassEAC0__ClassEAC0(ClassEAC0Obj *self, SkipShort2 *a1, void *a2, s32 a3) {
    GetClass6B5CCMethods()->ctor(self);
    self->methods = (ClassEAC0Methods *)Obj6EAC0__GetBaseMethods();
    self->methods->finishConstruct(self, a1, a2, a3);
}

void ClassEAC0__FinishConstruct(ClassEAC0Obj *self, SkipShort2 *a1, void *a2, s32 a3) {
    ClassEAC0Methods *methods;

    self->unk44 = a3;
    self->unk48 = 1;
    self->unk4C = 0;
    self->unk58 = 0;
    self->unk5C = 0;
    self->unk5E = 0;
    self->unk60 = a1->x;
    self->unk62 = a1->y;
    methods = self->methods;
    if (a2 == NULL) {
        a2 = D_8008A924;
    }
    methods->slotB8(self, 1, a2);
    self->methods->slotCC(self, 0xD);
}
