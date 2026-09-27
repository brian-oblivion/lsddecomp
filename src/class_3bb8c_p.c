/*
 * class_3bb8c_p -- vram 0x800574C4..0x80057DBC, carved round 17
 * (2026-09-04), immediately behind class_3bb8c_o. Actor methods
 * (include/Actor.h; before round 82 they carried DreamSys's name, but they
 * are occupants of the BASE table gActorMethods, +0x0C8..+0x0EC) plus one
 * unrelated constructor:
 *
 *  - The local-axis moves (Actor__MoveLocalX/Y, the shared
 *    Actor__MoveAlongLocalAxis, and Actor__MoveLocalZOrFindLink /
 *    MoveLocalXOrFindLink through Actor__MoveOrFindNearbyLink): write one
 *    component of the local move vector gActorLocalMove, apply it through
 *    addLocalTranslation, clear it, and fall back to a grid-based
 *    nearby-link search (Actor__FindNearbyLink, Actor__BuildLinkQueries,
 *    Actor__ScanLinkCandidates, Actor__ScanGridWindow, AcceptGridElem) when
 *    the move alone did not set linkTarget.
 *  - The link-command pair (Actor__OnActorLinkCommand,
 *    Actor__OnGridCellLinkCommand) forwarding through the SceneNode base
 *    table and, for an event in [5,9), the object's own tryAttachNearby.
 *  - Actor__SetLastOffsetValue/SetPendingExtra, GetActorMethods: plain
 *    setters/getter.
 *  - New_VariantSprite + VariantSprite__VariantSprite: allocator and
 *    constructor of an unrelated class, VariantSprite (a Sprite subclass,
 *    include/VariantSprite.h).
 *
 * No stalls: Actor__BuildLinkQueries, the last one, matched in round 75
 * (2-argument method call, see its report). No switch jump table in this slice, and no gp_rel/addiu_at/
 * nop_mflo_mfhi anywhere in it (all three are resolved toolchain
 * constructs anyway, CLAUDE.md "Open toolchain blockers").
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

/* Two-element s16 array -- Actor__MoveLocalX and Actor__MoveLocalY each write one
 * element (index 0 and 1 respectively) via a plain `sh` through a pointer
 * computed as %hi/%lo of `gActorLocalMove + 2*index`, so splat's single-word
 * dlabel is really this 2-element array, not a lone s32 (round 2026-09-04).
 * Not referenced anywhere else in the repo (checked with grep), so this is
 * this unit's own reading -- kept local rather than added to a shared
 * header. It is the x and y of Actor's local move vector: the z is the next
 * halfword, D_8008ABA8, which class_3bb8c_o's Actor__MoveLocalZ writes, and
 * SceneNode__RotateLocalVector reads src[0..2]. */
extern s16 gActorLocalMove[2];

void Actor__MoveLocalX(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &gActorLocalMove[0], val, notify, 7);
}

void Actor__MoveLocalY(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &gActorLocalMove[1], val, notify, 8);
}

/* `count` is `volatile` so it stays a stack reference reloaded at its one use
 * site, rather than being promoted to a callee-saved register across the
 * intervening Actor__AddLocalTranslation call -- confirmed with a standalone reproducer
 * through the pinned toolchain: dropping `volatile` grows the frame by one
 * callee-saved register (s3) and changes 0x1c/0x20 byte offsets throughout,
 * which is not what retail does (round 2026-09-04).
 *
 * `val` arrives as `s32` (its callers forward an incoming register with no
 * conversion -- typing it `s16` here made the CALLERS re-sign-extend it on
 * every call, which retail does not do), but the two stores below are
 * genuinely 16-bit (`sh`). Truncating once into a local `s16` and storing
 * THAT (rather than truncating `val` twice inline) is what reproduces
 * retail's callee-saved register assignment for `slot`/`extra`
 * (confirmed with a standalone reproducer: inline truncation swaps which
 * of s0/s1 holds which, round 2026-09-04). */
void Actor__MoveAlongLocalAxis(Actor *self, s16 *axis, s32 val, void *notify, volatile s32 event) {
    s16 val16 = (s16)val;
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

void Actor__MoveOrFindNearbyLink(Actor *self, void (*move)(Actor *, s32, void *), s32 val, void *notify) {
    self->linkTarget = NULL;
    move(self, val, notify);
    if (self->linkTarget == NULL) {
        Actor__FindNearbyLink(self);
    }
}

/* A 12-byte {s16,s16,s32,s32} query/result record. Built by this unit's
 * own Actor__BuildLinkQueries into caller-supplied buffers, and
 * walked as an array (stride 0xC) by Actor__ScanLinkCandidates. Describes a
 * rectangular window of the grid Actor__ScanGridWindow walks: `startCol`/
 * `startRow` is the window's origin bucket, `numCols`/`numRows` its extent --
 * confirmed directly from that function's own body (round 57 naming pass). */
typedef struct GridQuery {
    s16 startCol;
    s16 startRow;
    s32 numCols;
    s32 numRows;
} GridQuery;

/* The grid (self->grid) is a StageMap (include/StageMap.h). Its
 * elements (ChunkSlot) are what Actor__BuildLinkQueries collects: the
 * loader's headerReady gates Actor__ScanLinkCandidates, its ownerKey is the
 * element key BuildLinkQueries steps by one, and `cells` is the 20-wide grid
 * of GridCell cells (each with its `nextInCell` chain) Actor__ScanGridWindow
 * walks. */

void *AcceptGridElem(void *cell, void *offset, void *pos);
s32 Actor__BuildLinkQueries(Actor *self, GridQuery *queries, ChunkSlot **slots,
                            Descriptor10Ext *desc, s32 span);
void *Actor__ScanLinkCandidates(Actor *self, void *offset, void *pos, s32 count, GridQuery *queries,
                                ChunkSlot **slots);

s32 Actor__FindNearbyLink(Actor *self) {
    Descriptor10Ext desc;
    GridQuery queries[3];
    /* No known field needs this gap; empirically required to reproduce
     * retail's exact stack layout for slots/offset below (round 2026-09-04,
     * see this function's match report). */
    u8 pad48Tail[8];
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

s32 Actor__BuildLinkQueries(Actor *self, GridQuery *queries, ChunkSlot **slots,
                            Descriptor10Ext *desc, s32 span) {
    s32 cellCol = desc->base.b2;
    s32 cellRow = desc->base.b3;
    s32 numCols;
    s32 numRows;
    s32 col;
    s32 row;
    s32 count;

    numRows = !(span & 1) ? ++span : span;
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
        if (dims->isVertical != count) {
            return 1;
        }
        slotKey = slot->loader->elemKey;
        key = slotKey + 1;
        if (key < dims->rows) {
            slots[1] = map->methods->findSlotByNeighbour(map, key);
            count = 2;
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
    if (cellCol == 0x13) {
        numCols--;
    }
    if (cellRow == 0x13) {
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

/* Walks `count` entries of `arr1` (a `GridQuery[]`, stride 0xC) paired
 * element-for-element with `arr2` (a `ChunkSlot *[]`, stride 4),
 * skipping any entry whose element's loader does not have `headerReady`
 * set, and calling `Actor__ScanGridWindow` on the rest; returns the first
 * non-NULL result, or NULL if every entry was skipped or came back empty
 * (round 2026-09-04). */
void *Actor__ScanLinkCandidates(Actor *self, void *offset, void *pos, s32 count, GridQuery *queries,
                                ChunkSlot **slots) {
    s32 i;

    for (i = 0; i < count;) {
        ChunkSlot *slot = *slots;
        i++;
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

/* Scans a rectangular window of a grid of GridCell cell chains, rooted
 * at `source->cells`, `query->numRows` rows by `query->numCols` columns,
 * starting at row `query->startRow`, column `query->startCol` (each row is
 * 0x50 bytes = 20 bucket-head pointers; each column step is one bucket-head
 * pointer, 4 bytes). For each bucket, tries `AcceptGridElem` against the
 * head first, then each linked element in turn (`->nextInCell`), returning the
 * first one `AcceptGridElem` accepts (non-NULL); NULL if the whole window
 * comes up empty. `self` (this function's own first argument) is read
 * from `a0` in the disassembly but never touched by the body -- present
 * only to match its caller's calling convention (round 2026-09-04). */
void *Actor__ScanGridWindow(Actor *self, void *offset, void *pos, GridQuery *query, ChunkSlot *slot) {
    s32 row, col;
    GridCell **bucket;

    bucket = slot->cells + query->startRow * STAGE_CHUNK_CELLS + query->startCol;
    for (row = 0; row < query->numRows; row++) {
        for (col = 0; col < query->numCols; col++) {
            GridCell *node;

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

/* The real signature, established when code_d294_c matched this function in
 * round 57: it is a SceneNode method taking (self, out, target). This unit
 * had long declared it `(void)` and called it with no arguments, which is
 * byte-identical here only because arg0-arg2 are already in $a0-$a2 -- the
 * byte oracle cannot see a wrong prototype. Spelled out so the forwarding is
 * visible; verified byte-exact. */
extern s32 SceneNode__RaycastVertical(void *self, void *offset, void *pos);

void *AcceptGridElem(void *cell, void *offset, void *pos) {
    if (cell != NULL) {
        if (SceneNode__RaycastVertical(cell, offset, pos) != 0) {
            return cell;
        }
    }
    return NULL;
}

/* tryAttachNearby is called with (self, sender, event): SceneNode's slot
 * declares self alone (its occupant's second parameter arrives in the
 * caller's untouched $a1), and here both are reloaded after the base call,
 * so the call spells them out through a cast. */
void Actor__OnActorLinkCommand(Actor *self, void *sender, s32 event) {
    GetSceneNodeMethods()->dispatchLinkCommand((SceneNode *)self, sender, event);
    if (event < 9) {
        if (event >= 5) {
            ((void (*)(Actor *, void *, s32))self->methods->tryAttachNearby)(self, sender, event);
        }
    }
}

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

/* VariantSprite (include/VariantSprite.h, track 4, round 87): its allocator and
 * ctor. The other methods are in class_3bb8c_q.c and class_3bb8c_t.c. */
VariantSprite *New_VariantSprite(s32 variant, void *resetArg, void *texture) {
    void *obj = BMemPMgrAlloc(0xA8);
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

/* The base-class ctor, Sprite__Sprite, through GetSpriteMethods(), with
 * `self` upcast; then this class's table, and its reset slot
 * (VariantSprite__SetVariantClut) with the variant, through VariantSpriteResetFn.
 * The retail ctor ends in that call without setting $v0: it returns
 * nothing. */
void VariantSprite__VariantSprite(VariantSprite *self, s32 variant, void *resetArg, void *texture) {
    GetSpriteMethods()->ctor((Sprite *)self, texture, 0, &gVariantSpriteCells[variant], resetArg, 0);
    self->methods = GetVariantSpriteMethods();
    self->unkA4 = 0;
    ((VariantSpriteResetFn)self->methods->reset)(self, variant);
}
