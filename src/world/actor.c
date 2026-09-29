/*
 * Actor's methods (include/actor.h: the SceneNode that moves, the base of
 * TodActor, DreamSys and StyleEffect), in ROM order, ending with its getter
 * GetActorMethods:
 *  - New_Actor and the constructor, the child bookkeeping that keeps the
 *    grid manager and the frame clock in `grid` and `ticker`, Reset,
 *    NotifyMove (the hull sweep sent after a move), DispatchLinkCommand, and
 *    the translation setters;
 *  - the local-axis moves (MoveLocalZ/X/Y, MoveAlongLocalAxis) and the
 *    link search, occupants of the base table gActorMethods,
 *    +0x0C8..+0x0EC.
 *
 * The link search: Actor__FindNearbyLink asks the StageMap grid which cell
 * holds the Actor's position, BuildLinkQueries turns it into up to three
 * cell windows, ScanLinkCandidates and ScanGridWindow walk them, and
 * AcceptGridElem casts a vertical ray at each cell's model.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "actor.h"
#include "stage_map.h"
#include "frame_clock.h"
#include "grid_cell.h"
#include "lbd_file.h"
#include "bmem_pmgr.h"

void *New_Actor(void) {
    Actor *self = BMemPMgrAlloc(sizeof(Actor));

    if (self != NULL) {
        if (GetActorMethods()->ctor(self) != NULL) {
            return self;
        }
        BMemPMgrFree(self);
        return NULL;
    }
    return NULL;
}

Actor *Actor__Actor(Actor *self) {
    if (GetSceneNodeMethods()->ctor((SceneNode *)self) == NULL) {
        goto fail;
    }
    self->methods = GetActorMethods();
    self->state = 0;
    self->grid = NULL;
    self->ticker = NULL;
    self->methods->reset(self);
    return self;
fail:
    return NULL;
}

/* addChild, removeChild and removeAllChildren chain SceneNode's and keep
 * two companions: a StageMap child (class id 0x114, three nibbles) in
 * `grid`, a FrameClock child in `ticker`. */
void Actor__AddChild(Actor *self, BasicClass *child) {
    s32 classId;

    GetSceneNodeMethods()->addChild((SceneNode *)self, child);
    classId = child->methods->header;
    if ((classId & CLASS_ID_LEVEL3_MASK) == STAGEMAP_CLASS_ID) {
        self->grid = (struct StageMap *)child;
    } else if ((classId & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID) {
        self->ticker = child;
    }
}

void Actor__RemoveChild(Actor *self, BasicClass *child) {
    s32 classId = child->methods->header;

    if ((classId & CLASS_ID_LEVEL3_MASK) == STAGEMAP_CLASS_ID) {
        self->grid = NULL;
    } else if ((classId & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID) {
        self->ticker = NULL;
    }
    GetSceneNodeMethods()->removeChild((SceneNode *)self, child);
}

void Actor__RemoveAllChildren(Actor *self) {
    self->grid = NULL;
    self->ticker = NULL;
    GetSceneNodeMethods()->removeAllChildren((SceneNode *)self);
}

/* The distance NotifyMove stretches the hull by until a move or
 * setLastOffsetValue sets one; no extra. */
void Actor__Reset(Actor *self) {
    self->lastOffsetValue = 300;
    self->pendingExtra = 0;
}

/* notifyWithHull: SceneNode's, then, for events ACTOR_EVENT_UNSWEPT to
 * ACTOR_EVENT_MOVED_Y on a model with bounds, the model's hull goes to the
 * parents through transformAndNotifyParents. For a move it is first
 * stretched: one face pushed out by the last move's distance plus
 * pendingExtra, an x face for ACTOR_EVENT_MOVED_X (RotateAndOffsetHullList
 * turns the box a quarter first), a z face otherwise; the max face after a
 * forward move, the min face after a backward one. An Actor linkTarget then
 * gets onLinkUpdate, which every Actor class leaves empty. */
void Actor__NotifyMove(Actor *self, s32 event) {
    GetSceneNodeMethods()->notifyWithHull((SceneNode *)self, event);
    /* MATCHING: two nested ifs; `&&` folds into one unsigned compare */
    if (event <= ACTOR_EVENT_MOVED_Y) {
        if (event >= ACTOR_EVENT_UNSWEPT) {
            TmdHull hull;

            if (self->model != NULL && TmdModel__GetBoundsCount(self->model)) {
                self->methods->getModelHull(self, &hull);
                if (event != ACTOR_EVENT_UNSWEPT) {
                    s16 offset = self->lastOffsetValue;
                    s32 alongX = (event == ACTOR_EVENT_MOVED_X);
                    s32 forward = (offset >= 0);
                    s32 delta;

                    /* MATCHING: goto, not if/else, which lays the two faces out the other way */
                    if (offset < 0) {
                        goto backward;
                    }
                    delta = offset + self->pendingExtra;
                    goto offsetHull;
                backward:
                    delta = offset - self->pendingExtra;
                offsetHull:
                    RotateAndOffsetHullList(&hull, alongX, forward, delta);
                }
                self->methods->transformAndNotifyParents(self, &hull, event);
                if (self->linkTarget != NULL) {
                    if ((self->linkTarget->methods->header & CLASS_ID_LEVEL2_MASK) == ACTOR_CLASS_ID) {
                        ((Actor *)self->linkTarget)->methods->onLinkUpdate((Actor *)self->linkTarget);
                    }
                }
            }
        }
    }
}

/* Routes a link command by the sender's class id byte: from an Actor (or
 * any class below it) to onActorLinkCommand, from a GridCell to
 * onGridCellLinkCommand, from anything else nowhere. */
void Actor__DispatchLinkCommand(Actor *self, BasicClass *sender, s32 event) {
    if ((sender->methods->header & CLASS_ID_LEVEL2_MASK) == ACTOR_CLASS_ID) {
        self->methods->onActorLinkCommand(self, sender, event);
    } else if ((sender->methods->header & CLASS_ID_LEVEL2_MASK) == GRIDCELL_CLASS_ID) {
        self->methods->onGridCellLinkCommand(self, sender, event);
    }
}

void Actor__SetTranslation(Actor *self, LongVec3 *v) {
    Actor__UpdateTranslation(self, 1, v);
}

void Actor__AddTranslation(Actor *self, LongVec3 *delta) {
    Actor__UpdateTranslation(self, 0, delta);
}

/* Sets (set != 0) or adds to the offset from the parent, coord2->coord.t,
 * then clears coord2->flg so libgs recomputes the matrix. */
void Actor__UpdateTranslation(Actor *self, s32 set, LongVec3 *v) {
    Actor *actor = self; /* MATCHING: a second name for self; without it the code differs */
    GsCOORDINATE2 *coord = actor->coord2;

    if (set) {
        *(LongVec3 *)coord->coord.t = *v; /* MATCHING: one struct copy, loads before stores */
    } else {
        coord->coord.t[0] += v->x;
        coord->coord.t[1] += v->y;
        coord->coord.t[2] += v->z;
    }
    actor->coord2->flg = 0;
}

/* Moves by `local` turned by the actor's own rotation. */
void Actor__AddLocalTranslation(Actor *self, s16 *local) {
    LongVec3 delta;

    SceneNode__RotateLocalVector((SceneNode *)self, &delta, local);
    self->methods->addTranslation(self, &delta);
}

/* The z of the s16 local move vector whose x and y are sActorLocalMove
 * (below, with MoveLocalX/Y and MoveAlongLocalAxis). */
extern s16 sActorLocalMoveZ;

void Actor__MoveLocalZ(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &sActorLocalMoveZ, val, notify, ACTOR_EVENT_MOVED_Z);
}

/* The local move vector's x and y (s16; the z, sActorLocalMoveZ, is the next
 * halfword, above). All three stay 0 between moves: a move sets
 * one component, addLocalTranslation rotates the whole vector by the
 * actor's orientation, and the component is cleared again. */
extern s16 sActorLocalMove[2];

void Actor__MoveLocalX(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &sActorLocalMove[0], val, notify, ACTOR_EVENT_MOVED_X);
}

void Actor__MoveLocalY(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &sActorLocalMove[1], val, notify, ACTOR_EVENT_MOVED_Y);
}

/* Moves the actor by `val` along one local axis (`axis` is that component of
 * sActorLocalMove), keeps `val` in lastOffsetValue and, when `notify` is
 * non-NULL, sends `event` (6, 7, 8 for z, x, y) through notifyWithHull. */
void Actor__MoveAlongLocalAxis(Actor *self, s16 *axis, s32 val, void *notify, s32 event) {
    s16 val16 = (s16)val; /* MATCHING: truncating at each store does not match */
    *axis = val16;
    self->lastOffsetValue = val16;
    self->methods->addLocalTranslation(self, &sActorLocalMove[0]);
    *axis = 0;
    if (notify != NULL) {
        self->methods->notifyWithHull(self, event);
    }
}

void Actor__MoveLocalZOrFindLink(Actor *self, s32 val, void *notify) {
    Actor__MoveOrFindNearbyLink(self, self->methods->moveLocalZ, val, notify);
}

void Actor__MoveLocalXOrFindLink(Actor *self, s32 val, void *notify) {
    Actor__MoveOrFindNearbyLink(self, self->methods->moveLocalX, val, notify);
}

void Actor__NoOpSlotD8(void) {}

/* Clears linkTarget, moves; if the move did not link, looks for a link. */
void Actor__MoveOrFindNearbyLink(Actor *self, void (*move)(Actor *, s32, void *), s32 val, void *notify) {
    self->linkTarget = NULL;
    move(self, val, notify);
    if (self->linkTarget == NULL) {
        Actor__FindNearbyLink(self);
    }
}

/** @brief A window of one chunk slot's cells, in cells: from (startCol,
 * startRow), numCols to the right and numRows DOWN (to lower row indices;
 * Actor__ScanGridWindow). */
typedef struct GridQuery {
    s16 startCol; /**< the window's first column */
    s16 startRow; /**< the window's first (highest) row */
    s32 numCols;  /**< columns scanned, rightward */
    s32 numRows;  /**< rows scanned, toward lower row indices */
} GridQuery;

void *AcceptGridElem(void *cell, void *offset, void *pos);
s32 Actor__BuildLinkQueries(Actor *self, GridQuery *queries, ChunkSlot **slots,
                            Descriptor10Ext *desc, s32 span);
void *Actor__ScanLinkCandidates(Actor *self, void *offset, void *pos, s32 count, GridQuery *queries,
                                ChunkSlot **slots);

/* Looks in the actor's grid for a GridCell a vertical ray from the actor's
 * position hits: in the actor's own cell of its chunk slot and, in a
 * vertical grid, the same cell in the slots above and below
 * (BuildLinkQueries with span 1). On a hit the cell becomes linkTarget, the
 * actor moves by the ray's offset (addTranslation) and notifyWithHull gets
 * ACTOR_EVENT_FLOOR_FOUND; with no hit, linkTarget is NULL and it gets
 * ACTOR_EVENT_NO_FLOOR. Returns whether it
 * linked; 0 as well when the actor has no grid or no slot holds its
 * position. */
s32 Actor__FindNearbyLink(Actor *self) {
    Descriptor10Ext desc;
    GridQuery queries[3];
    u8 pad48Tail[8]; /* MATCHING: retail leaves these 8 bytes between queries and slots */
    ChunkSlot *slots[3];
    LongVec3 offset;

    if (self->grid != NULL) {
        void *pos = self->coord2->coord.t;

        if (self->grid->methods->computeFootprintDescriptor(self->grid, &desc, pos) == 0) {
            s32 count = Actor__BuildLinkQueries(self, queries, slots, &desc, 1);
            void *result = Actor__ScanLinkCandidates(self, &offset, pos, count, queries, slots);

            self->linkTarget = result;
            if (result != NULL) {
                self->methods->addTranslation(self, &offset);
                self->methods->notifyWithHull(self, ACTOR_EVENT_FLOOR_FOUND);
                return 1;
            }
            self->methods->notifyWithHull(self, ACTOR_EVENT_NO_FLOOR);
            return 0;
        }
    }
    return 0;
}

/* Fills queries[]/slots[] with the cell windows to search round the cell
 * `desc` locates, and returns how many. An even span is rounded up to odd.
 * Span 1: the cell itself in desc's slot and, when the grid is vertical,
 * the same cell in the slots whose elemKey is one more and one less (as far
 * as the grid has rows), up to 3. A wider span: one window in desc's slot,
 * starting one column lower and one row higher than the cell, clipped at the
 * chunk's edges (FindNearbyLink, the one caller, passes 1). */
s32 Actor__BuildLinkQueries(Actor *self, GridQuery *queries, ChunkSlot **slots,
                            Descriptor10Ext *desc, s32 span) {
    s32 cellCol = desc->base.b2;
    s32 cellRow = desc->base.b3;
    s32 numCols;
    s32 numRows;
    s32 col;
    s32 row;
    s32 count;

    numRows = !(span & 1) ? ++span : span; /* MATCHING: the ternary, inverted, into numRows */
    numCols = numRows;
    col = cellCol;
    row = cellRow;
    count = 1;
    if (span == 1) {
        StageMap *map;
        StageGridDimensions *dims;
        ChunkSlot *slot;
        s16 slotKey;
        s32 key;

        queries[0].startCol = col;
        queries[0].startRow = row;
        queries[0].numCols = numRows;
        queries[0].numRows = numRows;
        slot = desc->slot;
        slots[0] = slot;
        map = self->grid;
        dims = map->config;
        if (dims->isVertical != 1) {
            return 1;
        }
        slotKey = slot->loader->elemKey;
        key = slotKey + 1;
        if (key < dims->rows) {
            slots[1] = map->methods->findSlotByNeighbour(map, key);
            count = 2; /* MATCHING: after the call, not before it */
            queries[1] = queries[0];
        }
        key = slotKey - 1;
        if (key >= 0) {
            slots[count] = map->methods->findSlotByNeighbour(map, key);
            queries[count] = queries[0];
            count++;
        }
        return count;
    }

    if (col == 0) {
        numCols = numRows - 1;
    } else {
        col--;
    }
    if (cellCol == STAGE_CHUNK_CELLS - 1) {
        numCols--;
    }
    if (cellRow == STAGE_CHUNK_CELLS - 1) {
        numRows--;
    } else {
        row++;
    }
    queries[0].startCol = col;
    if (cellRow == 0) {
        numRows--;
    }
    queries[0].startRow = row;
    queries[0].numCols = numCols;
    queries[0].numRows = numRows;
    slots[0] = desc->slot;
    return 1;
}

void *Actor__ScanGridWindow(Actor *self, void *offset, void *pos, GridQuery *query, ChunkSlot *slot);

/* Scans each of the `count` windows in its slot, skipping a slot whose
 * LbdFile header is not in yet; returns the first cell AcceptGridElem
 * accepts, or NULL. */
void *Actor__ScanLinkCandidates(Actor *self, void *offset, void *pos, s32 count, GridQuery *queries,
                                ChunkSlot **slots) {
    s32 i;

    for (i = 0; i < count;) {
        ChunkSlot *slot = *slots;
        i++; /* MATCHING: here, not in the for header */
        if (slot->loader->headerReady != 0) {
            void *result = Actor__ScanGridWindow(self, offset, pos, queries, slot);
            if (result != NULL) {
                return result;
            }
        }
        queries++;
        slots++;
    }
    return NULL;
}

/* Tries every cell of the window, each lattice cell and then the cells
 * chained from it (nextInCell), and returns the first AcceptGridElem
 * accepts, or NULL. `self` is not used. */
void *Actor__ScanGridWindow(Actor *self, void *offset, void *pos, GridQuery *query, ChunkSlot *slot) {
    s32 row, col;
    GridCell **bucket;

    bucket = slot->cells + query->startRow * STAGE_CHUNK_CELLS + query->startCol;
    for (row = 0; row < query->numRows; row++) {
        for (col = 0; col < query->numCols; col++) {
            GridCell *node;

            /* MATCHING: *bucket re-read at each use; a local compiles differently */
            if (AcceptGridElem(*bucket, offset, pos) != NULL) {
                return *bucket;
            }
            for (node = (*bucket)->nextInCell; node != NULL; node = node->nextInCell) {
                if (AcceptGridElem(node, offset, pos) != NULL) {
                    return node;
                }
            }
            bucket++;
        }
        bucket -= query->numCols + STAGE_CHUNK_CELLS;
    }
    return NULL;
}

/* `cell` if it is non-NULL and a vertical ray from `pos` hits its model
 * (the offset to the hit into `offset`), else NULL. */
void *AcceptGridElem(void *cell, void *offset, void *pos) {
    if (cell != NULL) {
        if (SceneNode__RaycastVertical(cell, offset, pos) != 0) {
            return cell;
        }
    }
    return NULL;
}

/* gActorMethods +0x0DC: a link command from another Actor. SceneNode's
 * handling, then, for events 5..8, tryAttachNearby with (self, sender,
 * event): SceneNode's slot declares self alone, hence the cast. */
void Actor__OnActorLinkCommand(Actor *self, void *sender, s32 event) {
    GetSceneNodeMethods()->dispatchLinkCommand((SceneNode *)self, sender, event);
    if (event <= ACTOR_EVENT_MOVED_Y) {
        if (event >= ACTOR_EVENT_UNSWEPT) { /* MATCHING: nested, as && folds to one unsigned test */
            ((void (*)(Actor *, void *, s32))self->methods->tryAttachNearby)(self, sender, event);
        }
    }
}

/* gActorMethods +0x0E0: a link command from a GridCell; SceneNode's handling. */
void Actor__OnGridCellLinkCommand(Actor *self, void *sender, s32 event) {
    GetSceneNodeMethods()->dispatchLinkCommand((SceneNode *)self, sender, event);
}

void Actor__SetLastOffsetValue(Actor *self, s16 val) {
    self->lastOffsetValue = val;
}

void Actor__OnLinkUpdate(void) {}

void Actor__SetPendingExtra(Actor *self, s32 extra) {
    self->pendingExtra = extra;
}

ActorMethods *GetActorMethods(void) {
    return &gActorMethods;
}
