/*
 * class_3ac78 -- TimedTask's last two functions (TimedTask__PlaySound and
 * GetTimedTaskMethods, include/TimedTask.h; the rest are in class_39e08),
 * then the front half of Class866E8, the class whose method table is
 * gClass866E8Methods (80 slots, header 0x114; tools/classtable.py gClass866E8Methods). It
 * derives from SceneNode (code_d294) through LightRig (include/LightRig.h,
 * gLightRigMethods: the three flat lights and the ambient colour), whose ctor
 * and finalize its own chain to, and the game builds exactly one, at boot, in class_39e08's Class865C8__Class865C8 via New_Class866E8(0, 1).
 *
 * What it manages is a GRID. The object owns seven elements (elems[7]), each
 * pairing a loader, a placement list, a parent node, and a 0x668-byte heap
 * block holding that element's grid of GridCell cells; the constructor seeds every
 * cell with a world position on a 0x800 lattice. Indexing the grid uses a row
 * stride of 20 cells -- the same 20 that gDefaultGridSpan >> 11 produces
 * (0xA000 / 0x800, see Class866E8__SetGridSpan) and the same stride
 * class_3bb8c_b's byte-matched Class866E8__SetFootprintCellFlag walks.
 *
 * Work reaches the cells through a rectangle list (rects[4]/rectCount): a
 * notification arrives at Class866E8__OnNotify or Class866E8__DispatchLinkCommand,
 * Class866E8__ForwardAcceptedCommand filters the sender against acceptedTags,
 * Class866E8__ApplyToSenderFootprint turns the sender's position into one
 * rectangle, and Class866E8__DispatchToRectCells re-notifies every cell in it
 * and every cell chained behind it. The queries that build those rectangles,
 * and an element's resource and GPU sides, live in class_3bb8c*. The class is
 * declared once, in include/Class866E8.h (track 4, round 89).
 *
 * Every function in the unit is matched C; the last three stalls
 * (Class866E8__ResetAllElements, Class866E8__SetFootprintRect and
 * Class866E8__DispatchToRectCells) were matched in round 71. func_8004B324 keeps its placeholder name
 * deliberately -- it is an empty vtable stub with no established purpose, the
 * same case as SceneNode__NoOpSlot5C in code_d294_b.
 */
#include "common.h"
#include "Class866E8.h"
#include "VabStreamObj.h"
#include "LightRig.h"
#include "TimedTask.h"
#include "DrawSystem.h"
#include "Class6D940.h"
#include "LinkResource.h"
#include "LbdFile.h"
#include "GridCell.h"
#include "FlatLightObj.h"

extern void *BMemPMgrAlloc(s32 size);

/* TimedTask::sound is BasicClass * (it may be the ctor's own argument); when
 * it is a New_VabStreamObj object, +0x080 is VabStreamObj__PlayTone. */

void TimedTask__PlaySound(TimedTask *self, s32 tone) {
    VabStreamObj *sound = (VabStreamObj *)self->sound;

    if (sound != NULL) {
        sound->methods->playTone(sound, tone, 0x7F, 0x7F);
    }
}

TimedTaskMethods *GetTimedTaskMethods(void) {
    return &gTimedTaskMethods;
}

Class866E8 *New_Class866E8(LongVec3 *origin, s32 autoLoad) {
    Class866E8 *self;

    self = BMemPMgrAlloc(0x1E8);
    if (self != NULL) {
        GetClass866E8Methods()->ctor(self, origin, autoLoad);
        return self;
    }
    return NULL;
}

/*
 * Class866E8__Class866E8's own helpers -- all still-uncarved elsewhere, typed
 * purely from this call site's own register usage.
 */
extern void BMemPMgrFree(void *arg1);
extern LongVec3 gDefaultOrigin;

void Class866E8__Class866E8(Class866E8 *self, LongVec3 *origin, s32 autoLoad) {
    s32 i;
    ChunkSlot *entry;
    GridCell *obj;
    GridCell **cellp;
    u8 *p;
    u8 *end;
    s32 buf[3];

    GetLightRigMethods()->ctor((LightRig *)self);
    self->methods = GetClass866E8Methods();

    if (origin != NULL) {
        self->origin = *origin;
    } else {
        self->origin = gDefaultOrigin;
    }

    self->loadsPending = 0;
    self->unk1B4 = 0;
    self->chunksLoaded = 0;
    self->enabled = 0;
    self->target = NULL;
    self->acceptedTags = 0;
    self->rateCountdown = 0;

    for (i = 0; i < 7; i++) {
        entry = &self->elems[i];

        entry->loader = New_LbdFile();
        entry->loader->freeGuard = (entry->loader->buffer != NULL);
        entry->loader->elemKey = i;
        entry->loader->methods->setAutoLoadData(entry->loader, autoLoad);

        entry->heldObj = NULL;
        entry->unk18 = 0;
        entry->neighbour = i;
        entry->loadPending = 0;

        entry->placements = New_Class6D940(0);
        entry->cellParent = New_GridCell();
        entry->cellParent->methods->attachToParent(entry->cellParent, (SceneNode *)self, &self->origin);

        entry->cells = (GridCell **)BMemPMgrAlloc(0x668);
        if (entry->cells == NULL) {
            return;
        }

        buf[0] = 0x400;
        buf[1] = 0;
        buf[2] = 0x400;

        cellp = entry->cells;
        end = (u8 *)cellp + 0x668;
        p = (u8 *)cellp;
        while (p < end) {
            obj = New_GridCell();
            *(GridCell **)p = obj;
            obj->methods->attachToParent(obj, (SceneNode *)entry->cellParent, (LongVec3 *)buf);

            buf[0] += 0x800;
            if (buf[0] > 0xA400) {
                buf[0] = 0x400;
                buf[2] += 0x800;
            }

            obj = *(GridCell **)p;
            obj->methods->setLightMode(obj, 1);
            obj = *(GridCell **)p;
            p += 4;
            obj->attribute |= 0x80000000;
        }
    }

    self->methods->addChild(self, (BasicClass *)GetDrawSystem());
    self->methods->reset(self);
}

void Class866E8__Finalize(Class866E8 *self) {
    s32 i;
    ChunkSlot *entry;
    GridCell *obj;
    GridCell **cellp;
    u8 *p;
    u8 *end;

    self->methods->removeChild(self, (BasicClass *)GetDrawSystem());

    for (i = 0; i < 7; i++) {
        entry = &self->elems[i];
        ((Class866E8OnElementEventFn)self->methods->notifyWithHull)(self, 6, entry, i);

        if (entry->loader != NULL) {
            entry->loader->methods->release(entry->loader);
        }

        if (entry->placements != NULL) {
            if (entry->placements->linkResource != NULL) {
                entry->placements->linkResource->methods->release(entry->placements->linkResource);
            }
            entry->placements = entry->placements->methods->release(entry->placements);
        }

        if (entry->cellParent != NULL) {
            entry->cellParent->methods->release(entry->cellParent);
        }

        cellp = entry->cells;
        end = (u8 *)cellp + 0x668;
        p = (u8 *)cellp;
        while (p < end) {
            obj = *(GridCell **)p;
            if (obj != NULL) {
                obj->methods->release(obj);
            }
            p += 4;
        }

        BMemPMgrFree(entry->cells);
    }

    GetLightRigMethods()->finalize((LightRig *)self);
}

/* GetSceneNodeMethods: include/SceneNode.h. Round 59 measured the two
 * arguments these calls used to pass the no-argument getter as zero-cost (the
 * jal's delay slot holds a callee-save spill); track 4 dropped them. */

void Class866E8__OnNotify(Class866E8 *self, BasicClass *sender, s32 command) {
    GetSceneNodeMethods()->onNotify((SceneNode *)self, sender, command);

    if ((sender->methods->header & 0xF) == 1) {
        self->methods->onNotifyTag1(self, sender, command);
    }
}

extern s32 gDefaultGridSpan;

void Class866E8__Reset(Class866E8 *self) {
    self->config = NULL;
    self->acceptedTags = 0;
    self->rectCount = 0;
    self->methods->setGridSpan(self, gDefaultGridSpan);
    self->unk1CC = -1;
    self->unk1D0 = -1;
    self->unk1D4 = -1;
    self->unk1D8 = -1;
}

void Class866E8__OnElementEvent(Class866E8 *self, s32 command, ChunkSlot *elem) {
    GetSceneNodeMethods()->notifyWithHull((SceneNode *)self, command);

    if (command == 6)
        goto handle6;
    if (command == 7)
        goto merge;
    return;

handle6:
    if (elem->heldObj != NULL) {
        elem->heldObj = elem->heldObj->methods->release(elem->heldObj);
    }

merge:
    self->lastEventElem = elem;
    self->methods->notifyParents(self, command);
}

void Class866E8__UpdateIfEnabled(Class866E8 *self) {
    if (self->enabled) {
        self->methods->updateFootprintTracking(self);
        self->methods->advanceRateCountdown(self);
    }
}

void Class866E8__DispatchLinkCommand(Class866E8 *self, BasicClass *sender, s32 command) {
    if ((u8)sender->methods->header == 0x34) {
        self->methods->forwardAcceptedCommand(self, sender, command);
    }
}

/* Reset every one of the seven grid elements, then the two counters.
 * Matched round 71: `&self->elems[i]` is what produces retail's
 * base + running-offset walk (GCC's strength reduction), not a hand-rolled
 * byte offset. */
void Class866E8__ResetAllElements(Class866E8 *self) {
    s32 i;
    ChunkSlot *entry;
    Class6D940 *list;

    for (i = 0; i < 7; i++) {
        entry = &self->elems[i];
        entry->loader->methods->cancelRequests(entry->loader);
        entry->loadPending = 0;
        self->methods->resetElementCells(self, entry);
        list = entry->placements;
        if (list->linkResource != NULL) {
            list->linkResource = list->linkResource->methods->release(list->linkResource);
        }
        ((Class866E8OnElementEventFn)self->methods->notifyWithHull)(self, 6, entry, i);
        entry->loader->methods->releaseDataBlock(entry->loader);
    }

    self->chunksLoaded = 0;
    self->unk1B4 = 0;
    self->methods->flushRateLatch(self);
}

void Class866E8__SetChildParams(Class866E8 *self, s32 count, s32 dirs, s32 colors) {
    s32 i;
    FlatLightObj *light;

    for (i = 0; i < count; i++) {
        light = (FlatLightObj *)self->methods->getLight(self, i);
        light->methods->setColor(light, 1, (FlatLightColor *)colors);
        colors += 3;
        light->methods->setDirection(light, 1, (s16 *)dirs);
        dirs += 6;
    }
}

void Class866E8__SetCallback(Class866E8 *self, Class866E8ValueFn fn, void *ctx) {
    self->valueFn = fn;
    self->valueFnCtx = ctx;
}

void Class866E8__SetAcceptedTags(Class866E8 *self, s32 *tags) {
    self->acceptedTags = tags;
}

void Class866E8__ForwardAcceptedCommand(Class866E8 *self, void *sender, s32 command) {
    s32 *p;
    u8 unused[24];

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

    p = self->acceptedTags;
    if (p == NULL)
        return;
    if (*p == 0)
        return;

    do {
        if (*p == ((BasicClass *)sender)->methods->header) {
            self->methods->applyToSenderFootprint(self, sender, command);
        }
        p++;
    } while (*p != 0);
}

void Class866E8__ApplyToSenderFootprint(Class866E8 *self, SceneNode *sender, s32 command) {
    SplitLongVec3 *pos;
    GridSlotList866E8 saved;
    Descriptor10Ext buf;
    s32 savedRectCount;

    if (sender->parent != NULL) {
        pos = (SplitLongVec3 *)sender->coord2->unk38;
    } else {
        pos = NULL;
    }

    if (self->methods->computeFootprintDescriptor(self, &buf, pos) != 0) {
        return;
    }

    savedRectCount = self->rectCount;
    saved = self->rects;

    if (self->config->isVertical == 0) {
        Class866E8__SetFootprintFromCell(self, &buf, 3);
    } else {
        Class866E8__SetFootprintRect(self, &buf, 3);
    }

    Class866E8__DispatchToRectCells(self, sender, command);

    self->rectCount = savedRectCount;
    self->rects = saved;
}

void Class866E8__SetFootprintFromCell(Class866E8 *self, Descriptor10Ext *desc, s32 span) {
    s16 t;

    self->footprintCol = desc->base.b2 - 1;
    t = desc->base.b3 - 1;
    self->footprintWidth = span;
    self->footprintHeight = span;
    self->footprintRow = t;
    Class866E8__BuildFootprintSlots(self);
}

/* Clamp a span x span footprint centred on desc's cell to the 20 x 20 grid:
 * a cell on the low edge (0) loses one row/column, one on the high edge
 * (0x13) loses one too. The edge tests read a COPY of each byte taken before
 * the decrement, and the height companion is `span` itself. Matched round 71. */
void Class866E8__SetFootprintRect(Class866E8 *self, Descriptor10Ext *desc, s32 span) {
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
    if (origCol == 0x13) {
        width--;
    }

    if (origRow == 0) {
        span--;
    } else {
        row--;
    }
    if (origRow == 0x13) {
        span--;
    }

    self->rectCount = 1;
    self->rects.e[0].slotIndex = self->methods->findElemIndexByUnk30(self, desc->chunkIndex);
    self->rects.e[0].col = col;
    self->rects.e[0].row = row;
    self->rects.e[0].width = width;
    self->rects.e[0].height = span;
}

/* Notify every cell of every rectangle, and every object chained behind
 * each cell. Matched round 71: the ORDER of the comma-separated increments
 * is load-bearing in both loops (`entry++, i++` and `cell++, col++`); the
 * reverse order was the whole 95/117 residue. */
void Class866E8__DispatchToRectCells(Class866E8 *self, SceneNode *sender, s32 command) {
    s32 i;
    s32 row;
    s32 col;
    CellRect *entry;
    ChunkSlot *slot;
    GridCell **cell;
    GridCell *obj;

    entry = self->rects.e;
    for (i = 0; i < self->rectCount; entry++, i++) {
        slot = &self->elems[entry->slotIndex];
        if (slot->loader->headerReady != 0) {
            cell = (slot->cells + entry->col) + entry->row * 20;
            for (row = 0; row < entry->height; row++) {
                for (col = 0; col < entry->width; cell++, col++) {
                    *(u16 *)&self->curCell = *(u16 *)&self->targetCell;
                    self->curCell.b2 = entry->col + col;
                    self->curCell.b3 = entry->row + row;
                    NotifyGridCell(*cell, sender, command);
                    for (obj = (*cell)->nextInCell; obj != NULL; obj = obj->nextInCell) {
                        NotifyGridCell(obj, sender, command);
                    }
                }
                cell += 20 - entry->width;
            }
        }
    }
}

/* Widened this round (Class866E8__DispatchToRectCells) from a single-param signature to
 * accept two more, unused, forwarded params: Class866E8__DispatchToRectCells's own call
 * sites explicitly set up $a1/$a2 before every call here (unlike
 * GetSceneNodeMethods's "leftover, already-there" args -- these are real,
 * explicit `move` instructions), so the call itself needs a matching
 * 3-param prototype to compile. Confirmed harmless to THIS function's own
 * already-matched body: neither extra param is read, and GCC does not
 * reserve stack space for unused trailing integer/pointer args on this
 * target, so the definition's own bytes are unaffected (reverified
 * 18/18 after the widening). */
void NotifyGridCell(GridCell *cell, SceneNode *sender, s32 command) {
    if (cell != NULL && (cell->flags36 & 0x80)) {
        cell->methods->onNotify(cell, sender, command);
    }
}

Descriptor10 *Class866E8__GetCurrentCellKey(Class866E8 *self) {
    return &self->curCell;
}

void func_8004B324(void) {}

void Class866E8__SetGridSpan(Class866E8 *self, s32 span) {
    self->gridSpan = span;
    self->gridCells = (s16)(span >> 11);
    self->gridHalfCells = (s16)(span >> 12);
}

void Class866E8__SetConfig(Class866E8 *self, StageGridDimensions *config) {
    self->methods->reset(self);
    self->config = config;
}
