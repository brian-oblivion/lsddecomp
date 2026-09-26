/*
 * class_3bb8c_d -- the bulk of `Class86B60`'s own methods (include/Class86B60.h,
 * track 4 round 88; the allocator and ctor are in class_3bb8c_c.c), then the
 * getter, then TaskObjF's allocator and methods.
 *
 * Class86B60 is a TaskCore. Its overrides replace TaskCore's slot widgets
 * with one owned TextRow, `nameField` (setTarget/releaseTarget/
 * updateSlotElements/broadcastToSlots: Class86B60__CreateNameField/
 * DestroyNameField/ForwardToNameField/TickNameFieldCursor), and its own
 * slots drive `saveCtrl`, a TaskObjF whose calls carry "BISLPS-01556",
 * Sony's memory-card product code (Class86B60__Begin/EndMemcardSave and the
 * two UpdateMemcardSave* variants that tick runs for activeSlot 2 and 3).
 * Read together this is a memory-card save-naming screen: enter a save
 * name, then write it. The header's banner has the measured detail.
 *
 * Every function here is matched C. Names describe mechanics established
 * from the body and call sites; no in-game purpose is established for the
 * numeric activeSlot or event values. See each function's match report.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "DreamSys.h"
#include "TextRow.h"
#include "TimImage.h"
#include "Class86B60.h"
#include "TaskObjF.h"

void Class86B60__Finalize(Class86B60 *self) {
    if (self->saveCtrl != NULL) {
        self->saveCtrl->methods->release(self->saveCtrl);
        self->iconHandle->methods->release(self->iconHandle);
    }
    Get_vtable_TaskCore()->finalize((TaskCore *)self);
}

void Class86B60__OnNotify(Class86B60 *self, BasicClass *sender, s32 event) {
    Get_vtable_TaskCore()->onNotify((TaskCore *)self, sender, event);
    if ((sender->methods->header & 0xF) == 0xB) {
        self->methods->onTagBValue(self, sender, event);
    }
}

void Class86B60__Reset(Class86B60 *self) {
    self->unk34 = 0;
    self->unk2C = 0x190;
    self->methods->setSubHandle(self, D_800114E8, 0);
    self->methods->setFrameBound(self, 0xA);
    self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, 0, 0);
}

void Class86B60__OnDeinit(Class86B60 *self) {
    u32 i;
    u8 *entry;

    i = 0;
    entry = (u8 *)&D_80086DAC;
    for (; i < 2; i++) {
        ((Class86B60UnkC0Obj_3bb8c_d *)self->initArgs->unk0)
            ->methods->slot78((Class86B60UnkC0Obj_3bb8c_d *)self->initArgs->unk0, self->unk93, entry);
        entry += 0xC;
    }
}

void Class86B60__SetState(Class86B60 *self, s32 state) {
    Get_vtable_TaskCore()->setState((TaskCore *)self, state);
    if (state == 5) {
        self->methods->commitNameEntry(self, 0);
    }
    if (state == 0xA) {
        self->methods->onPadCancel(self);
        self->methods->setActiveSlot(self, self->target->unk8, 1);
        self->methods->onPadConfirm(self);
    }
}

void Class86B60__Tick(Class86B60 *self) {
    void (*fn)(Class86B60 *);

    Get_vtable_TaskCore()->tick((TaskCore *)self);
    switch (self->activeSlot) {
        case 1:
            self->result = 0;
            self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, 0, 1);
            fn = self->methods->refreshViewValue;
            break;
        case 2:
            fn = self->methods->updateMemcardSaveWithIcon;
            break;
        case 3:
            fn = self->methods->updateMemcardSaveStatus;
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

void Class86B60__RefreshViewValue(Class86B60 *self) {
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
void Class86B60__CreateNameField(Class86B60 *self, TaskCoreTarget *target) {
    u32 size;
    char *buf;

    if (target == NULL) {
        return;
    }
    if (self->dreamSys->methods->getNewGameFlag(self->dreamSys)) {
        strcpy((char *)D_8008AA18 + 0x18, (char *)D_8008AA14);
        CopyMemcardIconTemplate((s32)D_8008AA18, 0);
    }
    size = strlen((char *)D_8008AA18);
    size = (size >> 1) + 4;
    buf = BMemPMgrAlloc(size);
    DecodeFullWidthSjis(buf, D_8008AA18);
    self->nameField = New_TextRow(target->handle, size, buf);
    self->nameField->visibleCount = 8;
    self->nameField->firstVisible = 4;
    self->nameField->gapIndex = 9;
    BMemPMgrFree(buf);
}

void Class86B60__DestroyNameField(Class86B60 *self) {
    self->nameField->methods->release(self->nameField);
    Get_vtable_TaskCore()->releaseTarget((TaskCore *)self);
}

void Class86B60__ForwardToNameField(Class86B60 *self, void *parent) {
    Get_vtable_TaskCore()->updateSlotElements((TaskCore *)self, parent);
    self->nameField->methods->attachToParent(self->nameField, (Class6B5CC *)parent,
                                             (Vec3_d294 *)&D_8008A9B4);
}

/* MATCHED round 75 (was STALL round 43) -- see
 * docs/match-reports/Class86B60__TickNameFieldCursor.md. `base` is taken BEFORE the first call
 * (so it crosses a call and gets $s1), `buf = *color` is one struct copy
 * (SpriteRgb is three `s8`: three `lb`, then three `sb`), and each arm
 * indexes `base[D_8008AA28]` directly. */
void Class86B60__TickNameFieldCursor(Class86B60 *self, SpriteRgb *color) {
    SpriteRgb buf;
    u8 *base;

    base = (u8 *)&buf;
    Get_vtable_TaskCore()->broadcastToSlots((TaskCore *)self, (u8 *)color);
    if (self->inputMode != 0) {
        base[0] = 0;
        base[1] = 0;
        base[2] = 0;
        base[D_8008AA28] = 0x80;
    } else {
        buf = *color;
        if (D_8008AA2C < 0x80) {
            base[0] += 0x80;
        } else {
            base[D_8008AA28] += 0x80;
        }
    }
    if (++D_8008AA28 >= 3) {
        D_8008AA28 = 0;
    }
    D_8008AA2C++;
    if (D_8008AA2C >= 0x101) {
        D_8008AA2C = 0;
    }
    self->nameField->methods->setColor(self->nameField, &buf);
}

/* CheckSaveScoreFlag is ALREADY MATCHED (src/class_3bb8c_c.c), as a genuinely
 * 2-argument function -- but THIS call site sets up a 3rd argument
 * (self->dreamSys, in $a2) that the other unit's own 2-parameter view never
 * receives. Same independent-arities situation already documented for
 * Get_vtable_TaskCore until round 84: this unit's own local view
 * matches what THIS call site needs. */
extern void CheckSaveScoreFlag(void *arg0, void *arg1,
                               void *arg2); /* arity-ok: the definition is 2-parameter and the callee WRITES $a2 (`li a2,0x1` at 0x8004D690) before reading it, but the 3rd argument is byte-load-bearing here -- retail emits `lw a2,164(s0)` at 0x8004DE74 */

void Class86B60__CommitNameEntry(Class86B60 *self) {
    s32 size;
    s32 origSlot;
    char *buf1;
    s32 buf2;

    size = self->nameField->cellCount;
    origSlot = self->activeSlot;
    buf1 = BMemPMgrAlloc(size);
    DecodeFullWidthSjis(buf1, D_8008AA18);
    self->nameField->methods->setText(self->nameField, buf1);
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

void Class86B60__BeginMemcardSave(Class86B60 *self) {
    if (self->saveCtrl == NULL) {
        self->iconHandle = New_TimImage((char *)D_800114F8);
        self->saveCtrl = New_TaskObjF(1, 0);
    }
    self->saveCtrl->methods->init(self->saveCtrl, D_8008A9D0, (char **)&D_80086D6C,
                                  self->initArgs->unk4, self->unk10, (struct Class6B5CC *)self->unk14,
                                  (struct VabStreamObj *)self->sound);
    self->methods->addChild(self, (BasicClass *)self->saveCtrl);
    self->methods->removeChild(self, self->initArgs->unk4);
    self->methods->removeChild(self, self->unk10);
}

void Class86B60__EndMemcardSave(Class86B60 *self) {
    self->methods->addChild(self, self->initArgs->unk4);
    self->methods->addChild(self, self->unk10);
    self->methods->removeChild(self, (BasicClass *)self->saveCtrl);
    self->saveCtrl->methods->deinit(self->saveCtrl);
}

void Class86B60__UpdateMemcardSaveWithIcon(Class86B60 *self) {
    s32 buf;

    buf = self->slotCounts[5];
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &buf);
    self->methods->beginMemcardSave(self);
    if (self->dreamSys->methods->getNewGameFlag(self->dreamSys)) {
        *(u8 *)D_8008AA10 = 0;
    }
    self->saveCtrl->methods->beginSave(self->saveCtrl, D_8008AA10, D_8008AA18, 0xD, 3,
                                       self->iconHandle, self->saveBlock, self->saveBlockSize);
}

void Class86B60__UpdateMemcardSaveStatus(Class86B60 *self) {
    self->methods->beginMemcardSave(self);
    self->saveCtrl->methods->beginLoad(self->saveCtrl, D_8008AA10, D_8008AA18, self->saveBlock,
                                       self->saveBlockSize);
}

void Class86B60__OnTagBValue(Class86B60 *self, BasicClass *sender, s32 event) {
    if (event < 0x18) {
        if (event >= 0x16) {
            self->methods->endMemcardSave(self);
            if (event == 0x16) {
                self->dreamSys->methods->clearNewGameFlag(self->dreamSys);
                self->methods->commitNameEntry(self, 0x16);
            }
        }
    }
}

Class86B60Methods *GetClass86B60Methods(void) {
    return &gClass86B60Methods;
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
