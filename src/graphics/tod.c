/*
 * Tod's methods (include/tod.h: a FileResource over one TOD animation file),
 * in ROM order: the allocator, ctor and finalize, ScanPackets, then
 * ScanTodPackets and DecodeTodPacketWord, the packet walk and decode its
 * table and TodSet's share, ending with its getter GetTodMethods; its
 * method table closes the file. A (void *) entry in it is a method whose
 * declared parameters differ from the slot's, usually one inherited from a
 * parent class and declared on the parent's type.
 */
#include "common.h"
#include "tod.h"
#include "bmem_pmgr.h"
#include "data_source.h"

/* Allocate and construct a Tod. */
Tod *New_Tod(ResourceSource *src) {
    void *obj = BMemPMgrAlloc(sizeof(Tod));

    if (obj != NULL) {
        ((UnprototypedCtorTable *)GetTodMethods())->ctor(obj, src);
        return obj;
    }
    return NULL;
}

/* ctor (+0x008): adopt the descriptor's buffer, or request its file. */
void Tod__Tod(Tod *self, ResourceSource *src) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTodMethods();
    if (src->buffer != NULL) {
        self->buffer = src->buffer;
        self->bufferSize = 0;
        self->methods->onRequestDone(self);
    } else {
        self->methods->requestLoadFile(self, src->name);
    }
}

/* finalize (+0x00C): nothing of its own. */
void Tod__Finalize(Tod *self) {
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* +0x078: scanTodPackets over the TOD's first frame. */
u8 Tod__ScanPackets(Tod *self, u8 *out, u32 *tmdId) {
    return self->methods->scanTodPackets(self, out, tmdId, ((TodFile *)self->buffer)->frames);
}

/* scanTodPackets (+0x07C, both tables): walk the TOD frame at `data`.
 * Returns the number of object-create packets, whose object ids go to `out`
 * when there is one. With `out`, a model-id packet naming TMD `*tmdId`
 * sets `*tmdId` to the index in `out` of the object it belongs to; without,
 * `*tmdId` becomes the number of model-id packets. */
u8 ScanTodPackets(Tod *self, u8 *out, u32 *tmdId, u32 *data) {
    TodPacketHeader head;
    u32 packetCount;
    u32 i;
    s32 j;
    u8 created;
    s32 index;

    packetCount = ((TodFrame *)data)->packetCount;
    data = ((TodFrame *)data)->packets;
    i = 0;
    created = 0;
    index = 0;
    for (; i < packetCount; i++) {
        DecodeTodPacketWord(self, data, &head.objectId, &head.type, &head.flag, &head.length);
        if (head.type == TOD_PACKET_OBJECT_CONTROL && head.flag == TOD_OBJECT_CREATE) {
            created++;
            if (out != NULL) {
                *out++ = head.objectId;
            }
        } else if (head.type == TOD_PACKET_MODEL_ID) {
            if (out != NULL) {
                if (tmdId != NULL && ((TodPacket *)data)->tmdId == *tmdId) {
                    for (j = 0, out -= created; j < created; j++) {
                        if (*out++ == head.objectId) {
                            index = j;
                            break;
                        }
                    }
                }
            } else if (tmdId != NULL) {
                index++;
            }
        }
        data += head.length;
    }
    if (tmdId != NULL) {
        *tmdId = index;
    }
    return created;
}

/* decodePacketWord (+0x080, both tables): split a packet's header word;
 * returns the word after it. */
u32 *DecodeTodPacketWord(Tod *self, u32 *packet, u8 *objId, u8 *type, u8 *flag, u8 *len) {
    u32 word = *packet;

    *objId = word;
    *type = (word >> TOD_PACKET_TYPE_SHIFT) & TOD_PACKET_NIBBLE;
    *flag = (word >> TOD_PACKET_FLAG_SHIFT) & TOD_PACKET_NIBBLE;
    *len = word >> TOD_PACKET_LEN_SHIFT;
    return packet + 1;
}

TodMethods *GetTodMethods(void) {
    return &gTodMethods;
}

/* Tod: scan the animation's packets, with the free packet decoders. */
TodMethods gTodMethods = {
    /* +0x000 header */ TOD_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ Tod__Tod,
    /* +0x00C finalize */ Tod__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 slot40 */ NULL,
    /* +0x044 open */ NULL,
    /* +0x048 close */ NULL,
    /* +0x04C seek */ NULL,
    /* +0x050 slot50 */ NULL,
    /* +0x054 read */ NULL,
    /* +0x058 loadFile */ NULL,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ (void *)FileResource__OnRequestDone,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ Tod__ScanPackets,
    /* +0x07C scanTodPackets */ ScanTodPackets,
    /* +0x080 decodePacketWord */ DecodeTodPacketWord,
};
