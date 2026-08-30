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

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026170);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026254);

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
