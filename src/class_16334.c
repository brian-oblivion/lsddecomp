#include "common.h"
#include "class_16334.h"

Pad *New_Pad(void *arg1, s32 port) {
    Pad *self;

    self = BMemPMgrAlloc(0x20);
    if (self == NULL) {
        goto fail;
    }
    Get_vtable_Pad()->ctor(self, arg1, port);
    return self;
fail:
    return NULL;
}

void Pad__Pad(Pad *self, void *arg1, s32 port) {
    Get_vtable_BasicClass()->ctor(self);
    self->methods = Get_vtable_Pad();
    if (D_8008A848++ == 0) {
        PadInit(arg1);
    }
    self->methods->init(self, port);
}

void *Pad__Destroy(Pad *self) {
    if (--D_8008A848 == 0) {
        PadStop();
    }
    return Get_vtable_BasicClass()->dtor(self);
}

void Pad__Init(Pad *self, s32 port) {
    self->port = (port != 0);
    self->heldMask = 0;
    self->releasedMask = 0;
    self->pressedMask = 0;
    self->methods->loadButtonTable();
}

u32 Pad__UpdateMasks(Pad *self) {
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

void Pad__DispatchEvents(Pad *self) {
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
     * shapes were tried first; see docs/match-reports/Pad__DispatchEvents.md. */
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

void Pad__func_80025E14(void) {
}

void Pad__LoadButtonTable(void) {
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

void Pad__func_80025E94(void) {
}

PadMethods *Get_vtable_Pad(void) {
    return &D_8006D370;
}
