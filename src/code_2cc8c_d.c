#include "common.h"
#include "code_2cc8c.h"
#include "Viewport.h"

/*
 * The rest of Viewport's own table (gViewportMethods, include/Viewport.h,
 * whose banner says what the class does): the ctor, finalize and the child
 * overrides live in the sibling unit code_2cc8c_c.c; this unit holds the
 * onNotify override and its two per-sender handlers, the field setters,
 * the view-node attach/detach (the node, its GsRVIEW2 viewpoint, reference
 * point and twist, handed to Sony's GsSetRefView2), the double-buffered GsOT
 * allocation (InitOt/DeinitOt) and the two per-frame methods: Update
 * (projection, near clip, light mode, fog, ref view, clears this frame's OT
 * and draws the scene into it) and Flip (sorts the clear, draws the OT and
 * takes the next buffer index from the DrawSystem).
 */

/* Forwards to the base onNotify, then dispatches self's own onNotifyTag5 or
 * onNotifyTag1 by the sender's class-id nibble (5 or 1; the same tag idiom as
 * Viewport__AddChild). Neither dispatch happens for any other tag. */
void Viewport__OnNotify(Viewport *self, BasicClass *sender, s32 event) {
    s32 tag;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);

    tag = sender->methods->header & 0xF;
    if (tag == 5) {
        self->methods->onNotifyTag5(self, sender, event);
    } else if (tag == 1) {
        self->methods->onNotifyTag1(self, sender, event);
    }
}

/* MATCHED round 49. Two levers were needed, see docs/match-reports/Viewport__InitDefaults.md:
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
extern ViewportRgb D_8008A8F8;
extern ViewportRgb D_8008A8F8_b __asm__("D_8008A8F8");

void Viewport__InitDefaults(Viewport *self) {
    self->unk90 = 0;
    self->otReady = 0;
    /* Keeps the D_8008A8FC / D_8008A900 loads below the unk90 and otReady
     * zero stores; without it GCC hoists both loads to the top. */
    __asm__("");
    self->screenSize.width = D_8008A8FC;
    self->screenSize.height = D_8008A900;
    /* Keeps the two screenSize stores directly after their loads; without
     * it they sink below the otLength..fogNear constant stores. */
    __asm__("");
    self->otLength = 0xD;
    self->unk44 = 0x7D0;
    self->unk48 = 0x40;
    self->projH = 0x100;
    self->nearZ = 0xA;
    self->farZ = 0x10000;
    self->lightMode = 0;
    self->fogNear = 0x4E20;
    self->farColor = D_8008A8F8;
    self->clearColor = D_8008A8F8_b;
    self->unkB4 = 0;
    self->drawEnabled = 1;
}

void Viewport__SetScreenSize(Viewport *self, ViewportSize *size) {
    self->screenSize = *size;
}

void Viewport__SetOtLength(Viewport *self, s32 length) {
    self->otLength = length;
}

/* Only writes unk44 the first time (guarded by the otReady latch). */
void Viewport__SetUnk44(Viewport *self, s32 value) {
    if (self->otReady == 0) {
        self->unk44 = value;
    }
}

/* Same guard as Viewport__SetUnk44, writes unk48 instead. */
void Viewport__SetUnk48(Viewport *self, s32 value) {
    if (self->otReady == 0) {
        self->unk48 = value;
    }
}

void Viewport__SetProjection(Viewport *self, s32 h) {
    self->projH = h;
}

void func_8003EA6C(void) {}

void func_8003EA74(void) {}

void Viewport__SetLightMode(Viewport *self, s32 mode) {
    self->lightMode = mode;
}

void Viewport__SetClearColor(Viewport *self, ViewportRgb *color) {
    self->clearColor = *color;
}

void Viewport__SetFarColor(Viewport *self, ViewportRgb *color) {
    self->farColor = *color;
}

void Viewport__SetFogNear(Viewport *self, s32 fogNear) {
    self->fogNear = fogNear;
}

/* GsSetRefView2 is Sony's (`libgs/gs_131.o`, linked from the SDK object).
   Declared LOCALLY rather than in include/code_2cc8c.h, which six units
   include: the real `LIBGS.H` prototype for this name will collide there.
   This is only the shape THIS unit's call sites use -- the real one takes a
   GsRVIEW2*. */
extern void GsSetRefView2(void *arg0);

/* One-time init, skipped once a view node is set: adds `node` as a child
 * (addChild caches it as viewNode), sets the viewpoint, reference point and
 * twist (D_8008A8F4 when `twist` is NULL), then hands refView to
 * GsSetRefView2. */
void Viewport__AttachViewChild(Viewport *self, BasicClass *node, LongVec3 *vp, LongVec3 *vr,
                               Ratio16 *twist) {
    ViewportMethods *m = self->methods;

    if (self->viewNode != NULL) {
        return;
    }
    m->addChild(self, node);
    m->setViewPoint(self, vp);
    m->setViewRef(self, vr);
    m->setTwist(self, twist != NULL ? twist : &D_8008A8F4);
    GsSetRefView2(&self->refView);
}

/* Teardown counterpart to Viewport__AttachViewChild: removes the view node
 * as a child, if there is one. */
void Viewport__DetachViewChild(Viewport *self) {
    if (self->viewNode != NULL) {
        self->methods->removeChild(self, (BasicClass *)self->viewNode);
    }
}

/* Copies vp into refView.vp, only while a view node is set. */
void Viewport__SetViewPoint(Viewport *self, LongVec3 *vp) {
    if (self->viewNode != NULL) {
        self->refView.vp = *vp;
    }
}

/* Sibling of Viewport__SetViewPoint: copies vr into refView.vr, guarded the
 * same way. */
void Viewport__SetViewRef(Viewport *self, LongVec3 *vr) {
    if (self->viewNode != NULL) {
        self->refView.vr = *vr;
    }
}

void Viewport__SetTwist(Viewport *self, Ratio16 *twist) {
    s32 q1, r1, q2;

    if (self->viewNode != NULL) {
        q1 = twist->num / twist->den;
        r1 = twist->num % twist->den;
        q2 = (r1 << 12) / twist->den;
        self->refView.rz = (q1 << 12) + q2;
    }
}

void func_8003ECC0(void) {}

void func_8003ECC8(void) {}

/* One-time allocation of this object's two ordering tables (see
 * docs/match-reports/Viewport__InitOt.md). Each half of the buffer is a
 * 0x14-byte GsOT header, 4 << otLength bytes of OT tags, then unk48 * unk44
 * bytes of packet area. The header size is a LOCAL on purpose: written as a
 * literal, fold() reassociates the constant to the outside of the sum and
 * the final addu/addiu pair swaps (round 71). */
extern void GsClearOt(s32 a0, s32 a1, ViewportOt *ot);
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

void Viewport__InitOt(Viewport *self) {
    s32 size;
    s32 buf;
    s32 hdrSize = 0x14; /* sizeof(GsOT) */

    if (self->otReady != 0) {
        return;
    }

    size = (4 << self->otLength) + (self->unk48 * self->unk44 + hdrSize);

    buf = (s32)BMemPMgrAlloc(size * 2);
    if (buf == 0) {
        return;
    }

    self->ot[0] = (ViewportOt *)buf;
    self->otTags[0] = buf + 0x14;
    self->workBase[0] = (4 << self->otLength) + self->otTags[0];

    self->ot[1] = (ViewportOt *)(size + (s32)self->ot[0]);
    self->otTags[1] = size + self->otTags[0];
    self->workBase[1] = size + self->workBase[0];

    self->ot[0]->length = self->otLength;
    self->ot[0]->org = self->otTags[0];

    self->ot[1]->length = self->otLength;
    self->ot[1]->org = self->otTags[1];

    GsClearOt(0, 0, self->ot[0]);
    GsClearOt(0, 0, self->ot[1]);

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

/* Teardown counterpart to Viewport__InitOt's init. */
void Viewport__DeinitOt(Viewport *self) {
    if (self->otReady != 0) {
        DrawSync(0);
        BMemPMgrFree(self->ot[0]);
        self->otReady = 0;
    }
}

/* Counts the notification in unk90, and runs update on events 2 and 3. */
void Viewport__OnNotifyTag5(Viewport *self, BasicClass *sender, s32 event) {
    self->unk90 = self->unk90 + 1;
    if (event == 2 || event == 3) {
        self->methods->update(self);
    }
}

void Viewport__OnNotifyTag1(Viewport *self, BasicClass *sender, s32 event) {
    if (event == 2) {
        self->methods->flip(self);
    }
}

/* Per-frame update, only once Viewport__InitOt has succeeded: draws the
 * view node if it has a parent, sets the projection, near clip and light
 * mode (and the fog for light modes 1 and 3), sets the reference view and
 * marks its super coordinate for recompute, recomputes zDiv, sets this
 * half's packet area and clears its OT, then draws sceneRoot and the root
 * of the view node's parent chain. */
/* Psy-Q's GTE far-colour register writer (libgte/reg03, linked from Sony's
 * own SDK object). LOCAL to this unit, not code_2cc8c.h -- see the note on
 * SetGeomScreen below. This call site reads self->farColor's own three bytes
 * UNSIGNED (`lbu`, not `lb`) even though Viewport__SetFarColor writes them as signed
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
extern void GsClearOt(s32 a0, s32 a1, ViewportOt *ot);

void Viewport__Update(Viewport *self) {
    s32 idx;
    SceneNode *root;

    if (self->otReady == 0) {
        return;
    }

    if (self->viewNode->parent != NULL) {
        self->methods->drawNode(self, self->viewNode);
    }

    GsSetProjection(self->projH);
    GsSetNearClip(self->nearZ);
    GsSetLightMode(self->lightMode);

    if (self->lightMode == 1 || self->lightMode == 3) {
        u8 *rawBytes = (u8 *)&self->farColor;
        SetFarColor(rawBytes[0], rawBytes[1], rawBytes[2]);
        SetFogNear(self->fogNear, self->projH);
    }

    GsSetRefView2(&self->refView);
    self->refView.super->flg = 0;

    self->zDiv = (u32)(self->farZ - self->nearZ) / (u32)(1 << self->otLength) + 1;

    idx = self->otIndex;
    GsSetWorkBase((unsigned char *)self->workBase[idx]);

    idx = self->otIndex;
    GsClearOt(0, 0, self->ot[idx]);

    self->methods->drawNode(self, self->sceneRoot);

    if (self->viewNode != NULL) {
        root = GetRootNode(self->viewNode);
        self->methods->drawNode(self, root);
    }
}

/* Sony's `GsDrawOt` (libgs/gs_111, linked from the SDK object since round
 * 34; was func_8003FBF4, and was declared in include/code_2cc8c.h until this
 * round). Local for the same collision reason as the three above. Sony's own
 * argument is a `GsOT *`; this call site passes the same otIndex-indexed slot
 * it hands GsClearOt, as a plain word, and is left that way.
 * gs_111 and gs_112 are byte-identical objects defining GsDrawOt and
 * GsDrawOtIO at this one address -- gs_111/GsDrawOt is what the build links. */
extern void GsDrawOt(ViewportOt *ot);

/* Sony's `GsSortClear` (libgs/gs_001, fingerprint exact vs the disc corpus,
 * not yet linked from an SDK object). Local for the same collision reason as
 * the three above: LIBGS.H's own prototype is `void GsSortClear(u_char r,
 * u_char g, u_char b, GsOT *ot);`. This call site reads self->clearColor's own
 * three bytes UNSIGNED (same "writer reads signed, this reader reads
 * unsigned" situation as farColor/Viewport__Update) and passes the fourth as a
 * plain word, same as GsClearOt/GsDrawOt above. */
extern void GsSortClear(u8 a0, u8 a1, u8 a2, ViewportOt *ot);

/* Takes otIndex from the DrawSystem's getActiveBuffer (+0x054); when drawing
 * is enabled, resets the GPU, swaps (+0x050; once more on buffer 0 when
 * unkB4 is set), sorts the clear into this half's OT and draws it (and swaps
 * again on buffer 0 with unkB4); then flips otIndex to the other half. */
void Viewport__Flip(Viewport *self) {
    s32 idx;
    u8 *rawBytes;

    if (self->otReady == 0) {
        return;
    }

    self->otIndex = self->drawSystem->methods->getActiveBuffer(self->drawSystem);
    if (self->drawEnabled == 0) {
        goto tail_check;
    }

    ResetGraph(1);
    self->drawSystem->methods->swapBuffers(self->drawSystem);

    if (self->unkB4 != 0) {
        if (self->otIndex == 0) {
            self->drawSystem->methods->swapBuffers(self->drawSystem);
        }
    }

    idx = self->otIndex;
    rawBytes = (u8 *)&self->clearColor;
    GsSortClear(rawBytes[0], rawBytes[1], rawBytes[2], self->ot[idx]);

    idx = self->otIndex;
    GsDrawOt(self->ot[idx]);

    if (self->unkB4 != 0 && self->otIndex == 0) {
        self->drawSystem->methods->swapBuffers(self->drawSystem);
    }

tail_check:
    self->otIndex = (self->otIndex == 0);
}

/* Only while no view node is set: releases the current subHandle, installs
 * `handle`, and attaches it under sceneRoot at D_8008A904 (-100, -100). The
 * occupant of handle's +0x04C (BoxFill__AttachToParent, a Class6E99C's) takes a
 * screen position where SceneNode's attachToParent slot types a LongVec3
 * offset, hence the cast (include/Viewport.h, "Not settled here"). */
void Viewport__SetSubHandle(Viewport *self, SceneNode *handle) {
    if (self->viewNode != NULL) {
        return;
    }

    if (self->subHandle != NULL) {
        self->subHandle->methods->release(self->subHandle);
    }

    self->subHandle = handle;
    if (handle != NULL) {
        handle->methods->attachToParent(handle, self->sceneRoot, (LongVec3 *)D_8008A904);
    }
}

SceneNode *Viewport__GetSubHandle(Viewport *self) {
    return self->subHandle;
}

void Viewport__SetUnkB4(Viewport *self, s32 value) {
    self->unkB4 = value;
}

void Viewport__SetDrawEnabled(Viewport *self, s32 on) {
    self->drawEnabled = on;
}

/* Viewport's table getter. */
ViewportMethods *GetViewportMethods(void) {
    return &gViewportMethods;
}

/* Follows `parent` from `node` to the top of its hierarchy. Not a method
 * (in no table): Viewport__Update passes it the view node. */
SceneNode *GetRootNode(SceneNode *node) {
    while (node->parent != NULL) {
        node = node->parent;
    }
    return node;
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
