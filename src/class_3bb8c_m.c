/*
 * class_3bb8c_m -- seventh carved slice of the class_3bb8c block
 * (0x44518..0x44F14, vram 0x80053D18..0x80054714), 20 functions, ALL 20
 * MATCHED (0 INCLUDE_ASM, 0 NON_MATCHING). Carved round 15; all four former
 * toolchain-blocker functions matched round 23/44, once `addiu_at` and
 * `gp_rel` were resolved project-wide (CLAUDE.md, "Open toolchain
 * blockers"). This unit owns no switch jump table.
 *
 * NAMING PASS, round 69 (runner alpha). Two class identifications drive
 * every name below, both confirmed with `tools/classtable.py`, never by
 * guessing from a slot number:
 *
 *   - This unit's `self` (`ObjM`) is a subclass whose OWN vtable is
 *     `gObjMMethods` (`tools/classtable.py 0x80087034`, 53 slots) -- the same
 *     class as class_3bb8c_l's `Obj87034_3bb8c_l` (see that HEAD NOTE in
 *     include/class_3bb8c.h; NOT unified with it here, a struct-merge is its
 *     own change per that note). This unit's own 14 functions occupy that
 *     table's tail, offsets +0xA0..+0xD4, i.e. this class's own new virtual
 *     methods (the base Class86668/Obj865C8 table --
 *     docs/match-reports/ObjM__Finalize.md -- only goes up to about +0x88).
 *     `ObjMMethods::notifyParents`/`checkAuxTrigger`/`teardownPauseOverlay`
 *     (+0x030/+0x0B8/+0x0D4) are confirmed the same way: +0x030 is
 *     `BasicClass__NotifyParents`, and +0x0B8/+0x0D4 are this unit's own
 *     `ObjM__CheckAuxTrigger`/`ObjM__TeardownPauseOverlay`.
 *   - `self->dreamSys` (formerly `unk3C`) is `DreamSys*`
 *     (`tools/classtable.py 0x80087BDC`, DreamSys's real vtable,
 *     include/DreamSys.h): the six offsets this unit dispatches
 *     (0xF0/0xF4/0xFC/0x13C/0x17C/0x1A0) land EXACTLY on
 *     DreamSys__GetSetFlashbackSession/SetMoveOverride/BlockMovement/
 *     SelectCallback98/StopDrift/GetCurrentDayAndYear, both offset and
 *     argument count. Since track 4 (round 88) it is typed with the
 *     unified header, include/DreamSys.h.
 *
 * What the class itself IS remains TIER B, not asserted further:
 * `ObjM__AdvancePauseSetup`/`ObjM__TeardownPauseOverlay` build and tear down
 * an object literally constructed with the name "Pause"
 * (`D_8008AB44`, "Pause", asm/data/7B008.sdata.s), gated by a 5-step
 * counter and a `mode` field (`ObjM::mode`, ex-`unk20`) that other
 * functions here set to fixed small codes (0,4,5,6,7,8,0xA,0xB,0xC,0xD) and
 * forward to `ObjMMethods::notifyParents` -- consistent with a pause/dialog
 * overlay controller driving a small state machine and notifying its
 * parent object of transitions, but nothing here pins down the exact
 * gameplay meaning of any one mode code. `ObjM__NoOpSlotBC` (vtable slot
 * +0x0BC) is an empty `{}` body with no further evidence and is left
 * unnamed.
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
#include "Class81940.h"
#include "BoxFill.h"
#include "TextRow.h"
#include "ObjM.h"
#include "Class866E8.h"
#include "Class869D8.h"
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
extern char D_8008AB44[];   /* "Pause" (asm/data/7B008.sdata.s) */
extern s32 D_8008AB38;      /* two words: the position */
extern s32 D_8008AB40;      /* one word: the colour */

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

/* The viewport (IntermediateBase::viewport, a Class869D8) hands out its fade
 * box (getSubHandle, Viewport's New_Class6E99C). */
void ObjM__StartFadeUp(ObjM *self, s32 channels, s32 arg2, s32 step, s32 addChild) {
    Class6E99C *fade = (Class6E99C *)((Class869D8 *)self->viewport)->methods->getSubHandle((Class869D8 *)self->viewport);
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
        ((Class869D8 *)self->viewport)->methods->setClearColor((Class869D8 *)self->viewport, (ViewportRgb *)color);
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
    Class866E8Elem *elem = ((Class866E8 *)self->unk14)->methods->getLastTargetRateSplit((Class866E8 *)self->unk14, (u8 *)&out);
    void *thing = (void *)self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    result = TryDreamAuxTrigger((s32)elem->loader->dataBuffer, &out, thing);
    elem->heldObj = (BasicClass *)result;
    if (result != 0) {
        return 0;
    }
    elem->loader->methods->releaseDataBlock(elem->loader);
    return 1;
}

void ObjM__NoOpSlotBC(void) {
}

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
 * WBgm and the VabStreamObj (IntermediateBase::unk10, bgm, Class86668::sound). */
void ObjM__AdvancePauseSetup(ObjM *self) {
    s32 state = self->pauseSetupStep;
    if (state == 0) {
        self->pauseText = New_TextRow(self->etcTim, 5, &D_8008AB44[0]);
        self->pauseText->methods->attachToParent(self->pauseText, (Class6B5CC *)self->unk14, (Vec3_d294 *)&D_8008AB38);
        self->pauseText->methods->setColor(self->pauseText, (SpriteRgb *)&D_8008AB40);
        self->pauseSetupStep = state + 1;
        return;
    }
    self->pauseSetupStep = state + 1;
    if (state != 4) {
        return;
    }
    ((Class869D8 *)self->viewport)->methods->setDrawEnabled((Class869D8 *)self->viewport, 0);
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
    ((Class869D8 *)self->viewport)->methods->setDrawEnabled((Class869D8 *)self->viewport, 1);
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
    FillStyleFromConfig((struct StyleM *) &D_80087424, cfg);
    if (cfg[1] >= 4) {
        gStyleDecorColor = D_800872C4[cfg[2]];
    }
    return &D_80087424;
}

/* FillStyleFromConfig's destination is NOT an `ObjM`. That struct's +0x014 and +0x018
 * are already established as unrelated object pointers by five other functions
 * in this unit (`FieldM14 *`/`FieldM18 *`), whereas this function writes a
 * colour-table POINTER to +0x018 and a plain sign-extended byte to +0x014. So
 * this is a separate descriptor, and its view stays LOCAL rather than going
 * into include/class_3bb8c.h -- which eleven units share, and where adding
 * `unkC`/`unk1C` to `ObjM` on this evidence would be a claim the bytes do not
 * support.
 *
 * D_800872C4 is a table of 24 three-byte entries (0x48 bytes; the first four
 * are 00/00/00, 40/40/40, 80/80/80, FF/FF/FF -- a greyscale ramp, so RGB
 * triples). Indexing it as `u8[][3]` is what produces retail's `i*2 + i + base`
 * stride-3 address arithmetic. D_8008730C is six words, 0x6800 down to 0x0800. */
struct StyleM {
    u8 pad000[0x00C];
    const u8 *unkC;                 /* +0x00C, a D_800872C4 entry */
    u8 pad010[0x014 - 0x010];
    s32 unk14;                      /* +0x014, cfg[0] sign-extended */
    const u8 *unk18;                /* +0x018, a D_800872C4 entry */
    s32 unk1C;                      /* +0x01C, a D_8008730C value */
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
    s32 (*slotAC)(LocalSubObj *self);                          /* +0x0AC */
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
        gStyleDecorObj = (s32) New_BoxFill(&D_8008AB60, (void *) gStyleDecorColor, 0);
        ((BoxFill *) gStyleDecorObj)->methods->setSemiTrans((BoxFill *) gStyleDecorObj, 1);
        ((BoxFill *) gStyleDecorObj)->methods->setSemiTransRate((BoxFill *) gStyleDecorObj, 0);

        tmp = ((FieldAC7CHolder *) gStyleTargetObj)->unkC->methods->slotAC(
                ((FieldAC7CHolder *) gStyleTargetObj)->unkC);

        ((BoxFillAttachToParentFn)((BoxFill *) gStyleDecorObj)->methods->attachToParent)(
            (BoxFill *) gStyleDecorObj, (Class6B5CC *) tmp, (Pair32E99C *) &D_8008AB58);
    }
}
