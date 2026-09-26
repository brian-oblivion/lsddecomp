/*
 * class_3bb8c_c -- three small sibling classes, each built by its own
 * New_X/ctor pair (allocate, chain a base ctor, install the class's own
 * vtable): NodeGuardedViewport, GridCell and TitleMenu. All three follow
 * the same class-framework shape documented in
 * docs/research/class-framework.md and already used elsewhere in this
 * codebase (e.g. class_3ac78.c's Class866E8).
 *
 * NodeGuardedViewport (include/NodeGuardedViewport.h) is a Viewport whose
 * update skips the frame while no view node is attached; its seven table
 * methods, New_ and the getter are all here. GridCell (include/GridCell.h,
 * a SceneNode) is one cell of Class866E8's grid, carrying the model placed
 * there; its five table methods, New_ and the getter are all here too.
 * TitleMenu
 * (include/TitleMenu.h, a TaskCore) is the largest of the three -- its own
 * vtable (gTitleMenuMethods, 78 slots) is occupied mostly by the sibling
 * unit class_3bb8c_d.c; this unit contributes only the allocator and ctor.
 *
 * Two free functions round out the unit: CheckSaveScoreFlag, called
 * directly (not through any vtable) from
 * TitleMenu__CommitNameEntry, computes a 0/1 flag from its save block (TitleMenu::saveBlock); and
 * FormatNumberIntoBuffer, called from TitleMenu's own ctor, formats a
 * number into a shared buffer whose broader role (nearby rodata strings
 * hint at a memory-card save label) is not established from this unit
 * alone.
 *
 * All 20 definitions here are matched, 0 INCLUDE_ASM.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "DreamSys.h"
#include "SceneNode.h"
#include "Viewport.h"
#include "NodeGuardedViewport.h"
#include "GridCell.h"
#include "TitleMenu.h"
#include "VabStreamObj.h"

NodeGuardedViewport *New_NodeGuardedViewport(void) {
    NodeGuardedViewport *self;

    self = BMemPMgrAlloc(0xDC);
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

    self = BMemPMgrAlloc(0x3C);
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

/* Only the low byte of the sender's class id is read: 0x34 is an Actor
 * (Actor__DispatchLinkCommand makes the same test the other way round). */
void GridCell__DispatchLinkCommand(GridCell *self, BasicClass *sender, s32 event) {
    if (*(u8 *)sender->methods == 0x34) {
        self->methods->onActorLinkCommand(self, sender, event);
    }
}

/* tryAttachNearby keeps SceneNode's one-parameter slot type; this caller
 * passes the sender and event too, as Actor__OnActorLinkCommand does. */
void GridCell__OnActorLinkCommand(GridCell *self, void *sender, s32 event) {
    GetSceneNodeMethods()->dispatchLinkCommand((SceneNode *)self, sender, event);
    if (event >= 9) {
        return;
    }
    do {
        if (event < 5) {
            return;
        }
    } while (0);
    ((void (*)(GridCell *, void *, s32))self->methods->tryAttachNearby)(self, sender, event);
}

void *GridCell__ReturnSelf(GridCell *self) {
    return self;
}

GridCellMethods *GetGridCellMethods(void) {
    return &gGridCellMethods;
}

TitleMenu *New_TitleMenu(struct DreamSys *dreamSys) {
    TitleMenu *self;

    self = BMemPMgrAlloc(0xC4);
    if (self != NULL) {
        GetTitleMenuMethods()->ctor(self, dreamSys);
        return self;
    }
    return NULL;
}

void TitleMenu__TitleMenu(TitleMenu *self, struct DreamSys *dreamSys) {
    DreamSys *dream;
    VabStreamObj *sound;

    Get_vtable_TaskCore()->ctor((TaskCore *)self, &D_80086D44, (char *)D_800114DC, 0);
    self->methods = GetTitleMenuMethods();
    sound = (VabStreamObj *)self->sound;
    sound->methods->setPitchOffset(sound, -1);
    self->dreamSys = dreamSys;
    self->saveCtrl = 0;
    dream = dreamSys;
    self->saveBlock = dream->methods->getSaveBlock(dream, &self->saveBlockSize);
    FormatNumberIntoBuffer(dream->methods->getCurrentDayAndYear(dream, 0));
    self->methods->setTarget(self, &D_80086D44);
    ((TitleMenuResetCallFn)self->methods->resetCounters)(self, dreamSys);
}

void CheckSaveScoreFlag(Ctx678_3bb8c_c *ctx, Result678_3bb8c_c *out) {
    SaveBlock678_3bb8c_c *target = ctx->target;
    s32 flag = 1;

    if (target->unkC > 9999999) {
        flag = (target->unk2F4 == 0);
    }
    out->block[1] = flag;
}

/* FormatFullWidthNumber is GAME code (matched round 38, src/code_2cc8c_f.c -- its
 * own C definition, not a Sony object), which formats a1 as a zero-padded
 * `width`-digit decimal string into `self`, the output buffer (the
 * definition's `u8 *dst`; it was typed as a TextRow view until round 88).
 * This unit's own local view keeps it `void *`. */
extern void FormatFullWidthNumber(void *self, s32 a1, s32 width, s32 unpadded);

/* The 6-byte value formatted into D_8008AA24's buffer by FormatFullWidthNumber
 * above, copied whole into D_8008AA18's buffer at +0x12 as ONE struct
 * assignment. All-`s8` fields (alignment 1, not 2 or 4) is what makes
 * retail's block-move split this way: the leading 4 bytes go via the
 * unaligned lwl/lwr word copy regardless of declared alignment (same
 * idiom as Vec2s16, FlagLargePolyForDivide), but the trailing 2 bytes can no
 * longer be proven 2-byte aligned, so there is no safe halfword move for
 * them and the compiler falls back to two individual signed-byte
 * loads/stores. See docs/match-reports/FormatNumberIntoBuffer.md. */
typedef struct {
    s8 a, b, c, d, e, f;
} Buf6_3bb8c_c;

void FormatNumberIntoBuffer(s32 arg0) {
    FormatFullWidthNumber(D_8008AA24, arg0, 3, 0);
    *(Buf6_3bb8c_c *)((s8 *)D_8008AA18 + 0x12) = *(Buf6_3bb8c_c *)D_8008AA24;
}
