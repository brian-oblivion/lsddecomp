/*
 * NullDriver's methods (include/null_driver.h: the data-source driver whose
 * every file-I/O method is empty), in ROM order: its empty slots up to
 * CancelRequests, its getter GetNullDriverMethods, then its mode
 * accessors. NullDriver is the data source data_source.c selects when it
 * is not reading the CD. GetNullDriverMode, SetNullDriverMode and
 * GetNullDriverUseVSyncCallback answer the queries data_source.c's
 * GetActiveDataSource* functions otherwise forward to CdDriver: they keep
 * the two mode words and report no VSync callback. Its method table closes
 * the file; a (void *) entry in it is a function whose declared type
 * differs from its slot's, a method inherited from a parent class and
 * declared on the parent's type, or an empty method declared (void).
 */
#include "common.h"
#include "null_driver.h"

void NullDriver__NullDriver(void) {}

void NullDriver__Destroy(void) {}

void NullDriver__NoOpSlot40(void) {
    /* MATCHING: retail reserves a 64-byte frame it never touches. */
    char unused[64];
}

void NullDriver__Open(void) {
    /* MATCHING: the same unused 64-byte frame. */
    char unused[64];
}

void NullDriver__Close(void) {}

void NullDriver__Seek(void) {}

void NullDriver__NoOpSlot50(void) {}

/* SetNullDriverMode's two words, read back by GetNullDriverMode: 0 and 0
 * until it is called. */
static s32 sNullDriverMode SDATA = 0;
static s32 sNullDriverModeArg SDATA = 0;

s32 NullDriver__Read(void) {
    return 0;
}

void NullDriver__LoadFile(void) {}

void NullDriver__RunRequestQueue(void) {}

void NullDriver__RequestLoadFile(void) {}

void NullDriver__StopService(void) {}

void NullDriver__CancelRequests(void) {}

NullDriverMethods *GetNullDriverMethods(void) {
    return &gNullDriverMethods;
}

s32 GetNullDriverMode(s32 *outMode2) {
    if (outMode2 != NULL) {
        *outMode2 = sNullDriverModeArg;
    }
    return sNullDriverMode;
}

s32 SetNullDriverMode(s32 async, s32 mode2) {
    sNullDriverMode = async;
    sNullDriverModeArg = mode2;
    return 1;
}

s32 GetNullDriverUseVSyncCallback(void) {
    return 0;
}

/* NullDriver (include/null_driver.h): FileResource's slots up to +0x074,
 * the eleven data-source slots all empty. */
NullDriverMethods gNullDriverMethods = {
    /* +0x000 header */ NULLDRIVER_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ (void *)NullDriver__NullDriver,
    /* +0x00C finalize */ (void *)NullDriver__Destroy,
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
    /* +0x040 slot40 */ NullDriver__NoOpSlot40,
    /* +0x044 open */ (void *)NullDriver__Open,
    /* +0x048 close */ (void *)NullDriver__Close,
    /* +0x04C seek */ (void *)NullDriver__Seek,
    /* +0x050 slot50 */ NullDriver__NoOpSlot50,
    /* +0x054 read */ (void *)NullDriver__Read,
    /* +0x058 loadFile */ (void *)NullDriver__LoadFile,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ (void *)FileResource__OnRequestDone,
    /* +0x068 runRequestQueue */ NullDriver__RunRequestQueue,
    /* +0x06C requestLoadFile */ (void *)NullDriver__RequestLoadFile,
    /* +0x070 stopService */ (void *)NullDriver__StopService,
    /* +0x074 cancelRequests */ (void *)NullDriver__CancelRequests,
};
