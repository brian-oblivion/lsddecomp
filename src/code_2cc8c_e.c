#include "common.h"
#include "code_2cc8c.h"

/* ROUND 34: THIS UNIT LOST ITS FIRST EIGHT FUNCTIONS -- six of them to Sony,
 * two to files of their own -- and now begins at 0x305B0 / New_Class6E99C.
 * The segment it used to be is split three ways:
 *
 *   [c code_2cc8c_e0]  func_8003FB0C          game code, own file
 *   [o libgs/gs_123]   Gssub_make_matrix      was func_8003FB1C, matched C
 *   [c code_2cc8c_e1]  func_8003FBE4          game code, own file
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

Class6E99CObj *New_Class6E99C(void *a1, s32 a2, s32 a3) {
    Class6E99CObj *self;

    self = func_80017B34(0xA0);
    if (self != NULL) {
        Class6E99C__GetMethods()->ctor(self, a1, a2, a3);
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
    self->methods = Class6E99C__GetMethods();
    self->methods->slot40(self, a2);
}

void Class6E99C__FinishConstruct(Class6E99CObj *self, s32 a1) {
    self->unk70 = a1;
    self->unk6C = 0;
    self->unk74 = 0xA;
    self->unk78 = 0;
    self->unk7C = 0;
    self->methods->slot60(self, 0);
    self->methods->slot64(self, 0);
    self->unk98 = 0;
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
            self->unk64 += (u8)self->unk74;
        }
        if (self->unk78 & 2) {
            self->unk65 += (u8)self->unk74;
        }
        if (self->unk78 & 1) {
            self->unk66 += (u8)self->unk74;
        }
    } else {
        self->methods->slotE0(self, a1);
    }
}

void Class6E99C__SetStep(Class6E99CObj *self, s32 a1) {
    self->unk74 = a1;
}

#ifdef NON_MATCHING
/* NON_MATCHING: 29/35 words, length exact. Residue: instruction
 * scheduling (retail materializes `li $a1,1` immediately after the
 * slotDC dispatch, before computing idx*3; this body defers it to just
 * before the slotB8 call) (docs/match-reports/Class6E99C__StartFadeToIndex.md).
 * Hand-derived. */
void Class6E99C__StartFadeToIndex(Class6E99CObj *self) {
    s32 idx;

    if (self->unk6C != 0) {
        return;
    }
    idx = self->methods->slotDC(self);
    self->methods->slotB8(self, 1, &D_8006EA90[idx * 3]);
    self->unk6C = 1;
    self->unk74 = -self->unk74;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", Class6E99C__StartFadeToIndex);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 40/41 words, 1 word short. Residue: two residues --
 * the shared `li $a1,1`-scheduling class with Class6E99C__StartFadeToIndex, plus a
 * genuinely missing register-only dead-store delay-slot filler
 * (retail copies its unread 3rd argument into $t0 in a branch delay
 * slot; GCC 2.6.3 eliminates the equivalent C statement as dead code
 * before scheduling ever sees it) (docs/match-reports/Class6E99C__StartFadeDefault.md).
 * Hand-derived. */
void Class6E99C__StartFadeDefault(Class6E99CObj *self, s32 a1, s32 a2) {
    s32 idx;

    if (self->unk6C != 0) {
        return;
    }
    idx = self->methods->slotDC(self);
    if (self->unk98 != 0) {
        self->unk80--;
    } else {
        self->methods->slotB8(self, 1, &D_8006EAA8[idx * 3]);
    }
    self->unk6C = 2;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", Class6E99C__StartFadeDefault);
#endif

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
    q1 = 0x100 / self->unk74;
    self->unk7C = a3;
    self->unk80 = q1;
    if (self->unk98 != 0) {
        q2 = q1 / self->unk9C;
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
    if (self->unk6C == 0) {
        return;
    }
    if (self->unk6C == 1) {
        mode = 5;
        if (self->unk98 == 0) {
            methods->slot60(self, 0);
            methods->slot64(self, 0);
        }
    } else {
        mode = 6;
        if (self->unk98 != 0) {
            if (self->unk78 == 0xF) {
                methods->slotB8(self, 1, D_8006EAA8);
            }
            methods->slot64(self, 0);
        }
    }
    methods->slot14(self, a1);
    if (self->unk74 < 0) {
        self->unk74 = -self->unk74;
    }
    self->unk6C = 0;
    methods->slot30(self, mode);
}

void *Class6E99C__GetColor(Class6E99CObj *self) {
    if (self->unk78 == 0xF) {
        return D_8006EAA8;
    }
    return &D_8006EA90[self->unk78 * 3];
}

#ifdef NON_MATCHING
/* NON_MATCHING: 22/25 words, length exact. Residue: redundant-move
 * register residue -- retail additionally does `move $t0,$a1`
 * unconditionally in a branch delay slot and uses $t0 for both a1->x/
 * a1->y loads; nothing in the C forces an early copy of a1, so no
 * source shape tried reproduces the extra move. Permuter-exhausted
 * (~76k combined iterations, two independent runs)
 * (docs/match-reports/Class6E99C__PushPosition.md). Hand-derived. */
void Class6E99C__PushPosition(Class6E99CObj *self, SkipShort2 *a1, Pair32E99C *a2) {
    if (self->unkC != 0) {
        self->unk88 = self->unk60;
        self->unk8C = self->unk62;
        __asm__("" ::: "memory");
        self->unk90 = self->unk50;
        self->unk94 = self->unk54;
        __asm__("" ::: "memory");
        self->unk60 = a1->x;
        self->unk62 = a1->y;
        __asm__("" ::: "memory");
        *(Pair32E99C *)&self->unk50 = *a2;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", Class6E99C__PushPosition);
#endif

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
    self->unk98 = a1;
    self->unk9C = a2;
}

Class6E99CMethods *Class6E99C__GetMethods(void) {
    return &D_8006E99C;
}

ClassEAC0Obj *New_ClassEAC0(void *a0, void *a1, s32 a2) {
    ClassEAC0Obj *self;

    self = func_80017B34(0x6C);
    if (self != NULL) {
        ((ClassEAC0Methods *)Obj6EAC0__GetBaseMethods())->ctor(self, a0, a1, a2);
        return self;
    }
    return NULL;
}

void ClassEAC0__ClassEAC0(ClassEAC0Obj *self, SkipShort2 *a1, void *a2, s32 a3) {
    GetClass6B5CCMethods()->ctor(self);
    self->methods = (ClassEAC0Methods *)Obj6EAC0__GetBaseMethods();
    self->methods->slot40(self, a1, a2, a3);
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
