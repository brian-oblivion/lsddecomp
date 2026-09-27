#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_2cc8c.h"
#include "Viewport.h"
#include "FrameClock.h"

/*
 * code_2cc8c_d -- Viewport's methods from onNotify (+0x038) to the end of
 * gViewportMethods (include/Viewport.h, whose banner says what the class
 * is); the ctor, finalize and the child overrides are in code_2cc8c_c.c,
 * drawNode in code_2864.c. In table order:
 *  - onNotify and its two per-sender handlers: a FrameClock event runs
 *    update, the DrawSystem's VSync event runs flip;
 *  - initDefaults and the field setters (screen size, OT length, packet
 *    count and size, which only take before initOt, projection, light
 *    mode, clear and far colours, fog near); four slots hold empty bodies;
 *  - attachViewChild/detachViewChild and the setters of the GsRVIEW2 that
 *    GsSetRefView2 takes (viewpoint, reference point, twist);
 *  - initOt/deinitOt: one allocation holding both halves of the
 *    double-buffered GsOT, each a header, its tags and its packet area;
 *  - update, per frame: projection, near clip, light mode and fog, the
 *    reference view, then this half's packet area and cleared OT, and the
 *    scene drawn into it;
 *  - flip: takes the buffer index from the DrawSystem, swaps, sorts the
 *    clear into the OT and draws it;
 *  - setFadeBox/getFadeBox and two flag setters.
 * Then the table getter, GetRootNode (update's helper) and Sony's
 * GsSetProjection.
 *
 * refView is Viewport.h's ViewportRefView, GsRVIEW2's layout with vp and vr
 * as vectors (its comment says why), hence the (GsRVIEW2 *) cast at
 * GsSetRefView2.
 */

/* Forwards to the base onNotify, then dispatches on the sender's class-id
 * nibble: a FrameClock to onNotifyTag5, the DrawSystem to onNotifyTag1,
 * anything else nowhere. */
void Viewport__OnNotify(Viewport *self, BasicClass *sender, s32 event) {
    s32 tag;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);

    tag = sender->methods->header & CLASS_ID_ROOT_MASK;
    if (tag == FRAMECLOCK_CLASS_ID) {
        self->methods->onNotifyTag5(self, sender, event);
    } else if (tag == DRAWSYSTEM_CLASS_ID) {
        self->methods->onNotifyTag1(self, sender, event);
    }
}

extern s32 gDefaultViewportWidth;
extern s32 gDefaultViewportHeight;
extern ViewportRgb gDefaultViewportColor;
/* MATCHING: a second name for the same symbol, so cc1 cannot share one
 * address computation between the two copies; retail loads it twice. */
extern ViewportRgb gDefaultViewportColorAlias __asm__("gDefaultViewportColor");

/* InitDefaults' values. The OT has 1 << VIEWPORT_DEFAULT_OT_LENGTH (8192)
 * tags; with the near and far defaults Update's zDiv comes out 8. The packet
 * size is also TaskCore's reset value for what it passes to setPacketSize. */
#define VIEWPORT_DEFAULT_OT_LENGTH 13
#define VIEWPORT_DEFAULT_PACKET_COUNT 2000
#define VIEWPORT_DEFAULT_PACKET_SIZE 64
#define VIEWPORT_DEFAULT_PROJ_H 256 /* GsSetProjection's h */
#define VIEWPORT_DEFAULT_NEAR_Z 10
#define VIEWPORT_DEFAULT_FAR_Z 65536
#define VIEWPORT_DEFAULT_FOG_NEAR 20000

/* Every field's default; both colours start black (gDefaultViewportColor). */
void Viewport__InitDefaults(Viewport *self) {
    self->clockEventCount = 0;
    self->otReady = 0;
    /* MATCHING: without it both loads hoist above the two zero stores. */
    __asm__("");
    self->screenSize.width = gDefaultViewportWidth;
    self->screenSize.height = gDefaultViewportHeight;
    /* MATCHING: without it both stores sink below the constant stores. */
    __asm__("");
    self->otLength = VIEWPORT_DEFAULT_OT_LENGTH;
    self->packetCount = VIEWPORT_DEFAULT_PACKET_COUNT;
    self->packetSize = VIEWPORT_DEFAULT_PACKET_SIZE;
    self->projH = VIEWPORT_DEFAULT_PROJ_H;
    self->nearZ = VIEWPORT_DEFAULT_NEAR_Z;
    self->farZ = VIEWPORT_DEFAULT_FAR_Z;
    self->lightMode = GsLMODE_NORMAL;
    self->fogNear = VIEWPORT_DEFAULT_FOG_NEAR;
    self->farColor = gDefaultViewportColor;
    self->clearColor = gDefaultViewportColorAlias;
    self->extraSwapOnBuffer0 = 0;
    self->drawEnabled = 1;
}

void Viewport__SetScreenSize(Viewport *self, ViewportSize *size) {
    self->screenSize = *size;
}

void Viewport__SetOtLength(Viewport *self, s32 length) {
    self->otLength = length;
}

/* Takes only before InitOt, which sizes the packet areas from it. */
void Viewport__SetMaxPackets(Viewport *self, s32 count) {
    if (self->otReady == 0) {
        self->packetCount = count;
    }
}

/* The same guard, for the packet size. */
void Viewport__SetPacketSize(Viewport *self, s32 size) {
    if (self->otReady == 0) {
        self->packetSize = size;
    }
}

void Viewport__SetProjection(Viewport *self, s32 h) {
    self->projH = h;
}

void Viewport__NoOpSlot58(void) {}

void Viewport__NoOpSlot5C(void) {}

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

void Viewport__NoOpSlot84(void) {}

void Viewport__NoOpSlot88(void) {}

/* One-time allocation of the two ordering tables. Each half of the buffer
 * is a GsOT header, its 1 << otLength tags, then packetSize * packetCount
 * bytes of packet area. */
void Viewport__InitOt(Viewport *self) {
    s32 size;
    s32 buf;
    /* MATCHING: written inline, the constant reassociates out of the sum and
     * the final addu/addiu pair swaps. */
    s32 hdrSize = sizeof(GsOT);

    if (self->otReady != 0) {
        return;
    }

    size = (sizeof(GsOT_TAG) << self->otLength) + (self->packetSize * self->packetCount + hdrSize);

    buf = (s32)BMemPMgrAlloc(size * 2);
    if (buf == 0) {
        return;
    }

    self->ot[0] = (GsOT *)buf;
    self->otTags[0] = (GsOT_TAG *)(buf + sizeof(GsOT));
    self->workBase[0] = (PACKET *)self->otTags[0] + (sizeof(GsOT_TAG) << self->otLength);

    self->ot[1] = (GsOT *)((PACKET *)self->ot[0] + size);
    self->otTags[1] = (GsOT_TAG *)((PACKET *)self->otTags[0] + size);
    self->workBase[1] = self->workBase[0] + size;

    self->ot[0]->length = self->otLength;
    self->ot[0]->org = self->otTags[0];

    self->ot[1]->length = self->otLength;
    self->ot[1]->org = self->otTags[1];

    GsClearOt(0, 0, self->ot[0]);
    GsClearOt(0, 0, self->ot[1]);

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

/* A FrameClock event: counts every one in clockEventCount, and runs update
 * on a tick whether the clock is running or paused (not on
 * FRAMECLOCK_EVENT_FLAG14). */
void Viewport__OnNotifyTag5(Viewport *self, BasicClass *sender, s32 event) {
    self->clockEventCount = self->clockEventCount + 1;
    if (event == FRAMECLOCK_EVENT_RUNNING || event == FRAMECLOCK_EVENT_PAUSED) {
        self->methods->update(self);
    }
}

/* A DrawSystem event: its per-VSync event runs flip. */
void Viewport__OnNotifyTag1(Viewport *self, BasicClass *sender, s32 event) {
    if (event == DRAWSYSTEM_EVENT_VSYNC) {
        self->methods->flip(self);
    }
}

/* Per-frame update, only once Viewport__InitOt has succeeded: draws the
 * view node if it has a parent, sets the projection, near clip and light
 * mode (and the fog, in both fog modes), sets the reference view and
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

    if (self->lightMode == GsLMODE_FOG || self->lightMode == (GsLMODE_LOFF | GsLMODE_FOG)) {
        SetFarColor((u8)self->farColor.r, (u8)self->farColor.g, (u8)self->farColor.b);
        SetFogNear(self->fogNear, self->projH);
    }

    GsSetRefView2((GsRVIEW2 *)&self->refView);
    self->refView.super->flg = 0;

    self->zDiv = (u32)(self->farZ - self->nearZ) / (u32)(1 << self->otLength) + 1;

    idx = self->otIndex;
    GsSetWorkBase(self->workBase[idx]);

    idx = self->otIndex;
    GsClearOt(0, 0, self->ot[idx]);

    self->methods->drawNode(self, self->sceneRoot);

    if (self->viewNode != NULL) {
        root = GetRootNode(self->viewNode);
        self->methods->drawNode(self, root);
    }
}

/* Takes otIndex from the DrawSystem's getActiveBuffer; when drawing is
 * enabled, resets the GPU, swaps, sorts the clear into this half's OT and
 * draws it, with one more swap before the clear and one after the draw on
 * buffer 0 when extraSwapOnBuffer0 is set; then flips otIndex to the other
 * half. */
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

    if (self->extraSwapOnBuffer0 != 0) {
        if (self->otIndex == 0) {
            self->drawSystem->methods->swapBuffers(self->drawSystem);
        }
    }

    idx = self->otIndex;
    GsSortClear(self->clearColor.r, self->clearColor.g, self->clearColor.b, self->ot[idx]);

    idx = self->otIndex;
    GsDrawOt(self->ot[idx]);

    if (self->extraSwapOnBuffer0 != 0 && self->otIndex == 0) {
        self->drawSystem->methods->swapBuffers(self->drawSystem);
    }

tail_check:
    self->otIndex = (self->otIndex == 0);
}

/* Only while no view node is set: releases the current fade box, installs
 * `fadeBox`, and attaches it under sceneRoot at gFadeBoxAttachPos
 * (-100, -100). A FadeBox's attachToParent (BoxFill__AttachToParent) takes a
 * screen position where SceneNode's slot types a LongVec3 offset, hence the
 * cast (include/Viewport.h, "Not settled here"). */
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

void Viewport__SetExtraSwap(Viewport *self, s32 on) {
    self->extraSwapOnBuffer0 = on;
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

/* Sony's libgs GsSetProjection (gs_106). No SDK object places it, so it is
 * carried as C; progress.py counts it as library. */
void GsSetProjection(long h) {
    SetGeomScreen(h);
}
