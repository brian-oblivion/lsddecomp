/*
 * class_3bb8c_d -- TitleMenu's methods (include/TitleMenu.h; the allocator and
 * ctor are in class_3bb8c_c.c), then its getter, then TaskObjF's allocator
 * and ctor.
 *
 * TitleMenu is the TaskCore menu between days: START, FLASHBACK, SAVE, LOAD,
 * GRAPH and SHAKE over ETC\TITLE.TIM. In ROM order here: finalize, onNotify,
 * reset, onDeinit, setState and tick (which acts on the chosen entry),
 * refreshViewValue (stores SHAKE's setting), the four overrides that manage
 * the save-title TextRow in place of TaskCore's slot widgets, refreshMenu,
 * and the memory-card methods that drive `saveCtrl`, a TaskObjF, for SAVE
 * and LOAD. The header's banner describes the class.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "DreamSys.h"
#include "TextRow.h"
#include "TimImage.h"
#include "TitleMenu.h"
#include "TaskObjF.h"

void TitleMenu__Finalize(TitleMenu *self) {
    if (self->saveCtrl != NULL) {
        self->saveCtrl->methods->release(self->saveCtrl);
        self->saveIcon->methods->release(self->saveIcon);
    }
    Get_vtable_TaskCore()->finalize((TaskCore *)self);
}

void TitleMenu__OnNotify(TitleMenu *self, BasicClass *sender, s32 event) {
    Get_vtable_TaskCore()->onNotify((TaskCore *)self, sender, event);
    if ((sender->methods->header & 0xF) == 0xB) {
        self->methods->onCardEvent(self, sender, event);
    }
}

void TitleMenu__Reset(TitleMenu *self) {
    self->unk34 = 0;
    self->unk2C = 0x190;
    self->methods->setSubHandle(self, sTitleTimPath, 0);
    self->methods->setFrameBound(self, 0xA);
    self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, 0, 0);
}

void TitleMenu__OnDeinit(TitleMenu *self) {
    u32 i;
    DrawRect *rect;

    i = 0;
    rect = sDisplayBufferRects;
    for (; i < 2; i++) {
        ((DrawSystem *)self->initArgs->drawSystem)
            ->methods->clearImage((DrawSystem *)self->initArgs->drawSystem, self->unk93, rect);
        rect++;
    }
}

void TitleMenu__SetState(TitleMenu *self, s32 state) {
    Get_vtable_TaskCore()->setState((TaskCore *)self, state);
    if (state == 5) {
        self->methods->refreshMenu(self, 0);
    }
    if (state == 0xA) {
        self->methods->onPadCancel(self);
        self->methods->setActiveSlot(self, self->target->unk8, 1);
        self->methods->onPadConfirm(self);
    }
}

void TitleMenu__Tick(TitleMenu *self) {
    void (*fn)(TitleMenu *);

    Get_vtable_TaskCore()->tick((TaskCore *)self);
    switch (self->activeSlot) {
        case 1:
            self->result = 0;
            self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, 0, 1);
            fn = self->methods->refreshViewValue;
            break;
        case 2:
            fn = self->methods->saveToCard;
            break;
        case 3:
            fn = self->methods->loadFromCard;
            break;
        case 4:
            self->result = 2;
            fn = self->methods->refreshViewValue;
            break;
        default:
            return;
    }
    fn(self);
}

void TitleMenu__RefreshViewValue(TitleMenu *self) {
    s32 buf;

    Get_vtable_TaskCore()->refreshViewValue((TaskCore *)self);
    buf = self->slotCounts[5];
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &buf);
}

/* Sony's, linked from libc2 (config/psyq-objects.txt: libc2/strcpy,
 * libc2/strlen); declared locally per this project's established
 * per-unit convention for these two (see e.g. src/class_3bb8c_i.c,
 * src/class_3bb8c_j.c). */
extern char *strcpy(char *dest, char *src);
extern s32 strlen(char *s);

/* This unit's own view of DecodeFullWidthSjis (already matched,
 * src/code_2cc8c_f.c) -- return value unused at this call site, unlike
 * that unit's own `u8 *` view, so kept minimal per the project's
 * independent-arities convention. */
extern void DecodeFullWidthSjis(void *dst, void *src);

/* The setTarget override: `target` is the TaskCoreTarget the ctor passes
 * (&D_80086D44); only its `handle` is read, as the TextRow's texture. */
void TitleMenu__CreateSaveTitle(TitleMenu *self, TaskCoreTarget *target) {
    u32 size;
    char *buf;

    if (target == NULL) {
        return;
    }
    if (self->dreamSys->methods->getNewGameFlag(self->dreamSys)) {
        strcpy((char *)gSaveTitle + 0x18, (char *)sSaveTitleBlanks);
        StampSaveTitleFileLetter((char *)gSaveTitle, NULL);
    }
    size = strlen((char *)gSaveTitle);
    size = (size >> 1) + 4;
    buf = BMemPMgrAlloc(size);
    DecodeFullWidthSjis(buf, gSaveTitle);
    self->saveTitle = New_TextRow(target->handle, size, buf);
    self->saveTitle->visibleCount = 8;
    self->saveTitle->firstVisible = 4;
    self->saveTitle->gapIndex = 9;
    BMemPMgrFree(buf);
}

void TitleMenu__DestroySaveTitle(TitleMenu *self) {
    self->saveTitle->methods->release(self->saveTitle);
    Get_vtable_TaskCore()->releaseTarget((TaskCore *)self);
}

void TitleMenu__AttachSaveTitle(TitleMenu *self, void *parent) {
    Get_vtable_TaskCore()->updateSlotElements((TaskCore *)self, parent);
    self->saveTitle->methods->attachToParent(self->saveTitle, (SceneNode *)parent,
                                             (LongVec3 *)&sSaveTitleOffset);
}

/* MATCHED round 75 (was STALL round 43) -- see
 * docs/match-reports/TitleMenu__CycleSaveTitleColor.md. `base` is taken BEFORE the first call
 * (so it crosses a call and gets $s1), `buf = *color` is one struct copy
 * (SpriteRgb is three `s8`: three `lb`, then three `sb`), and each arm
 * indexes `base[sSaveTitleColorChannel]` directly. */
void TitleMenu__CycleSaveTitleColor(TitleMenu *self, SpriteRgb *color) {
    SpriteRgb buf;
    u8 *base;

    base = (u8 *)&buf;
    Get_vtable_TaskCore()->broadcastToSlots((TaskCore *)self, (u8 *)color);
    if (self->inputMode != 0) {
        base[0] = 0;
        base[1] = 0;
        base[2] = 0;
        base[sSaveTitleColorChannel] = 0x80;
    } else {
        buf = *color;
        if (D_8008AA2C < 0x80) {
            base[0] += 0x80;
        } else {
            base[sSaveTitleColorChannel] += 0x80;
        }
    }
    if (++sSaveTitleColorChannel >= 3) {
        sSaveTitleColorChannel = 0;
    }
    D_8008AA2C++;
    if (D_8008AA2C >= 0x101) {
        D_8008AA2C = 0;
    }
    self->saveTitle->methods->setColor(self->saveTitle, &buf);
}

/* CheckSaveScoreFlag is ALREADY MATCHED (src/class_3bb8c_c.c), as a genuinely
 * 2-argument function -- but THIS call site sets up a 3rd argument
 * (self->dreamSys, in $a2) that the other unit's own 2-parameter view never
 * receives. Same independent-arities situation already documented for
 * Get_vtable_TaskCore until round 84: this unit's own local view
 * matches what THIS call site needs. */
extern void CheckSaveScoreFlag(void *arg0, void *arg1,
                               void *arg2); /* arity-ok: the definition is 2-parameter and the callee WRITES $a2 (`li a2,0x1` at 0x8004D690) before reading it, but the 3rd argument is byte-load-bearing here -- retail emits `lw a2,164(s0)` at 0x8004DE74 */

void TitleMenu__RefreshMenu(TitleMenu *self) {
    s32 size;
    s32 origSlot;
    char *buf1;
    s32 buf2;

    size = self->saveTitle->cellCount;
    origSlot = self->activeSlot;
    buf1 = BMemPMgrAlloc(size);
    DecodeFullWidthSjis(buf1, gSaveTitle);
    self->saveTitle->methods->setText(self->saveTitle, buf1);
    BMemPMgrFree(buf1);
    CheckSaveScoreFlag(self, self->target, self->dreamSys);
    self->methods->updateSlotElements(self, self->unk14);
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &buf2);
    self->activeSlot = 5;
    self->methods->setState(self, 0xB);
    self->methods->setSlotCursor(self, buf2, 1);
    self->methods->setState(self, 0xF);
    self->methods->setActiveSlot(self, origSlot, 0);
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &buf2);
}

void TitleMenu__BeginCardAccess(TitleMenu *self) {
    if (self->saveCtrl == NULL) {
        self->saveIcon = New_TimImage((char *)sSaveIconTimPath);
        self->saveCtrl = New_TaskObjF(1, 0);
    }
    self->saveCtrl->methods->init(self->saveCtrl, sCardFilePrefix, (char **)&sSaveFileSuffixes,
                                  self->initArgs->pad, self->unk10, (struct SceneNode *)self->unk14,
                                  (struct VabStreamObj *)self->sound);
    self->methods->addChild(self, (BasicClass *)self->saveCtrl);
    self->methods->removeChild(self, self->initArgs->pad);
    self->methods->removeChild(self, self->unk10);
}

void TitleMenu__EndCardAccess(TitleMenu *self) {
    self->methods->addChild(self, self->initArgs->pad);
    self->methods->addChild(self, self->unk10);
    self->methods->removeChild(self, (BasicClass *)self->saveCtrl);
    self->saveCtrl->methods->deinit(self->saveCtrl);
}

void TitleMenu__SaveToCard(TitleMenu *self) {
    s32 buf;

    buf = self->slotCounts[5];
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &buf);
    self->methods->beginCardAccess(self);
    if (self->dreamSys->methods->getNewGameFlag(self->dreamSys)) {
        *(u8 *)sSaveFileName = 0;
    }
    self->saveCtrl->methods->beginSave(self->saveCtrl, sSaveFileName, gSaveTitle, 0xD, 3,
                                       self->saveIcon, self->saveBlock, self->saveBlockSize);
}

void TitleMenu__LoadFromCard(TitleMenu *self) {
    self->methods->beginCardAccess(self);
    self->saveCtrl->methods->beginLoad(self->saveCtrl, sSaveFileName, gSaveTitle, self->saveBlock,
                                       self->saveBlockSize);
}

void TitleMenu__OnCardEvent(TitleMenu *self, BasicClass *sender, s32 event) {
    if (event < 0x18) {
        if (event >= 0x16) {
            self->methods->endCardAccess(self);
            if (event == 0x16) {
                self->dreamSys->methods->clearNewGameFlag(self->dreamSys);
                self->methods->refreshMenu(self, 0x16);
            }
        }
    }
}

TitleMenuMethods *GetTitleMenuMethods(void) {
    return &gTitleMenuMethods;
}

TaskObjF *New_TaskObjF(s32 padEnable, s32 cardSlot) {
    TaskObjF *self;

    self = BMemPMgrAlloc(0x84);
    if (self == NULL) {
        goto fail;
    }
    GetTaskObjFMethods()->ctor(self, padEnable, cardSlot);
    return self;
fail:
    return NULL;
}

/* libcard, linked SDK objects (config/psyq-objects.txt: libcard/a74,
 * libcard/a75, libcard/c112 -- see docs/match-reports/TaskObjF__TaskObjF.md).
 * Declared locally rather than in the shared header, same policy as
 * malloc/free/printf (CLAUDE.md, "To include/ has one exception"). */
extern void InitCARD(s32 padEnable);
extern void StartCARD(void);
extern void _bu_init(void);

void TaskObjF__TaskObjF(TaskObjF *self, s32 padEnable, s32 cardSlot) {
    s32 count;

    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetTaskObjFMethods();
    count = D_8008AA30;
    D_8008AA30 = count + 1;
    if (count == 0) {
        InitCARD(padEnable);
        StartCARD();
        _bu_init();
    }
    TaskObjF__ClearResourceSlots(self);
    self->methods->setCardSlot(self, cardSlot);
}
