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
 * class_3bb8c_l -- sixth carved slice of the class_3bb8c block
 * (0x435E0..0x44518, vram 0x80052DE0..0x80053D18), 20 functions.
 * Carved round 15.
 *
 * Blocker profile. The carve-time census was a THREE-grep screen and one of
 * its three blockers is DEAD: `addiu_at` was RESOLVED in round 21 (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md). Re-screened with
 * `python3 tools/nearmiss.py` on 2026-09-08 (round 24):
 *   func_800534C8  gp_rel        -- MATCHED round 45 (122/122 words). The
 *                  `gp_rel` blocker itself was RESOLVED round 42; see the
 *                  file-top banner above.
 *   func_80052F10  was addiu-$at ONLY -- NOT BLOCKED. 137w.
 *                  ROUND 32: MATCHED, 137/137.
 *   func_80053984  was addiu-$at ONLY -- NOT BLOCKED. 82w.
 *                  ROUND 32: MATCHED, 82/82, first attempt.
 *                  It OWNS jtbl_8001174C; the rodata slot at 0x1F4C is
 *                  attached to this unit for that reason, and a
 *                  `%lo(jtbl_*)` load is ordinary matchable code now.
 * The previous version of this comment ended "All three have stub reports;
 * do not attempt them" -- a stale DIRECTIVE, which is worse than a stale
 * count. Two of the three are free ground.
 * The other 17 are clean (ObjM__NoOpSlot40 and ObjM__NoOpSlot7C are bare
 * `jr $ra; nop` stubs splat generated itself, so 15 are real work).
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include "class_3bb8c.h"

/* This unit's local view of GetClass86668Methods's table, and the prototype for
 * that getter. Declared HERE, not in include/class_3bb8c.h: the canonical
 * `extern Class86668Methods *GetClass86668Methods(void);` lives in
 * include/class_39e08.h, and any unit including BOTH headers gets
 * `conflicting types for 'GetClass86668Methods'` -- which is exactly how this was
 * found, when class_3bb8c_k was merged. class_3bb8c_l does not include
 * class_39e08.h, so a unit-local declaration is safe here.
 *
 * class_39e08.h's Class86668Methods already declares slot44 and slot48 at
 * these same offsets with ABI-identical shapes. Prefer unifying onto that
 * type when someone next touches this unit, as its own change with the
 * whole-image SHA1 re-verified after. */
typedef struct BaseMethods87034_3bb8c_l {
    u8 pad00[0x044];
    s32 (*slot44)(Obj87034_3bb8c_l *self, Obj87034_3bb8c_l *arg1, s32 arg2); /* +0x044, ObjM__AttachTarget */
    void (*slot48)(Obj87034_3bb8c_l *self); /* +0x048, ObjM__DetachTarget */
} BaseMethods87034_3bb8c_l;
extern BaseMethods87034_3bb8c_l *GetClass86668Methods(void);

void ObjM__NoOpSlot40(void) {
}

void ObjM__AttachTarget(Obj87034_3bb8c_l *self, Obj87034_3bb8c_l *arg1, s32 arg2) {
    arg1->unkC->methods->slotC8(arg1->unkC, ObjM__OnRegistrantEvent, self);
    self->unk3C = (DreamSysObj_3bb8c_l *)arg2;
    GetClass86668Methods()->slot44(self, arg1, 1);
    self->methods->slot10(self, arg2);
}

void ObjM__OnRegistrantEvent(Obj87034_3bb8c_l *self, s32 code, s32 arg2, s32 arg3) {
    if (code >= 0) {
        func_80049060(self->unk38);
    } else {
        func_80049098(self->unk38, arg2, arg3);
    }
}

void ObjM__DetachTarget(Obj87034_3bb8c_l *self) {
    self->methods->slot14(self, self->unk3C);
    GetClass86668Methods()->slot48(self);
}

/* Cross-unit helpers with no established prototype elsewhere; declared
 * K&R-free with the argument widths this call site's registers show.
 * Return types are opaque (register-width values forwarded to further
 * calls, never dereferenced here). */
extern s32 func_80048F84(void *arg0, s32 arg1);
extern s32 func_80048EA0(void *arg0, s32 arg1, s32 arg2);
extern s32 func_80043008(s32 arg0);
extern void func_8001EF60(s32 arg0);
extern s32 RegisterStyleConfig(void *arg0, s32 arg1, s32 *arg2, s32 arg3, s32 arg4);

/* Opaque data blobs, referenced only by address (never loaded here) and
 * forwarded to method-table calls of unidentified classes. */
extern s32 D_8008715C;
extern s32 D_80087168;
extern s32 D_80087118[];
extern s32 D_80087150;

void func_80052F10(Obj87034_3bb8c_l *self, s32 arg1, Unk50Struct_3bb8c_l *arg2, s32 arg3) {
    DreamSysObj_3bb8c_l *unk18 = self->unk18;
    s32 ret1;
    s32 flag;

    unk18->methods->slot74(unk18);
    self->unk60 = 1;
    ret1 = func_80048F84(self->unk38, 0);
    self->unk54->methods->slot5C(self->unk54, ret1);

    ret1 = self->unk3C->methods->slot1A0(self->unk3C, 0);
    ret1 = func_80048EA0(self->unk38, 0, ret1);
    self->unk58 = (Obj87034_3bb8c_l *) func_80043008(ret1);

    unk18->methods->slot70(unk18, self->unk3C, &D_8008715C, &D_80087168, 0);

    self->unk78 = unk18;
    ret1 = self->unk3C->methods->slot1A0(self->unk3C, 0);
    self->unk50 = (Unk50Struct_3bb8c_l *) RegisterStyleConfig(self->unk14, self->unk38, &self->unk6C, ret1, 0);
    if (arg2 != 0) {
        self->unk50 = arg2;
    }

    self->unk4C = arg3;
    if (self->unk38 != 0) {
        s32 unk38val;
        s32 three;

        /* Retail reloads self->unk38 here even though the outer `if`
         * just read it and nothing wrote it in between -- a volatile-
         * qualified POINTER TYPE at the read site (not a volatile
         * object) forces the reload without changing unk38's own
         * declared type, the same idiom code_179d8_m.c documents for
         * D_8008EA26's `*(u8 *)&sym`, used here in the opposite
         * direction (forcing a reload instead of permitting a fold). */
        unk38val = (s32) *(void * volatile *) &self->unk38;
        self->unk40 = 0x10;
        three = 3;
        /* Order-only: without this barrier the scheduler moves `three`'s
         * `li` past the `self->unk40` store; removing it does not change
         * which register holds which value. */
        __asm__("");
        flag = (unk38val == 5);
        if (unk38val == 6) {
            flag = 1;
        }
        self->unk44 = three;
        if (unk38val == three) {
            flag = 1;
        }
        self->unk14->methods->slot134(self->unk14, 0);
    } else {
        self->unk40 = 0x10;
        self->unk44 = 2;
        flag = 1;
        self->unk14->methods->slot134(self->unk14, &D_80087150);
    }

    self->unk48 = arg1;
    if (arg1 == 0) {
        self->unk48 = 0xA000;
    }
    func_8001EF60(flag);

    self->unk3C->methods->slotEC(self->unk3C, D_80087118[(s32) self->unk38]);
    self->unk20 = 5;
}

void func_80053134(Obj87034_3bb8c_l *self) {
    self->methods->slot84(self);
    TickDreamAuxSlots2();
    StyleTeardown();
    self->unk54->methods->slot48(self->unk54);
}

void func_800531A0(Obj87034_3bb8c_l *self, void *arg1, s32 sel) {
    if (sel == 2) {
        func_800531CC(self, self->unk58);
    }
}

void func_800531CC(Obj87034_3bb8c_l *self, Obj87034_3bb8c_l *other) {
    s32 ret;
    s32 sel;
    void *a1;
    Obj87034Methods_3bb8c_l *m;

    if (self->unk60 != 0) {
        if (other->unk80 != 0) {
            other->methods->slot04(other);
            self->unk60 = 0;
            self->methods->slot80(self);
            ret = self->unk3C->methods->slot108(self->unk3C);
            self->unk3C->methods->slot104(self->unk3C, ret + 0x1E);
        } else if (other->unk3C != 0) {
            sel = self->unk50->unk14;
            m = other->methods;
            if (sel != 2) {
                a1 = self->unk50->unk18;
            } else {
                a1 = self->unk50->unkC;
            }
            m->slot7C(other, a1);
            other->methods->slot04(other);
            self->unk60 = 0;
            self->methods->slot80(self);
        }
    }
    if (self->unk60 == 0) {
        if (self->unk14->unk1B4 == 0 && self->unk68 == 0) {
            self->unk64 = 1;
            self->methods->slot88(self);
        }
    }
}

void func_80053358(Obj87034_3bb8c_l *self, void *arg1, s32 eventId) {
    Obj87034Methods_3bb8c_l *m = self->methods;
    void (*fn)(Obj87034_3bb8c_l *);

    if (self->unk68 == 0) {
        return;
    }
    if (eventId == 0x16) {
        goto case_c8;
    }
    if (eventId < 0x17) {
        if (eventId == 0xC) {
            goto case_c0;
        }
        return;
    }
    if (eventId == 0x21) {
        goto case_74;
    }
    if (eventId == 0x2C) {
        goto case_c4;
    }
    return;
case_74:
    fn = m->slot74;
    goto call;
case_c0:
    fn = m->slotC0;
    goto call;
case_c8:
    fn = m->slotC8;
    goto call;
case_c4:
    fn = m->slotC4;
call:
    fn(self);
}

void func_800533F0(Obj87034_3bb8c_l *self) {
    void (*fn)(Obj87034_3bb8c_l *);

    if (self->unk68 != 0) {
        self->unk1C++;
        if (self->unk80 != 0) {
            fn = self->methods->slotD0;
        } else {
            fn = self->methods->slot8C;
        }
        fn(self);
    }
}

void func_80053458(Obj87034_3bb8c_l *self) {
    Obj87034Methods_3bb8c_l *m = self->methods;

    if (self->unk80 != 0) {
        m->slotC4(self);
        m->slotD4(self);
    } else {
        m->slotD0(self);
    }
}

void ObjM__NoOpSlot7C(void) {
}

/* self->unkC's real pointee for THIS function -- a DIFFERENT reading from
 * `RegistrantObj_3bb8c_l` (ObjM__AttachTarget's own local view of the same
 * field): here `*(self->unkC)` (one dereference through unkC's own first
 * word) yields an object with its OWN methods pointer at +0x000, matching
 * `RegistrantObj_3bb8c_l`'s own layout (a single `methods` field) exactly
 * -- so no field-type change is needed, just a same-shape reinterpret at
 * this call site, per the project's independent-arities convention. */
typedef struct UnkCObj_3bb8c_l UnkCObj_3bb8c_l;
typedef struct UnkCObjMethods_3bb8c_l UnkCObjMethods_3bb8c_l;
struct UnkCObjMethods_3bb8c_l {
    u8 pad000[0x07C];
    /* +0x07C, round 45's func_800534C8: `(self, 0)`, returning a pointer
     * to a single `s32` this function dereferences immediately. */
    s32 *(*slot7C)(UnkCObj_3bb8c_l *self, s32 arg1); /* +0x07C */
};
struct UnkCObj_3bb8c_l {
    UnkCObjMethods_3bb8c_l *methods; /* +0x000 */
};

/* Uncarved cross-unit helper (code_4cd08.c, MATCHED round 43) -- no header
 * declares it, so this unit's own call-site typing is local, all-`s32`
 * per that function's own definition. */
extern void SetDreamAuxWorld(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4);

/* This unit's own local reading of the already-matched `GetStageGridDimensions`
 * (include/StageGrid.h, `(s32 index)` -> `StageGridDimensions *`) -- kept
 * opaque `void *` here since the return value is only forwarded, never
 * dereferenced, and this unit does not otherwise include that header. */
extern void *GetStageGridDimensions(s32 index);

/* VALUE-of `%gp_rel`, round 45's own local view -- a plain `s32` bias
 * added to the derived value passed to `DreamSysMethods_3bb8c_l::slot54`. */
extern s32 D_8008AB34;

/* VALUE-of `%gp_rel`... actually address-of only here (`&D_8008710C`),
 * round 45's own local view -- an opaque .data block (asm/data/76DC8.data.s),
 * same convention as D_8008715C/D_80087168 above, forwarded to
 * `Obj14Methods_3bb8c_l::slotCC`. */
extern s32 D_8008710C;

void func_800534C8(Obj87034_3bb8c_l *self) {
    DreamSysObj_3bb8c_l *unk18 = self->unk18;
    Unk50Struct_3bb8c_l *unk50 = self->unk50;
    UnkCObj_3bb8c_l *obj;
    s32 val;
    Obj14_3bb8c_l *unk14;

    unk18->methods->slot74(unk18);

    obj = *(UnkCObj_3bb8c_l **)self->unkC;
    val = *obj->methods->slot7C(obj, 0);
    unk18->methods->slot54(unk18, val / 2 * 5 / 3 + D_8008AB34);

    unk18->methods->slot70(unk18, self->unk3C, &D_8008715C, &D_80087168, 0);

    SetDreamAuxWorld((s32)self->unk38, (s32)self->unk14, (s32)self->unk3C, self->unk34, self->unk10);

    unk14 = self->unk14;
    self->methods->slot10(self, (s32)unk14);

    unk14->methods->slotBC(unk14, unk50->unk8, 0);
    unk14->methods->slotC4(unk14, 3, unk50->unk0, unk50->unk4);
    unk14->methods->slotE0(unk14, GetStageGridDimensions((s32)self->unk38));
    self->unk3C->methods->slot4C(self->unk3C, unk14);
    unk14->methods->slotDC(unk14, self->unk48);
    unk14->methods->slotCC(unk14, &D_8008710C);
}

void func_800536B0(Obj87034_3bb8c_l *self) {
    self->methods->slotD4(self);
    self->unk3C->methods->slotFC(self->unk3C);
    self->unk3C->methods->slot50(self->unk3C);
    self->unk18->methods->slot74(self->unk18);
    self->methods->slot14(self, self->unk14);
}

void func_80053764(Obj87034_3bb8c_l *self) {
    DreamSysMethods_3bb8c_l *m;
    DreamSysMethods_3bb8c_l *m2;
    DreamSysObj_3bb8c_l *unk18;
    DreamSysObj_3bb8c_l *newObj;
    Unk50Struct_3bb8c_l *unk50;
    s32 local10;
    s32 ret;
    s32 a2;
    void *a1;

    self->unk68 = 1;
    self->unk3C->methods->slotF8(self->unk3C, self->unk44, self->unk40);
    self->unk14->methods->slotEC(self->unk14);

    unk18 = self->unk18;
    unk50 = self->unk50;
    unk18->methods->slot60(unk18, 1);
    unk18->methods->slot64(unk18, unk50->unkC);
    unk18->methods->slot6C(unk18, unk50->unk1C);
    m = unk18->methods;
    if (unk50->unk14 != 1) {
        a1 = unk50->unk18;
    } else {
        a1 = unk50->unkC;
    }
    m->slot68(unk18, a1);
    unk18->methods->slotB0(unk18, 0);
    unk18->methods->slotB4(unk18, 1);

    newObj = unk18->methods->slotAC(unk18);
    self->methods->slot10(self, (s32)newObj);

    ret = self->unk3C->methods->slotF0(self->unk3C, &local10, -1);
    newObj->methods->slotF0(newObj, (s32 *)ret, (ret != 0) ? 3 : 0);
    m2 = newObj->methods;
    if (ret == 0) {
        a2 = -1;
    } else {
        a2 = local10;
    }
    m2->slotD4(newObj, self->unk10, a2, 0);
}

void func_8005393C(Obj87034_3bb8c_l *self) {
    TickStyle(self->unk14->methods->slot10C(self->unk14, 0, 0), 0, 0);
}

void func_80053984(Obj87034_3bb8c_l *self, void *arg1, s32 code) {
    if (self->unk20 == 0) {
        switch (code - 0xA) {
        case 0:
            self->methods->slot94(self);
            break;
        case 1:
            break;
        case 2:
            self->methods->slot98(self);
            break;
        case 3:
            self->methods->slot9C(self);
            break;
        case 4:
            self->methods->slotA0(self);
            break;
        case 5:
            self->methods->slotA4(self);
            break;
        case 6:
            self->methods->slotA8(self);
            break;
        case 7:
            self->methods->slotAC(self);
            break;
        }
    } else if (code >= 9) {
        self->unk3C->unk44 = 0;
    }
}

/* func_80053ACC's (and func_80053BE8's/func_80053C94's, further below) own
 * helper, and it lives in the sibling slice class_3bb8c_m, where round 15's
 * runner echo matched it byte-exact as `void ObjM__ForwardToSubChild(ObjM *self, s32,
 * s32, s32, s32)`. Declared locally rather than in include/class_3bb8c.h on
 * purpose: this unit's view of the class is `Obj87034_3bb8c_l` and echo's is
 * `ObjM`, the two are the same class (see the HEAD NOTE in that header), and
 * a shared-header declaration would put two incompatible prototypes for one
 * function in front of both translation units. The return type is echo's,
 * from the definition. */
extern void ObjM__ForwardToSubChild(Obj87034_3bb8c_l *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

void func_80053ACC(Obj87034_3bb8c_l *self) {
    s32 local18;
    s32 span;
    s32 t;
    s32 arg3;

    self->unk20 = 4;
    if (self->unk3C->methods->slotF0(self->unk3C, &local18, -1) == 0) {
        span = (self->unk1C + (s32)self->unk38) & 3;
        t = span;
        if (t == 0) {
            self->methods->slot30(self, 4);
            return;
        }
        arg3 = 0xA;
        switch (t) {
        case 1:
            local18 = 0;
            break;
        case 2:
            local18 = 4;
            break;
        case 3:
            local18 = 7;
            arg3 = 5;
            break;
        }
        ObjM__ForwardToSubChild(self, local18, 0, arg3, 1);
        return;
    }
    ObjM__ForwardToSubChild(self, 0, 0, 5, 1);
}

void func_80053BE8(Obj87034_3bb8c_l *self) {
    s32 color;

    if (self->unk3C->unk164 < 0) {
        self->methods->slot9C(self);
    } else {
        self->unk20 = 5;
        color = self->unk3C->methods->slot200(self->unk3C);
        ObjM__ForwardToSubChild(self, color, 0, 0xA, 1);
        self->unk3C->methods->slotFC(self->unk3C);
    }
}

void func_80053C94(Obj87034_3bb8c_l *self) {
    s32 color;

    self->unk20 = 6;
    color = self->unk3C->methods->slot200(self->unk3C);
    ObjM__ForwardToSubChild(self, color, 0, 0x1E, 1);
    self->unk3C->methods->slotFC(self->unk3C);
}
