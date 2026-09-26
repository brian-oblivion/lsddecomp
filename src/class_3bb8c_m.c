/*
 * class_3bb8c_m -- seventh carved slice of the class_3bb8c block
 * (0x44518..0x44F14, vram 0x80053D18..0x80054714), 20 functions, ALL 20
 * MATCHED (0 INCLUDE_ASM, 0 NON_MATCHING). Carved round 15; all four former
 * toolchain-blocker functions matched round 23/44, once `addiu_at` and
 * `gp_rel` were resolved project-wide (CLAUDE.md, "Open toolchain
 * blockers"). This unit owns no switch jump table.
 *
 * ObjM's methods from +0x0A0 to the end of its table (gObjMMethods,
 * include/ObjM.h; track 4, round 89 unified the class_3bb8c_k/_l/_m views
 * there), and its getter: EnterState7/8/A and NotifyParentsCodeB (the
 * DreamSys codes 0xE..0x11, which set IntermediateBase::state and start a
 * fade), StartFadeUp (the viewport's Class6E99C fade box), the fade box's
 * and the Class866E8's notification handlers (OnFadeNotify: 5 fade down
 * done, 6 fade up done; OnClass866E8Notify: 7 runs CheckAuxTrigger), and
 * the "Pause" overlay: AdvancePauseSetup builds the TextRow and, four
 * calls later, pauses the FrameClock, the WBgm and the VabStreamObj and
 * hides the viewport; TeardownPauseOverlay undoes it; the close-ready flag
 * and CloseAndNotifyC/D report 0xC/0xD to the parent (Class865C8's
 * onObjMNotify). NoOpSlotBC is empty. What the state codes mean in the
 * game is not established.
 *
 * A separate, unrelated cluster of free functions (RegisterStyleConfig /
 * ApplyStyleConfig / FillStyleFromConfig / ApplyStyleDecorationIfSet) reads
 * and writes a small set of `.sdata`/`.sbss` globals to configure a
 * `StyleM` colour/config descriptor (struct defined below, own comment) --
 * unrelated to the ObjM/DreamSys machinery above beyond living in the same
 * carved address range.
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "DreamSys.h"
#include "LbdFile.h"
#include "BoxFill.h"
#include "TextRow.h"
#include "ObjM.h"
#include "Class866E8.h"
#include "NodeGuardedViewport.h"
#include "Class6E99C.h"
#include "FrameClock.h"
#include "WBgm.h"
#include "VabStreamObj.h"

/* ObjM__CheckAuxTrigger's one external call, src/code_4cd08.c (MATCHED; its
 * definition reads the second argument as `s16 *`). The third argument is
 * the DreamSys's getCurrentDayAndYear result. */
extern s32 TryDreamAuxTrigger(s32 arg0, s32 *arg1, void *arg2);

/* ObjM__AdvancePauseSetup's literals, all reached by address: the "Pause"
 * text, the TextRow's position (attachToParent) and its colour (setColor). */
extern char D_8008AB44[]; /* "Pause" (asm/data/7B008.sdata.s) */
extern s32 D_8008AB38;    /* two words: the position */
extern s32 D_8008AB40;    /* one word: the colour */

void ObjM__EnterState7(ObjM *self) {
    s32 val;
    self->state = 7;
    self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, (DreamColors *)&val, -1);
    ObjM__StartFadeUp(self, val, 0, 5, 1);
    self->dreamSys->methods->blockMovement(self->dreamSys);
}

void ObjM__EnterState8(ObjM *self) {
    self->state = 8;
    ObjM__StartFadeUp(self, 0, 0, 6, 1);
    self->dreamSys->methods->setMoveOverride(self->dreamSys, 1);
}

void ObjM__EnterStateA(ObjM *self) {
    self->state = 0xA;
    ObjM__StartFadeUp(self, 0, 0, 6, 1);
    self->dreamSys->methods->selectCallback98(self->dreamSys, 2);
    self->dreamSys->methods->setMoveOverride(self->dreamSys, 2);
}

void ObjM__NotifyParentsCodeB(ObjM *self) {
    self->methods->notifyParents(self, 0xB);
}

/* The viewport (IntermediateBase::viewport, a NodeGuardedViewport) hands out its fade
 * box (getSubHandle, Viewport's New_Class6E99C). */
void ObjM__StartFadeUp(ObjM *self, s32 channels, s32 arg2, s32 step, s32 addChild) {
    Class6E99C *fade =
        (Class6E99C *)((NodeGuardedViewport *)self->viewport)->methods->getSubHandle((NodeGuardedViewport *)self->viewport);
    if (step != 0) {
        fade->methods->setStep(fade, step);
    }
    if (addChild != 0) {
        self->methods->addChild(self, (BasicClass *)fade);
    }
    fade->methods->startFadeUp(fade, self->unk10, channels, arg2);
}

void ObjM__OnFadeNotify(ObjM *self, Class6E99C *sender, s32 event) {
    void *color;
    switch (event) {
        case 5:
            self->methods->removeChild(self, (BasicClass *)sender);
            self->dreamSys->methods->setMoveOverride(self->dreamSys, 0);
            self->state = 0;
            break;
        case 6:
            self->methods->removeChild(self, (BasicClass *)sender);
            color = sender->methods->getColor(sender);
            ((NodeGuardedViewport *)self->viewport)
                ->methods->setClearColor((NodeGuardedViewport *)self->viewport, (ViewportRgb *)color);
            if (self->state != 5 && self->state != 8 && self->state == 0xA) {
                self->dreamSys->methods->stopDrift(self->dreamSys, 1);
                self->dreamSys->methods->setMoveOverride(self->dreamSys, 0);
                self->state = 4;
            }
            self->methods->notifyParents(self, self->state);
            break;
    }
}

void ObjM__OnClass866E8Notify(ObjM *self, BasicClass *sender, s32 event) {
    if (event == 7) {
        self->methods->checkAuxTrigger(self);
    }
}

s32 ObjM__CheckAuxTrigger(ObjM *self) {
    s32 out;
    s32 result;
    Class866E8Elem *elem =
        ((Class866E8 *)self->unk14)->methods->getLastTargetRateSplit((Class866E8 *)self->unk14, (u8 *)&out);
    void *thing = (void *)self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    result = TryDreamAuxTrigger((s32)elem->loader->dataBuffer, &out, thing);
    elem->heldObj = (BasicClass *)result;
    if (result != 0) {
        return 0;
    }
    elem->loader->methods->releaseDataBlock(elem->loader);
    return 1;
}

void ObjM__NoOpSlotBC(void) {}

void ObjM__UpdateCloseReadyFlag(ObjM *self) {
    if (self->pauseSetupStep != 0 && self->state == 0) {
        self->closeReady = 1;
    }
}

void ObjM__ClearCloseReadyFlag(ObjM *self) {
    self->closeReady = 0;
}

void ObjM__CloseAndNotifyD(ObjM *self) {
    if (self->closeReady) {
        self->methods->teardownPauseOverlay(self);
        self->methods->notifyParents(self, 0xD);
    }
}

void ObjM__CloseAndNotifyC(ObjM *self) {
    if (self->closeReady) {
        self->methods->teardownPauseOverlay(self);
        self->methods->notifyParents(self, 0xC);
    }
}

/* The pause: step 0 builds the "Pause" TextRow under the Class866E8; the
 * fourth call after it hides the viewport and pauses the FrameClock, the
 * WBgm and the VabStreamObj (IntermediateBase::unk10, bgm, TimedTask::sound). */
void ObjM__AdvancePauseSetup(ObjM *self) {
    s32 state = self->pauseSetupStep;
    if (state == 0) {
        self->pauseText = New_TextRow(self->etcTim, 5, &D_8008AB44[0]);
        self->pauseText->methods->attachToParent(self->pauseText, (SceneNode *)self->unk14,
                                                 (LongVec3 *)&D_8008AB38);
        self->pauseText->methods->setColor(self->pauseText, (SpriteRgb *)&D_8008AB40);
        self->pauseSetupStep = state + 1;
        return;
    }
    self->pauseSetupStep = state + 1;
    if (state != 4) {
        return;
    }
    ((NodeGuardedViewport *)self->viewport)->methods->setDrawEnabled((NodeGuardedViewport *)self->viewport, 0);
    ((FrameClock *)self->unk10)->methods->pause((FrameClock *)self->unk10);
    self->bgm->methods->pause(self->bgm);
    ((VabStreamObj *)self->sound)->methods->mute((VabStreamObj *)self->sound);
}

void ObjM__TeardownPauseOverlay(ObjM *self) {
    if (self->pauseSetupStep != 0) {
        self->pauseText->methods->release(self->pauseText);
    }
    ((VabStreamObj *)self->sound)->methods->unmute((VabStreamObj *)self->sound);
    self->bgm->methods->resume(self->bgm);
    ((FrameClock *)self->unk10)->methods->resume((FrameClock *)self->unk10);
    ((NodeGuardedViewport *)self->viewport)->methods->setDrawEnabled((NodeGuardedViewport *)self->viewport, 1);
    self->pauseSetupStep = 0;
}

ObjMMethods *GetObjMMethods(void) {
    return &gObjMMethods;
}

struct StyleM;

extern s32 gStyleCueSelf;
extern s32 gStyleKind;
extern s32 gStyleTickCount;
extern s32 gStyleCounter;
extern s32 D_8008AC78;
extern s32 gStyleTargetObj;
extern s32 gStyleVariant;
extern s32 D_8008ACA0;

extern void *ApplyStyleConfig(void);

s32 RegisterStyleConfig(s32 a0, s32 a1, s32 a2, s32 a3, s32 arg4) {
    s32 *p;
    s32 i;

    if (gStyleCueSelf == 0) {
        i = 1;
        p = &D_8008ACA0;
        gStyleCueSelf = a0;
        gStyleKind = a1;
        gStyleTargetObj = a2;
        gStyleVariant = -1;
        gStyleCounter = a3;
        D_8008AC78 = arg4;
        gStyleTickCount = 0;
        do {
            *p = 0;
            i--;
            p--;
        } while (i >= 0);
        return ApplyStyleConfig();
    }
    return 0;
}

extern s32 D_80087424;
extern s8 *D_800873EC[];
extern s8 *PickStyleFallbackConfig(void);
extern void FillStyleFromConfig(struct StyleM *style, s8 *cfg);
extern u8 D_800872C4[][3];
extern const u8 *gStyleDecorColor;

void *ApplyStyleConfig(void) {
    s8 *cfg = D_800873EC[gStyleKind];

    if (cfg == 0) {
        cfg = PickStyleFallbackConfig();
    }
    FillStyleFromConfig((struct StyleM *)&D_80087424, cfg);
    if (cfg[1] >= 4) {
        gStyleDecorColor = D_800872C4[cfg[2]];
    }
    return &D_80087424;
}

/* FillStyleFromConfig's destination (D_80087424, via ApplyStyleConfig) is
 * not an ObjM: it is the record ObjM keeps as `styleConfig`, which
 * include/class_3bb8c.h views as Unk50Struct_3bb8c_l (its +0x00C/+0x018
 * colours and +0x01C fog value agree with the fields below). The two views
 * stay separate here: the record is not a class, and merging them is a
 * global's type (track 4b).
 *
 * D_800872C4 is a table of 24 three-byte entries (0x48 bytes; the first four
 * are 00/00/00, 40/40/40, 80/80/80, FF/FF/FF -- a greyscale ramp, so RGB
 * triples). Indexing it as `u8[][3]` is what produces retail's `i*2 + i + base`
 * stride-3 address arithmetic. D_8008730C is six words, 0x6800 down to 0x0800. */
struct StyleM {
    u8 pad000[0x00C];
    const u8 *unkC; /* +0x00C, a D_800872C4 entry */
    u8 pad010[0x014 - 0x010];
    s32 unk14;       /* +0x014, cfg[0] sign-extended */
    const u8 *unk18; /* +0x018, a D_800872C4 entry */
    s32 unk1C;       /* +0x01C, a D_8008730C value */
};

extern u8 D_800872C4[][3];
extern s32 D_8008730C[];

void FillStyleFromConfig(struct StyleM *style, s8 *cfg) {
    style->unkC = D_800872C4[cfg[3]];
    style->unk18 = D_800872C4[cfg[2]];
    style->unk1C = D_8008730C[cfg[1]];
    style->unk14 = cfg[0];
}

/* gStyleDecorObj is a BoxFill (include/BoxFill.h), kept in an s32 global
 * (track 4b's to retype). */

/* gStyleTargetObj's own local reading here: only its +0xC field (a "self"
 * pointer into a THIRD object, dispatched only through +0xAC) is ever
 * touched by this function. */
typedef struct LocalSubObj LocalSubObj;
typedef struct LocalSubMethods LocalSubMethods;

struct LocalSubMethods {
    u8 pad00[0xAC];
    s32 (*slotAC)(LocalSubObj *self); /* +0x0AC */
};

struct LocalSubObj {
    LocalSubMethods *methods;
};

typedef struct FieldAC7CHolder {
    u8 pad0[0xC];
    LocalSubObj *unkC;
} FieldAC7CHolder;

extern s32 gStyleDecorObj;
extern s32 D_8008AB60;
extern s32 D_8008AB58;

void ApplyStyleDecorationIfSet(void) {
    s32 tmp;

    if (gStyleDecorColor != 0) {
        gStyleDecorObj = (s32)New_BoxFill(&D_8008AB60, (void *)gStyleDecorColor, 0);
        ((BoxFill *)gStyleDecorObj)->methods->setSemiTrans((BoxFill *)gStyleDecorObj, 1);
        ((BoxFill *)gStyleDecorObj)->methods->setSemiTransRate((BoxFill *)gStyleDecorObj, 0);

        tmp = ((FieldAC7CHolder *)gStyleTargetObj)
                  ->unkC->methods->slotAC(((FieldAC7CHolder *)gStyleTargetObj)->unkC);

        ((BoxFillAttachToParentFn)((BoxFill *)gStyleDecorObj)->methods->attachToParent)(
            (BoxFill *)gStyleDecorObj, (SceneNode *)tmp, (Pair32E99C *)&D_8008AB58);
    }
}
