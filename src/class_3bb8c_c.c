/*
 * class_3bb8c_c -- three small sibling classes, each built by its own
 * New_X/ctor pair (allocate, chain a base ctor, install the class's own
 * vtable): NodeGuardedViewport, GridCell and TitleMenu. All three follow
 * the same class-framework shape documented in
 * docs/research/class-framework.md and already used elsewhere in this
 * codebase (e.g. class_3ac78.c's StageMap).
 *
 * NodeGuardedViewport (include/NodeGuardedViewport.h) is a Viewport whose
 * update skips the frame while no view node is attached; its seven table
 * methods, New_ and the getter are all here. GridCell (include/GridCell.h,
 * a SceneNode) is one cell of StageMap's grid, carrying the model placed
 * there; its five table methods, New_ and the getter are all here too.
 * TitleMenu (include/TitleMenu.h, a TaskCore) is the menu between days;
 * only its allocator and ctor are here, its other methods in
 * class_3bb8c_d.c.
 *
 * Two free functions serve TitleMenu: UpdateFlashbackLock (called from
 * TitleMenu__RefreshMenu) writes the menu's FLASHBACK lock,
 * registrationSlots[1], from two words of the save block; and
 * StampSaveTitleDay (called from the ctor with the current day) writes
 * the day as three full-width digits into the save title, "LSD   Day001"
 * (FullWidthChars3, include/TitleMenu.h's type for the title's characters).
 *
 * All 20 definitions here are matched, 0 INCLUDE_ASM.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "DreamSys.h"
#include "SceneNode.h"
#include "Actor.h"
#include "Viewport.h"
#include "NodeGuardedViewport.h"
#include "GridCell.h"
#include "TitleMenu.h"
#include "VabStreamObj.h"

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

/* Only the low byte of the sender's class id is read: 0x34 is an Actor
 * (Actor__DispatchLinkCommand makes the same test the other way round). */
void GridCell__DispatchLinkCommand(GridCell *self, BasicClass *sender, s32 event) {
    if ((u8)sender->methods->header == ACTOR_CLASS_ID) {
        self->methods->onActorLinkCommand(self, sender, event);
    }
}

/* tryAttachNearby keeps SceneNode's one-parameter slot type; this caller
 * passes the sender and event too, as Actor__OnActorLinkCommand does. */
void GridCell__OnActorLinkCommand(GridCell *self, void *sender, s32 event) {
    GetSceneNodeMethods()->dispatchLinkCommand((SceneNode *)self, sender, event);
    if (event < 9) {
        if (event >= 5) { /* MATCHING: nested, as && folds to one unsigned test */
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

TitleMenu *New_TitleMenu(struct DreamSys *dreamSys) {
    TitleMenu *self;

    self = BMemPMgrAlloc(sizeof(TitleMenu));
    if (self != NULL) {
        GetTitleMenuMethods()->ctor(self, dreamSys);
        return self;
    }
    return NULL;
}

void TitleMenu__TitleMenu(TitleMenu *self, struct DreamSys *dreamSys) {
    VabStreamObj *sound;

    Get_vtable_TaskCore()->ctor((TaskCore *)self, &sTitleMenuTarget,
                                (char *)sTitleMenuSoundBankPath, NULL);
    self->methods = GetTitleMenuMethods();
    sound = (VabStreamObj *)self->sound;
    sound->methods->setPitchOffset(sound, -1); /* 36 semitones down */
    self->dreamSys = dreamSys;
    self->saveCtrl = NULL;
    self->saveBlock = dreamSys->methods->getSaveBlock(dreamSys, &self->saveBlockSize);
    StampSaveTitleDay(dreamSys->methods->getCurrentDayAndYear(dreamSys, NULL));
    self->methods->setTarget(self, &sTitleMenuTarget);
    ((TitleMenuResetCallFn)self->methods->resetCounters)(self, dreamSys);
}

/* The total flashback unlock score FLASHBACK needs to be past. */
#define FLASHBACK_UNLOCK_SCORE 9999999

void UpdateFlashbackLock(TitleMenu *self, TaskCoreTarget *target) {
    DreamSaveBlock *save = (DreamSaveBlock *)self->saveBlock;
    s32 locked = 1;

    if (save->totalFlasbackUnlockScore > FLASHBACK_UNLOCK_SCORE) {
        locked = (save->amountFlashbacksAvailable == 0);
    }
    /* A NULL entry is a slot the cursor can stop on; slot 1 is FLASHBACK. */
    target->registrationSlots[TITLEMENU_FLASHBACK] = (void *)locked;
}

/* src/code_2cc8c_f.c: `value` as `width` full-width decimal digits into dst,
 * zero-padded unless `unpadded`. */
extern void FormatFullWidthNumber(u8 *dst, s32 value, s32 width, s32 unpadded);

/* The save title's day number, full-width characters 9..11 of
 * "LSD   Day001" (class_3bb8c_g.c's layout of the title). */
#define SAVE_TITLE_DAY 9
#define SAVE_TITLE_DAY_DIGITS 3

/* Formats the day as three full-width digits in sDayDigits's buffer and
 * copies them into the save title's day number, characters 9..11. */
void StampSaveTitleDay(s32 day) {
    FormatFullWidthNumber(sDayDigits, day, SAVE_TITLE_DAY_DIGITS, 0);
    *(FullWidthChars3 *)&((FullWidthChar *)gSaveTitle)[SAVE_TITLE_DAY] = *(FullWidthChars3 *)sDayDigits;
}
