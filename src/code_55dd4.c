/*
 * Class65650 (include/Class65650.h): an Actor subclass that owns one
 * Actor "part" per object of a TOD animation and plays TODs over them.
 * Method table gClass65650Methods; Entity derives from it.
 *
 * - construction: modelData (+0x5C) is borrowed from the ctor's arg1 or made
 *   by New_ModelData; CreateParts allocates partCount parts and their TOD
 *   object ids from it; Destructor/ReleaseModelData undo both.
 * - base-slot overrides: OnNotify, InitDefaults, AttachToParent,
 *   DetachFromParent, SetDisplay/SetLightMode (forwarded to every part), and
 *   Update (+0x098: code 2 -> Tick, 4 -> Release).
 * - Tick: once per call, runs the selected tick callback (A/B/C) and, while
 *   a TOD is playing, applies its next frame and wraps at the frame count.
 * - ApplyTodFrame/ApplyTodPacket: walk a TOD frame's packets and apply the
 *   attribute, coordinate (GsCOORD2PARAM rotate/scale/trans), model-id and
 *   parent packets to the part the packet's object id names.
 */
#include "common.h"
#include "code_55dd4.h"

void *New_Class65650(void *arg1, void *arg2)
{
    Class65650 *self;
    Class65650Methods *vt;

    self = (Class65650 *)BMemPMgrAlloc(0x98);
    if (self == NULL) {
        return NULL;
    }
    vt = Get_vtable_Class65650();
    if (vt->ctor(self, arg1, arg2) != NULL) {
        return self;
    }
    BMemPMgrFree(self);
    return NULL;
}

Class65650 *Class65650__Class65650(Class65650 *self, void *arg1, void *arg2)
{
    ActorMethods *base;

    base = GetActorMethods();
    if (base->ctor((Actor *)self) == NULL) {
        return NULL;
    }
    self->methods = Get_vtable_Class65650();
    self->arg2 = arg2;
    self->modelData = NULL;
    self->mainPart = NULL;
    self->parts = NULL;
    self->peer = 0;
    if (self->methods->setupModelData(self, arg1) != 0) {
        base = GetActorMethods();
        base->finalize((Actor *)self);
        return NULL;
    }
    self->methods->addChild(self, (BasicClass *)self->modelData);
    self->methods->reset(self);
    return self;
}

void Class65650__Finalize(Class65650 *self)
{
    self->methods->teardownModelData(self);
    GetActorMethods()->finalize((Actor *)self);
}

void Class65650__OnNotify(Class65650 *self, TagCheckArg *arg1, s32 arg2)
{
    ActorMethods *base;

    base = GetActorMethods();
    base->onNotify((Actor *)self, arg1, arg2);
    if (arg1->methods->header == MODEL_DATA_CLASS_HEADER && arg2 == 1 && self->ownsModelData == 0) {
        self->methods->release(self);
    }
}

void Class65650__Reset(Class65650 *self)
{
    ActorMethods *base;

    base = GetActorMethods();
    base->setDisplay((Actor *)self, 0);
    self->methods->setUnk64(self, 1);
    self->methods->setLastOffsetValue(self, 0x12C);
    self->methods->disableTickCallback(self);
    self->methods->selectTickCallback(self, TICK_CALLBACK_A);
    self->methods->stopTod(self);
    self->methods->setTod(self, 0);
    if (self->mainPart != NULL) {
        Class6B5CC__LinkModel((Class6B5CC *)self, self->mainPart->model);
    }
}

void Class65650__AttachToParent(Class65650 *self, Class65650 *other, void *arg2, void *arg3, void *arg4)
{
    ActorMethods *base;

    if (self->parent == 0) {
        base = GetActorMethods();
        base->attachToParent((Actor *)self, arg3, arg4);
        if (arg2 != NULL && self->ticker == NULL) {
            self->methods->addChild(self, arg2);
        }
        self->methods->linkPeer(self, other);
    }
}

void Class65650__DetachFromParent(Class65650 *self)
{
    if (self->parent != 0) {
        self->methods->unlinkPeer(self);
        if (self->ticker != NULL) {
            self->methods->removeChild(self, self->ticker);
        }
        GetActorMethods()->detachFromParent((Actor *)self);
    }
}

void Class65650__SetDisplay(Class65650 *self, void *arg)
{
    Actor **p;
    s32 i;

    p = self->parts;
    for (i = 0; i < self->partCount; p++) {
        i++;
        (*p)->methods->setDisplay(*p, (s32)arg);
    }
}

void Class65650__SetLightMode(Class65650 *self, void *arg)
{
    Actor **p;
    s32 i;

    p = self->parts;
    for (i = 0; i < self->partCount; i++, p++) {
        (*p)->methods->setLightMode(*p, (u32)arg);
    }
    GetActorMethods()->setLightMode((Actor *)self, (u32)arg);
}

void Class65650__Update(Class65650 *self, void *arg1, s32 val)
{
    if (val == 2) {
        self->methods->tick(self);
    }
    if (val == 4) {
        self->methods->release(self);
    }
}

void Class65650__SetUnk64(Class65650 *self, s32 value)
{
    self->unk64 = value;
}

s32 Class65650__SetupModelData(Class65650 *self, void *arg1)
{
    if (self->modelData != NULL) {
        return 0;
    }
    return Class65650__AcquireModelData(self, arg1);
}

void Class65650__TeardownModelData(Class65650 *self)
{
    if (self->modelData != NULL) {
        Class65650__ReleaseModelData(self);
    }
}

s32 Class65650__AcquireModelData(Class65650 *self, UnkArg1Obj *other)
{
    if (other->modelData != NULL) {
        self->modelData = other->modelData;
        self->ownsModelData = 0;
    } else {
        self->modelData = New_ModelData((struct Src6F240 *)other);
        self->ownsModelData = 1;
    }
    if (self->modelData == NULL) {
        goto fail;
    }
    return self->methods->setupParts(self);
fail:
    Class65650__ReleaseModelData(self);
    return 1;
}

void Class65650__ReleaseModelData(Class65650 *self)
{
    ModelData *result;

    self->methods->teardownParts(self);
    if (self->ownsModelData != 0) {
        result = self->modelData->methods->release(self->modelData);
    } else {
        result = NULL;
    }
    self->modelData = result;
}

s32 Class65650__FindPartIndex(Class65650 *self, s32 value)
{
    u8 *arr;
    s32 count;
    s32 i;
    u8 target;
    u8 unused[8];

    if (self->partIds == NULL) {
        return -1;
    }
    arr = self->partIds;
    __asm__("");
    count = self->partCount;
    if (count <= 0) {
        return -1;
    }
    i = 0;
    target = (u8)value;
    do {
        if (*arr == target) {
            return i;
        }
        i++;
        arr++;
    } while (i < count);
    return -1;
}

s32 Class65650__SetupParts(Class65650 *self)
{
    if (self->parts != NULL) {
        return 0;
    }
    return Class65650__CreateParts(self);
}

void Class65650__TeardownParts(Class65650 *self)
{
    if (self->parts != NULL) {
        Class65650__DestroyParts(self);
    }
}

s32 Class65650__CreateParts(Class65650 *self)
{
    s32 buf[4];
    s32 count;
    s32 i;
    Actor **p;

    count = self->modelData->methods->scanPackets(self->modelData, 0, (s32)buf) & 0xFF;
    self->parts = BMemPMgrAlloc(count * 4);
    if (self->parts == NULL) {
        goto alloc_fail;
    }
    self->partIds = BMemPMgrAlloc(count);
    if (self->partIds == NULL) {
        goto alloc_fail;
    }
    self->modelData->methods->scanPackets(self->modelData, (s32)self->partIds, (s32)buf);

    p = self->parts;
    i = 0;
    self->partCount = 0;
    if (count != 0) {
        do {
            if ((*p++ = New_Actor()) == NULL) {
                goto fail;
            }
            self->partCount++;
            i++;
        } while (i < count);
    }
    self->mainPart = self->parts[buf[0]];
    return 0;

alloc_fail:
    self->partIds = NULL;
fail:
    Class65650__DestroyParts(self);
    return 1;
}

void Class65650__DestroyParts(Class65650 *self)
{
    Actor **p;

    if (self->parts != NULL && self->partIds != NULL) {
        p = self->parts;
        while (self->partCount-- > 0) {
            (*p)->methods->release(*p);
            p++;
        }
        self->mainPart = 0;
    }
    self->partIds = BMemPMgrFree(self->partIds);
    self->parts = BMemPMgrFree(self->parts);
}

void Class65650__Tick(Class65650 *self)
{
    self->tick = self->tick + 1;
    if (self->tickCallbackEnabled != 0) {
        ((void (*)(void))self->tickCallback)();
    }
    if (self->todPlaying != 0 && self->todFrameCount >= 2) {
        self->todFramePtr = self->methods->applyTodFrame(self, self->todFramePtr, 0);
        self->todFrame = self->todFrame + 1;
        if (self->todFrame >= self->todFrameCount) {
            self->todFrame = 0;
            self->todFramePtr = (u8 *)(*(GroupObj **)((u8 *)self->modelData->todSet->buffer + 8 + self->todIndex * 4))->tod + 8;
        }
    }
    self->coord2->flg = 0;
}

void Class65650__SelectTickCallback(Class65650 *self, s32 value)
{
    switch ((u8)value) {
    case TICK_CALLBACK_A:
        self->tickCallback = self->methods->tickCallbackA;
        break;
    case TICK_CALLBACK_B:
        self->tickCallback = self->methods->tickCallbackB;
        break;
    case TICK_CALLBACK_C:
        self->tickCallback = self->methods->tickCallbackC;
        break;
    }
}

s32 Class65650__EnableTickCallback(Class65650 *self)
{
    return self->tickCallbackEnabled = 1;
}

void Class65650__DisableTickCallback(Class65650 *self)
{
    self->tickCallbackEnabled = 0;
}

void Class65650__TickCallbackA(Class65650 *self)
{
    self->methods->moveLocalZ(self, -0x1E, 0);
    if (self->unk64 == 1 && self->mainPart != NULL) {
        self->mainPart->methods->notifyIfUnk20Active(self->mainPart, 6);
    }
}

void Class65650__TickCallbackB(void) {
}

void Class65650__TickCallbackC(void) {
}

void Class65650__func_800661D4(Class65650 *self, void *arg1)
{
    UnkArg2Obj *obj;

    obj = self->arg2;
    if (obj != NULL) {
        obj->methods->slot80(obj, arg1, 0x6E, 0x6E);
    }
}

void Class65650__SetTod(Class65650 *self, s32 index)
{
    self->todIndex = index;
    self->todFrameCount = (*(GroupObj **)((u8 *)self->modelData->todSet->buffer + 8 + index * 4))->tod->frameCount;
    self->todFramePtr = (u8 *)(*(GroupObj **)((u8 *)self->modelData->todSet->buffer + 8 + self->todIndex * 4))->tod + 8;
    self->todFrame = 0;
    self->methods->applyTodFrame(self, self->todFramePtr, 0);
}

s32 Class65650__PlayTod(Class65650 *self)
{
    return self->todPlaying = 1;
}

void Class65650__StopTod(Class65650 *self)
{
    self->todPlaying = 0;
}

void *Class65650__ApplyTodFrame(Class65650 *self, void *hdr, void *extra)
{
    s32 count;
    u32 i;

    count = *(u16 *)((u8 *)hdr + 2);
    hdr = (u8 *)hdr + 8;
    for (i = 0; i < count;) {
        i++;
        hdr = self->methods->applyTodPacket(self, hdr, extra);
    }
    return hdr;
}

void *Class65650__ApplyTodPacket(Class65650 *self, void *acc, void *extra)
{
    u8 outbuf[4];
    void *data;
    s32 idx;
    Actor *elem;
    Class6B5CCSub14 *coord;
    TimeTargetObj *param;
    s32 i;

    data = self->modelData->methods->decodePacketWord(self->modelData, (s32)acc, (s32)&outbuf[0], (s32)&outbuf[1],
                                                      (s32)&outbuf[2], (s32)&outbuf[3]);
    idx = Class65650__FindPartIndex(self, outbuf[0]);
    if (idx < 0) {
        goto end;
    }
    elem = self->parts[idx];
    coord = elem->coord2;
    coord->flg = 0;
    param = (TimeTargetObj *)coord->param;

    switch (outbuf[1]) {
    case TOD_PACKET_ATTRIBUTE:
        elem->attribute = (elem->attribute & ((s32 *)data)[0]) | ((s32 *)data)[1];
        break;
    case TOD_PACKET_COORDINATE: {
        if (outbuf[2] & TOD_COORD_DIFFERENTIAL) {
            if (outbuf[2] & TOD_COORD_ROTATE) {
                s16 *p16 = param->rotate;

                for (i = 0; i < 3; i++, p16++) {
                    s16 tmp;

                    tmp = *p16 + ((s32 *)data)[i] / 360;
                    *p16 = tmp;
                    *p16 = tmp % 4096;
                }
                data = (u8 *)data + 0xC;
            }
            if (outbuf[2] & TOD_COORD_SCALE) {
                s32 *p32 = param->scale;

                for (i = 0; i < 3; i++, p32++) {
                    *p32 = (((s16 *)data)[i] * *p32) / 4096;
                }
                data = (u8 *)data + 8;
            }
            if (!(outbuf[2] & TOD_COORD_TRANSLATE)) {
                goto end;
            }
            {
                s32 *p32 = param->trans;

                for (i = 0; i < 3; i++, p32++) {
                    *p32 += ((s32 *)data)[i];
                }
            }
        } else {
            if (outbuf[2] & TOD_COORD_ROTATE) {
                s16 *p16 = param->rotate;

                for (i = 0; i < 3; i++, p16++) {
                    *p16 = ((s32 *)data)[i] / 360;
                }
                data = (u8 *)data + 0xC;
            }
            if (outbuf[2] & TOD_COORD_SCALE) {
                s32 *p32 = param->scale;

                for (i = 0; i < 3; i++, p32++) {
                    *p32 = ((s16 *)data)[i];
                }
                data = (u8 *)data + 8;
            }
            if (!(outbuf[2] & TOD_COORD_TRANSLATE)) {
                goto end;
            }
            {
                s32 *p32 = param->trans;

                for (i = 0; i < 3; i++, p32++) {
                    *p32 = ((s32 *)data)[i];
                }
            }
        }
        {
            Class6B5CCSub14 *coordB;
            s32 v1, v2, v3;

            coordB = elem->coord2;
            v1 = param->trans[0];
            v2 = param->trans[1];
            v3 = param->trans[2];
            coordB->tx = v1;
            coordB->ty = v2;
            coordB->tz = v3;
            __asm__("");
        }
        break;
    }
    case TOD_PACKET_MODEL_ID: {
        u16 count;

        count = *(u16 *)data;
        if (count != 0 && elem->model == NULL) {
            s32 v;

            v = ((Unk2CObj *)self->modelData->linkResource)->methods->getModel(self->modelData->linkResource, count - 1);
            Class6B5CC__LinkModel((Class6B5CC *)elem, (void *)v);
        }
        break;
    }
    case TOD_PACKET_PARENT: {
        s32 v1;

        v1 = *(s32 *)data;
        if (v1 == 0 || v1 == 0xFFFF) {
            elem->methods->attachToParent(elem, (Class6B5CC *)self, NULL);
        } else {
            s32 idx2;

            idx2 = Class65650__FindPartIndex(self, *(u8 *)data);
            elem->methods->attachToParent(elem, (Class6B5CC *)self->parts[idx2], NULL);
        }
        break;
    }
    }

end:
    return (u8 *)acc + outbuf[3] * 4;
}

void Class65650__LinkPeer(Class65650 *self, Class65650 *other)
{
    if (other != NULL) {
        other->methods->addChild(other, (BasicClass *)self);
        self->methods->addChild(self, (BasicClass *)other);
        self->peer = other;
    }
}

void Class65650__UnlinkPeer(Class65650 *self)
{
    Class65650 *other;

    other = self->peer;
    if (other != NULL) {
        other->methods->removeChild(other, (BasicClass *)self);
        self->methods->removeChild(self, (BasicClass *)self->peer);
        self->peer = NULL;
    }
}

Class65650Methods *Get_vtable_Class65650(void)
{
    return &gClass65650Methods;
}
