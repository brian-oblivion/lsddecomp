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

void *New_TodActor(void *desc, void *sound) {
    TodActor *self;
    TodActorMethods *vt;

    self = (TodActor *)BMemPMgrAlloc(sizeof(TodActor));
    if (self == NULL) {
        return NULL;
    }
    vt = GetTodActorMethods();
    if (vt->ctor(self, desc, sound) != NULL) {
        return self;
    }
    BMemPMgrFree(self);
    return NULL;
}

TodActor *TodActor__TodActor(TodActor *self, void *desc, void *sound) {
    ActorMethods *base;

    base = GetActorMethods();
    if (base->ctor((Actor *)self) == NULL) {
        return NULL;
    }
    self->methods = GetTodActorMethods();
    self->sound = sound;
    self->modelData = NULL;
    self->mainPart = NULL;
    self->parts = NULL;
    self->peer = 0;
    if (self->methods->setupModelData(self, desc) != 0) {
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

void TodActor__OnNotify(TodActor *self, TagCheckArg *sender, s32 event) {
    ActorMethods *base;

    base = GetActorMethods();
    base->onNotify((Actor *)self, sender, event);
    if (sender->methods->header == MODEL_DATA_CLASS_HEADER && event == 1 && self->ownsModelData == 0) {
        self->methods->release(self);
    }
}

void TodActor__Reset(TodActor *self) {
    ActorMethods *base;

    base = GetActorMethods();
    base->setDisplay((Actor *)self, 0);
    self->methods->setMainPartNotifies(self, 1);
    self->methods->setLastOffsetValue(self, 300);
    self->methods->disableTickCallback(self);
    self->methods->selectTickCallback(self, TICK_CALLBACK_A);
    self->methods->stopTod(self);
    self->methods->setTod(self, 0);
    if (self->mainPart != NULL) {
        SceneNode__LinkModel((SceneNode *)self, self->mainPart->model);
    }
}

void TodActor__AttachToParent(TodActor *self, TodActor *peer, void *companion, void *parent,
                              void *offset) {
    ActorMethods *base;

    if (self->parent == 0) {
        base = GetActorMethods();
        base->attachToParent((Actor *)self, parent, offset);
        if (companion != NULL && self->ticker == NULL) {
            self->methods->addChild(self, companion);
        }
        self->methods->linkPeer(self, peer);
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

void TodActor__SetDisplay(TodActor *self, void *on) {
    Actor **p;
    s32 i;

    p = self->parts;
    for (i = 0; i < self->partCount; p++) {
        i++;
        (*p)->methods->setDisplay(*p, (s32)on);
    }
}

void TodActor__SetLightMode(TodActor *self, void *mode) {
    Actor **p;
    s32 i;

    p = self->parts;
    for (i = 0; i < self->partCount; i++, p++) {
        (*p)->methods->setLightMode(*p, (u32)mode);
    }
    GetActorMethods()->setLightMode((Actor *)self, (u32)mode);
}

void TodActor__Update(TodActor *self, void *sender, s32 event) {
    if (event == 2) {
        self->methods->tick(self);
    }
    if (event == 4) {
        self->methods->release(self);
    }
}

void TodActor__SetMainPartNotifies(TodActor *self, s32 on) {
    self->mainPartNotifies = on;
}

s32 TodActor__SetupModelData(TodActor *self, void *desc) {
    if (self->modelData != NULL) {
        return 0;
    }
    return TodActor__AcquireModelData(self, desc);
}

void TodActor__TeardownModelData(TodActor *self) {
    if (self->modelData != NULL) {
        TodActor__ReleaseModelData(self);
    }
}

s32 TodActor__AcquireModelData(TodActor *self, TodActorDesc *desc) {
    if (desc->modelData != NULL) {
        self->modelData = desc->modelData;
        self->ownsModelData = 0;
    } else {
        self->modelData = New_ModelData(&desc->src);
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
    ModelData *next;

    self->methods->teardownParts(self);
    if (self->ownsModelData != 0) {
        next = self->modelData->methods->release(self->modelData);
    } else {
        next = NULL;
    }
    self->modelData = next;
}

s32 TodActor__FindPartIndex(TodActor *self, s32 id) {
    u8 *ids;
    s32 count;
    s32 i;
    u8 wanted;
    u8 unused[8];

    if (self->partIds == NULL) {
        return -1;
    }
    ids = self->partIds;
    /* Keeps ids's register copy above the self->partCount load; without it
     * GCC moves the copy into the delay slot of the count <= 0 branch. */
    __asm__("");
    count = self->partCount;
    if (count <= 0) {
        return -1;
    }
    i = 0;
    wanted = (u8)id;
    do {
        if (*ids == wanted) {
            return i;
        }
        i++;
        ids++;
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
    s32 tmdId[4];
    s32 count;
    s32 i;
    Actor **p;

    count = self->modelData->methods->scanPackets(self->modelData, 0, (s32)tmdId);
    self->parts = BMemPMgrAlloc(count * sizeof(Actor *));
    if (self->parts == NULL) {
        goto alloc_fail;
    }
    self->partIds = BMemPMgrAlloc(count);
    if (self->partIds == NULL) {
        goto alloc_fail;
    }
    self->modelData->methods->scanPackets(self->modelData, (s32)self->partIds, (s32)tmdId);

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
    self->mainPart = self->parts[tmdId[0]];
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
            self->todFramePtr =
                ((TodHeader *)TODSET_TOD(self->modelData->todSet, self->todIndex)->buffer)->frames;
        }
    }
    self->coord2->flg = 0;
}

void TodActor__SelectTickCallback(TodActor *self, s32 which) {
    switch ((u8)which) {
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
    self->methods->moveLocalZ(self, TODACTOR_STEP_Z, 0);
    if (self->mainPartNotifies == 1 && self->mainPart != NULL) {
        self->mainPart->methods->notifyWithHull(self->mainPart, 6);
    }
}

void TodActor__TickCallbackB(void) {}

void TodActor__TickCallbackC(void) {}

void TodActor__PlayTone(TodActor *self, s32 index) {
    VabStreamObj *sound;

    sound = self->sound;
    if (sound != NULL) {
        sound->methods->playTone(sound, index, TODACTOR_TONE_VOLUME, TODACTOR_TONE_VOLUME);
    }
}

void TodActor__SetTod(TodActor *self, s32 index) {
    self->todIndex = index;
    self->todFrameCount = ((TodHeader *)TODSET_TOD(self->modelData->todSet, index)->buffer)->frameCount;
    self->todFramePtr =
        ((TodHeader *)TODSET_TOD(self->modelData->todSet, self->todIndex)->buffer)->frames;
    self->todFrame = 0;
    self->methods->applyTodFrame(self, self->todFramePtr, 0);
}

s32 TodActor__PlayTod(TodActor *self) {
    return self->todPlaying = 1;
}

void TodActor__StopTod(TodActor *self) {
    self->todPlaying = 0;
}

void *TodActor__ApplyTodFrame(TodActor *self, void *frame, void *extra) {
    s32 count;
    u32 i;

    count = ((TodFrame *)frame)->packetCount;
    frame = ((TodFrame *)frame)->packets;
    for (i = 0; i < count;) {
        i++;
        frame = self->methods->applyTodPacket(self, frame, extra);
    }
    return frame;
}

void *TodActor__ApplyTodPacket(TodActor *self, void *packet, void *extra) {
    TodPacketHeader head;
    s32 *data;
    s32 partIndex;
    Actor *part;
    GsCOORDINATE2 *coord;
    GsCOORD2PARAM *param;
    s32 i;

    data = self->modelData->methods->decodePacketWord(self->modelData, (s32)packet,
                                                      (s32)&head.objectId, (s32)&head.type,
                                                      (s32)&head.flag, (s32)&head.length);
    partIndex = TodActor__FindPartIndex(self, head.objectId);
    if (partIndex < 0) {
        goto end;
    }
    part = self->parts[partIndex];
    coord = part->coord2;
    coord->flg = 0;
    param = coord->param;

    switch (head.type) {
        case TOD_PACKET_ATTRIBUTE:
            part->attribute = (part->attribute & data[0]) | data[1];
            break;
        case TOD_PACKET_COORDINATE: {
            if (head.flag & TOD_COORD_DIFFERENTIAL) {
                if (head.flag & TOD_COORD_ROTATE) {
                    s16 *rot = &param->rotate.vx;

                    for (i = 0; i < 3; i++, rot++) {
                        s16 angle;

                        angle = *rot + data[i] / TOD_ROTATE_PER_ANGLE;
                        *rot = angle;
                        *rot = angle % ONE;
                    }
                    data += 3;
                }
                if (head.flag & TOD_COORD_SCALE) {
                    long *scale = &param->scale.vx;

                    for (i = 0; i < 3; i++, scale++) {
                        *scale = (((s16 *)data)[i] * *scale) / ONE;
                    }
                    data += 2;
                }
                if (!(head.flag & TOD_COORD_TRANSLATE)) {
                    goto end;
                }
                {
                    long *trans = &param->trans.vx;

                    for (i = 0; i < 3; i++, trans++) {
                        *trans += data[i];
                    }
                }
            } else {
                if (head.flag & TOD_COORD_ROTATE) {
                    s16 *rot = &param->rotate.vx;

                    for (i = 0; i < 3; i++, rot++) {
                        *rot = data[i] / TOD_ROTATE_PER_ANGLE;
                    }
                    data += 3;
                }
                if (head.flag & TOD_COORD_SCALE) {
                    long *scale = &param->scale.vx;

                    for (i = 0; i < 3; i++, scale++) {
                        *scale = ((s16 *)data)[i];
                    }
                    data += 2;
                }
                if (!(head.flag & TOD_COORD_TRANSLATE)) {
                    goto end;
                }
                {
                    long *trans = &param->trans.vx;

                    for (i = 0; i < 3; i++, trans++) {
                        *trans = data[i];
                    }
                }
            }
            {
                GsCOORDINATE2 *partCoord;
                s32 x, y, z;

                partCoord = part->coord2;
                x = param->trans.vx;
                y = param->trans.vy;
                z = param->trans.vz;
                partCoord->coord.t[0] = x;
                partCoord->coord.t[1] = y;
                partCoord->coord.t[2] = z;
                /* Keeps the partCoord->coord.t[2] store ahead of the break's jump, leaving retail's
             * nop in its delay slot; without it GCC moves the store into the slot. */
                __asm__("");
            }
            break;
        }
        case TOD_PACKET_MODEL_ID: {
            u16 modelId;

            modelId = *(u16 *)data;
            if (modelId != 0 && part->model == NULL) {
                struct TmdModel *model;

                model = self->modelData->linkResource->methods->getModel(self->modelData->linkResource,
                                                                         modelId - 1);
                SceneNode__LinkModel((SceneNode *)part, model);
            }
            break;
        }
        case TOD_PACKET_PARENT: {
            s32 parentId;

            parentId = data[0];
            if (parentId == 0 || parentId == TOD_PARENT_ROOT) {
                part->methods->attachToParent(part, (SceneNode *)self, NULL);
            } else {
                s32 parentIndex;

                parentIndex = TodActor__FindPartIndex(self, *(u8 *)data);
                part->methods->attachToParent(part, (SceneNode *)self->parts[parentIndex], NULL);
            }
            break;
        }
    }

end:
    return (u32 *)packet + head.length;
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
