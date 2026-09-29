/*
 * TodSet's methods (include/tod_set.h: a Tod over a buffer of several TOD
 * animations, one Tod built over each), in ROM order: the allocator, ctor
 * and finalize, BuildTods (onRequestDone) and ScanPackets, ending with its
 * getter GetTodSetMethods; its method table closes the file. A (void *)
 * entry in it is a method whose declared parameters differ from the
 * slot's, usually one inherited from a parent class and declared on the
 * parent's type.
 */
#include "common.h"
#include "tod_set.h"
#include "bmem_pmgr.h"

/* Allocate and construct a TodSet; NULL, the object freed, when the ctor
 * fails. */
TodSet *New_TodSet(ResourceSource *src) {
    void *obj = BMemPMgrAlloc(sizeof(TodSet));

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetTodSetMethods())->ctor(obj, src)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}

/* ctor (+0x008): Tod's; with an adopted buffer, build the Tods (NULL when
 * that fails). */
void *TodSet__TodSet(TodSet *self, ResourceSource *src) {
    GetTodMethods()->ctor((Tod *)self, src);
    self->methods = GetTodSetMethods();
    if (src->buffer != NULL) {
        if (((s32 (*)())self->methods->onRequestDone)(self)) {
            return NULL;
        }
    }
    return self;
}

/* finalize (+0x00C): release the Tods. */
void TodSet__Finalize(TodSet *self) {
    SubBlockTable *buf = self->buffer;

    ReleaseBasicClassArray((BasicClass **)buf->entries, buf->count);
    GetTodMethods()->finalize((Tod *)self);
}

/* onRequestDone (+0x064): build a Tod over each of the buffer's sub-blocks, in
 * place of its offset; 1, with those built released, when one fails. */
s32 TodSet__BuildTods(TodSet *self) {
    ResourceRequest req;
    SubBlockTable *buf;
    Tod **p;
    s32 i;
    s32 n;

    ResourceRequest__Set(&req, 0, 0, 1);
    buf = self->buffer;
    i = 0;
    n = buf->count;
    p = (Tod **)buf->entries;
    for (; i < n; i++) {
        req.src.buffer = (u8 *)self->buffer + ((SubBlockTable *)self->buffer)->entries[i];
        *p = New_Tod(&req.src);
        if (*p == NULL) {
            while (i != 0) {
                i--;
                p--;
                (*p)->methods->release(*p);
            }
            return 1;
        }
        p++;
    }
    return 0;
}

/* +0x078: scanTodPackets over the first frame of the TOD that follows the
 * counted array. */
u8 TodSet__ScanPackets(TodSet *self, u8 *out, u32 *tmdId) {
    SubBlockTable *buf = self->buffer;

    return self->methods->scanTodPackets(self, out, tmdId, ((TodFile *)&buf->entries[buf->count])->frames);
}

TodSetMethods *GetTodSetMethods(void) {
    return &gTodSetMethods;
}

/* TodSet: Tod's table, building one Tod per animation. */
TodSetMethods gTodSetMethods = {
    /* +0x000 header */ TODSET_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ (void *)TodSet__TodSet,
    /* +0x00C finalize */ TodSet__Finalize,
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
    /* +0x064 onRequestDone */ (void *)TodSet__BuildTods,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ TodSet__ScanPackets,
    /* +0x07C scanTodPackets */ (void *)ScanTodPackets,
    /* +0x080 decodePacketWord */ (void *)DecodeTodPacketWord,
};
