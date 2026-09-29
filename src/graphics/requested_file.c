/*
 * RequestedFile's methods (include/requested_file.h: a FileResource that
 * asks the active data-source driver for one named file when it is built),
 * in ROM order: the allocator, ctor, finalize and onRequestDone
 * (MarkLoaded), ending with its getter GetRequestedFileMethods; its method
 * table closes the file. A (void *) entry in it is a method inherited from
 * a parent class and declared on the parent's type.
 */
#include "common.h"
#include "requested_file.h"
#include "bmem_pmgr.h"
#include <strings.h>
#include "data_source.h"

/* Allocate and construct a RequestedFile, requesting the file `name`. */
RequestedFile *New_RequestedFile(char *name) {
    RequestedFile *obj = BMemPMgrAlloc(sizeof(RequestedFile));

    if (obj != NULL) {
        GetRequestedFileMethods()->ctor(obj, name);
        return obj;
    }
    return NULL;
}

/* gRequestedFileMethods slot +0x008 (ctor): the active driver's ctor, install
 * the table, clear `loaded`, and pass a stack copy of the name to
 * requestLoadFile (+0x06C). */
void RequestedFile__RequestedFile(RequestedFile *self, char *name) {
    char nameCopy[REQUESTEDFILE_NAME_SIZE];

    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetRequestedFileMethods();
    self->loaded = 0;
    if (name != NULL) {
        strcpy(nameCopy, name);
        self->methods->requestLoadFile(self, nameCopy);
    }
}

/* gRequestedFileMethods slot +0x00C (finalize): clear `loaded`, then the active
 * driver's finalize. */
void RequestedFile__Finalize(RequestedFile *self) {
    self->loaded = 0;
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* gRequestedFileMethods slot +0x064 (onRequestDone): the driver reports the requested
 * file loaded. */
void RequestedFile__MarkLoaded(RequestedFile *self) {
    self->loaded = 1;
}

/* Returns the gRequestedFileMethods method table. */
RequestedFileMethods *GetRequestedFileMethods(void) {
    return &gRequestedFileMethods;
}

/* RequestedFile (include/requested_file.h): FileResource's slots up to
 * +0x074 with its ctor, finalize and onRequestDone; the file-I/O slots are
 * NULL until SetActiveDataSource binds the active driver's. */
RequestedFileMethods gRequestedFileMethods = {
    /* +0x000 header */ REQUESTEDFILE_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ RequestedFile__RequestedFile,
    /* +0x00C finalize */ RequestedFile__Finalize,
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
    /* +0x064 onRequestDone */ RequestedFile__MarkLoaded,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
};
