/*
 * TriggerWorld's methods (include/trigger_world.h: a ModelData over a
 * counted set of model files, one ModelData built over each), in ROM
 * order: the allocator, ctor and finalize, onRequestDone (Load),
 * BuildResources, ReleaseResources and GetModelData, ending with its
 * getter GetTriggerWorldMethods; its method table closes the file. A
 * (void *) entry in it is a method whose declared parameters differ from
 * the slot's, usually one inherited from a parent class and declared on
 * the parent's type.
 */
#include "common.h"
#include "trigger_world.h"
#include "bmem_pmgr.h"

/* Allocate and construct a TriggerWorld; NULL, the object freed, when the
 * ctor fails. */
TriggerWorld *New_TriggerWorld(ResourceSource *src) {
    TriggerWorld *obj = BMemPMgrAlloc(sizeof(TriggerWorld));

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetTriggerWorldMethods())->ctor(obj, src)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}

/* ctor (+0x008): ModelData's, not owning; with an adopted buffer, build the
 * ModelData array (NULL when that fails). */
void *TriggerWorld__TriggerWorld(TriggerWorld *self, ResourceSource *src) {
    ((UnprototypedCtorTable *)GetModelDataMethods())->ctor(self, src, 0);
    self->methods = GetTriggerWorldMethods();
    if (src->buffer != NULL) {
        if (((s32 (*)())self->methods->onRequestDone)(self)) {
            return NULL;
        }
    }
    return self;
}

/* finalize (+0x00C): releaseResources first. */
void TriggerWorld__Finalize(TriggerWorld *self) {
    self->methods->releaseResources(self);
    GetModelDataMethods()->finalize((ModelData *)self);
}

/* onRequestDone (+0x064): BuildResources. */
void TriggerWorld__Load(TriggerWorld *self) {
    ((s32 (*)())self->methods->processBuffer)(self);
}

/* +0x078: build a ModelData over each of the buffer's sub-blocks, in place
 * of its offset; 1, with those built released, when one fails. */
s32 TriggerWorld__BuildResources(TriggerWorld *self) {
    ResourceRequest req;
    SubBlockTable *buf;
    s32 *p;
    s32 i;
    s32 n;

    ResourceRequest__Set(&req, 0, 0, 1);
    buf = self->buffer;
    i = 0;
    n = buf->count;
    p = buf->entries;
    self->modelDataCount = 0;
    for (; i < n; i++) {
        req.src.buffer = (u8 *)self->buffer + ((SubBlockTable *)self->buffer)->entries[i];
        *p = (s32)New_ModelData(&req.src);
        if (*p == 0) {
            goto fail; /* MATCHING: releasing and returning here lays the cleanup out inside the loop */
        }
        self->modelDataCount++;
        p++;
    }
    return 0;
fail:
    self->methods->releaseResources(self);
    return 1;
}

/* releaseResources (+0x07C): release the ModelData built so far. */
void TriggerWorld__ReleaseResources(TriggerWorld *self) {
    ReleaseBasicClassArray((BasicClass **)((SubBlockTable *)self->buffer)->entries, self->modelDataCount);
    self->modelDataCount = 0;
}

/* +0x088: ModelData `index`, NULL when out of range. */
ModelData *TriggerWorld__GetModelData(TriggerWorld *self, u32 index) {
    SubBlockTable *buf = self->buffer;

    if (index < buf->count) {
        return (ModelData *)buf->entries[index];
    }
    return NULL;
}

TriggerWorldMethods *GetTriggerWorldMethods(void) {
    return &gTriggerWorldMethods;
}

/* TriggerWorld: ModelData's table over a counted set of model files, then
 * getModelData. */
TriggerWorldMethods gTriggerWorldMethods = {
    /* +0x000 header */ TRIGGERWORLD_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ (void *)TriggerWorld__TriggerWorld,
    /* +0x00C finalize */ TriggerWorld__Finalize,
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
    /* +0x064 onRequestDone */ TriggerWorld__Load,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ TriggerWorld__BuildResources,
    /* +0x07C releaseResources */ TriggerWorld__ReleaseResources,
    /* +0x080 scanPackets */ (void *)ModelData__ForwardScanPackets,
    /* +0x084 decodePacketWord */ (void *)ModelData__ForwardDecodePacketWord,
    /* +0x088 getModelData */ TriggerWorld__GetModelData,
};
