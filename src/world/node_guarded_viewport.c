/*
 * NodeGuardedViewport's methods (include/node_guarded_viewport.h: the
 * Viewport DayTask builds, which skips its frame while no view node is
 * attached), in ROM order: the allocator (allocate, then the ctor through
 * the class's table), the ctor, which chains to Viewport's, the empty
 * initDefaults, update and four empty slots, ending with its getter
 * GetNodeGuardedViewportMethods. Its method table closes the file. It
 * belongs with its maker's code: DayTask's ctor (day_task.c) makes it.
 * GridCell and TitleMenu, which follow it in ROM, are in grid_cell.c and
 * title_menu.c.
 */
#include "common.h"
#include "node_guarded_viewport.h"
#include "bmem_pmgr.h"

NodeGuardedViewport *New_NodeGuardedViewport(void) {
    NodeGuardedViewport *self;

    self = BMemPMgrAlloc(sizeof(NodeGuardedViewport));
    if (self != NULL) {
        GetNodeGuardedViewportMethods()->ctor(self);
        return self;
    }
    return NULL;
}

void NodeGuardedViewport__NodeGuardedViewport(NodeGuardedViewport *self) {
    GetViewportMethods()->ctor((Viewport *)self);
    self->methods = GetNodeGuardedViewportMethods();
    self->methods->initDefaults(self);
}

void NodeGuardedViewport__InitDefaults(void) {}

void NodeGuardedViewport__Update(NodeGuardedViewport *self) {
    if (self->viewNode != NULL && self->otReady != 0) {
        GetViewportMethods()->update((Viewport *)self);
    }
}

void NodeGuardedViewport__NoOpSlotB8(void) {}

void NodeGuardedViewport__NoOpSlotBC(void) {}

void NodeGuardedViewport__NoOpSlotC0(void) {}

void NodeGuardedViewport__NoOpSlotC4(void) {}

NodeGuardedViewportMethods *GetNodeGuardedViewportMethods(void) {
    return &gNodeGuardedViewportMethods;
}

/* The method table. A (void *) entry is a function whose declared type
 * differs from its slot's: a method inherited from a parent class and
 * declared on the parent's type, or an empty method declared (void). */

/* NodeGuardedViewport (include/node_guarded_viewport.h): Viewport's table
 * with its own initDefaults and update, then four empty slots. */
NodeGuardedViewportMethods gNodeGuardedViewportMethods = {
    /* +0x000 header */ NODEGUARDEDVIEWPORT_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ NodeGuardedViewport__NodeGuardedViewport,
    /* +0x00C finalize */ (void *)Viewport__Finalize,
    /* +0x010 addChild */ (void *)Viewport__AddChild,
    /* +0x014 removeChild */ (void *)Viewport__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)Viewport__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)Viewport__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 initDefaults */ (void *)NodeGuardedViewport__InitDefaults,
    /* +0x044 setScreenSize */ (void *)Viewport__SetScreenSize,
    /* +0x048 setOtLength */ (void *)Viewport__SetOtLength,
    /* +0x04C setMaxPackets */ (void *)Viewport__SetMaxPackets,
    /* +0x050 setPacketSize */ (void *)Viewport__SetPacketSize,
    /* +0x054 setProjection */ (void *)Viewport__SetProjection,
    /* +0x058 slot58 */ Viewport__NoOpSlot58,
    /* +0x05C slot5C */ Viewport__NoOpSlot5C,
    /* +0x060 setLightMode */ (void *)Viewport__SetLightMode,
    /* +0x064 setClearColor */ (void *)Viewport__SetClearColor,
    /* +0x068 setFarColor */ (void *)Viewport__SetFarColor,
    /* +0x06C setFogNear */ (void *)Viewport__SetFogNear,
    /* +0x070 attachViewChild */ (void *)Viewport__AttachViewChild,
    /* +0x074 detachViewChild */ (void *)Viewport__DetachViewChild,
    /* +0x078 setViewPoint */ (void *)Viewport__SetViewPoint,
    /* +0x07C setViewRef */ (void *)Viewport__SetViewRef,
    /* +0x080 setTwist */ (void *)Viewport__SetTwist,
    /* +0x084 slot84 */ Viewport__NoOpSlot84,
    /* +0x088 slot88 */ Viewport__NoOpSlot88,
    /* +0x08C initOt */ (void *)Viewport__InitOt,
    /* +0x090 deinitOt */ (void *)Viewport__DeinitOt,
    /* +0x094 onFrameClockEvent */ (void *)Viewport__OnFrameClockEvent,
    /* +0x098 onDrawSystemEvent */ (void *)Viewport__OnDrawSystemEvent,
    /* +0x09C update */ NodeGuardedViewport__Update,
    /* +0x0A0 drawNode */ (void *)Viewport__DrawNode,
    /* +0x0A4 flip */ (void *)Viewport__Flip,
    /* +0x0A8 setFadeBox */ (void *)Viewport__SetFadeBox,
    /* +0x0AC getFadeBox */ (void *)Viewport__GetFadeBox,
    /* +0x0B0 setExtraSwap */ (void *)Viewport__SetExtraSwap,
    /* +0x0B4 setDrawEnabled */ (void *)Viewport__SetDrawEnabled,
    /* +0x0B8 slotB8 */ NodeGuardedViewport__NoOpSlotB8,
    /* +0x0BC slotBC */ NodeGuardedViewport__NoOpSlotBC,
    /* +0x0C0 slotC0 */ NodeGuardedViewport__NoOpSlotC0,
    /* +0x0C4 slotC4 */ NodeGuardedViewport__NoOpSlotC4,
};
