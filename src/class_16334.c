#include "common.h"
#include "class_16334.h"

Pad *func_80025B34(void *arg1, s32 port) {
    Pad *self;

    self = func_80017B34(0x20);
    if (self == NULL) {
        goto fail;
    }
    func_80025E9C()->ctor(self, arg1, port);
    return self;
fail:
    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/class_16334", func_80025BA0);

INCLUDE_ASM("asm/nonmatchings/class_16334", func_80025C30);

void func_80025C84(Pad *self, s32 port) {
    self->port = (port != 0);
    self->heldMask = 0;
    self->releasedMask = 0;
    self->pressedMask = 0;
    self->methods->loadButtonTable();
}

u32 func_80025CC4(Pad *self) {
    u32 newMask;
    u32 oldMask;
    u32 changed;

    newMask = func_80025EFC(self->port);
    oldMask = self->heldMask;
    self->heldMask = newMask;
    changed = newMask ^ oldMask;
    self->releasedMask = changed & oldMask;
    self->pressedMask = changed & newMask;
    return newMask;
}

INCLUDE_ASM("asm/nonmatchings/class_16334", func_80025D10);

void func_80025E14(void) {
}

void func_80025E1C(void) {
    Block64 local;
    u32 *dst;
    u32 *src;
    s32 i;

    dst = D_8008B388;
    local = D_80010764;
    i = 0;
    src = local.w;
    for (; i < 16; i++) {
        *dst++ = *src++;
    }
}

void func_80025E94(void) {
}

PadMethods *func_80025E9C(void) {
    return &D_8006D370;
}
