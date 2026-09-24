/*
 * class_3bb8c_d -- the bulk of `Class86B60`'s own methods (started in
 * class_3bb8c_c.c: the ctor, `Class86B60__Class86B60`/`New_Class86B60`).
 * All 20 functions here dispatch at fixed slots of `gClass86B60Methods`
 * itself (confirmed via `tools/classtable.py 0x80086B60`); `Class86B60__Tick`
 * is the base class's per-frame entry point, forwarding to a `state`-keyed
 * sub-dispatch that also drives `Class86B60__SetState`.
 *
 * Two owned sub-objects carry most of the class's real work: `nameField`
 * (constructed/destroyed by Class86B60__CreateNameField/DestroyNameField,
 * a text-entry field holding an SJIS-decoded name, with its own cursor
 * blink/colour tick, Class86B60__TickNameFieldCursor) and `unkAC` (a
 * `New_TaskObjF`-constructed controller whose dispatches carry
 * "BISLPS-01556", Sony's memcard save-header game-ID string --
 * Class86B60__Begin/EndMemcardSave and the two Tick-driven
 * Class86B60__UpdateMemcardSave* variants). Read together this is a
 * memory-card save-naming UI: enter/edit a save name, then write it.
 *
 * Every function here is matched C. Names below describe mechanics
 * established from the body and call sites (tier B throughout except the
 * getter/ctor/allocator/destructor shapes, tier A); no in-game purpose is
 * established for any of the numeric `state`/tag values. See each
 * function's own match report for its evidence.
 */
#include "common.h"
#include "class_3bb8c.h"

void Class86B60__Dtor(Class86B60 *self)
{
    if (self->saveCtrl != NULL) {
        self->saveCtrl->methods->release(self->saveCtrl);
        self->iconHandle->methods->release(self->iconHandle);
    }
    Get_vtable_TaskCore()->slot0C(self);
}

void Class86B60__ForwardIfTagB(Class86B60 *self, GenericHeaderObj_3bb8c_d *arg1, s32 arg2)
{
    Get_vtable_TaskCore()->slot38(self, arg1, arg2);
    if ((arg1->methods->header & 0xF) == 0xB) {
        self->methods->slot138(self, arg1, arg2);
    }
}

void Class86B60__ShowTitleIcon(Class86B60 *self)
{
    self->unk34 = 0;
    self->unk2C = 0x190;
    self->methods->slotD4(self, &D_800114E8, 0);
    self->methods->slot6C(self, 0xA);
    self->dreamSysView->methods->slotF0(self->dreamSysView, 0, 0);
}

void Class86B60__RegisterHandlers(Class86B60 *self)
{
    u32 i;
    u8 *entry;

    i = 0;
    entry = (u8 *)&D_80086DAC;
    for (; i < 2; i++) {
        self->handlerTable->unk0->methods->slot78(self->handlerTable->unk0, &self->unk93, entry);
        entry += 0xC;
    }
}

void Class86B60__SetState(Class86B60 *self, s32 arg1)
{
    Get_vtable_TaskCore()->slot60(self, arg1);
    if (arg1 == 5) {
        self->methods->slot124(self, 0);
    }
    if (arg1 == 0xA) {
        self->methods->slot7C(self);
        self->methods->slotF0(self, self->unk4C->unk8, 1);
        self->methods->slot78(self);
    }
}

void Class86B60__Tick(Class86B60 *self)
{
    void (*fn)(Class86B60 *);

    Get_vtable_TaskCore()->slot90(self);
    switch (self->state) {
    case 1:
        self->unk38 = 0;
        self->dreamSysView->methods->slotF0(self->dreamSysView, 0, 1);
        fn = self->methods->slot94;
        break;
    case 2:
        fn = self->methods->slot130;
        break;
    case 3:
        fn = self->methods->slot134;
        break;
    case 4:
        self->unk38 = 2;
        fn = self->methods->slot94;
        break;
    default:
        return;
    }
    fn(self);
}

void Class86B60__RefreshViewValue(Class86B60 *self)
{
    s32 buf;

    Get_vtable_TaskCore()->slot94(self);
    buf = self->unk60->unk14;
    self->dreamSysView->methods->slot19C(self->dreamSysView, &buf);
}

/* Class86B60__CreateNameField's own `arg1`: only its own +0x004 field is read, forwarded
 * opaquely as New_Obj6EAC0's `ctx` argument. */
typedef struct Arg1DB18_3bb8c_d Arg1DB18_3bb8c_d;
struct Arg1DB18_3bb8c_d {
    u8 pad0[0x004];
    void *unk4; /* +0x004 */
};

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

void Class86B60__CreateNameField(Class86B60 *self, Arg1DB18_3bb8c_d *arg1)
{
    u32 size;
    char *buf;

    if (arg1 == NULL) {
        return;
    }
    if (self->dreamSysView->methods->slot1AC(self->dreamSysView)) {
        strcpy((char *)D_8008AA18 + 0x18, (char *)D_8008AA14);
        CopyMemcardIconTemplate((s32)D_8008AA18, 0);
    }
    size = strlen((char *)D_8008AA18);
    size = (size >> 1) + 4;
    buf = BMemPMgrAlloc(size);
    DecodeFullWidthSjis(buf, D_8008AA18);
    self->nameField = (Class86B60UnkB0Obj_3bb8c_d *)New_Obj6EAC0(arg1->unk4, size, buf);
    self->nameField->unkAB = 8;
    self->nameField->unkAC = 4;
    self->nameField->unkAA = 9;
    BMemPMgrFree(buf);
}

void Class86B60__DestroyNameField(Class86B60 *self)
{
    self->nameField->methods->release(self->nameField);
    Get_vtable_TaskCore()->slotDC(self);
}

void Class86B60__ForwardToNameField(Class86B60 *self, s32 arg1)
{
    Get_vtable_TaskCore()->slotE0(self, arg1);
    self->nameField->methods->slot4C(self->nameField, arg1, &D_8008A9B4);
}

/* Class86B60__TickNameFieldCursor's own `arg1`: a 3-byte colour-like triple, copied whole into
 * a local (the copy is a BLKmode struct move: three `lb`, then three `sb`).
 * Kept a minimal, distinct local type rather than reusing this header's
 * broader `Descriptor10` (same 3-byte shape, but an unrelated context --
 * nothing here shows a 4th byte or the two trailing halfwords). */
typedef struct Arg1DCD0_3bb8c_d Arg1DCD0_3bb8c_d;
struct Arg1DCD0_3bb8c_d {
    s8 b0;
    s8 b1;
    s8 b2;
};

/* MATCHED round 75 (was STALL round 43) -- see
 * docs/match-reports/Class86B60__TickNameFieldCursor.md. `base` is taken BEFORE the first call
 * (so it crosses a call and gets $s1), `buf = *arg1` is one struct copy, and
 * each arm indexes `base[D_8008AA28]` directly. */
void Class86B60__TickNameFieldCursor(Class86B60 *self, Arg1DCD0_3bb8c_d *arg1)
{
    Arg1DCD0_3bb8c_d buf;
    u8 *base;

    base = (u8 *)&buf;
    Get_vtable_TaskCore()->slotE4(self, arg1);
    if (self->unk3C != 0) {
        base[0] = 0;
        base[1] = 0;
        base[2] = 0;
        base[D_8008AA28] = 0x80;
    } else {
        buf = *arg1;
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
    self->nameField->methods->slotB8(self->nameField, &buf);
}

/* CheckObj866E8CountFlag is ALREADY MATCHED (src/class_3bb8c_c.c), as a genuinely
 * 2-argument function -- but THIS call site sets up a 3rd argument
 * (self->unkA4, in $a2) that the other unit's own 2-parameter view never
 * receives. Same independent-arities situation already documented for
 * Get_vtable_TaskCore/BaseTaskCtorTable_3bb8c_c: this unit's own local view
 * matches what THIS call site needs. */
extern void CheckObj866E8CountFlag(void *arg0, void *arg1, void *arg2); /* arity-ok: the definition is 2-parameter and the callee WRITES $a2 (`li a2,0x1` at 0x8004D690) before reading it, but the 3rd argument is byte-load-bearing here -- retail emits `lw a2,164(s0)` at 0x8004DE74 */

void Class86B60__CommitNameEntry(Class86B60 *self)
{
    s32 size;
    s32 origState;
    void *buf1;
    s32 buf2;

    size = self->nameField->unkA9;
    origState = self->state;
    buf1 = BMemPMgrAlloc(size);
    DecodeFullWidthSjis(buf1, D_8008AA18);
    self->nameField->methods->slotCC(self->nameField, buf1);
    BMemPMgrFree(buf1);
    CheckObj866E8CountFlag(self, self->unk4C, self->dreamSysView);
    self->methods->slotE0(self, self->unk14);
    self->dreamSysView->methods->slot19C(self->dreamSysView, &buf2);
    self->state = 5;
    self->methods->slot60(self, 0xB);
    self->methods->slot11C(self, buf2, 1);
    self->methods->slot60(self, 0xF);
    self->methods->slotF0(self, (void *)origState, 0);
    self->dreamSysView->methods->slot19C(self->dreamSysView, &buf2);
}

/* This unit's own local view of func_8003B39C (already matched elsewhere,
 * many independent-arity views project-wide -- see e.g.
 * src/class_3bb8c_g.c, src/class_3bb8c_i.c). Return type matches what
 * this call site actually stores it into (`self->iconHandle`). */
extern GenericReleaseObj_3bb8c_d *func_8003B39C(const char *path);

void Class86B60__BeginMemcardSave(Class86B60 *self)
{
    if (self->saveCtrl == NULL) {
        self->iconHandle = func_8003B39C(D_800114F8);
        self->saveCtrl = New_TaskObjF((void *)1, NULL);
    }
    self->saveCtrl->methods->slot6C(self->saveCtrl, D_8008A9D0, &D_80086D6C,
                                  self->handlerTable->unk4, self->unk10, self->unk14,
                                  self->unk48);
    self->methods->slot10(self, self->saveCtrl);
    self->methods->slot14(self, self->handlerTable->unk4);
    self->methods->slot14(self, self->unk10);
}

void Class86B60__EndMemcardSave(Class86B60 *self)
{
    self->methods->slot10(self, self->handlerTable->unk4);
    self->methods->slot10(self, self->unk10);
    self->methods->slot14(self, self->saveCtrl);
    self->saveCtrl->methods->slot70(self->saveCtrl);
}

void Class86B60__UpdateMemcardSaveWithIcon(Class86B60 *self)
{
    s32 buf;

    buf = self->unk60->unk14;
    self->dreamSysView->methods->slot19C(self->dreamSysView, &buf);
    self->methods->slot128(self);
    if (self->dreamSysView->methods->slot1AC(self->dreamSysView)) {
        *(u8 *)D_8008AA10 = 0;
    }
    self->saveCtrl->methods->slot78(self->saveCtrl, D_8008AA10, D_8008AA18, 0xD, 3,
                                  self->iconHandle, self->unkBC, self->unkC0);
}

void Class86B60__UpdateMemcardSaveStatus(Class86B60 *self)
{
    self->methods->slot128(self);
    self->saveCtrl->methods->slot74(self->saveCtrl, D_8008AA10, D_8008AA18,
                                  self->unkBC, self->unkC0);
}

void Class86B60__OnTagBValue(Class86B60 *self, s32 arg1, s32 value)
{
    if (value < 0x18) {
        if (value >= 0x16) {
            self->methods->slot12C(self);
            if (value == 0x16) {
                self->dreamSysView->methods->slot1A8(self->dreamSysView);
                self->methods->slot124(self, 0x16);
            }
        }
    }
}

Class86B60Methods *GetClass86B60Methods(void)
{
    return &gClass86B60Methods;
}

void *New_TaskObjF(void *arg0, void *arg1)
{
    void *self;

    self = BMemPMgrAlloc(0x84);
    if (self == NULL) {
        goto fail;
    }
    GetTaskObjFMethods()->ctor(self, arg0, arg1);
    return self;
fail:
    return NULL;
}

/* TaskObjF__ClearResourceSlots is ALREADY MATCHED, but in class_3bb8c_e.c under its own
 * local type (`Node3bb8cE *`) -- this unit's own independent local view
 * of the same real object per the project's convention; only `self` is
 * ever forwarded here, never dereferenced. */
extern void TaskObjF__ClearResourceSlots(void *self);

/* libcard, linked SDK objects (config/psyq-objects.txt: libcard/a74,
 * libcard/a75, libcard/c112 -- see docs/match-reports/TaskObjF__TaskObjF.md).
 * Declared locally rather than in the shared header, same policy as
 * malloc/free/printf (CLAUDE.md, "To include/ has one exception"). */
extern void InitCARD(s32 padEnable);
extern void StartCARD(void);
extern void _bu_init(void);

void TaskObjF__TaskObjF(GenericCtorObj_3bb8c_d *self, s32 arg1, s32 arg2)
{
    s32 count;

    Get_vtable_BasicClass()->ctor(self);
    self->methods = GetTaskObjFMethods();
    count = D_8008AA30;
    D_8008AA30 = count + 1;
    if (count == 0) {
        InitCARD(arg1);
        StartCARD();
        _bu_init();
    }
    TaskObjF__ClearResourceSlots(self);
    self->methods->setCardSlot(self, arg2);
}
