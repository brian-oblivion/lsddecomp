/*
 * class_3bb8c_l -- sixth carved slice of the class_3bb8c block
 * (0x435E0..0x44518, vram 0x80052DE0..0x80053D18), 20 functions, ALL
 * MATCHED. Carved round 15; fully matched by round 45.
 *
 * This slice is entirely ObjM's own methods (gObjMMethods, include/ObjM.h;
 * track 4, round 89 unified the class_3bb8c_k/_l/_m views there), slots
 * +0x040..+0x09C in table order: the init/deinit pair (AttachTarget keeps
 * the DreamSys and hooks the Class866E8's callback, DetachTarget), onInit
 * and onDeinit (InitStyleAndWorld, TeardownStyle), onTag1Notify with the
 * TimBlockSrc poll it runs (PollTimBlockLoad), onPadEvent
 * (DispatchPadEvent), update, togglePause (ObjM__TogglePause), the style scene
 * slots +0x080..+0x08C, the DreamSys notification dispatcher
 * (OnDreamSysNotify, owning `jtbl_8001174C`) and EnterState4/5/6, which
 * set IntermediateBase::state and start a fade (ObjM__StartFadeUp,
 * class_3bb8c_m). NoOpSlot40 and NoOpSlot7C are empty.
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "TimedTask.h"
#include "DreamSys.h"
#include "ObjM.h"
#include "Class866E8.h"
#include "NodeGuardedViewport.h"
#include "Class6E99C.h"
#include "TimBlockSrc.h"
#include "WBgm.h"

void ObjM__NoOpSlot40(void) {}

/* init. `args` is the building Class865C8's init args: args->unkC is its
 * Class866E8 (IntermediateBase__Init keeps it as unk14), whose callback
 * becomes ObjM__OnRegistrantEvent. */
void ObjM__AttachTarget(ObjM *self, IntermediateBaseInitArgs *args, DreamSys *dreamSys) {
    ((Class866E8 *)args->lightRig)
        ->methods->setCallback((Class866E8 *)args->lightRig,
                               (Class866E8ValueFn)ObjM__OnRegistrantEvent, self);
    self->dreamSys = dreamSys;
    GetTimedTaskMethods()->init((TimedTask *)self, args, 1);
    self->methods->addChild(self, (BasicClass *)dreamSys);
}

void ObjM__OnRegistrantEvent(ObjM *self, s32 code, s32 arg2, s32 arg3) {
    if (code >= 0) {
        GetGridRecordAt(self->stage, code);
    } else {
        GetGridRecordXY(self->stage, arg2, arg3);
    }
}

void ObjM__DetachTarget(ObjM *self) {
    self->methods->removeChild(self, (BasicClass *)self->dreamSys);
    GetTimedTaskMethods()->deinit((TimedTask *)self);
}

/* Cross-unit helpers, src/code_39094.c (PickVariant, PickDailyVariant:
 * `Rec1C *(s32 index, ...)`, read here as the value handed on),
 * src/code_d294_c.c (func_8001EF60) and class_3bb8c_m (RegisterStyleConfig,
 * whose third argument is kept as gStyleTargetObj). */
extern s32 PickVariant(s32 index, s32 arg1);
extern s32 PickDailyVariant(s32 index, s32 arg1, s32 day);
extern void func_8001EF60(s32 arg0);
extern s32 RegisterStyleConfig(void *arg0, s32 arg1, s32 *arg2, s32 arg3, s32 arg4);

/* Data reached by address: the viewport's view point and view reference
 * (attachViewChild), the Class866E8's bounds; and D_80087118, one
 * setPendingExtra value per stage. */
extern s32 D_8008715C;
extern s32 D_80087168;
extern s32 D_80087118[];
extern s32 D_80087150;

/* onInit (IntermediateBase__Init passes 0, 0, 0). */
void ObjM__InitStyleAndWorld(ObjM *self, s32 gridSpan, Unk50Struct_3bb8c_l *style, s32 arg3) {
    NodeGuardedViewport *vp = (NodeGuardedViewport *)self->viewport;
    s32 ret1;
    s32 flag;

    vp->methods->detachViewChild(vp);
    self->timBlockPending = 1;
    ret1 = PickVariant(self->stage, 0);
    self->bgm->methods->setSeq(self->bgm, (char *)ret1);

    ret1 = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    ret1 = PickDailyVariant(self->stage, 0, ret1);
    self->timBlockSrc = (TimBlockSrc *)New_TimBlockSrc(ret1);

    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, (LongVec3 *)&D_8008715C,
                                 (LongVec3 *)&D_80087168, 0);

    self->cachedViewport = vp;
    ret1 = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    self->styleConfig = (Unk50Struct_3bb8c_l *)RegisterStyleConfig(self->unk14, self->stage,
                                                                   (s32 *)&self->ctorSound, ret1, 0);
    if (style != 0) {
        self->styleConfig = style;
    }

    self->unk4C = arg3;
    if (self->stage != 0) {
        s32 stage;
        s32 three;

        /* Retail reloads self->stage here even though the outer `if`
         * just read it and nothing wrote it in between -- a volatile-
         * qualified POINTER TYPE at the read site (not a volatile
         * object) forces the reload without changing the field's own
         * declared type, the same idiom code_179d8_m.c documents for
         * D_8008EA26's `*(u8 *)&sym`, used here in the opposite
         * direction (forcing a reload instead of permitting a fold). */
        stage = *(s32 volatile *)&self->stage;
        self->unk40 = 0x10;
        three = 3;
        /* Order-only: without this barrier the scheduler moves `three`'s
         * `li` past the `self->unk40` store; removing it does not change
         * which register holds which value. */
        __asm__("");
        flag = (stage == 5);
        if (stage == 6) {
            flag = 1;
        }
        self->unk44 = three;
        if (stage == three) {
            flag = 1;
        }
        ((Class866E8 *)self->unk14)->methods->setBounds((Class866E8 *)self->unk14, 0);
    } else {
        self->unk40 = 0x10;
        self->unk44 = 2;
        flag = 1;
        ((Class866E8 *)self->unk14)
            ->methods->setBounds((Class866E8 *)self->unk14, (Bounds866E8_3bb8c_b *)&D_80087150);
    }

    self->gridSpan = gridSpan;
    if (gridSpan == 0) {
        self->gridSpan = 0xA000;
    }
    func_8001EF60(flag);

    self->dreamSys->methods->setPendingExtra(self->dreamSys, D_80087118[self->stage]);
    self->state = 5;
}

/* onDeinit. */
void ObjM__TeardownStyle(ObjM *self) {
    self->methods->exitSceneStyle(self);
    TickDreamAuxSlots2();
    StyleTeardown();
    self->bgm->methods->stop(self->bgm);
}

void ObjM__OnTag1Notify(ObjM *self, void *sender, s32 event) {
    if (event == 2) {
        ObjM__PollTimBlockLoad(self, self->timBlockSrc);
    }
}

/* `src` is always self->timBlockSrc. Loaded, its CLUT rows fade to a
 * styleConfig colour; failed or loaded, it is released and the scene set
 * up (a failure also adds 0x1E to the DreamSys's time limit). With nothing
 * pending and the Class866E8 idle, the style session starts. */
void ObjM__PollTimBlockLoad(ObjM *self, TimBlockSrc *src) {
    s32 ret;
    s32 sel;
    TimBlockSrcColor *color;
    TimBlockSrcMethods *m;

    if (self->timBlockPending != 0) {
        if (src->failed != 0) {
            src->methods->release(src);
            self->timBlockPending = 0;
            self->methods->setupSceneStyle(self);
            ret = self->dreamSys->methods->getDreamTimerScaled(self->dreamSys);
            self->dreamSys->methods->getSetDreamTimeLimit(self->dreamSys, ret + 0x1E);
        } else if (src->loaded != 0) {
            sel = self->styleConfig->unk14;
            m = src->methods;
            if (sel != 2) {
                color = self->styleConfig->unk18;
            } else {
                color = self->styleConfig->unkC;
            }
            m->fadeAllEntries(src, color);
            src->methods->release(src);
            self->timBlockPending = 0;
            self->methods->setupSceneStyle(self);
        }
    }
    if (self->timBlockPending == 0) {
        if (((Class866E8 *)self->unk14)->unk1B4 == 0 && self->inSession == 0) {
            self->unk64 = 1;
            self->methods->enterStyleSession(self);
        }
    }
}

/* onPadEvent: 0x21 togglePause, 0xC updateCloseReadyFlag, 0x2C
 * clearCloseReadyFlag, 0x16 closeAndNotifyD; only in session. */
void ObjM__DispatchPadEvent(ObjM *self, void *sender, s32 code) {
    ObjMMethods *m = self->methods;
    void (*fn)(ObjM *);

    if (self->inSession == 0) {
        return;
    }
    if (code == 0x16) {
        goto case_c8;
    }
    if (code < 0x17) {
        if (code == 0xC) {
            goto case_c0;
        }
        return;
    }
    if (code == 0x21) {
        goto case_74;
    }
    if (code == 0x2C) {
        goto case_c4;
    }
    return;
case_74:
    fn = m->togglePause;
    goto call;
case_c0:
    fn = m->updateCloseReadyFlag;
    goto call;
case_c8:
    fn = m->closeAndNotifyD;
    goto call;
case_c4:
    fn = m->clearCloseReadyFlag;
call:
    fn(self);
}

/* update, the FrameClock case of onNotify. */
void ObjM__Update(ObjM *self) {
    void (*fn)(ObjM *);

    if (self->inSession != 0) {
        self->frameCounter++;
        if (self->pauseSetupStep != 0) {
            fn = self->methods->advancePauseSetup;
        } else {
            fn = self->methods->tickStyle;
        }
        fn(self);
    }
}

/* togglePause. */
void ObjM__TogglePause(ObjM *self) {
    ObjMMethods *m = self->methods;

    if (self->pauseSetupStep != 0) {
        m->clearCloseReadyFlag(self);
        m->teardownPauseOverlay(self);
    } else {
        m->advancePauseSetup(self);
    }
}

void ObjM__NoOpSlot7C(void) {}

/* initArgs->unk0 as SetupSceneStyle reads it: its +0x07C returns a
 * pointer to one word (class_39e08.h's SubObjE is the same call from
 * Class865C8__OnInit). */
typedef struct UnkCObj_3bb8c_l UnkCObj_3bb8c_l;
typedef struct UnkCObjMethods_3bb8c_l UnkCObjMethods_3bb8c_l;

struct UnkCObjMethods_3bb8c_l {
    u8 pad000[0x07C];
    s32 *(*slot7C)(UnkCObj_3bb8c_l *self, s32 arg1); /* +0x07C */
};

struct UnkCObj_3bb8c_l {
    UnkCObjMethods_3bb8c_l *methods; /* +0x000 */
};

/* code_4cd08.c's (MATCHED round 43); no header declares it. `world` is the
 * DreamSys it installs as gDreamAuxWorld (track 4, round 88). */
extern void SetDreamAuxWorld(s32 a0, s32 a1, DreamSys *world, s32 a3, s32 a4);

/* GetStageGridDimensions comes from include/StageGrid.h, through
 * DreamSys.h. */

/* A plain `s32` bias added to the projection distance (%gp_rel value). */
extern s32 D_8008AB34;

/* The Class866E8's accepted tags (setAcceptedTags), an opaque .data block
 * (asm/data/76DC8.data.s) reached by address. */
extern s32 D_8008710C;

void ObjM__SetupSceneStyle(ObjM *self) {
    NodeGuardedViewport *vp = (NodeGuardedViewport *)self->viewport;
    Unk50Struct_3bb8c_l *style = self->styleConfig;
    UnkCObj_3bb8c_l *obj;
    s32 val;
    Class866E8 *rig;

    vp->methods->detachViewChild(vp);

    obj = (UnkCObj_3bb8c_l *)self->initArgs->drawSystem;
    val = *obj->methods->slot7C(obj, 0);
    vp->methods->setProjection(vp, val / 2 * 5 / 3 + D_8008AB34);

    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, (LongVec3 *)&D_8008715C,
                                 (LongVec3 *)&D_80087168, 0);

    SetDreamAuxWorld(self->stage, (s32)self->unk14, self->dreamSys, (s32)self->sound, (s32)self->unk10);

    rig = (Class866E8 *)self->unk14;
    self->methods->addChild(self, (BasicClass *)rig);

    rig->methods->setAmbientColor(rig, (LightRigRgb *)style->unk8, 0);
    rig->methods->setChildParams(rig, 3, style->unk0, style->unk4);
    rig->methods->setConfig(rig, (Unk68Struct *)GetStageGridDimensions(self->stage));
    ((DreamSysAttachToParentFn)self->dreamSys->methods->attachToParent)(self->dreamSys, rig);
    rig->methods->setGridSpan(rig, self->gridSpan);
    rig->methods->setAcceptedTags(rig, &D_8008710C);
}

void ObjM__ExitSceneStyle(ObjM *self) {
    self->methods->teardownPauseOverlay(self);
    self->dreamSys->methods->blockMovement(self->dreamSys);
    self->dreamSys->methods->detachFromParent(self->dreamSys);
    ((NodeGuardedViewport *)self->viewport)->methods->detachViewChild((NodeGuardedViewport *)self->viewport);
    self->methods->removeChild(self, self->unk14);
}

void ObjM__EnterStyleSession(ObjM *self) {
    NodeGuardedViewportMethods *m;
    Class6E99CMethods *m2;
    NodeGuardedViewport *vp;
    Class6E99C *fade;
    Unk50Struct_3bb8c_l *style;
    s32 local10;
    s32 ret;
    s32 a2;
    void *a1;

    self->inSession = 1;
    self->dreamSys->methods->resetLinkState(self->dreamSys, self->unk44, self->unk40);
    ((Class866E8 *)self->unk14)->methods->enable((Class866E8 *)self->unk14);

    vp = (NodeGuardedViewport *)self->viewport;
    style = self->styleConfig;
    vp->methods->setLightMode(vp, 1);
    vp->methods->setClearColor(vp, style->unkC);
    vp->methods->setFogNear(vp, style->unk1C);
    m = vp->methods;
    if (style->unk14 != 1) {
        a1 = style->unk18;
    } else {
        a1 = style->unkC;
    }
    m->setFarColor(vp, a1);
    vp->methods->setUnkB4(vp, 0);
    vp->methods->setDrawEnabled(vp, 1);

    fade = (Class6E99C *)vp->methods->getSubHandle(vp);
    self->methods->addChild(self, (BasicClass *)fade);

    ret = self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, (DreamColors *)&local10, -1);
    fade->methods->setDivisorMode(fade, ret, (ret != 0) ? 3 : 0);
    m2 = fade->methods;
    if (ret == 0) {
        a2 = -1;
    } else {
        a2 = local10;
    }
    m2->startFadeDown(fade, self->unk10, a2, 0);
}

void ObjM__TickStyle(ObjM *self) {
    TickStyle(((Class866E8 *)self->unk14)->methods->getTargetDescriptor((Class866E8 *)self->unk14, 0, 0),
              0, 0);
}

/* The DreamSys's codes 0xA..0x11 run enterState4..notifyParentsCodeB while
 * the state is 0; any code from 9 up otherwise clears the DreamSys's own
 * state (Actor::state). */
void ObjM__OnDreamSysNotify(ObjM *self, BasicClass *sender, s32 code) {
    if (self->state == 0) {
        switch (code - 0xA) {
            case 0:
                self->methods->enterState4(self);
                break;
            case 1:
                break;
            case 2:
                self->methods->enterState5(self);
                break;
            case 3:
                self->methods->enterState6(self);
                break;
            case 4:
                self->methods->enterState7(self);
                break;
            case 5:
                self->methods->enterState8(self);
                break;
            case 6:
                self->methods->enterStateA(self);
                break;
            case 7:
                self->methods->notifyParentsCodeB(self);
                break;
        }
    } else if (code >= 9) {
        self->dreamSys->state = 0;
    }
}

void ObjM__EnterState4(ObjM *self) {
    s32 local18;
    s32 span;
    s32 t;
    s32 arg3;

    self->state = 4;
    if (self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, (DreamColors *)&local18, -1) == 0) {
        span = (self->frameCounter + self->stage) & 3;
        t = span;
        if (t == 0) {
            self->methods->notifyParents(self, 4);
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
        ObjM__StartFadeUp(self, local18, 0, arg3, 1);
        return;
    }
    ObjM__StartFadeUp(self, 0, 0, 5, 1);
}

void ObjM__EnterState5(ObjM *self) {
    s32 color;

    if (self->dreamSys->currentStage < 0) {
        self->methods->enterState6(self);
    } else {
        self->state = 5;
        color = self->dreamSys->methods->getDreamColor(self->dreamSys);
        ObjM__StartFadeUp(self, color, 0, 0xA, 1);
        self->dreamSys->methods->blockMovement(self->dreamSys);
    }
}

void ObjM__EnterState6(ObjM *self) {
    s32 color;

    self->state = 6;
    color = self->dreamSys->methods->getDreamColor(self->dreamSys);
    ObjM__StartFadeUp(self, color, 0, 0x1E, 1);
    self->dreamSys->methods->blockMovement(self->dreamSys);
}
