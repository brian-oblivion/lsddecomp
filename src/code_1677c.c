#include "common.h"
#include "Class6D3C8.h"

INCLUDE_ASM("asm/nonmatchings/code_1677c", new_class_6d3c8);

/* Constructs a Class6D3C8 instance: runs the intermediate base class's own
 * constructor (through its ctor slot), installs this class's own vtable,
 * stores the ctor argument, loads the "ETC\DREAME5.TMD" model, builds this
 * object's owned DreamSys from it, dispatches one DreamSys init call, then
 * runs this class's own slot40 (func_800260A4) once. */
void func_80025FDC(Class6D3C8 *self, Class6D3C8CtorArgs *arg) {
    LoadModelRequest req;

    func_8003B20C()->ctor(self, arg->unk00);
    self->methods = func_800269E0();
    self->arg = arg;
    func_800270AC(func_80048CF0());
    req.type = 0;
    req.path = D_800107A4;
    self->dreamSys = New_DreamSys(func_80043840(&req), 0, 0);
    self->unk24 = 0;
    self->dreamSys->vt->func_228(self->dreamSys, arg->unk14);
    self->methods->slot40(self);
}

extern void func_80048CFC(s32 day, s32 unused);

/* Advances the day cursor: reads the running tick count kept in scratchpad
 * (0x1F800000, the PS-X data-cache-as-RAM region) and reduces it mod 365. */
void func_800260A4(void) {
    func_80048CFC(*(s32 *)0x1F800000 % 365, 0);
}

/* Defers to the base class's own implementation of this slot when this
 * object hasn't been given an override (unk18 == 0). */
void func_80026108(Class6D3C8 *self, void *a1, void *a2) {
    if (self->unk18 == 0) {
        func_8003B20C()->slot44(self, a1, a2, 0);
    }
}

/* Optional stream-load block, gated by self->arg->unk0C: registers a
 * "loader" task for "ETC\ASMKLOGO.TIM" (func_80026254), then a separate
 * "stream" task for whatever type code func_800490F4 hands back
 * ("ETC\ASMK.STR"), then a second loader task for "ETC\OSDLOGO.TIM". */
void func_80026170(Class6D3C8 *self) {
    const char *streamName;
    s32 typeCode;
    s32 typeLookup;
    StreamTask *task;

    if (self->arg->unk0C != 0) {
        func_80026F34(0, 0, 0);
        func_80026254(self, D_800107B4);
        task = func_8003B854(0, 0, 0, 0);
        streamName = func_800490F4(&typeCode);
        typeLookup = func_800493C8(typeCode);
        task->methods->slot44(task, self->unk1C, streamName, typeLookup, 1);
        task->methods->slot4(task);
        func_80026254(self, D_800107C8);
    }
}

/* Registers a "loader" task for the given resource path: allocates the
 * task, gives it a completion callback (func_80026328) and context
 * (self), then sets its remaining parameters (path, self->unk1C) and
 * starts it. */
void func_80026254(Class6D3C8 *self, const char *path) {
    LoaderTask *task = func_8003BE94(0, 0, 0);

    task->methods->slot98(task, func_80026328, self);
    task->methods->slot6C(task, 0);
    task->methods->slotD4(task, path, 0);
    task->methods->slot44(task, self->unk1C, 0);
    task->methods->slot4(task);
}

extern s32 func_8004A070(s32 a0);

s32 func_80026328(void) {
    return func_8004A070(0);
}

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026348);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026410);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026518);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_8002658C);

void func_80026690(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026698);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_8002677C);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026900);
