#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_2cc8c.h"
#include "Viewport.h"
#include "FrameClock.h"

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
 * (1) retail reloads the address of `gDefaultViewportColor` INDEPENDENTLY for each of
 * the two whole-struct copies (two separate lui/addiu pairs); GCC 2.6.3
 * otherwise CSEs that into one shared computation. Declaring a second
 * extern name aliased to the same symbol via `__asm__("gDefaultViewportColor")` (the
 * same alternate-name idiom `code_179d8_m.c` already uses) gives the
 * second copy a textually distinct symbol, defeating the CSE without
 * `volatile`'s much worse codegen. (2) Two bare `__asm__("")` scheduling
 * barriers pin the two independent zero-inits and the two global-loaded
 * stores to retail's own early positions instead of letting the scheduler
 * defer them to just before the byte copies. */
extern s32 gDefaultViewportWidth;
extern s32 gDefaultViewportHeight;
extern ViewportRgb gDefaultViewportColor;
extern ViewportRgb D_8008A8F8_b __asm__("gDefaultViewportColor");

void Viewport__InitDefaults(Viewport *self) {
    self->unk90 = 0;
    self->otReady = 0;
    /* Keeps the gDefaultViewportWidth / gDefaultViewportHeight loads below the unk90 and otReady
     * zero stores; without it GCC hoists both loads to the top. */
    __asm__("");
    self->screenSize.width = gDefaultViewportWidth;
    self->screenSize.height = gDefaultViewportHeight;
    /* Keeps the two screenSize stores directly after their loads; without
     * it they sink below the otLength..fogNear constant stores. */
    __asm__("");
    self->otLength = 13;
    self->unk44 = 2000;
    self->unk48 = 64;
    self->projH = 256;
    self->nearZ = 10;
    self->farZ = 65536;
    self->lightMode = 0;
    self->fogNear = 20000;
    self->farColor = gDefaultViewportColor;
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

void Viewport__func_8003EA6C(void) {}

void Viewport__func_8003EA74(void) {}

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

/* One-time init, skipped once a view node is set: adds `node` as a child
 * (addChild caches it as viewNode), sets the viewpoint, reference point and
 * twist (gDefaultViewTwist when `twist` is NULL), then hands refView to
 * GsSetRefView2. */
void Viewport__AttachViewChild(Viewport *self, BasicClass *node, LongVec3 *vp, LongVec3 *vr,
                               Ratio16 *twist) {
    ViewportMethods *methods = self->methods;

    if (self->viewNode != NULL) {
        return;
    }
    methods->addChild(self, node);
    methods->setViewPoint(self, vp);
    methods->setViewRef(self, vr);
    methods->setTwist(self, twist != NULL ? twist : &gDefaultViewTwist);
    GsSetRefView2((GsRVIEW2 *)&self->refView);
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
    s32 whole, rem, frac;

    if (self->viewNode != NULL) {
        whole = twist->num / twist->den;
        rem = twist->num % twist->den;
        frac = rem * ONE / twist->den;
        self->refView.rz = whole * ONE + frac;
    }
}

void Viewport__func_8003ECC0(void) {}

void Viewport__func_8003ECC8(void) {}

/* One-time allocation of this object's two ordering tables (see
 * docs/match-reports/Viewport__InitOt.md). Each half of the buffer is a
 * 0x14-byte GsOT header, 4 << otLength bytes of OT tags, then unk48 * unk44
 * bytes of packet area. The header size is a LOCAL on purpose: written as a
 * literal, fold() reassociates the constant to the outside of the sum and
 * the final addu/addiu pair swaps (round 71). */
void Viewport__InitOt(Viewport *self) {
    s32 size;
    s32 buf;
    s32 hdrSize = sizeof(GsOT);

    if (self->otReady != 0) {
        return;
    }

    size = (4 << self->otLength) + (self->unk48 * self->unk44 + hdrSize);

    buf = (s32)BMemPMgrAlloc(size * 2);
    if (buf == 0) {
        return;
    }

    self->ot[0] = (ViewportOt *)buf;
    self->otTags[0] = buf + sizeof(GsOT);
    self->workBase[0] = (4 << self->otLength) + self->otTags[0];

    self->ot[1] = (ViewportOt *)(size + (s32)self->ot[0]);
    self->otTags[1] = size + self->otTags[0];
    self->workBase[1] = size + self->workBase[0];

    self->ot[0]->length = self->otLength;
    self->ot[0]->org = self->otTags[0];

    self->ot[1]->length = self->otLength;
    self->ot[1]->org = self->otTags[1];

    GsClearOt(0, 0, (GsOT *)self->ot[0]);
    GsClearOt(0, 0, (GsOT *)self->ot[1]);

    self->otReady = 1;
    self->otIndex = 0;
}

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
    if (event == FRAMECLOCK_EVENT_RUNNING || event == FRAMECLOCK_EVENT_PAUSED) {
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

    if (self->lightMode == GsLMODE_FOG || self->lightMode == 3) {
        SetFarColor((u8)self->farColor.r, (u8)self->farColor.g, (u8)self->farColor.b);
        SetFogNear(self->fogNear, self->projH);
    }

    GsSetRefView2((GsRVIEW2 *)&self->refView);
    self->refView.super->flg = 0;

    self->zDiv = (u32)(self->farZ - self->nearZ) / (u32)(1 << self->otLength) + 1;

    idx = self->otIndex;
    GsSetWorkBase((PACKET *)self->workBase[idx]);

    idx = self->otIndex;
    GsClearOt(0, 0, (GsOT *)self->ot[idx]);

    self->methods->drawNode(self, self->sceneRoot);

    if (self->viewNode != NULL) {
        root = GetRootNode(self->viewNode);
        self->methods->drawNode(self, root);
    }
}

/* Takes otIndex from the DrawSystem's getActiveBuffer (+0x054); when drawing
 * is enabled, resets the GPU, swaps (+0x050; once more on buffer 0 when
 * unkB4 is set), sorts the clear into this half's OT and draws it (and swaps
 * again on buffer 0 with unkB4); then flips otIndex to the other half. */
void Viewport__Flip(Viewport *self) {
    s32 idx;

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
    GsSortClear(self->clearColor.r, self->clearColor.g, self->clearColor.b, (GsOT *)self->ot[idx]);

    idx = self->otIndex;
    GsDrawOt((GsOT *)self->ot[idx]);

    if (self->unkB4 != 0 && self->otIndex == 0) {
        self->drawSystem->methods->swapBuffers(self->drawSystem);
    }

tail_check:
    self->otIndex = (self->otIndex == 0);
}

/* Only while no view node is set: releases the current fadeBox, installs
 * `handle`, and attaches it under sceneRoot at gFadeBoxAttachPos (-100, -100). The
 * occupant of handle's +0x04C (BoxFill__AttachToParent, a FadeBox's) takes a
 * screen position where SceneNode's attachToParent slot types a LongVec3
 * offset, hence the cast (include/Viewport.h, "Not settled here"). */
void Viewport__SetFadeBox(Viewport *self, SceneNode *fadeBox) {
    if (self->viewNode != NULL) {
        return;
    }

    if (self->fadeBox != NULL) {
        self->fadeBox->methods->release(self->fadeBox);
    }

    self->fadeBox = fadeBox;
    if (fadeBox != NULL) {
        fadeBox->methods->attachToParent(fadeBox, self->sceneRoot, (LongVec3 *)gFadeBoxAttachPos);
    }
}

SceneNode *Viewport__GetFadeBox(Viewport *self) {
    return self->fadeBox;
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

void GsSetProjection(long h) {
    SetGeomScreen(h);
}
