/*
 * Two small classes the dream day's scene uses, in ROM order:
 *
 * - NodeGuardedViewport (include/node_guarded_viewport.h), the Viewport
 *   DayTask builds, which skips its frame while no view node is attached;
 * - GridCell (include/grid_cell.h), one cell of StageMap's grid;
 * - the two method tables.
 * Each class runs from its New_ (allocate, then the ctor through the
 * class's table) to its table getter; a ctor chains to its parent's, then
 * installs its own table. They belong with their makers' code: DayTask's
 * ctor (day_task.c) makes the NodeGuardedViewport, and StageMap
 * (stage_map.c) is GridCell's only maker. TitleMenu, which follows them in
 * ROM, is in title_menu.c.
 */
#include "common.h"
#include <libgte.h>
#include "dream_sys.h"
#include "node_guarded_viewport.h"
#include "grid_cell.h"
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

GridCell *New_GridCell(void) {
    GridCell *self;

    self = BMemPMgrAlloc(GRIDCELL_SIZE);
    if (self != NULL) {
        GetGridCellMethods()->ctor(self);
        return self;
    }
    return NULL;
}

void GridCell__GridCell(GridCell *self) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetGridCellMethods();
    self->unk34 = 0;
    self->flags36 = 0;
    self->nextInCell = NULL;
}

void GridCell__Reset(void) {}

/* Passes on only a link command from an Actor or a class below it (the low
 * byte of the sender's class id); Actor__DispatchLinkCommand makes the same
 * test for a GridCell sender. */
void GridCell__DispatchLinkCommand(GridCell *self, BasicClass *sender, s32 event) {
    if ((sender->methods->header & CLASS_ID_LEVEL2_MASK) == ACTOR_CLASS_ID) {
        self->methods->onActorLinkCommand(self, sender, event);
    }
}

/* SceneNode's handling, then tryAttachNearby for the Actor move events
 * (ACTOR_EVENT_UNSWEPT..MOVED_Y), the body of Actor__OnActorLinkCommand. tryAttachNearby keeps SceneNode's one-parameter
 * slot type; this caller passes the sender and event too. */
void GridCell__OnActorLinkCommand(GridCell *self, void *sender, s32 event) {
    GetSceneNodeMethods()->dispatchLinkCommand((SceneNode *)self, sender, event);
    if (event <= ACTOR_EVENT_MOVED_Y) {
        if (event >= ACTOR_EVENT_UNSWEPT) { /* MATCHING: nested, as && folds to one unsigned test */
            ((void (*)(GridCell *, void *, s32))self->methods->tryAttachNearby)(self, sender, event);
        }
    }
}

void *GridCell__ReturnSelf(GridCell *self) {
    return self;
}

GridCellMethods *GetGridCellMethods(void) {
    return &gGridCellMethods;
}

/* The file's method tables, in the order the image keeps them. A (void *)
 * entry is a function whose declared type differs from its slot's: a
 * method inherited from a parent class and declared on the parent's type,
 * or an empty method declared (void). */

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

/* GridCell (include/grid_cell.h): SceneNode's table with reset and
 * dispatchLinkCommand, then onActorLinkCommand and returnSelf. */
GridCellMethods gGridCellMethods = {
    /* +0x000 header */ GRIDCELL_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)GridCell__GridCell,
    /* +0x00C finalize */ (void *)SceneNode__Finalize,
    /* +0x010 addChild */ (void *)SceneNode__AddChild,
    /* +0x014 removeChild */ (void *)SceneNode__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)SceneNode__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)SceneNode__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 reset */ (void *)GridCell__Reset,
    /* +0x044 updateRotation */ (void *)SceneNode__UpdateRotation,
    /* +0x048 updateScale */ (void *)SceneNode__UpdateScale,
    /* +0x04C attachToParent */ (void *)SceneNode__AttachToParent,
    /* +0x050 detachFromParent */ (void *)SceneNode__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ (void *)SceneNode__SetDisplay,
    /* +0x064 setSemiTransOn */ (void *)SceneNode__SetSemiTrans,
    /* +0x068 setSemiTransRate */ (void *)SceneNode__SetSemiTransRate,
    /* +0x06C setLighting */ (void *)SceneNode__SetLighting,
    /* +0x070 setLightMode */ (void *)SceneNode__SetLightMode,
    /* +0x074 setLightDim */ (void *)SceneNode__SetLightDim,
    /* +0x078 setUseZ */ (void *)SceneNode__SetUseZ,
    /* +0x07C setSubdivision */ (void *)SceneNode__SetSubdivision,
    /* +0x080 setBackClip */ (void *)SceneNode__SetBackClip,
    /* +0x084 getRotMatrix */ (void *)SceneNode__GetRotMatrix,
    /* +0x088 notifyWithHull */ (void *)SceneNode__NotifyWithHull,
    /* +0x08C getModelHull */ (void *)SceneNode__GetModelHull,
    /* +0x090 transformAndNotifyParents */ (void *)SceneNode__TransformAndNotifyParents,
    /* +0x094 onPadEvent */ (void *)SceneNode__OnPadEvent,
    /* +0x098 update */ (void *)SceneNode__Update,
    /* +0x09C dispatchLinkCommand */ (void *)GridCell__DispatchLinkCommand,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ (void *)SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ (void *)SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ SceneNode__NoOpSlotB0,
    /* +0x0B4 addToActorParents */ (void *)SceneNode__AddToActorParents,
    /* +0x0B8 onActorLinkCommand */ GridCell__OnActorLinkCommand,
    /* +0x0BC returnSelf */ GridCell__ReturnSelf,
};
