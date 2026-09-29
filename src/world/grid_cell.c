/*
 * GridCell's methods (include/grid_cell.h: one cell of StageMap's grid, a
 * SceneNode that passes link commands on to the Actors it holds), in ROM
 * order: the allocator (allocate, then the ctor through the class's
 * table), ctor and the empty reset, DispatchLinkCommand,
 * OnActorLinkCommand and ReturnSelf, ending with its getter
 * GetGridCellMethods. Its method table closes the file; a (void *) entry
 * in it is a function whose declared type differs from its slot's, a
 * method inherited from a parent class and declared on the parent's type,
 * or an empty method declared (void). StageMap (src/world/stage_map.c) is
 * its only maker.
 */
#include "common.h"
#include "dream_sys.h"
#include "grid_cell.h"
#include "bmem_pmgr.h"

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
