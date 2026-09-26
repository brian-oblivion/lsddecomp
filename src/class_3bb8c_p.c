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
 *    component of the local move vector D_8008ABA4, apply it through
 *    addLocalTranslation, clear it, and fall back to a grid-based
 *    nearby-link search (Actor__FindNearbyLink, Actor__BuildLinkQueries,
 *    Actor__ScanLinkCandidates, Actor__ScanGridWindow, AcceptGridElem) when
 *    the move alone did not set linkTarget.
 *  - The link-command pair (Actor__OnActorLinkCommand,
 *    Actor__OnGridCellLinkCommand) forwarding through the SceneNode base
 *    table and, for an event in [5,9), the object's own tryAttachNearby.
 *  - Actor__SetLastOffsetValue/SetPendingExtra, GetActorMethods: plain
 *    setters/getter.
 *  - New_Class879C4 + Class879C4__Class879C4: allocator and constructor of
 *    an unrelated class, Class879C4 (a Sprite subclass, include/Class879C4.h).
 *
 * No stalls: Actor__BuildLinkQueries, the last one, matched in round 75
 * (2-argument method call, see its report). No switch jump table in this slice, and no gp_rel/addiu_at/
 * nop_mflo_mfhi anywhere in it (all three are resolved toolchain
 * constructs anyway, CLAUDE.md "Open toolchain blockers").
 */
#include "common.h"
#include "Actor.h"
#include "DreamSys.h"
#include "StageMap.h"
#include "LbdFile.h"
#include "GridCell.h"
#include "Class879C4.h"

/* Two-element s16 array -- Actor__MoveLocalX and Actor__MoveLocalY each write one
 * element (index 0 and 1 respectively) via a plain `sh` through a pointer
 * computed as %hi/%lo of `D_8008ABA4 + 2*index`, so splat's single-word
 * dlabel is really this 2-element array, not a lone s32 (round 2026-09-04).
 * Not referenced anywhere else in the repo (checked with grep), so this is
 * this unit's own reading -- kept local rather than added to a shared
 * header. It is the x and y of Actor's local move vector: the z is the next
 * halfword, D_8008ABA8, which class_3bb8c_o's Actor__MoveLocalZ writes, and
 * SceneNode__RotateLocalVector reads src[0..2]. */
extern s16 D_8008ABA4[2];

void Actor__MoveLocalX(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &D_8008ABA4[0], val, notify, 7);
}

void Actor__MoveLocalY(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &D_8008ABA4[1], val, notify, 8);
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
    self->methods->addLocalTranslation(self, &D_8008ABA4[0]);
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

/* Output buffer filled in by the grid's computeFootprintDescriptor (a
 * Descriptor10Ext, include/StageMap.h: queryCol/queryRow are base.b2/b3,
 * source is unk24; this view is 0x30 bytes, and the frame needs it) and read back by this unit's own Actor__BuildLinkQueries.
 * Only the three fields actually touched are named. `queryCol`/`queryRow`
 * feed straight into GridQuery::startCol/startRow (Actor__BuildLinkQueries,
 * matched round 75; round 57 naming pass). */
typedef struct LinkQueryBuf {
    u8 pad00[0x2];
    s8 queryCol;
    s8 queryRow;
    u8 pad04[0x24 - 0x4];
    ChunkSlot *source;
    u8 pad28[0x30 - 0x28];
} LinkQueryBuf;

void *AcceptGridElem(void *arg0, void *arg1, void *arg2);
s32 Actor__BuildLinkQueries(Actor *self, GridQuery *arr1, ChunkSlot **arr2, LinkQueryBuf *arg3,
                            s32 arg4);
void *Actor__ScanLinkCandidates(Actor *self, void *arg1, void *arg2, s32 count, GridQuery *arr1,
                                ChunkSlot **arr2);

s32 Actor__FindNearbyLink(Actor *self) {
    LinkQueryBuf sp18;
    GridQuery sp48[3];
    /* No known field needs this gap; empirically required to reproduce
     * retail's exact stack layout for sp78/sp88 below (round 2026-09-04,
     * see this function's match report). */
    u8 pad48Tail[8];
    ChunkSlot *sp78[3];
    LongVec3 sp88;

    if (self->grid != NULL) {
        void *pos = &self->coord2->tx;

        if (self->grid->methods->computeFootprintDescriptor(self->grid, (Descriptor10Ext *)&sp18,
                                                            pos) == 0) {
            s32 count = Actor__BuildLinkQueries(self, sp48, sp78, &sp18, 1);
            void *result = Actor__ScanLinkCandidates(self, &sp88, pos, count, sp48, sp78);

            self->linkTarget = result;
            if (result != NULL) {
                self->methods->addTranslation(self, &sp88);
                self->methods->notifyWithHull(self, -1);
                return 1;
            }
            self->methods->notifyWithHull(self, -2);
            return 0;
        }
    }
    return 0;
}

s32 Actor__BuildLinkQueries(Actor *self, GridQuery *arr1, ChunkSlot **arr2, LinkQueryBuf *arg3,
                            s32 arg4) {
    s32 f2 = arg3->queryCol;
    s32 f3 = arg3->queryRow;
    s32 numCols;
    s32 numRows;
    s32 col;
    s32 row;
    s32 idx;

    numRows = !(arg4 & 1) ? ++arg4 : arg4;
    numCols = numRows;
    col = f2;
    row = f3;
    idx = 1;
    if (arg4 == 1) {
        StageMap *unk4C;
        StageGridDimensions *unk68;
        ChunkSlot *src;
        s16 s3;
        s32 pos;

        arr1[0].startCol = col;
        arr1[0].startRow = row;
        arr1[0].numCols = numRows;
        arr1[0].numRows = numRows;
        src = arg3->source;
        arr2[0] = src;
        unk4C = self->grid;
        unk68 = unk4C->config;
        if (unk68->isVertical != idx) {
            return 1;
        }
        s3 = src->loader->elemKey;
        pos = s3 + 1;
        if (pos < unk68->rows) {
            arr2[1] = unk4C->methods->findElemByUnk32(unk4C, pos);
            idx = 2;
            arr1[1] = arr1[0];
        }
        pos = s3 - 1;
        if (pos >= 0) {
            arr2[idx] = unk4C->methods->findElemByUnk32(unk4C, pos);
            arr1[idx] = arr1[0];
            idx++;
        }
        return idx;
    }

    if (col == 0) {
        numCols = numRows - 1;
    } else {
        col--;
    }
    if (f2 == 0x13) {
        numCols--;
    }
    if (f3 == 0x13) {
        numRows--;
    } else {
        row++;
    }
    arr1[0].startCol = col;
    if (f3 == 0) {
        numRows--;
    }
    arr1[0].startRow = row;
    arr1[0].numCols = numCols;
    arr1[0].numRows = numRows;
    arr2[0] = arg3->source;
    return 1;
}

void *Actor__ScanGridWindow(Actor *self, void *arg1, void *arg2, GridQuery *query, ChunkSlot *source);

/* Walks `count` entries of `arr1` (a `GridQuery[]`, stride 0xC) paired
 * element-for-element with `arr2` (a `ChunkSlot *[]`, stride 4),
 * skipping any entry whose element's loader does not have `headerReady`
 * set, and calling `Actor__ScanGridWindow` on the rest; returns the first
 * non-NULL result, or NULL if every entry was skipped or came back empty
 * (round 2026-09-04). */
void *Actor__ScanLinkCandidates(Actor *self, void *arg1, void *arg2, s32 count, GridQuery *arr1,
                                ChunkSlot **arr2) {
    s32 i;

    for (i = 0; i < count;) {
        ChunkSlot *elem = *arr2;
        i++;
        if (elem->loader->headerReady != 0) {
            void *result = Actor__ScanGridWindow(self, arg1, arg2, arr1, elem);
            if (result != NULL) {
                return result;
            }
        }
        arr1 = (GridQuery *)((u8 *)arr1 + 0xC);
        arr2++;
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
void *Actor__ScanGridWindow(Actor *self, void *arg1, void *arg2, GridQuery *query,
                            ChunkSlot *source) {
    s32 row, col;
    GridCell **bucket;

    bucket = (GridCell **)((u8 *)source->cells + query->startRow * 0x50 + query->startCol * 4);
    for (row = 0; row < query->numRows; row++) {
        for (col = 0; col < query->numCols; col++) {
            GridCell *node;

            if (AcceptGridElem(*bucket, arg1, arg2) != NULL) {
                return *bucket;
            }
            for (node = (*bucket)->nextInCell; node != NULL; node = node->nextInCell) {
                if (AcceptGridElem(node, arg1, arg2) != NULL) {
                    return node;
                }
            }
            bucket++;
        }
        bucket = (GridCell **)((u8 *)bucket - (query->numCols * 4 + 0x50));
    }
    return NULL;
}

/* The real signature, established when code_d294_c matched this function in
 * round 57: it is a SceneNode method taking (self, out, target). This unit
 * had long declared it `(void)` and called it with no arguments, which is
 * byte-identical here only because arg0-arg2 are already in $a0-$a2 -- the
 * byte oracle cannot see a wrong prototype. Spelled out so the forwarding is
 * visible; verified byte-exact. */
extern s32 SceneNode__RaycastVertical(void *self, void *out, void *target);

void *AcceptGridElem(void *arg0, void *arg1, void *arg2) {
    if (arg0 != NULL) {
        if (SceneNode__RaycastVertical(arg0, arg1, arg2) != 0) {
            return arg0;
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

/* Class879C4 (include/Class879C4.h, track 4, round 87): its allocator and
 * ctor. The other methods are in class_3bb8c_q.c and class_3bb8c_t.c. */
Class879C4 *New_Class879C4(s32 variant, void *arg2, void *texture) {
    void *obj = BMemPMgrAlloc(0xA8);
    if (obj != NULL) {
        GetClass879C4Methods()->ctor(obj, variant, arg2, texture);
        return obj;
    }
    return NULL;
}

/* Class879C4's two texture cells, forwarded as the Sprite ctor's `rect`
 * (Sprite__Reset copies it into Sprite.rect): u,v = (0x00,0x20) and
 * (0x10,0x20), 16x16. */
extern SpriteRect gClass879C4Cells[2];

/* The base-class ctor, Sprite__Sprite, through GetSpriteMethods(), with
 * `self` upcast; then this class's table, and its reset slot
 * (Class879C4__SetVariantClut) with the variant, through Class879C4ResetFn.
 * The retail ctor ends in that call without setting $v0: it returns
 * nothing. */
void Class879C4__Class879C4(Class879C4 *self, s32 variant, void *arg2, void *texture) {
    GetSpriteMethods()->ctor((Sprite *)self, texture, 0, &gClass879C4Cells[variant], arg2, 0);
    self->methods = GetClass879C4Methods();
    self->unkA4 = 0;
    ((Class879C4ResetFn)self->methods->reset)(self, variant);
}
