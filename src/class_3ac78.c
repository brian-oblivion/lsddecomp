/*
 * class_3ac78 -- TimedTask's last two functions (TimedTask__PlaySound and
 * GetTimedTaskMethods, include/TimedTask.h; the rest are in class_39e08),
 * then the front third of StageMap (include/StageMap.h): the loaded part of
 * a stage's map, seven chunk slots each laid out as a lattice of GridCells.
 *
 * This third holds the object's life and its command path: the allocator
 * and ctor (seven slots, each an LbdFile, a placement list, a cellParent
 * GridCell attached at `origin` and STAGE_SLOT_CELLS cells STAGE_CELL_SIZE
 * apart, row stride STAGE_CHUNK_CELLS), Finalize, OnNotify, Reset,
 * OnSlotEvent, the per-tick update (UpdateIfEnabled: footprint tracking,
 * then the scale ramp), UnloadAllSlots, the setters ObjM configures it
 * through (SetChildParams, SetCallback, SetAcceptedTags, SetGridSpan,
 * SetConfig), and the path a command takes to the cells:
 * DispatchLinkCommand passes on an Actor sender's, ForwardAcceptedCommand
 * filters the sender against acceptedTags, ApplyToSenderFootprint turns
 * the sender's position into a 3 x 3 cell footprint (SetFootprintFromCell
 * or SetFootprintRect), and DispatchToRectCells hands the command to every
 * cell in it and every cell chained behind each (NotifyGridCell).
 * StageMap__NoOpSlotD8 is the empty +0x0D8 slot; nothing calls it.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "StageMap.h"
#include "Actor.h"
#include "VabStreamObj.h"
#include "LightRig.h"
#include "TimedTask.h"
#include "DrawSystem.h"
#include "PlacementGrid.h"
#include "LinkResource.h"
#include "LbdFile.h"
#include "GridCell.h"
#include "FlatLightObj.h"
#include "BMemPMgr.h"

/* TimedTask::sound is BasicClass * (it may be the ctor's own argument); when
 * it is a New_VabStreamObj object, +0x080 is VabStreamObj__PlayTone. Plays
 * `tone` at full volume (127) throughout. */
void TimedTask__PlaySound(TimedTask *self, s32 tone) {
    VabStreamObj *sound = (VabStreamObj *)self->sound;

    if (sound != NULL) {
        sound->methods->playTone(sound, tone, 127, 127);
    }
}

TimedTaskMethods *GetTimedTaskMethods(void) {
    return &gTimedTaskMethods;
}

StageMap *New_StageMap(LongVec3 *origin, s32 autoLoad) {
    StageMap *self;

    self = BMemPMgrAlloc(sizeof(StageMap));
    if (self != NULL) {
        GetStageMapMethods()->ctor(self, origin, autoLoad);
        return self;
    }
    return NULL;
}

extern LongVec3 gDefaultOrigin;

void StageMap__StageMap(StageMap *self, LongVec3 *origin, s32 autoLoad) {
    s32 i;
    ChunkSlot *slot;
    GridCell *cell;
    GridCell **cells;
    GridCell **cursor;
    GridCell **end;
    LongVec3 pos;

    GetLightRigMethods()->ctor((LightRig *)self);
    self->methods = GetStageMapMethods();

    if (origin != NULL) {
        self->origin = *origin;
    } else {
        self->origin = gDefaultOrigin;
    }

    self->loadsPending = 0;
    self->pendingLoadCount = 0;
    self->chunksLoaded = 0;
    self->enabled = 0;
    self->target = NULL;
    self->acceptedTags = NULL;
    self->scaleRampTicks = 0;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];

        slot->loader = New_LbdFile();
        slot->loader->freeGuard = (slot->loader->buffer != NULL);
        slot->loader->elemKey = i;
        slot->loader->methods->setAutoLoadData(slot->loader, autoLoad);

        slot->heldObj = NULL;
        slot->unk18 = 0;
        slot->neighbour = i;
        slot->loadPending = 0;

        slot->placements = New_PlacementGrid(0);
        slot->cellParent = New_GridCell();
        slot->cellParent->methods->attachToParent(slot->cellParent, (SceneNode *)self, &self->origin);

        slot->cells = (GridCell **)BMemPMgrAlloc(STAGE_SLOT_CELLS * sizeof(GridCell *));
        if (slot->cells == NULL) {
            return;
        }

        pos.x = STAGE_CELL_SIZE / 2;
        pos.y = 0;
        pos.z = STAGE_CELL_SIZE / 2;

        /* MATCHING: `end` from `cells` before `cursor = cells`, or the load
         * of slot->cells no longer goes through $v0. */
        cells = slot->cells;
        end = cells + STAGE_SLOT_CELLS;
        cursor = cells;
        while (cursor < end) {
            cell = New_GridCell();
            *cursor = cell;
            cell->methods->attachToParent(cell, (SceneNode *)slot->cellParent, &pos);

            /* Cell centres, row by row; the test lets a row reach
             * STAGE_CHUNK_CELLS + 1 positions before it wraps. */
            pos.x += STAGE_CELL_SIZE;
            if (pos.x > STAGE_CHUNK_SIZE + STAGE_CELL_SIZE / 2) {
                pos.x = STAGE_CELL_SIZE / 2;
                pos.z += STAGE_CELL_SIZE;
            }

            /* MATCHING: each use reloads *cursor. */
            cell = *cursor;
            cell->methods->setLightMode(cell, 1); /* GsFOG */
            cell = *cursor;
            cursor++;
            cell->attribute |= GsDOFF;
        }
    }

    self->methods->addChild(self, (BasicClass *)GetDrawSystem());
    self->methods->reset(self);
}

void StageMap__Finalize(StageMap *self) {
    s32 i;
    ChunkSlot *slot;
    GridCell *cell;
    GridCell **cells;
    GridCell **cursor;
    GridCell **end;

    self->methods->removeChild(self, (BasicClass *)GetDrawSystem());

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(self, STAGEMAP_EVENT_SLOT_RELEASE,
                                                               slot, i);

        if (slot->loader != NULL) {
            slot->loader->methods->release(slot->loader);
        }

        if (slot->placements != NULL) {
            if (slot->placements->linkResource != NULL) {
                slot->placements->linkResource->methods->release(slot->placements->linkResource);
            }
            slot->placements = slot->placements->methods->release(slot->placements);
        }

        if (slot->cellParent != NULL) {
            slot->cellParent->methods->release(slot->cellParent);
        }

        cells = slot->cells;
        end = cells + STAGE_SLOT_CELLS;
        cursor = cells;
        while (cursor < end) {
            cell = *cursor;
            if (cell != NULL) {
                cell->methods->release(cell);
            }
            cursor++;
        }

        BMemPMgrFree(slot->cells);
    }

    GetLightRigMethods()->finalize((LightRig *)self);
}

/* A sender of DrawSystem's class (id nibble 0x1) goes on to onNotifyTag1. */
void StageMap__OnNotify(StageMap *self, BasicClass *sender, s32 command) {
    GetSceneNodeMethods()->onNotify((SceneNode *)self, sender, command);

    if ((sender->methods->header & 0xF) == 1) {
        self->methods->onNotifyTag1(self, sender, command);
    }
}

extern s32 gDefaultGridSpan;

void StageMap__Reset(StageMap *self) {
    self->config = NULL;
    self->acceptedTags = NULL;
    self->rectCount = 0;
    self->methods->setGridSpan(self, gDefaultGridSpan);
    self->unk1CC = -1;
    self->unk1D0 = -1;
    self->unk1D4 = -1;
    self->unk1D8 = -1;
}

/* Records the slot and passes a slot event on to the parents; a
 * STAGEMAP_EVENT_SLOT_RELEASE also releases the slot's heldObj. */
void StageMap__OnSlotEvent(StageMap *self, s32 command, ChunkSlot *slot) {
    GetSceneNodeMethods()->notifyWithHull((SceneNode *)self, command);

    /* MATCHING: two literal if+goto tests; an if/else-if chain inverts the
     * branches. */
    if (command == STAGEMAP_EVENT_SLOT_RELEASE)
        goto release;
    if (command == STAGEMAP_EVENT_SLOT_DATA_READY)
        goto record;
    return;

release:
    if (slot->heldObj != NULL) {
        slot->heldObj = slot->heldObj->methods->release(slot->heldObj);
    }

record:
    self->lastEventSlot = slot;
    self->methods->notifyParents(self, command);
}

void StageMap__UpdateIfEnabled(StageMap *self) {
    if (self->enabled) {
        self->methods->updateFootprintTracking(self);
        self->methods->stepScaleRamp(self);
    }
}

void StageMap__DispatchLinkCommand(StageMap *self, BasicClass *sender, s32 command) {
    if ((u8)sender->methods->header == ACTOR_CLASS_ID) {
        self->methods->forwardAcceptedCommand(self, sender, command);
    }
}

/* Cancel every slot's load, clear and release what it holds, then zero
 * the load counters and end the scale ramp. */
void StageMap__UnloadAllSlots(StageMap *self) {
    s32 i;
    ChunkSlot *slot;
    PlacementGrid *placements;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        slot->loader->methods->cancelRequests(slot->loader);
        slot->loadPending = 0;
        self->methods->clearSlotCells(self, slot);
        placements = slot->placements;
        if (placements->linkResource != NULL) {
            placements->linkResource =
                placements->linkResource->methods->release(placements->linkResource);
        }
        ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(self, STAGEMAP_EVENT_SLOT_RELEASE,
                                                               slot, i);
        slot->loader->methods->releaseDataBlock(slot->loader);
    }

    self->chunksLoaded = 0;
    self->pendingLoadCount = 0;
    self->methods->endScaleRamp(self);
}

void StageMap__SetChildParams(StageMap *self, s32 count, s32 dirs, s32 colors) {
    s32 i;
    FlatLightObj *light;

    for (i = 0; i < count; i++) {
        light = (FlatLightObj *)self->methods->getLight(self, i);
        light->methods->setColor(light, 1, (FlatLightColor *)colors);
        colors += sizeof(FlatLightColor);
        light->methods->setDirection(light, 1, (s16 *)dirs);
        dirs += 3 * sizeof(s16);
    }
}

void StageMap__SetCallback(StageMap *self, ChunkFileFn fn, void *ctx) {
    self->chunkFileFn = fn;
    self->chunkFileCtx = ctx;
}

void StageMap__SetAcceptedTags(StageMap *self, s32 *tags) {
    self->acceptedTags = tags;
}

void StageMap__ForwardAcceptedCommand(StageMap *self, void *sender, s32 command) {
    s32 *tag;
    u8 unused[24]; /* MATCHING: retail's frame is 24 bytes larger than the locals need */

    switch (command) {
        case 2:
        case 3:
        case 5:
        case 6:
        case 7:
        case 8:
            break;
        default:
            return;
    }

    tag = self->acceptedTags;
    if (tag == NULL)
        return;
    if (*tag == 0)
        return;

    do {
        if (*tag == ((BasicClass *)sender)->methods->header) {
            self->methods->applyToSenderFootprint(self, sender, command);
        }
        tag++;
    } while (*tag != 0);
}

void StageMap__ApplyToSenderFootprint(StageMap *self, SceneNode *sender, s32 command) {
    SplitLongVec3 *pos;
    CellRectSet savedRects;
    Descriptor10Ext desc;
    s32 savedRectCount;

    if (sender->parent != NULL) {
        pos = (SplitLongVec3 *)sender->coord2->workm.t;
    } else {
        pos = NULL;
    }

    if (self->methods->computeFootprintDescriptor(self, &desc, pos) != 0) {
        return;
    }

    savedRectCount = self->rectCount;
    savedRects = self->rects;

    if (self->config->isVertical == 0) {
        StageMap__SetFootprintFromCell(self, &desc, 3);
    } else {
        StageMap__SetFootprintRect(self, &desc, 3);
    }

    StageMap__DispatchToRectCells(self, sender, command);

    self->rectCount = savedRectCount;
    self->rects = savedRects;
}

void StageMap__SetFootprintFromCell(StageMap *self, Descriptor10Ext *desc, s32 span) {
    s16 row;

    self->footprintCol = desc->base.b2 - 1;
    row = desc->base.b3 - 1;
    self->footprintWidth = span;
    self->footprintHeight = span;
    self->footprintRow = row;
    StageMap__BuildFootprintRects(self);
}

/* Clamp a span x span footprint centred on desc's cell to the chunk's
 * STAGE_CHUNK_CELLS x STAGE_CHUNK_CELLS lattice: a cell on the low edge (0)
 * or the high edge loses one row/column there.
 * MATCHING: the edge tests read copies taken before the decrement, and the
 * height is `span` itself. */
void StageMap__SetFootprintRect(StageMap *self, Descriptor10Ext *desc, s32 span) {
    s32 col;
    s32 row;
    s32 width;
    s32 origCol;
    s32 origRow;

    width = span;
    col = desc->base.b2;
    row = desc->base.b3;
    origCol = col;
    origRow = row;

    if (col == 0) {
        width = span - 1;
    } else {
        col--;
    }
    if (origCol == STAGE_CHUNK_CELLS - 1) {
        width--;
    }

    if (origRow == 0) {
        span--;
    } else {
        row--;
    }
    if (origRow == STAGE_CHUNK_CELLS - 1) {
        span--;
    }

    self->rectCount = 1;
    self->rects.e[0].slotIndex = self->methods->findSlotIndexByChunk(self, desc->chunkIndex);
    self->rects.e[0].col = col;
    self->rects.e[0].row = row;
    self->rects.e[0].width = width;
    self->rects.e[0].height = span;
}

/* Notify every cell of every rectangle, and every object chained behind
 * each cell, with the cell's key in curCell while it is notified.
 * MATCHING: the comma increments go `rect++, i++` and `cell++, col++`. */
void StageMap__DispatchToRectCells(StageMap *self, SceneNode *sender, s32 command) {
    s32 i;
    s32 row;
    s32 col;
    CellRect *rect;
    ChunkSlot *slot;
    GridCell **cell;
    GridCell *chained;

    rect = self->rects.e;
    for (i = 0; i < self->rectCount; rect++, i++) {
        slot = &self->slots[rect->slotIndex];
        if (slot->loader->headerReady != 0) {
            cell = (slot->cells + rect->col) + rect->row * STAGE_CHUNK_CELLS;
            for (row = 0; row < rect->height; row++) {
                for (col = 0; col < rect->width; cell++, col++) {
                    /* MATCHING: b0/b1 as one halfword; two byte copies are two lb/sb */
                    *(u16 *)&self->curCell = *(u16 *)&self->targetCell;
                    self->curCell.b2 = rect->col + col;
                    self->curCell.b3 = rect->row + row;
                    NotifyGridCell(*cell, sender, command);
                    for (chained = (*cell)->nextInCell; chained != NULL; chained = chained->nextInCell) {
                        NotifyGridCell(chained, sender, command);
                    }
                }
                cell += STAGE_CHUNK_CELLS - rect->width;
            }
        }
    }
}

/* Hands the command to a cell flagged GRIDCELL_FLAG_TAKES_COMMANDS. sender
 * and command arrive as DispatchToRectCells' own. */
void NotifyGridCell(GridCell *cell, SceneNode *sender, s32 command) {
    if (cell != NULL && (cell->flags36 & GRIDCELL_FLAG_TAKES_COMMANDS)) {
        cell->methods->onNotify(cell, sender, command);
    }
}

Descriptor10 *StageMap__GetCurrentCellKey(StageMap *self) {
    return &self->curCell;
}

void StageMap__NoOpSlotD8(void) {}

void StageMap__SetGridSpan(StageMap *self, s32 span) {
    self->gridSpan = span;
    self->gridCells = (s16)(span >> STAGE_CELL_SHIFT);
    self->gridHalfCells = (s16)(span >> (STAGE_CELL_SHIFT + 1));
}

void StageMap__SetConfig(StageMap *self, StageGridDimensions *config) {
    self->methods->reset(self);
    self->config = config;
}
