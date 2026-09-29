/*
 * ModelData's methods (include/model_data.h: a model file's TMD and TODs,
 * a LinkResource and a TodSet built over one MOM file), in ROM order: the
 * allocator, ctor and finalize, onRequestDone (Load), BuildResources and
 * ReleaseResources, and the two packet calls it forwards to its TodSet,
 * ending with its getter GetModelDataMethods; its method table closes the
 * file. A (void *) entry in it is a method whose declared parameters
 * differ from the slot's, usually one inherited from a parent class and
 * declared on the parent's type.
 */
#include "common.h"
#include "model_data.h"
#include "link_resource.h"
#include "bmem_pmgr.h"
#include "data_source.h"

/* Allocate and construct a ModelData that owns its LinkResource and TodSet;
 * NULL, the object freed, when the ctor fails. */
ModelData *New_ModelData(ResourceSource *src) {
    void *obj = BMemPMgrAlloc(sizeof(ModelData));

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetModelDataMethods())->ctor(obj, src, 1)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}

/* ctor (+0x008): adopt the descriptor's buffer and load it (NULL when that
 * fails), or request its file. */
void *ModelData__ModelData(ModelData *self, ResourceSource *src, s32 owns) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetModelDataMethods();
    self->ownsResources = owns;
    if (src->buffer != NULL) {
        self->buffer = src->buffer;
        self->bufferSize = 0;
        if (((s32 (*)())self->methods->onRequestDone)(self)) {
            goto fail; /* MATCHING: a return NULL here lays the failure out before the success return */
        }
    } else {
        self->methods->requestLoadFile(self, src->name);
    }
    return self;
fail:
    return NULL;
}

/* finalize (+0x00C): releaseResources first. */
void ModelData__Finalize(ModelData *self) {
    self->methods->releaseResources(self);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* onRequestDone (+0x064): the driver's, then BuildResources. */
void ModelData__Load(ModelData *self) {
    GetActiveDataSourceMethods()->onRequestDone((FileResource *)self);
    ((s32 (*)())self->methods->processBuffer)(self);
}

/** @brief A ModelData's buffer (a .MOM file: InitDreamAux requests
 * ETC\\SYMSPY.MOM through New_ModelData): the LinkResource's TMD at
 * `tmdOffset`, the TodSet's data from +0x0C. */
typedef struct ModelDataHeader {
    /* +0x00 */ u8 pad0[8];
    /* +0x08 */ s32 tmdOffset; /**< the TMD's offset from the buffer's start */
    /* +0x0C */ u8 tods[1];    /**< the TodSet's data */
} ModelDataHeader;

/* +0x078: when it owns them, build the LinkResource and the TodSet over
 * the buffer; 1, with both released, when either fails. */
s32 ModelData__BuildResources(ModelData *self) {
    ResourceRequest req;

    if (self->ownsResources != 0) {
        ResourceRequest__Set(&req, (u8 *)self->buffer + ((ModelDataHeader *)self->buffer)->tmdOffset,
                             0, 1);
        self->linkResource = New_LinkResource(&req.src);
        if (self->linkResource != NULL) {
            req.src.buffer = ((ModelDataHeader *)self->buffer)->tods;
            self->todSet = New_TodSet(&req.src);
            if (self->todSet != NULL) {
                return 0;
            }
            self->todSet = NULL;
        }
        self->methods->releaseResources(self);
        return 1;
    }
    return 0;
}

/* releaseResources (+0x07C): release the TodSet and the LinkResource when
 * it owns them. */
void ModelData__ReleaseResources(ModelData *self) {
    if (self->ownsResources != 0) {
        if (self->todSet != NULL) {
            self->todSet->methods->release(self->todSet);
        }
        if (self->linkResource != NULL) {
            self->linkResource->methods->release(self->linkResource);
        }
    }
}

/* scanPackets (+0x080): the TodSet's +0x078 (TodSet__ScanPackets). */
u8 ModelData__ForwardScanPackets(ModelData *self, u8 *out, u32 *tmdId) {
    return ((s32 (*)())self->todSet->methods->processBuffer)(self->todSet, out, tmdId);
}

/* decodePacketWord (+0x084): the TodSet's. */
void *ModelData__ForwardDecodePacketWord(ModelData *self, u32 *packet, u8 *objId, u8 *type,
                                         u8 *flag, u8 *len) {
    return self->todSet->methods->decodePacketWord(self->todSet, packet, objId, type, flag, len);
}

ModelDataMethods *GetModelDataMethods(void) {
    return &gModelDataMethods;
}

/* ModelData: load, build and release the model's TMD and TODs, and the
 * packet scans forwarded to its Tods. */
ModelDataMethods gModelDataMethods = {
    /* +0x000 header */ MODELDATA_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ (void *)ModelData__ModelData,
    /* +0x00C finalize */ ModelData__Finalize,
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
    /* +0x064 onRequestDone */ ModelData__Load,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ ModelData__BuildResources,
    /* +0x07C releaseResources */ ModelData__ReleaseResources,
    /* +0x080 scanPackets */ ModelData__ForwardScanPackets,
    /* +0x084 decodePacketWord */ ModelData__ForwardDecodePacketWord,
};
