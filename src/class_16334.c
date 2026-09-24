#include "common.h"
#include "class_16334.h"

Pad *func_80025B34(void *arg1, s32 port) {
    Pad *self;

    self = BMemPMgrAlloc(0x20);
    if (self == NULL) {
        goto fail;
    }
    func_80025E9C()->ctor(self, arg1, port);
    return self;
fail:
    return NULL;
}

void func_80025BA0(Pad *self, void *arg1, s32 port) {
    Get_vtable_BasicClass()->ctor(self);
    self->methods = func_80025E9C();
    if (D_8008A848++ == 0) {
        PadInit(arg1);
    }
    self->methods->init(self, port);
}

void *func_80025C30(Pad *self) {
    if (--D_8008A848 == 0) {
        PadStop();
    }
    return Get_vtable_BasicClass()->dtor(self);
}

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

    newMask = PadRead(self->port);
    oldMask = self->heldMask;
    self->heldMask = newMask;
    changed = newMask ^ oldMask;
    self->releasedMask = changed & oldMask;
    self->pressedMask = changed & newMask;
    return newMask;
}

void func_80025D10(Pad *self) {
    s32 events[16];
    void (*onButtonEvent)(Pad *self, s32 event);
    u32 held;
    u32 released;
    u32 pressed;
    s32 *p;
    s32 code;
    s32 i;

    /* Bare scheduling barrier. Without it the four prologue register stores
     * come out in the order s0, ra, s2, s1 instead of retail's ra, s2, s1, s0
     * -- same registers, same stack offsets, order only, so this is the
     * permitted form under the project rule and not a register pin. Ten source
     * shapes were tried first; see docs/match-reports/func_80025D10.md. */
    __asm__("");
    held = self->heldMask;
    released = self->releasedMask;
    pressed = self->pressedMask;
    p = events;
    if (held == 0 && released == 0 && pressed == 0) {
        return;
    }

    for (i = 0; i < 16; i++) {
        u32 mask = D_8008B388[i];

        code = -1;
        if (released & mask) {
            code = 0x22;
        } else if (pressed & mask) {
            code = 0x12;
        } else if (held & mask) {
            code = 0x02;
        }
        if (code >= 0) {
            *p++ = code + i;
        }
    }

    onButtonEvent = self->methods->onButtonEvent;
    for (p--; p >= events; p--) {
        onButtonEvent(self, *p);
    }
}

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
