/*
 * LinkResource's methods (include/link_resource.h: a FileResource over a
 * TMD file, one TmdModel built over each of its objects), in ROM order:
 * the allocator, ctor and finalize, BuildModels (onRequestDone), MapModel,
 * GetTmdObject, GetModel and an empty slot, ending with its getter
 * GetLinkResourceMethods; its method table closes the file. A (void *)
 * entry in it is a method whose declared parameters differ from the
 * slot's, usually one inherited from a parent class and declared on the
 * parent's type.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "tmd_model.h"
#include "link_resource.h"
#include "bmem_pmgr.h"
#include "data_source.h"

/* Allocate and construct a LinkResource; NULL, the object freed, when the
 * ctor fails. */
LinkResource *New_LinkResource(ResourceSource *src) {
    void *obj = BMemPMgrAlloc(sizeof(LinkResource));

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetLinkResourceMethods())->ctor(obj, src)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}

/* ctor (+0x008): adopt the descriptor's buffer and build the models (NULL
 * when that fails), or request its file. */
void *LinkResource__LinkResource(LinkResource *self, ResourceSource *src) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetLinkResourceMethods();
    if (src != NULL) {
        if (src->buffer != NULL) {
            self->buffer = src->buffer;
            self->bufferSize = 0;
            if (((LinkResourceBuildModelsFn)self->methods->onRequestDone)(self)) {
                goto fail; /* MATCHING: a return NULL here lays the failure out before the success return */
            }
        } else {
            self->methods->requestLoadFile(self, src->name);
        }
    }
    return self;
fail:
    return NULL;
}

/* finalize (+0x00C): release every model, then the array. */
void LinkResource__Finalize(LinkResource *self) {
    TmdModel **models = self->models;

    while (*models != NULL) {
        (*models)->methods->release(*models);
        models++;
    }
    BMemPMgrFree(self->models);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* onRequestDone (+0x064): map the TMD, then build a NULL-ended array of one
 * TmdModel per TMD object. 1 when an allocation fails, with everything
 * built so far released; else 0. */
s32 LinkResource__BuildModels(LinkResource *self) {
    TmdModel **models;
    u32 i;

    models = BMemPMgrAlloc((((TmdFile *)self->buffer)->nobj + 1) * sizeof(*models));
    if (models == NULL) {
        return 1;
    }
    self->models = models;
    ((LinkResourceMapModelFn)self->methods->processBuffer)(self);
    for (i = 0; i < ((TmdFile *)self->buffer)->nobj; i++) {
        *models = New_TmdModel(&((TmdFile *)self->buffer)->objects[i]);
        if (*models == NULL) {
            while (i != 0) {
                i--;
                models--;
                (*models)->methods->release(*models);
            }
            BMemPMgrFree(models);
            return 1;
        }
        models++;
    }
    *models = NULL;
    GetActiveDataSourceMethods()->onRequestDone((FileResource *)self);
    return 0;
}

/* +0x078: GsMapModelingData over the TMD in the buffer (from its flags
 * word, past the id). */
void LinkResource__MapModel(LinkResource *self) {
    GsMapModelingData((unsigned long *)&((TmdFile *)self->buffer)->flags);
}

/* +0x07C: the TMD's object `index`. */
TmdObject *LinkResource__GetTmdObject(LinkResource *self, s32 index) {
    return &((TmdFile *)self->buffer)->objects[index];
}

/* +0x080: model `index`. */
TmdModel *LinkResource__GetModel(LinkResource *self, s32 index) {
    return self->models[index];
}

void LinkResource__NoOp(void) {}

LinkResourceMethods *GetLinkResourceMethods(void) {
    return &gLinkResourceMethods;
}

/* LinkResource: build the TmdModels, map the TMD, then the getters. */
LinkResourceMethods gLinkResourceMethods = {
    /* +0x000 header */ LINKRESOURCE_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ (void *)LinkResource__LinkResource,
    /* +0x00C finalize */ LinkResource__Finalize,
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
    /* +0x064 onRequestDone */ (void *)LinkResource__BuildModels,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ LinkResource__MapModel,
    /* +0x07C getTmdObject */ LinkResource__GetTmdObject,
    /* +0x080 getModel */ LinkResource__GetModel,
    /* +0x084 slot84 */ LinkResource__NoOp,
};
