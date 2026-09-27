/*
 * class_3bb8c_p -- Actor's movement and link-search methods (include/Actor.h,
 * occupants of the base table gActorMethods, +0x0C8..+0x0EC), and
 * VariantSprite's allocator and ctor.
 *
 *  - Local-axis moves. Actor__MoveLocalX/Y put `val` into one component of
 *    the local move vector gActorLocalMove, apply it through
 *    addLocalTranslation and clear it again (Actor__MoveAlongLocalAxis;
 *    MoveLocalZ is in class_3bb8c_o.c).
 *  - Move, else find a link. Actor__MoveLocalZOrFindLink/XOrFindLink clear
 *    linkTarget and move; when the move set no linkTarget,
 *    Actor__FindNearbyLink searches the StageMap grid round the actor's
 *    position for a GridCell whose model a vertical ray hits
 *    (BuildLinkQueries, ScanLinkCandidates, ScanGridWindow,
 *    AcceptGridElem), links to it and moves onto the hit.
 *  - The link-command pair (Actor__OnActorLinkCommand,
 *    Actor__OnGridCellLinkCommand): SceneNode's dispatchLinkCommand and,
 *    for an Actor sender's events 5..8, tryAttachNearby.
 *  - Actor__SetLastOffsetValue/SetPendingExtra and GetActorMethods.
 *  - New_VariantSprite and VariantSprite__VariantSprite, of an unrelated
 *    class (include/VariantSprite.h) that happens to follow in ROM.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "Actor.h"
#include "DreamSys.h"
#include "StageMap.h"
#include "LbdFile.h"
#include "GridCell.h"
#include "VariantSprite.h"

/* The local move vector's x and y (s16; the z, D_8008ABA8, is the next
 * halfword, class_3bb8c_o.c). All three stay 0 between moves: a move sets
 * one component, addLocalTranslation rotates the whole vector by the
 * actor's orientation, and the component is cleared again. */
extern s16 gActorLocalMove[2];

void Actor__MoveLocalX(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &gActorLocalMove[0], val, notify, 7);
}

void Actor__MoveLocalY(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &gActorLocalMove[1], val, notify, 8);
}

/* Moves the actor by `val` along one local axis (`axis` is that component of
 * gActorLocalMove), keeps `val` in lastOffsetValue and, when `notify` is
 * non-NULL, sends `event` (6, 7, 8 for z, x, y) through notifyWithHull. */
void Actor__MoveAlongLocalAxis(Actor *self, s16 *axis, s32 val, void *notify, s32 event) {
    s16 val16 = (s16)val; /* MATCHING: truncating at each store does not match */
    *axis = val16;
    self->lastOffsetValue = val16;
    self->methods->addLocalTranslation(self, &gActorLocalMove[0]);
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

/* A window of one chunk slot's cells, in cells: from (startCol, startRow),
 * numCols to the right and numRows DOWN (to lower row indices;
 * Actor__ScanGridWindow). */
typedef struct GridQuery {
    s16 startCol;
    s16 startRow;
    s32 numCols;
    s32 numRows;
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
 * -1; with no hit, linkTarget is NULL and it gets -2. Returns whether it
 * linked; 0 as well when the actor has no grid or no slot holds its
 * position. */
s32 Actor__FindNearbyLink(Actor *self) {
    Descriptor10Ext desc;
    GridQuery queries[3];
    u8 pad48Tail[8]; /* MATCHING: retail leaves these 8 bytes between queries and slots */
    ChunkSlot *slots[3];
    LongVec3 offset;

    if (self->grid != NULL) {
        void *pos = &self->coord2->tx;

        if (self->grid->methods->computeFootprintDescriptor(self->grid, &desc, pos) == 0) {
            s32 count = Actor__BuildLinkQueries(self, queries, slots, &desc, 1);
            void *result = Actor__ScanLinkCandidates(self, &offset, pos, count, queries, slots);

            self->linkTarget = result;
            if (result != NULL) {
                self->methods->addTranslation(self, &offset);
                self->methods->notifyWithHull(self, -1);
                return 1;
            }
            self->methods->notifyWithHull(self, -2);
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
            count = 2; /* MATCHING: after the call, for the delay-slot fill */
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

            /* MATCHING: *bucket re-read at each use; a local costs a register */
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

/* code_d294_c.c: casts a vertical ray from `pos` against the node's model,
 * one way and then the other; on a hit writes the hit less the ray's start
 * to `offset` and returns 1. */
extern s32 SceneNode__RaycastVertical(void *self, void *offset, void *pos);

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
    if (event < 9) {
        if (event >= 5) { /* MATCHING: nested, as && folds to one unsigned test */
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

void Actor__NoOpSlotE8(void) {}

void Actor__SetPendingExtra(Actor *self, s32 extra) {
    self->pendingExtra = extra;
}

ActorMethods *GetActorMethods(void) {
    return &gActorMethods;
}

extern void *BMemPMgrAlloc(s32 size);

/* VariantSprite's allocator; its other methods are in class_3bb8c_q.c and
 * class_3bb8c_t.c. */
VariantSprite *New_VariantSprite(s32 variant, void *resetArg, void *texture) {
    void *obj = BMemPMgrAlloc(sizeof(VariantSprite));
    if (obj != NULL) {
        GetVariantSpriteMethods()->ctor(obj, variant, resetArg, texture);
        return obj;
    }
    return NULL;
}

/* VariantSprite's two texture cells, forwarded as the Sprite ctor's `rect`
 * (Sprite__Reset copies it into Sprite.rect): u,v = (0x00,0x20) and
 * (0x10,0x20), 16x16. */
extern SpriteRect gVariantSpriteCells[2];

/* Sprite's ctor with the variant's cell, then this class's table, and the
 * reset slot (VariantSprite__SetVariantClut) with the variant, through
 * VariantSpriteResetFn. Returns nothing: it ends in that call and sets no
 * $v0. */
void VariantSprite__VariantSprite(VariantSprite *self, s32 variant, void *resetArg, void *texture) {
    GetSpriteMethods()->ctor((Sprite *)self, texture, 0, &gVariantSpriteCells[variant], resetArg, 0);
    self->methods = GetVariantSpriteMethods();
    self->unkA4 = 0;
    ((VariantSpriteResetFn)self->methods->reset)(self, variant);
}
