/*
 * class_3bb8c_l -- sixth carved slice of the class_3bb8c block
 * (0x435E0..0x44518, vram 0x80052DE0..0x80053D18), 20 functions, ALL
 * MATCHED. Carved round 15; fully matched by round 45.
 *
 * This slice is entirely `ObjM`'s own methods -- confirmed, not guessed:
 * `tools/classtable.py 0x80087034` resolves the class's vtable directly to
 * `ObjM__ObjM` (ctor) and `ObjM__Dtor` (dtor), the SAME class sibling unit
 * class_3bb8c_m independently reached and named. The two units are NOT
 * unified (round-13 struct-edit hazard; see the HEAD NOTE above
 * `Obj87034_3bb8c_l`'s definition in include/class_3bb8c.h) -- this unit
 * keeps its own local struct view, `Obj87034_3bb8c_l`, but round 78 named
 * every function here with the confirmed `ObjM__` prefix to match.
 *
 * Mechanically this is the class's target/child attach-detach pair
 * (`ObjM__AttachTarget`/`ObjM__DetachTarget`), its style/scene/world setup
 * and teardown routines, an event dispatcher, and three of the class's
 * `EnterStateN` handlers (`ObjM__EnterState4/5/6`) -- continuing, on the
 * same `phase` field, the numbering class_3bb8c_m already established for
 * `ObjM__EnterState7/8/A`. `ObjM__HandleStateCode` is the state-transition
 * dispatcher that routes codes 0xA..0x11 onto those `EnterStateN` slots
 * one-to-one (owns `jtbl_8001174C`). Two slots (`ObjM__NoOpSlot40`,
 * `ObjM__NoOpSlot7C`) are splat-generated `jr $ra; nop` stubs, not work.
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "Class86668.h"
#include "DreamSys.h"

void ObjM__NoOpSlot40(void) {
}

void ObjM__AttachTarget(Obj87034_3bb8c_l *self, Obj87034_3bb8c_l *arg1, s32 arg2) {
    arg1->unkC->methods->slotC8(arg1->unkC, ObjM__OnRegistrantEvent, self);
    self->target = (DreamSys *)arg2;
    GetClass86668Methods()->init((Class86668 *)self, (IntermediateBaseInitArgs *)arg1, 1);
    self->methods->slot10(self, arg2);
}

void ObjM__OnRegistrantEvent(Obj87034_3bb8c_l *self, s32 code, s32 arg2, s32 arg3) {
    if (code >= 0) {
        GetGridRecordAt(self->unk38, code);
    } else {
        GetGridRecordXY(self->unk38, arg2, arg3);
    }
}

void ObjM__DetachTarget(Obj87034_3bb8c_l *self) {
    self->methods->slot14(self, self->target);
    GetClass86668Methods()->deinit((Class86668 *)self);
}

/* Cross-unit helpers with no established prototype elsewhere; declared
 * K&R-free with the argument widths this call site's registers show.
 * Return types are opaque (register-width values forwarded to further
 * calls, never dereferenced here). */
extern s32 PickVariant(void *arg0, s32 arg1);
extern s32 PickDailyVariant(void *arg0, s32 arg1, s32 arg2);
extern s32 New_TimBlockSrc(s32 arg0);
extern void func_8001EF60(s32 arg0);
extern s32 RegisterStyleConfig(void *arg0, s32 arg1, s32 *arg2, s32 arg3, s32 arg4);

/* Opaque data blobs, referenced only by address (never loaded here) and
 * forwarded to method-table calls of unidentified classes. */
extern s32 D_8008715C;
extern s32 D_80087168;
extern s32 D_80087118[];
extern s32 D_80087150;

void ObjM__InitStyleAndWorld(Obj87034_3bb8c_l *self, s32 arg1, Unk50Struct_3bb8c_l *arg2, s32 arg3) {
    StyleWorldObj_3bb8c_l *unk18 = self->world;
    s32 ret1;
    s32 flag;

    unk18->methods->slot74(unk18);
    self->hasTarget = 1;
    ret1 = PickVariant(self->unk38, 0);
    self->unk54->methods->slot5C(self->unk54, ret1);

    ret1 = self->target->methods->getCurrentDayAndYear(self->target, 0);
    ret1 = PickDailyVariant(self->unk38, 0, ret1);
    self->pendingOther = (Obj87034_3bb8c_l *) New_TimBlockSrc(ret1);

    unk18->methods->slot70(unk18, self->target, &D_8008715C, &D_80087168, 0);

    self->cachedWorld = unk18;
    ret1 = self->target->methods->getCurrentDayAndYear(self->target, 0);
    self->styleConfig = (Unk50Struct_3bb8c_l *) RegisterStyleConfig(self->unk14, self->unk38, &self->unk6C, ret1, 0);
    if (arg2 != 0) {
        self->styleConfig = arg2;
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

    self->target->methods->setPendingExtra(self->target, D_80087118[(s32) self->unk38]);
    self->phase = 5;
}

void ObjM__TeardownStyle(Obj87034_3bb8c_l *self) {
    self->methods->slot84(self);
    TickDreamAuxSlots2();
    StyleTeardown();
    self->unk54->methods->slot48(self->unk54);
}

void ObjM__OnSelectTransfer(Obj87034_3bb8c_l *self, void *arg1, s32 sel) {
    if (sel == 2) {
        ObjM__TransferToOther(self, self->pendingOther);
    }
}

void ObjM__TransferToOther(Obj87034_3bb8c_l *self, Obj87034_3bb8c_l *other) {
    s32 ret;
    s32 sel;
    void *a1;
    Obj87034Methods_3bb8c_l *m;

    if (self->hasTarget != 0) {
        if (other->unk80 != 0) {
            other->methods->slot04(other);
            self->hasTarget = 0;
            self->methods->slot80(self);
            ret = self->target->methods->getDreamTimerScaled(self->target);
            self->target->methods->getSetDreamTimeLimit(self->target, ret + 0x1E);
        } else if (other->target != 0) {
            sel = self->styleConfig->unk14;
            m = other->methods;
            if (sel != 2) {
                a1 = self->styleConfig->unk18;
            } else {
                a1 = self->styleConfig->unkC;
            }
            m->slot7C(other, a1);
            other->methods->slot04(other);
            self->hasTarget = 0;
            self->methods->slot80(self);
        }
    }
    if (self->hasTarget == 0) {
        if (self->unk14->unk1B4 == 0 && self->attached == 0) {
            self->unk64 = 1;
            self->methods->slot88(self);
        }
    }
}

void ObjM__DispatchEvent(Obj87034_3bb8c_l *self, void *arg1, s32 eventId) {
    Obj87034Methods_3bb8c_l *m = self->methods;
    void (*fn)(Obj87034_3bb8c_l *);

    if (self->attached == 0) {
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

void ObjM__TickTarget(Obj87034_3bb8c_l *self) {
    void (*fn)(Obj87034_3bb8c_l *);

    if (self->attached != 0) {
        self->unk1C++;
        if (self->unk80 != 0) {
            fn = self->methods->slotD0;
        } else {
            fn = self->methods->slot8C;
        }
        fn(self);
    }
}

void ObjM__DispatchActiveState(Obj87034_3bb8c_l *self) {
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
    /* +0x07C, round 45's ObjM__SetupSceneStyle: `(self, 0)`, returning a pointer
     * to a single `s32` this function dereferences immediately. */
    s32 *(*slot7C)(UnkCObj_3bb8c_l *self, s32 arg1); /* +0x07C */
};
struct UnkCObj_3bb8c_l {
    UnkCObjMethods_3bb8c_l *methods; /* +0x000 */
};

/* code_4cd08.c's (MATCHED round 43); no header declares it. `world` is the
 * DreamSys it installs as gDreamAuxWorld (track 4, round 88). */
extern void SetDreamAuxWorld(s32 a0, s32 a1, DreamSys *world, s32 a3, s32 a4);

/* GetStageGridDimensions comes from include/StageGrid.h, through
 * DreamSys.h (the local `void *` reading that stood here went with it). */

/* VALUE-of `%gp_rel`, round 45's own local view -- a plain `s32` bias
 * added to the derived value passed to `StyleWorldMethods_3bb8c_l::slot54`. */
extern s32 D_8008AB34;

/* VALUE-of `%gp_rel`... actually address-of only here (`&D_8008710C`),
 * round 45's own local view -- an opaque .data block (asm/data/76DC8.data.s),
 * same convention as D_8008715C/D_80087168 above, forwarded to
 * `Obj14Methods_3bb8c_l::slotCC`. */
extern s32 D_8008710C;

void ObjM__SetupSceneStyle(Obj87034_3bb8c_l *self) {
    StyleWorldObj_3bb8c_l *unk18 = self->world;
    Unk50Struct_3bb8c_l *unk50 = self->styleConfig;
    UnkCObj_3bb8c_l *obj;
    s32 val;
    Obj14_3bb8c_l *unk14;

    unk18->methods->slot74(unk18);

    obj = *(UnkCObj_3bb8c_l **)self->unkC;
    val = *obj->methods->slot7C(obj, 0);
    unk18->methods->slot54(unk18, val / 2 * 5 / 3 + D_8008AB34);

    unk18->methods->slot70(unk18, self->target, &D_8008715C, &D_80087168, 0);

    SetDreamAuxWorld((s32)self->unk38, (s32)self->unk14, self->target, self->unk34, self->unk10);

    unk14 = self->unk14;
    self->methods->slot10(self, (s32)unk14);

    unk14->methods->slotBC(unk14, unk50->unk8, 0);
    unk14->methods->slotC4(unk14, 3, unk50->unk0, unk50->unk4);
    unk14->methods->slotE0(unk14, GetStageGridDimensions((s32)self->unk38));
    ((DreamSysAttachToParentFn)self->target->methods->attachToParent)(self->target, unk14);
    unk14->methods->slotDC(unk14, self->unk48);
    unk14->methods->slotCC(unk14, &D_8008710C);
}

void ObjM__ExitSceneStyle(Obj87034_3bb8c_l *self) {
    self->methods->slotD4(self);
    self->target->methods->blockMovement(self->target);
    self->target->methods->detachFromParent(self->target);
    self->world->methods->slot74(self->world);
    self->methods->slot14(self, self->unk14);
}

void ObjM__EnterStyleSession(Obj87034_3bb8c_l *self) {
    StyleWorldMethods_3bb8c_l *m;
    StyleWorldMethods_3bb8c_l *m2;
    StyleWorldObj_3bb8c_l *unk18;
    StyleWorldObj_3bb8c_l *newObj;
    Unk50Struct_3bb8c_l *unk50;
    s32 local10;
    s32 ret;
    s32 a2;
    void *a1;

    self->attached = 1;
    self->target->methods->resetLinkState(self->target, self->unk44, self->unk40);
    self->unk14->methods->slotEC(self->unk14);

    unk18 = self->world;
    unk50 = self->styleConfig;
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

    ret = self->target->methods->getSetFlashbackSession(self->target, (DreamColors *)&local10, -1);
    newObj->methods->slotF0(newObj, (s32 *)ret, (ret != 0) ? 3 : 0);
    m2 = newObj->methods;
    if (ret == 0) {
        a2 = -1;
    } else {
        a2 = local10;
    }
    m2->slotD4(newObj, self->unk10, a2, 0);
}

void ObjM__TickStyle(Obj87034_3bb8c_l *self) {
    TickStyle(self->unk14->methods->slot10C(self->unk14, 0, 0), 0, 0);
}

void ObjM__HandleStateCode(Obj87034_3bb8c_l *self, void *arg1, s32 code) {
    if (self->phase == 0) {
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
        self->target->state = 0;
    }
}

/* ObjM__EnterState4's (and ObjM__EnterState5's/ObjM__EnterState6's, further below) own
 * helper, and it lives in the sibling slice class_3bb8c_m, where round 15's
 * runner echo matched it byte-exact as `void ObjM__ForwardToSubChild(ObjM *self, s32,
 * s32, s32, s32)`. Declared locally rather than in include/class_3bb8c.h on
 * purpose: this unit's view of the class is `Obj87034_3bb8c_l` and echo's is
 * `ObjM`, the two are the same class (see the HEAD NOTE in that header), and
 * a shared-header declaration would put two incompatible prototypes for one
 * function in front of both translation units. The return type is echo's,
 * from the definition. */
extern void ObjM__ForwardToSubChild(Obj87034_3bb8c_l *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

void ObjM__EnterState4(Obj87034_3bb8c_l *self) {
    s32 local18;
    s32 span;
    s32 t;
    s32 arg3;

    self->phase = 4;
    if (self->target->methods->getSetFlashbackSession(self->target, (DreamColors *)&local18, -1) == 0) {
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

void ObjM__EnterState5(Obj87034_3bb8c_l *self) {
    s32 color;

    if (self->target->currentStage < 0) {
        self->methods->slot9C(self);
    } else {
        self->phase = 5;
        color = self->target->methods->getDreamColor(self->target);
        ObjM__ForwardToSubChild(self, color, 0, 0xA, 1);
        self->target->methods->blockMovement(self->target);
    }
}

void ObjM__EnterState6(Obj87034_3bb8c_l *self) {
    s32 color;

    self->phase = 6;
    color = self->target->methods->getDreamColor(self->target);
    ObjM__ForwardToSubChild(self, color, 0, 0x1E, 1);
    self->target->methods->blockMovement(self->target);
}
