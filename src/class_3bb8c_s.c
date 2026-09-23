/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * class_3bb8c_s -- functions 44..53 of the class_3bb8c remainder,
 * 0x46D20..0x475F0 (10 functions).  Carved round 21 (2026-09-06).
 *
 * Blocker census at carve time (four screens, canonical shell forms), fixed
 * up per the round 22 Gate 1 screen: `addiu_at` was RESOLVED in round 21 (see
 * docs/research/addiu-at-blocker.md) and is no longer a blocker at all -- the
 * 3 functions previously counted against it are workable, and their stub
 * reports were retired. Only `gp_rel` remains live in this unit, on
 * Class876FC__InitByKind, Class876FC__UpdateByKind and Class876FC__SpawnSprites -- each still carries a
 * current stall report and none of the three is touched here.
 *
 * MATCHED this round: Class876FC__ReleaseByKind, AddVec3, AttachWithRotScale,
 * Class876FC__PlaceModelChildren, Class876FC__ReleaseModelChildren -- byte-exact, see docs/match-reports/.
 *
 * Class876FC__BuildRandomSprites was attempted (the addiu_at hit in it is the now-resolved
 * construct, not a real blocker) and got to a one-instruction residue -- a
 * retail dead store (`li $a1, 1`) that never gets read on either branch it
 * precedes, the same "redundant move" class MATCHING-GUIDE.md already
 * documents as permuter territory. ~8000 permuter iterations did not find a
 * zero; restored to INCLUDE_ASM per the hard rule. See its match report.
 *
 * Class876FC__DriftModelChildren is still untouched (its only screen hit is the same
 * now-resolved addiu_at construct; it remains fresh ground).
 *
 * This slice spans (at least) parts of the same class as the neighbouring
 * `class_3bb8c_o` slice: `self` here is the SAME kind of node as that unit's
 * `LinkOwnerObj` (a 5-element `arr84` link array is confirmed via
 * Class876FC__SpawnSprites's occupancy of it), extended with more fields this unit
 * actually reads (+0x054 dispatch state, +0x064/+0x068/+0x06C/+0x070/+0x074/
 * +0x078 and a 2-element +0x07C array). Kept as this unit's OWN local view,
 * `LinkNode`/`LinkNodeMethods` -- `class_3bb8c_o.c`'s `LinkOwnerObj` is not
 * this unit's to edit, and per the multiple-independent-local-views
 * convention there is no reason a fresh view here should match its fields.
 * Confirmed with a straight read of this unit's own asm/nonmatchings .s files,
 * cross-checked against the (read-only, not edited) disassembly of the three
 * still-blocked functions in this same unit, which occupy the same self and
 * establish the +0x084 5-element array and the class_3bb8c_o.c call targets.
 */

#include "common.h"

/* ------------------------------------------------------------------ *
 * LinkNode is this unit's ONE local view of every object it touches: the
 * owner (a D_800876FC instance, 0x98 bytes -- the size func_80056320
 * allocates, which is exactly where `sprites` ends) and, for vtable
 * calls only, its children (`modelChildren`: BaseObjO, table D_800878D4;
 * `sprites`: D800879C4 objects). Class876FC is the same struct under the
 * owner's name, used in the owner's method signatures. Fields +0x058..
 * +0x07B are the 0x24-byte parameter block the class's slot +0x040
 * (func_800564A4, class_3bb8c_r.c) copies in whole.
 * ------------------------------------------------------------------ */

typedef struct LinkNode LinkNode;
typedef struct LinkNode Class876FC;

typedef struct Vec3S {
    s32 x, y, z;
} Vec3S;

/* Slot names follow the base implementation they dispatch to (resolved with
 * tools/classtable.py on D_800876FC / D_800878D4 / D_800879C4). `set` is
 * 1 = assign, 0 = accumulate; `data` is a triple of s16 num/den ratios
 * (RatioToFixed12). */
typedef struct LinkNodeMethods {
    u8 pad0[0x44];
    void (*updateRotation)(LinkNode *self, s32 set, s32 data);  /* +0x044, func_8001CEB4 (degrees; sprites: func_80042170, rotate) */
    void (*updateScale)(LinkNode *self, s32 set, void *data);   /* +0x048, func_8001D008 (GsCOORD2PARAM.scale; sprites: func_80057DF4) */
    void (*attachToParent)(LinkNode *self, void *parent, void *trans); /* +0x04C, func_8001D0EC (coord2 super = parent's, coord.t = trans) */
    u8 pad50[0x60 - 0x50];
    void (*setDisplay)(LinkNode *self, s32 on);                 /* +0x060, func_8001D344: GsDOFF = !on */
    void (*setSemiTrans)(LinkNode *self, s32 on);               /* +0x064, func_8001D374: GsALON */
    void (*setSemiTransRate)(LinkNode *self, s32 rate);         /* +0x068, func_8001D3A0: attribute bits 28-29 (GsAZERO..GsATHREE) */
    u8 pad6C[0xB8 - 0x6C];
    /* +0x0B8 is CLASS-DEPENDENT, so it keeps its placeholder: on the owner
     * and on modelChildren it is BaseObjO__SetVec14 (set translation); on
     * sprites it is func_8004229C, which copies three bytes into the
     * embedded GsSPRITE's r,g,b. */
    void (*slotB8)(LinkNode *self, void *arg1);                 /* +0x0B8 */
    void (*addTranslation)(LinkNode *self, void *delta);        /* +0x0BC, BaseObjO__AddVec14; only called on modelChildren */
} LinkNodeMethods;

struct LinkNode {
    LinkNodeMethods *methods;   /* +0x000 */
    u8 pad4[0x14 - 0x4];        /* +0x004 .. +0x013, unknown */
    s32 *coord2;                /* +0x014, the GsDOBJ2's GsCOORDINATE2 (code_d294.h);
                                   `*coord2 = 0` is its flg (recompute) */
    u8 pad18[0x20 - 0x18];      /* +0x018 .. +0x01F, unknown */
    s32 model;                  /* +0x020, the object Class6B5CC__LinkModel linked */
    s32 tick;                   /* +0x024, zeroed by slot +0x040, incremented once per
                                   slot +0x0EC update (func_800564F4) */
    u8 pad28[0x54 - 0x28];      /* +0x028 .. +0x053, unknown */
    s32 kind;                   /* +0x054, 0..3: New's first argument, stored by the ctor */
    Vec3S offset;               /* +0x058, added to the caller's position */
    s32 rotation;               /* +0x064, ratio triple for updateRotation */
    void *scale;                /* +0x068, ratio triple for updateScale; its first
                                   s16 (x numerator) also scales the child spacing */
    s32 modelChildLayout;       /* +0x06C, 0 = no modelChildren, else an index
                                   (1..4) into gModelChildSpacing: 1-2 space along x,
                                   3-4 along y */
    s32 tableIndex;             /* +0x070, index into gModelChildDriftZ and gSpriteShiftX */
    void *color;                /* +0x074, passed to every sprite's slotB8 (RGB) */
    void *altColor;             /* +0x078, sprites[1]'s colour instead, when non-NULL */
    LinkNode *modelChildren[2]; /* +0x07C..+0x083, BaseObjO children */
    LinkNode *sprites[5];       /* +0x084..+0x097, D800879C4 children; class_3bb8c_o.c's
                                   LinkOwnerObj::links is the same array */
};

extern void LinkOwnerObj__ReleaseLinks(void *self);   /* class_3bb8c_o.c, LinkOwnerObj* */
extern void LinkOwnerObj__ReleaseLinksB(void *self);   /* class_3bb8c_o.c, LinkOwnerObj* */
extern void BaseObjO__AddVec14(void *self, Vec3S *v);   /* class_3bb8c_t.c */
extern void Class876FC__SpawnSprites(void *self, s32 a1, s32 a2, void *tbl); /* below */
extern s32 gSpriteShiftX[];
extern s32 gSpriteScaleLarge[];
extern s32 gSpriteScaleHalf[];
extern s32 gSpriteScaleSmall[];
extern Vec3S gSpriteShiftScratch;

void Class876FC__ReleaseModelChildren(Class876FC *self);
void Class876FC__PlaceModelChildren(Class876FC *self, s32 reuse);
void AddVec3(Vec3S *dst, Vec3S *a, Vec3S *b);
void AttachWithRotScale(LinkNode *node, void *parent, void *trans, s32 rotation, void *scale);
extern void Class6B5CC__LinkModel(void *self, s32 arg); /* established, code_55dd4.h */

/* NoOpIgnoreArgs/LinkOwnerObj__func_56e1c/LinkOwnerObj__RandomizeLinks are defined in class_3bb8c_o.c
 * (own their addresses, own local view "LinkOwnerObj"/"LinkElemObj") with
 * signatures of 0 or 1 pointer argument. Every call site in THIS unit still
 * sets up a second argument register that those bodies never read (retail's
 * own caller-side view evidently did not carry a narrower prototype either)
 * -- so these are declared here the same old-style (unprototyped) way,
 * which is what lets this file's calls pass the extra dead argument without
 * a parameter-count mismatch against their real, narrower definitions
 * elsewhere. Class876FC__BuildRandomSprites is this same idiom but for a function defined
 * later IN THIS FILE (Class876FC__InitByKind calls it, ROM-earlier than its own
 * definition). Class876FC__DriftModelChildren is likewise defined later in this file, and
 * Class876FC__UpdateByKind forwards a dead second argument to it the same way. */
extern void NoOpIgnoreArgs();
extern void LinkOwnerObj__func_56e1c(); /* arity-ok: definition is 1-parameter and the body WRITES $a1/$a2/$a3 to zero before any read, but Class876FC__InitByKind's dead 2nd argument is byte-load-bearing -- retail emits `move a1,zero` at 0x80056624 */
extern void LinkOwnerObj__RandomizeLinks(); /* arity-ok: definition is 1-parameter and the body reads only $a0 (`addiu s0,a0,136`), but Class876FC__UpdateByKind's dead 2nd argument is byte-load-bearing -- retail emits `move a1,s1` at 0x800566FC */
extern void Class876FC__BuildRandomSprites(); /* arity-ok: the definition is 1-parameter and LIVES IN THIS FILE (below, ROM-later), the body reading only $a0 (`move s1,a0`); Class876FC__InitByKind's dead 2nd argument is byte-load-bearing -- retail emits `move a1,zero` at 0x80056614 */
extern void Class876FC__DriftModelChildren();

/* class_3bb8c_p.c; fully prototyped since every call site here uses all
 * three arguments for real. */
extern void *New_D800879C4(void *arg1, void *arg2, void *arg3);

/* Three globals a class_3bb8c_o.c ctor-shaped function (BaseObjO__func_56f5c)
 * captures once from its own three pointer-typed parameters -- D_8008ACA4 is
 * some other object (first field a methods pointer, called through a new
 * +0x080 slot below), D_8008ACA8 is forwarded opaquely to New_D800879C4 as
 * its own third argument, and D_8008ACAC's pointee has a lookup field at
 * +0x018 that Class876FC__InitByKind/Class876FC__UpdateByKind snapshot/diff via gTrackedYSnapshot. */
typedef struct {
    u8 pad0[0x80];
    s32 (*slot80)(void *self, s32 arg);
} D_8008ACA4Methods;
typedef struct {
    D_8008ACA4Methods *methods;
} D_8008ACA4Obj;
extern D_8008ACA4Obj *D_8008ACA4;
extern void *D_8008ACA8;
extern void *D_8008ACAC;
extern s32 gTrackedYSnapshot;
extern s32 D_8008AB98[];

/* Called once, from the class's ctor (func_800563C0): place self under
 * `parent` at pos + offset, then build the per-kind parts. Kinds 0 and 1
 * link a model fetched from D_8008ACA4 by D_8008AB98[kind]; kind 0 also
 * gets two model children, kind 2 five randomised sprites, kind 3 five
 * plain sprites (LinkOwnerObj__func_56e1c is Class876FC__SpawnSprites(self,
 * 0, 0, NULL)). */
void Class876FC__InitByKind(Class876FC *self, void *parent, Vec3S *pos) {
    Vec3S local;
    s32 state;

    gTrackedYSnapshot = *(s32 *)((u8 *)D_8008ACAC + 0x18);
    AddVec3(&local, pos, &self->offset);
    AttachWithRotScale(self, parent, &local, self->rotation, self->scale);

    state = self->kind;
    if (state < 2) {
        s32 ret = D_8008ACA4->methods->slot80(D_8008ACA4, D_8008AB98[state]);
        Class6B5CC__LinkModel(self, ret);
        state = self->kind;
    }

    switch (state) {
    case 0:
        Class876FC__PlaceModelChildren(self, 0);
        break;
    case 2:
        Class876FC__BuildRandomSprites(self, 0);
        break;
    case 3:
        LinkOwnerObj__func_56e1c(self, 0);
        break;
    default:
        break;
    }
}

/* Called every frame from the class's slot +0x0EC (func_800564F4, right
 * after it increments `tick`): set self's translation (owner's slotB8 is
 * BaseObjO__SetVec14) to pos + offset, plus however far D_8008ACAC's +0x018
 * word has moved since Class876FC__InitByKind snapshotted it, then run the
 * per-kind update. */
void Class876FC__UpdateByKind(Class876FC *self, void *pos) {
    Vec3S local;

    AddVec3(&local, (Vec3S *)pos, &self->offset);
    local.y += *(s32 *)((u8 *)D_8008ACAC + 0x18) - gTrackedYSnapshot;
    self->methods->slotB8(self, &local);

    switch (self->kind) {
    case 0:
        Class876FC__DriftModelChildren(self, pos);
        break;
    case 2:
        NoOpIgnoreArgs(self, pos);
        break;
    case 3:
        LinkOwnerObj__RandomizeLinks(self, pos);
        break;
    default:
        break;
    }
}

/* Called from the class's dtor (func_80056464): release whichever child
 * array this kind built (kinds 2 and 3 both release `sprites`, through two
 * identical class_3bb8c_o.c functions). */
void Class876FC__ReleaseByKind(Class876FC *self) {
    switch (self->kind) {
    case 0:
        Class876FC__ReleaseModelChildren(self);
        break;
    case 2:
        LinkOwnerObj__ReleaseLinks(self);
        break;
    case 3:
        LinkOwnerObj__ReleaseLinksB(self);
        break;
    default:
        break;
    }
}

/* Plain Vec3 add: dst = a + b. Frameless -- no self/vtable involved. */
void AddVec3(Vec3S *dst, Vec3S *a, Vec3S *b) {
    dst->x = a->x + b->x;
    dst->y = a->y + b->y;
    dst->z = a->z + b->z;
}

/* Attach `node` under `parent` at translation `trans`, then assign (set = 1)
 * its rotation and scale. Used on the owner itself and on each model child. */
void AttachWithRotScale(LinkNode *node, void *parent, void *trans, s32 rotation, void *scale) {
    node->methods->attachToParent(node, parent, trans);
    node->methods->updateRotation(node, 1, rotation);
    node->methods->updateScale(node, 1, scale);
}

/* Lay the two model children out in a row: child i sits at (i + 1) *
 * gModelChildSpacing[modelChildLayout], along x (scaled by scale's x
 * numerator) for layouts 1-2 and along y for 3-4. reuse = 0 creates them
 * (New_BaseObjO, sharing the owner's model, attached to the owner);
 * reuse = 1 only resets their translation (their slotB8 is
 * BaseObjO__SetVec14). */
extern void *New_BaseObjO(void);        /* class_3bb8c_o.c, New_X allocator */
extern void Class6B5CC__LinkModel(void *self, s32 arg); /* established, code_55dd4.h */
extern Vec3S gModelChildOffsetInit;
extern s32 gModelChildSpacing[];

void Class876FC__PlaceModelChildren(Class876FC *self, s32 reuse) {
    Vec3S accum;
    LinkNode **p;
    s32 i;
    s32 count = self->modelChildLayout;

    if (count == 0) {
        return;
    }
    accum = gModelChildOffsetInit;
    p = self->modelChildren;
    for (i = 0; i < 2; i++, p++) {
        if (count < 3) {
            accum.x += *(s16 *)self->scale * gModelChildSpacing[count];
        } else {
            accum.y += gModelChildSpacing[count];
        }
        if (reuse) {
            LinkNode *child = *p;
            child->methods->slotB8(child, &accum);
        } else {
            LinkNode *child = New_BaseObjO();
            *p = child;
            Class6B5CC__LinkModel(child, self->model);
            AttachWithRotScale(*p, self, &accum, self->rotation, self->scale);
        }
    }
}

/* Per-`tableIndex` z step for the model children (0 = no drift), also the
 * divisor of the 24500 / step reset period below. Same index space as
 * gSpriteShiftX. */
extern s32 gModelChildDriftZ[];
/* All-zero Vec3S, the start value of each child's per-frame z delta. */
extern Vec3S gModelChildDriftInit;
/* Ratio triple {0/1, 1/10, 0/1}: the per-frame rotation increment
 * updateRotation(.., 0, ..) adds to self and to each model child. */
extern s32 gSpinRotStep[];

#if 0
/* After 500 frames (tick >= 0x1F5), for kinds with model children and a
 * nonzero gModelChildDriftZ step: spin self and both children, move the
 * children along z, and every 24500 / step frames snap them back to their
 * layout. Always marks self's coord2 for recompute.
 *
 * round 44 (2026-09-15): best-reached body, 117/121 words, NOT byte-exact.
 * See docs/match-reports/Class876FC__DriftModelChildren.md for the residue and what was
 * tried. Kept here per the hard rule -- restore this ahead of any future
 * attempt rather than re-deriving from scratch. */
void Class876FC__DriftModelChildren(Class876FC *self)
{
    s32 idx;
    s32 *tab70;
    s32 *tab70b;
    s32 accumOffset;
    s32 divq;
    s32 modend;
    LinkNode **p;
    s32 i;

    idx = self->tableIndex;
    if (self->modelChildLayout != 0) {
        tab70 = &gModelChildDriftZ[idx];
        if (*tab70 != 0 && (u32) self->tick >= 0x1F5) {
            p = self->modelChildren;
            self->methods->updateRotation(self, 0, (s32) gSpinRotStep);

            i = 0;
            tab70b = tab70;
            accumOffset = 0;
            for (; i < 2; i++) {
                Vec3S local = gModelChildDriftInit;
                local.z += accumOffset + *tab70b;
                (*p)->methods->addTranslation(*p, &local);
                accumOffset += 3;
                (*p)->methods->updateRotation(*p, 0, (s32) gSpinRotStep);
                p++;
            }

            divq = 24500 / gModelChildDriftZ[idx];
            modend = self->tick;
            if (divq >= 0) {
                if ((u32) modend % (u32) divq == 0) {
                    Class876FC__PlaceModelChildren(self, 1);
                }
            } else {
                u32 adivq = ~divq + 1;
                if ((u32) modend % adivq == 0) {
                    Class876FC__PlaceModelChildren(self, 1);
                }
            }
        }
    }
    *self->coord2 = 0;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_s", Class876FC__DriftModelChildren);

extern void ReleaseBasicClassArray(void **array, s32 count);

/* Release the two model children, if this layout made any. */
void Class876FC__ReleaseModelChildren(Class876FC *self) {
    if (self->modelChildLayout != 0) {
        ReleaseBasicClassArray((void **)self->modelChildren, 2);
    }
}

/* Kind 2's init: five sprites, all scaled by gSpriteScaleHalf on an even
 * rand(); then sprites[1] is either shifted along x by
 * gSpriteShiftX[tableIndex] and recoloured (tableIndex >= 2) or made
 * semi-transparent (rate 0) and rescaled, and sprites[2] is hidden. */
void Class876FC__BuildRandomSprites(Class876FC *self) {
    s32 parity = rand() % 2;
    void *tblOrNull = parity ? NULL : gSpriteScaleHalf;
    LinkNode *child;
    void *arg;

    Class876FC__SpawnSprites(self, 0, 0, tblOrNull);

    if (self->tableIndex >= 2) {
        LinkNodeMethods *m;

        child = self->sprites[1];
        gSpriteShiftScratch.x = gSpriteShiftX[self->tableIndex];
        BaseObjO__AddVec14(child, &gSpriteShiftScratch);
        m = child->methods;
        arg = (self->altColor != NULL) ? self->altColor : self->color;
        m->slotB8(child, arg);
    } else {
        child = self->sprites[1];
        child->methods->setSemiTrans(child, 1);
        child->methods->setSemiTransRate(child, 0);
        child->methods->updateScale(child, 1, (parity != 0) ? gSpriteScaleLarge : gSpriteScaleSmall);
    }

    self->sprites[2]->methods->setDisplay(self->sprites[2], 0);
}

/* Create the five sprites (New_D800879C4), attach each to self at no offset,
 * give each self's colour (a sprite's slotB8 sets GsSPRITE r,g,b), and
 * assign `tbl` as their scale when non-NULL. `self` stays `void *`: it is
 * the prototype class_3bb8c_o.c calls through, and a typed local alias of
 * it costs a callee-saved register (see this function's report). */
void Class876FC__SpawnSprites(void *self, s32 a1, s32 a2, void *tbl) {
    LinkNode **p = (LinkNode **)((u8 *)self + 0x84);
    LinkNode *node;
    s32 i;

    for (i = 0; i < 5; i++, p++) {
        node = New_D800879C4((void *)a2, 0, D_8008ACA8);
        *p = node;
        node->methods->attachToParent(node, self, 0);
        (*p)->methods->slotB8(*p, ((LinkNode *)self)->color);
        if (tbl != 0) {
            (*p)->methods->updateScale(*p, 1, tbl);
        }
    }
}
