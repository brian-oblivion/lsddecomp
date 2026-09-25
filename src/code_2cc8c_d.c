#include "common.h"
#include "code_2cc8c.h"

/*
 * The rest of `Unk18Obj`'s own vtable (`D_8006E8E4`): the constructor chain
 * itself (`Unk18Obj__Unk18Obj`, `Unk18Obj__Finalize`, `Unk18Obj__AddChild`/
 * `Unk18Obj__RemoveChild`) lives in the sibling unit `code_2cc8c_c.c`; this
 * unit carves everything after that -- the `OnNotify` override and its two
 * tag-dispatched handlers, the plain field setters/getters, the paired
 * "view child" attach/detach (registers a `GenericObj` child and its
 * position, then hands it to Sony's `GsSetRefView2`), the paired OT
 * (ordering-table) init/deinit that allocates a double-buffered `GsOT` pair
 * for `GsClearOt`, and the two per-frame methods that drive them:
 * `Unk18Obj__Update` (light mode, fog, ref view, both OT halves, notifies
 * children) and `Unk18Obj__Flip` (drains the current OT half via
 * `GsSortClear`+`GsDrawOt` and toggles the double-buffer index for next
 * frame). In short: `Unk18Obj` is the scene's GPU-facing viewport/renderer
 * object -- IntermediateBase::viewport's pointee (TaskCore's, `code_2c054.c`) -- and this
 * unit is its rendering half. NAMING PARKED: the `Unk18Obj` type name itself,
 * several fields shared with `code_2cc8c_c.c` (unkC/unk10/unk30/unkAC/unkB0),
 * and the OT-internals field cluster (unk3C/unk44/unk48/unk78/unk7C/unk80/
 * unk84/unk88/unk8C/unk90/unk98) are all PROPOSED, not renamed here -- see
 * each function's own `## Proposed field names` and the round-73 broadcast.
 */

/* Forwards to the inherited BasicClass slot38, then dispatches self's OWN
 * slot94 or slot98 depending on arg1's dynamic class tag (5 or 1
 * respectively, per its header nibble -- same tag idiom as Unk18Obj__AddChild,
 * round 13). Neither dispatch happens for any other tag. */
void Unk18Obj__OnNotify(Unk18Obj *self, GenericObj *arg1, s32 arg2) {
    s32 tag;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, arg1, arg2);

    tag = arg1->methods->header & 0xF;
    if (tag == 5) {
        self->methods->slot94(self, arg1, arg2);
    } else if (tag == 1) {
        self->methods->slot98(self, arg1, arg2);
    }
}

/* MATCHED round 49. Two levers were needed, see docs/match-reports/Unk18Obj__InitDefaults.md:
 * (1) retail reloads the address of `D_8008A8F8` INDEPENDENTLY for each of
 * the two whole-struct copies (two separate lui/addiu pairs); GCC 2.6.3
 * otherwise CSEs that into one shared computation. Declaring a second
 * extern name aliased to the same symbol via `__asm__("D_8008A8F8")` (the
 * same alternate-name idiom `code_179d8_m.c` already uses) gives the
 * second copy a textually distinct symbol, defeating the CSE without
 * `volatile`'s much worse codegen. (2) Two bare `__asm__("")` scheduling
 * barriers pin the two independent zero-inits and the two global-loaded
 * stores to retail's own early positions instead of letting the scheduler
 * defer them to just before the byte copies. */
extern s32 D_8008A8FC;
extern s32 D_8008A900;
extern SByte3_d294 D_8008A8F8;
extern SByte3_d294 D_8008A8F8_b __asm__("D_8008A8F8");

void Unk18Obj__InitDefaults(Unk18Obj *self) {
    self->unk90 = 0;
    self->otReady = 0;
    __asm__("");
    self->unk34.a = D_8008A8FC;
    self->unk34.b = D_8008A900;
    __asm__("");
    self->unk3C = 0xD;
    self->unk44 = 0x7D0;
    self->unk48 = 0x40;
    self->unk40 = 0x100;
    self->unk4C = 0xA;
    self->unk50 = 0x10000;
    self->lightMode = 0;
    self->fogNear = 0x4E20;
    self->farColor = D_8008A8F8;
    self->clearColor = D_8008A8F8_b;
    self->unkB4 = 0;
    self->unkB8 = 1;
}

void Unk18Obj__SetUnk34(Unk18Obj *self, Pair32_d294 *pair) {
    self->unk34 = *pair;
}

void Unk18Obj__SetUnk3C(Unk18Obj *self, s32 a1) {
    self->unk3C = a1;
}

/* Only writes unk44 the first time (guarded by the otReady latch). */
void Unk18Obj__SetUnk44(Unk18Obj *self, s32 a1) {
    if (self->otReady == 0) {
        self->unk44 = a1;
    }
}

/* Same guard as Unk18Obj__SetUnk44, writes unk48 instead. */
void Unk18Obj__SetUnk48(Unk18Obj *self, s32 a1) {
    if (self->otReady == 0) {
        self->unk48 = a1;
    }
}

void Unk18Obj__SetUnk40(Unk18Obj *self, s32 a1) {
    self->unk40 = a1;
}

void func_8003EA6C(void) {
}

void func_8003EA74(void) {
}

void Unk18Obj__SetLightMode(Unk18Obj *self, s32 a1) {
    self->lightMode = a1;
}

void Unk18Obj__SetClearColor(Unk18Obj *self, SByte3_d294 *src) {
    self->clearColor = *src;
}

void Unk18Obj__SetFarColor(Unk18Obj *self, SByte3_d294 *src) {
    self->farColor = *src;
}

void Unk18Obj__SetFogNear(Unk18Obj *self, s32 a1) {
    self->fogNear = a1;
}

/* GsSetRefView2 is Sony's (`libgs/gs_131.o`, linked from the SDK object).
   Declared LOCALLY rather than in include/code_2cc8c.h, which six units
   include: the real `LIBGS.H` prototype for this name will collide there.
   This is only the shape THIS unit's call sites use -- the real one takes a
   GsRVIEW2*. */
extern void GsSetRefView2(void *arg0);

/* One-time init, guarded by self->unk10: registers `a1` as a child (via
 * the inherited BasicClass "addChild" slot10), dispatches slot78/slot7C
 * with a2/a3, dispatches slot80 with arg5 (or a default, D_8008A8F4, when
 * arg5 is NULL), then hands &self->unk14 to GsSetRefView2. Does nothing at
 * all once self->unk10 is already set. */
void Unk18Obj__AttachViewChild(Unk18Obj *self, void *a1, void *a2, void *a3, void *arg5) {
    Unk18ObjMethods *m = self->methods;

    if (self->unk10 != NULL) {
        return;
    }
    m->addChild(self, a1);
    m->slot78(self, a2);
    m->slot7C(self, a3);
    m->slot80(self, arg5 != NULL ? arg5 : D_8008A8F4);
    GsSetRefView2(&self->unk14);
}

/* Teardown counterpart to Unk18Obj__AttachViewChild's init: removes self->unk10 as a
 * child (inherited BasicClass "removeChild") if it was ever set. */
void Unk18Obj__DetachViewChild(Unk18Obj *self) {
    if (self->unk10 != NULL) {
        self->methods->removeChild(self, self->unk10);
    }
}

/* Copies a1 wholesale into self->unk14, but only when self->unk10 is set. */
void Unk18Obj__SetViewPos(Unk18Obj *self, Vec3_2cc8c *a1) {
    if (self->unk10 != NULL) {
        self->unk14 = *a1;
    }
}

/* Sibling of Unk18Obj__SetViewPos: copies a1 wholesale into self->unk20, guarded
 * by the same self->unk10 flag. */
void Unk18Obj__SetUnk20(Unk18Obj *self, Vec3_2cc8c *a1) {
    if (self->unk10 != NULL) {
        self->unk20 = *a1;
    }
}

void Unk18Obj__SetRatio12(Unk18Obj *self, s16 *pair) {
    s32 q1, r1, q2;

    if (self->unk10 != NULL) {
        q1 = pair[0] / pair[1];
        r1 = pair[0] % pair[1];
        q2 = (r1 << 12) / pair[1];
        self->unk2C = (q1 << 12) + q2;
    }
}

void func_8003ECC0(void) {
}

void func_8003ECC8(void) {
}

/* One-time allocation of this object's two ordering tables (see
 * docs/match-reports/Unk18Obj__InitOt.md). Each half of the buffer is a
 * 0x14-byte GsOT header, 4 << unk3C bytes of OT tags, then unk48 * unk44
 * bytes of packet area. The header size is a LOCAL on purpose: written as a
 * literal, fold() reassociates the constant to the outside of the sum and
 * the final addu/addiu pair swaps (round 71). */
extern void GsClearOt(s32 a0, s32 a1, s32 a2);
/* Two more, identified in round 78 (FINISHING-PLAN track 2) and moved here
 * from include/code_2cc8c.h. Both prototypes are LIBGS.H's own; PACKET is
 * LIBGS.H's `typedef unsigned char PACKET`.
 *   GsSetNearClip   libgs/gs_101   was func_8003FB0C
 *   GsSetWorkBase   libgs/gs_124   was func_8003FBE4
 * And one identified in round 79, also LIBGS.H's own prototype:
 *   GsSetProjection libgs/gs_106   was Unk18Obj__SetGeomScreen (its argument
 *                                  is the projection distance h, which this
 *                                  unit also passes as SetFogNear's h) */
extern void GsSetNearClip(long clip_near);
extern void GsSetWorkBase(unsigned char *outpacketp);
extern void GsSetProjection(long h);
extern void *BMemPMgrAlloc(s32 size);

void Unk18Obj__InitOt(Unk18Obj *self) {
    s32 size;
    s32 buf;
    s32 hdrSize = 0x14; /* sizeof(GsOT) */

    if (self->otReady != 0) {
        return;
    }

    size = (4 << self->unk3C) + (self->unk48 * self->unk44 + hdrSize);

    buf = (s32)BMemPMgrAlloc(size * 2);
    if (buf == 0) {
        return;
    }

    self->unk78 = buf;
    self->unk80 = buf + 0x14;
    self->unk88 = (4 << self->unk3C) + self->unk80;

    self->unk7C = size + self->unk78;
    self->unk84 = size + self->unk80;
    self->unk8C = size + self->unk88;

    *(s32 *)self->unk78 = self->unk3C;
    *(s32 *)(self->unk78 + 4) = self->unk80;

    *(s32 *)self->unk7C = self->unk3C;
    *(s32 *)(self->unk7C + 4) = self->unk84;

    GsClearOt(0, 0, self->unk78);
    GsClearOt(0, 0, self->unk7C);

    self->otReady = 1;
    self->otIndex = 0;
}

/* Sony's `DrawSync` (libgpu/sys, fingerprint exact vs the disc corpus, not
 * yet linked from an SDK object). LOCAL to this unit, not code_2cc8c.h --
 * see the note on ResetGraph/GsClearOt above: a second declaration of this
 * name in a header six units include is exactly where LIBGPU.H's own
 * prototype (`extern int DrawSync(int mode);`) will one day collide. This
 * call site passes a literal 0 and ignores the return. */
extern void DrawSync(s32 mode);

/* Teardown counterpart to Unk18Obj__InitOt's init. */
void Unk18Obj__DeinitOt(Unk18Obj *self) {
    if (self->otReady != 0) {
        DrawSync(0);
        BMemPMgrFree((void *)self->unk78);
        self->otReady = 0;
    }
}

/* Increments self->unk90 unconditionally, and additionally dispatches
 * self->methods->slot9C when arg2 is 2 or 3. */
void Unk18Obj__OnNotifyTag5(Unk18Obj *self, GenericObj *arg1, s32 arg2) {
    self->unk90 = self->unk90 + 1;
    if (arg2 == 2 || arg2 == 3) {
        self->methods->slot9C(self);
    }
}

void Unk18Obj__OnNotifyTag1(Unk18Obj *self, GenericObj *arg1, s32 arg2) {
    if (arg2 == 2) {
        self->methods->slotA4(self);
    }
}

/* Per-frame update, guarded by self->otReady (only runs once Unk18Obj__InitOt's
 * init has succeeded). Notifies slotA0 if self->unk10->unkC is set,
 * updates three sub-objects (unk40/unk4C/lightMode), conditionally re-notifies
 * a PsyQ helper when lightMode is 1 or 3, resets self->unk30's pointee,
 * recomputes self->unk98 from the (unk50-unk4C)/(1<<unk3C) division,
 * forwards the current otIndex-indexed slot to two more helpers, dispatches
 * slotA0 again with self->unkAC, and finally -- if self->unk10 is set --
 * walks it to its list tail and dispatches slotA0 a third time with that
 * tail. */
/* Psy-Q's GTE far-colour register writer (libgte/reg03, linked from Sony's
 * own SDK object). LOCAL to this unit, not code_2cc8c.h -- see the note on
 * SetGeomScreen below. This call site reads self->farColor's own three bytes
 * UNSIGNED (`lbu`, not `lb`) even though Unk18Obj__SetFarColor writes them as signed
 * bytes; the disagreement is kept as a local cast rather than a retype of the
 * field. Sony's own argument type is `long` for each. */
extern void SetFarColor(u8 a0, u8 a1, u8 a2);

/* Three more of Sony's, linked from the SDK objects since round 34 and
 * declared LOCALLY for the same reason as GsSetRefView2 above: they used to
 * sit in include/code_2cc8c.h as `func_8003Fxxx`, and under their real names
 * a header six units include is exactly where LIBGS.H's own prototypes will
 * one day collide. These are only the shapes THIS unit's call sites use.
 *   GsSetLightMode  libgs/gs_108   was func_8003FC70
 *   SetFogNear      libgte/fog_01  was func_8003FD4C
 *   GsClearOt       libgs/gs_113   was func_8003FC18
 * GsClearOt's real third argument is a `GsOT *` (its first two are Sony's
 * `offset` and `point`). This call site already passed it as a plain word, so
 * it is left that way -- the shape the header carried before round 14 retyped
 * it to a `TexPageDesc *`, which was code_2cc8c_e.c's reading of that same
 * GsOT. */
extern void GsSetLightMode(s32 a0);
extern void SetFogNear(s32 a0, s32 a1);
extern void GsClearOt(s32 a0, s32 a1, s32 a2);

void Unk18Obj__Update(Unk18Obj *self) {
    s32 idx;
    Unk18Obj *tail;

    if (self->otReady == 0) {
        return;
    }

    if (self->unk10->unkC != NULL) {
        ((void (*)(Unk18Obj *, GenericObj *))self->methods->slotA0)(self, self->unk10);
    }

    GsSetProjection(self->unk40);
    GsSetNearClip(self->unk4C);
    GsSetLightMode(self->lightMode);

    if (self->lightMode == 1 || self->lightMode == 3) {
        u8 *rawBytes = (u8 *)&self->farColor;
        SetFarColor(rawBytes[0], rawBytes[1], rawBytes[2]);
        SetFogNear(self->fogNear, self->unk40);
    }

    GsSetRefView2(&self->unk14);
    *(s32 *)self->unk30 = 0;

    self->unk98 = (u32)(self->unk50 - self->unk4C) / (u32)(1 << self->unk3C) + 1;

    idx = self->otIndex;
    GsSetWorkBase(*(unsigned char **)((u8 *)self + 0x88 + idx * 4));

    idx = self->otIndex;
    GsClearOt(0, 0, *(s32 *)((u8 *)self + 0x78 + idx * 4));

    ((void (*)(Unk18Obj *, void *))self->methods->slotA0)(self, self->unkAC);

    if (self->unk10 != NULL) {
        tail = Unk18Obj__GetTail((Unk18Obj *)self->unk10);
        ((void (*)(Unk18Obj *, Unk18Obj *))self->methods->slotA0)(self, tail);
    }
}

/* Sony's `GsDrawOt` (libgs/gs_111, linked from the SDK object since round
 * 34; was func_8003FBF4, and was declared in include/code_2cc8c.h until this
 * round). Local for the same collision reason as the three above. Sony's own
 * argument is a `GsOT *`; this call site passes the same otIndex-indexed slot
 * it hands GsClearOt, as a plain word, and is left that way.
 * gs_111 and gs_112 are byte-identical objects defining GsDrawOt and
 * GsDrawOtIO at this one address -- gs_111/GsDrawOt is what the build links. */
extern void GsDrawOt(s32 a0);

/* Sony's `GsSortClear` (libgs/gs_001, fingerprint exact vs the disc corpus,
 * not yet linked from an SDK object). Local for the same collision reason as
 * the three above: LIBGS.H's own prototype is `void GsSortClear(u_char r,
 * u_char g, u_char b, GsOT *ot);`. This call site reads self->clearColor's own
 * three bytes UNSIGNED (same "writer reads signed, this reader reads
 * unsigned" situation as farColor/Unk18Obj__Update) and passes the fourth as a
 * plain word, same as GsClearOt/GsDrawOt above. */
extern void GsSortClear(u8 a0, u8 a1, u8 a2, s32 a3);

/* Recomputes self->otIndex from self->unkC->methods->slot54, optionally
 * resets the graphics context and re-notifies self->unkC->methods->slot50
 * (once, or twice more if otIndex is still 0), forwards the current
 * otIndex-indexed slot to two rendering helpers, then finally collapses
 * self->otIndex to a plain boolean (1 if it was 0, else 0). */
void Unk18Obj__Flip(Unk18Obj *self) {
    s32 idx;
    u8 *rawBytes;

    if (self->otReady == 0) {
        return;
    }

    self->otIndex = self->unkC->methods->slot54(self->unkC);
    if (self->unkB8 == 0) {
        goto tail_check;
    }

    ResetGraph(1);
    self->unkC->methods->slot50(self->unkC);

    if (self->unkB4 != 0) {
        if (self->otIndex == 0) {
            self->unkC->methods->slot50(self->unkC);
        }
    }

    idx = self->otIndex;
    rawBytes = (u8 *)&self->clearColor;
    GsSortClear(rawBytes[0], rawBytes[1], rawBytes[2],
                *(s32 *)((u8 *)self + 0x78 + idx * 4));

    idx = self->otIndex;
    GsDrawOt(*(s32 *)((u8 *)self + 0x78 + idx * 4));

    if (self->unkB4 != 0 && self->otIndex == 0) {
        self->unkC->methods->slot50(self->unkC);
    }

tail_check:
    self->otIndex = (self->otIndex == 0);
}

/* Only runs when self->unk10 is NULL: releases the current self->unkB0 (if
 * any) via its own slot4, then -- if arg1 is non-NULL -- installs arg1 as
 * the new self->unkB0 and notifies it (slot4C) with self->unkAC and the
 * shared D_8008A904 constant. */
void Unk18Obj__SetSubHandle(Unk18Obj *self, SubHandleObj *arg1) {
    if (self->unk10 != NULL) {
        return;
    }

    if (self->unkB0 != NULL) {
        self->unkB0->methods->slot4(self->unkB0);
    }

    self->unkB0 = arg1;
    if (arg1 != NULL) {
        arg1->methods->slot4C(arg1, self->unkAC, D_8008A904);
    }
}

SubHandleObj *Unk18Obj__GetSubHandle(Unk18Obj *self) {
    return self->unkB0;
}

void Unk18Obj__SetUnkB4(Unk18Obj *self, s32 a1) {
    self->unkB4 = a1;
}

void Unk18Obj__SetUnkB8(Unk18Obj *self, s32 a1) {
    self->unkB8 = a1;
}

/* Plain no-arg getter for Unk18Obj's own vtable. */
Unk18ObjMethods *GetUnk18ObjMethods(void) {
    return &D_8006E8E4;
}

/* Walks self->unkC repeatedly while non-NULL, finding the tail of a
 * singly-linked list rooted at self, threaded through unkC. Returns self
 * itself unchanged if self->unkC is already NULL. */
Unk18Obj *Unk18Obj__GetTail(Unk18Obj *self) {
    while (self->unkC != NULL) {
        self = (Unk18Obj *)self->unkC;
    }
    return self;
}

/* Psy-Q's GTE far-colour and geometric-screen-distance register writers
 * (libgte/reg03, linked from Sony's own SDK object). Declared LOCAL to this
 * unit rather than in code_2cc8c.h, which nine units include, since they
 * belong to another translation unit (CLAUDE.md's header-contention rule).
 * Both take `long` as LIBGTE.H declares them.
 *
 * GsSetProjection below is Sony's libgs/gs_106 (round 79, FINISHING-PLAN
 * track 2: an 18-way EXACT tie that position settles -- it is the last word
 * before the placed libgs run, zero gap to gs_131, and gs_106 is the only
 * libgs module among the ties -- and LIBGS.H's `GsSetProjection(long h)`
 * agrees with its call site). It is kept here as matched C because no object
 * places it; progress.py counts it as library via its `identified` line. */
extern void SetGeomScreen(long h);

void GsSetProjection(long h) {
    SetGeomScreen(h);
}
