/*
 * TodActor's methods (include/TodActor.h: an Actor that owns one Actor part
 * per object of a TOD animation and plays the TOD over them), in ROM order,
 * ending with its getter GetTodActorMethods.
 *
 * - construction and teardown: New_TodActor, the ctor, Finalize; the
 *   ModelData is borrowed from the descriptor or made (AcquireModelData /
 *   ReleaseModelData), and one part is made per TOD object (CreateParts /
 *   DestroyParts), each pair behind a Setup/Teardown guard.
 * - overrides of Actor's slots: OnNotify, Reset, AttachToParent /
 *   DetachFromParent (with the peer and the companion), SetDisplay and
 *   SetLightMode (forwarded to every part), Update (event 2 ticks, 4
 *   releases).
 * - playback: Tick, the tick callbacks and their selector, SetTod / PlayTod /
 *   StopTod, and ApplyTodFrame / ApplyTodPacket, which write a frame's
 *   packets into the parts.
 * - PlayTone, on the sound bank the ctor was given; LinkPeer / UnlinkPeer.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_55dd4.h"

void *New_TodActor(void *arg1, void *arg2) {
    TodActor *self;
    TodActorMethods *vt;

    self = (TodActor *)BMemPMgrAlloc(0x98);
    if (self == NULL) {
        return NULL;
    }
    vt = GetTodActorMethods();
    if (vt->ctor(self, arg1, arg2) != NULL) {
        return self;
    }
    BMemPMgrFree(self);
    return NULL;
}

TodActor *TodActor__TodActor(TodActor *self, void *arg1, void *arg2) {
    ActorMethods *base;

    base = GetActorMethods();
    if (base->ctor((Actor *)self) == NULL) {
        return NULL;
    }
    self->methods = GetTodActorMethods();
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

void TodActor__Finalize(TodActor *self) {
    self->methods->teardownModelData(self);
    GetActorMethods()->finalize((Actor *)self);
}

void TodActor__OnNotify(TodActor *self, TagCheckArg *arg1, s32 arg2) {
    ActorMethods *base;

    base = GetActorMethods();
    base->onNotify((Actor *)self, arg1, arg2);
    if (arg1->methods->header == MODEL_DATA_CLASS_HEADER && arg2 == 1 && self->ownsModelData == 0) {
        self->methods->release(self);
    }
}

void TodActor__Reset(TodActor *self) {
    ActorMethods *base;

    base = GetActorMethods();
    base->setDisplay((Actor *)self, 0);
    self->methods->setMainPartNotifies(self, 1);
    self->methods->setLastOffsetValue(self, 0x12C);
    self->methods->disableTickCallback(self);
    self->methods->selectTickCallback(self, TICK_CALLBACK_A);
    self->methods->stopTod(self);
    self->methods->setTod(self, 0);
    if (self->mainPart != NULL) {
        SceneNode__LinkModel((SceneNode *)self, self->mainPart->model);
    }
}

void TodActor__AttachToParent(TodActor *self, TodActor *other, void *arg2, void *arg3, void *arg4) {
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

void TodActor__DetachFromParent(TodActor *self) {
    if (self->parent != 0) {
        self->methods->unlinkPeer(self);
        if (self->ticker != NULL) {
            self->methods->removeChild(self, self->ticker);
        }
        GetActorMethods()->detachFromParent((Actor *)self);
    }
}

void TodActor__SetDisplay(TodActor *self, void *arg) {
    Actor **p;
    s32 i;

    p = self->parts;
    for (i = 0; i < self->partCount; p++) {
        i++;
        (*p)->methods->setDisplay(*p, (s32)arg);
    }
}

void TodActor__SetLightMode(TodActor *self, void *arg) {
    Actor **p;
    s32 i;

    p = self->parts;
    for (i = 0; i < self->partCount; i++, p++) {
        (*p)->methods->setLightMode(*p, (u32)arg);
    }
    GetActorMethods()->setLightMode((Actor *)self, (u32)arg);
}

void TodActor__Update(TodActor *self, void *arg1, s32 val) {
    if (val == 2) {
        self->methods->tick(self);
    }
    if (val == 4) {
        self->methods->release(self);
    }
}

void TodActor__SetMainPartNotifies(TodActor *self, s32 on) {
    self->mainPartNotifies = on;
}

s32 TodActor__SetupModelData(TodActor *self, void *arg1) {
    if (self->modelData != NULL) {
        return 0;
    }
    return TodActor__AcquireModelData(self, arg1);
}

void TodActor__TeardownModelData(TodActor *self) {
    if (self->modelData != NULL) {
        TodActor__ReleaseModelData(self);
    }
}

s32 TodActor__AcquireModelData(TodActor *self, TodActorDesc *other) {
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
    TodActor__ReleaseModelData(self);
    return 1;
}

void TodActor__ReleaseModelData(TodActor *self) {
    ModelData *result;

    self->methods->teardownParts(self);
    if (self->ownsModelData != 0) {
        result = self->modelData->methods->release(self->modelData);
    } else {
        result = NULL;
    }
    self->modelData = result;
}

s32 TodActor__FindPartIndex(TodActor *self, s32 value) {
    u8 *arr;
    s32 count;
    s32 i;
    u8 target;
    u8 unused[8];

    if (self->partIds == NULL) {
        return -1;
    }
    arr = self->partIds;
    /* Keeps arr's register copy above the self->partCount load; without it
     * GCC moves the copy into the delay slot of the count <= 0 branch. */
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

s32 TodActor__SetupParts(TodActor *self) {
    if (self->parts != NULL) {
        return 0;
    }
    return TodActor__CreateParts(self);
}

void TodActor__TeardownParts(TodActor *self) {
    if (self->parts != NULL) {
        TodActor__DestroyParts(self);
    }
}

s32 TodActor__CreateParts(TodActor *self) {
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
    TodActor__DestroyParts(self);
    return 1;
}

void TodActor__DestroyParts(TodActor *self) {
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

void TodActor__Tick(TodActor *self) {
    self->tick = self->tick + 1;
    if (self->tickCallbackEnabled != 0) {
        ((void (*)(void))self->tickCallback)();
    }
    if (self->todPlaying != 0 && self->todFrameCount >= 2) {
        self->todFramePtr = self->methods->applyTodFrame(self, self->todFramePtr, 0);
        self->todFrame = self->todFrame + 1;
        if (self->todFrame >= self->todFrameCount) {
            self->todFrame = 0;
            self->todFramePtr = (u8 *)TODSET_TOD(self->modelData->todSet, self->todIndex)->buffer + 8;
        }
    }
    self->coord2->flg = 0;
}

void TodActor__SelectTickCallback(TodActor *self, s32 value) {
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

s32 TodActor__EnableTickCallback(TodActor *self) {
    return self->tickCallbackEnabled = 1;
}

void TodActor__DisableTickCallback(TodActor *self) {
    self->tickCallbackEnabled = 0;
}

void TodActor__TickCallbackA(TodActor *self) {
    self->methods->moveLocalZ(self, -0x1E, 0);
    if (self->mainPartNotifies == 1 && self->mainPart != NULL) {
        self->mainPart->methods->notifyWithHull(self->mainPart, 6);
    }
}

void TodActor__TickCallbackB(void) {}

void TodActor__TickCallbackC(void) {}

void TodActor__PlayTone(TodActor *self, s32 index) {
    VabStreamObj *sound;

    sound = self->arg2;
    if (sound != NULL) {
        sound->methods->playTone(sound, index, 0x6E, 0x6E);
    }
}

void TodActor__SetTod(TodActor *self, s32 index) {
    self->todIndex = index;
    self->todFrameCount = ((TodHeader *)TODSET_TOD(self->modelData->todSet, index)->buffer)->frameCount;
    self->todFramePtr = (u8 *)TODSET_TOD(self->modelData->todSet, self->todIndex)->buffer + 8;
    self->todFrame = 0;
    self->methods->applyTodFrame(self, self->todFramePtr, 0);
}

s32 TodActor__PlayTod(TodActor *self) {
    return self->todPlaying = 1;
}

void TodActor__StopTod(TodActor *self) {
    self->todPlaying = 0;
}

void *TodActor__ApplyTodFrame(TodActor *self, void *hdr, void *extra) {
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

void *TodActor__ApplyTodPacket(TodActor *self, void *acc, void *extra) {
    u8 outbuf[4];
    void *data;
    s32 idx;
    Actor *elem;
    SceneNodeSub14 *coord;
    GsCOORD2PARAM *param;
    s32 i;

    data = self->modelData->methods->decodePacketWord(
        self->modelData, (s32)acc, (s32)&outbuf[0], (s32)&outbuf[1], (s32)&outbuf[2], (s32)&outbuf[3]);
    idx = TodActor__FindPartIndex(self, outbuf[0]);
    if (idx < 0) {
        goto end;
    }
    elem = self->parts[idx];
    coord = elem->coord2;
    coord->flg = 0;
    param = (GsCOORD2PARAM *)coord->param;

    switch (outbuf[1]) {
        case TOD_PACKET_ATTRIBUTE:
            elem->attribute = (elem->attribute & ((s32 *)data)[0]) | ((s32 *)data)[1];
            break;
        case TOD_PACKET_COORDINATE: {
            if (outbuf[2] & TOD_COORD_DIFFERENTIAL) {
                if (outbuf[2] & TOD_COORD_ROTATE) {
                    s16 *p16 = &param->rotate.vx;

                    for (i = 0; i < 3; i++, p16++) {
                        s16 tmp;

                        tmp = *p16 + ((s32 *)data)[i] / 360;
                        *p16 = tmp;
                        *p16 = tmp % 4096;
                    }
                    data = (u8 *)data + 0xC;
                }
                if (outbuf[2] & TOD_COORD_SCALE) {
                    long *p32 = &param->scale.vx;

                    for (i = 0; i < 3; i++, p32++) {
                        *p32 = (((s16 *)data)[i] * *p32) / 4096;
                    }
                    data = (u8 *)data + 8;
                }
                if (!(outbuf[2] & TOD_COORD_TRANSLATE)) {
                    goto end;
                }
                {
                    long *p32 = &param->trans.vx;

                    for (i = 0; i < 3; i++, p32++) {
                        *p32 += ((s32 *)data)[i];
                    }
                }
            } else {
                if (outbuf[2] & TOD_COORD_ROTATE) {
                    s16 *p16 = &param->rotate.vx;

                    for (i = 0; i < 3; i++, p16++) {
                        *p16 = ((s32 *)data)[i] / 360;
                    }
                    data = (u8 *)data + 0xC;
                }
                if (outbuf[2] & TOD_COORD_SCALE) {
                    long *p32 = &param->scale.vx;

                    for (i = 0; i < 3; i++, p32++) {
                        *p32 = ((s16 *)data)[i];
                    }
                    data = (u8 *)data + 8;
                }
                if (!(outbuf[2] & TOD_COORD_TRANSLATE)) {
                    goto end;
                }
                {
                    long *p32 = &param->trans.vx;

                    for (i = 0; i < 3; i++, p32++) {
                        *p32 = ((s32 *)data)[i];
                    }
                }
            }
            {
                SceneNodeSub14 *coordB;
                s32 v1, v2, v3;

                coordB = elem->coord2;
                v1 = param->trans.vx;
                v2 = param->trans.vy;
                v3 = param->trans.vz;
                coordB->tx = v1;
                coordB->ty = v2;
                coordB->tz = v3;
                /* Keeps the coordB->tz store ahead of the break's jump, leaving retail's
             * nop in its delay slot; without it GCC moves the store into the slot. */
                __asm__("");
            }
            break;
        }
        case TOD_PACKET_MODEL_ID: {
            u16 count;

            count = *(u16 *)data;
            if (count != 0 && elem->model == NULL) {
                s32 v;

                v = (s32)self->modelData->linkResource->methods->getModel(
                    self->modelData->linkResource, count - 1);
                SceneNode__LinkModel((SceneNode *)elem, (void *)v);
            }
            break;
        }
        case TOD_PACKET_PARENT: {
            s32 v1;

            v1 = *(s32 *)data;
            if (v1 == 0 || v1 == 0xFFFF) {
                elem->methods->attachToParent(elem, (SceneNode *)self, NULL);
            } else {
                s32 idx2;

                idx2 = TodActor__FindPartIndex(self, *(u8 *)data);
                elem->methods->attachToParent(elem, (SceneNode *)self->parts[idx2], NULL);
            }
            break;
        }
    }

end:
    return (u8 *)acc + outbuf[3] * 4;
}

void TodActor__LinkPeer(TodActor *self, TodActor *other) {
    if (other != NULL) {
        other->methods->addChild(other, (BasicClass *)self);
        self->methods->addChild(self, (BasicClass *)other);
        self->peer = other;
    }
}

void TodActor__UnlinkPeer(TodActor *self) {
    TodActor *other;

    other = self->peer;
    if (other != NULL) {
        other->methods->removeChild(other, (BasicClass *)self);
        self->methods->removeChild(self, (BasicClass *)self->peer);
        self->peer = NULL;
    }
}

TodActorMethods *GetTodActorMethods(void) {
    return &gTodActorMethods;
}
