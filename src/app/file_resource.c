/*
 * FileResource's own methods (include/file_resource.h: the base of every
 * class the game loads from a file, which owns one file buffer), in ROM
 * order: release, the ctor and finalize, LoadFile and FreeBuffer, the
 * empty NoOp slot and OnRequestDone, ending with its getter
 * GetFileResourceMethods; its method table closes the file. The
 * active-data-source layer on top of it follows in data_source.c. A
 * (void *) entry in the table is a method inherited from BasicClass and
 * declared on its type.
 */
#include "common.h"
#include <libgte.h>
#include "file_resource.h"
#include "bmem_pmgr.h"
#include <stdio.h>

void *FileResource__Release(FileResource *self) {
    self->freeGuard = 0;
    self->methods->finalize(self);
    GetBasicClassMethods()->finalize((BasicClass *)self);
    BMemPMgrFree(self);
    return NULL;
}

void FileResource__FileResource(FileResource *self) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetFileResourceMethods();
    self->isOpen = 0;
    self->buffer = NULL;
    self->bufferSize = 0;
    self->freeGuard = 0;
    self->pendingRequests = 0;
    self->flags = 0;
    self->inQueueDispatch = 0;
    self->loadState = 0;
}

void FileResource__Finalize(FileResource *self) {
    self->methods->close(self);
    self->methods->freeBuffer(self);
}

void FileResource__LoadFile(FileResource *self, char *name) {
    s32 savedIsOpen;
    s32 size;
    void *buffer;

    if (self->buffer != NULL) {
        return;
    }
    savedIsOpen = self->isOpen;
    self->isOpen = 0;
    self->methods->open(self, name, 1, 0);
    size = self->methods->seek(self, 0, SEEK_END);
    buffer = BMemPMgrAlloc(size);
    if (buffer != NULL) {
        self->methods->seek(self, 0, SEEK_SET);
        self->methods->read(self, buffer, size);
        self->methods->close(self);
        self->buffer = buffer;
        self->bufferSize = size;
        self->isOpen = savedIsOpen;
    } else {
        BMemPMgrFree(NULL);
        self->methods->close(self);
    }
}

void FileResource__FreeBuffer(FileResource *self) {
    if (self->buffer == NULL) {
        return;
    }
    if (self->bufferSize == 0) {
        return;
    }
    if (self->freeGuard != 0) {
        return;
    }
    BMemPMgrFree(self->buffer);
    self->buffer = NULL;
}

void NoOp(void) {}

void FileResource__OnRequestDone(FileResource *self) {
    self->flags |= 1;
}

FileResourceMethods *GetFileResourceMethods(void) {
    return &gFileResourceMethods;
}

/* FileResource (include/file_resource.h): BasicClass's slots, loadFile,
 * freeBuffer and onRequestDone. The data-source slots are NULL until
 * SetActiveDataSource binds the active driver's, and processBuffer is each
 * subclass's. */
FileResourceMethods gFileResourceMethods = {
    /* +0x000 header */ FILERESOURCE_CLASS_ID,
    /* +0x004 release */ FileResource__Release,
    /* +0x008 ctor */ FileResource__FileResource,
    /* +0x00C finalize */ FileResource__Finalize,
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
    /* +0x058 loadFile */ FileResource__LoadFile,
    /* +0x05C freeBuffer */ FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ FileResource__OnRequestDone,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ NULL,
};
