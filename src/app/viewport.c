/*
 * Viewport's methods (include/viewport.h: the screen's view, its ordering
 * tables and its fade box, which the tasks draw through), except drawNode
 * (src/graphics/viewport_draw.c), in ROM order: New_Viewport, the ctor,
 * finalize, and the addChild/removeChild/removeAllChildren overrides,
 * which cache a DrawSystem child and a SceneNode child (the view node,
 * whose coord2 the reference view hangs from) by root class id; then the
 * rest of its table, introduced below, its getter GetViewportMethods and
 * GetRootNode. Its method table closes the file.
 */
#include "common.h"
#include <libgte.h>
#include "fade_box.h"
#include "bmem_pmgr.h"
#include "viewport.h"
#include "frame_clock.h"

/* Viewport__AttachViewChild's twist when its own is NULL: 0 / 1, none. */
static Ratio16 sDefaultViewTwist SDATA = {0, 1};

/* InitDefaults' screen size, 256 x 240, and both its colours, black. */
static ColorRgb sDefaultViewportColor SDATA = {0, 0, 0};
static s32 sDefaultViewportWidth SDATA = 256;
static s32 sDefaultViewportHeight SDATA = 240;
/* MATCHING: a second name for the same symbol, so its address is computed twice. */
#ifdef HOST_BUILD
/* The host needs no second address, and an assembler name misses the symbol
 * where C names get a prefix (32-bit Windows' "_"). */
#define sDefaultViewportColorAlias sDefaultViewportColor
#else
extern ColorRgb sDefaultViewportColorAlias __asm__("sDefaultViewportColor");
#endif

/* Viewport's ctor data: sFadeBoxAttachPos is the (-100, -100) screen
 * position the ctor and SetSubHandle attach the sub handle at, and
 * sViewportFadeBoxSize the 320 x 240 it passes New_FadeBox. */
static BoxFillPos sFadeBoxAttachPos SDATA = {-100, -100};
static BoxFillSize sViewportFadeBoxSize SDATA = {320, 240};

/* MATCHING: four words nothing reads end the unit's .sdata in retail. */
static s32 sViewportUnusedWords[4] SDATA = {10, 10, -10, -10};

Viewport *New_Viewport(void) {
    Viewport *self;

    self = BMemPMgrAlloc(sizeof(Viewport));
    if (self != NULL) {
        GetViewportMethods()->ctor(self);
        return self;
    }
    return NULL;
}

void Viewport__Viewport(Viewport *self) {
    SceneNode *fadeBox;

    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetViewportMethods();
    self->drawSystem = NULL;
    self->viewNode = NULL;
    self->sceneRoot = New_SceneNode();
    fadeBox = (SceneNode *)New_FadeBox(&sViewportFadeBoxSize, 0, 0);
    self->fadeBox = fadeBox;
    fadeBox->methods->attachToParent(fadeBox, self->sceneRoot, (LongVec3 *)&sFadeBoxAttachPos);
    self->methods->initDefaults(self);
}

void Viewport__Finalize(Viewport *self) {
    self->methods->deinitOt(self);
    self->methods->detachViewChild(self);
    self->sceneRoot->methods->release(self->sceneRoot);
    self->methods->setFadeBox(self, 0);
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

void Viewport__AddChild(Viewport *self, BasicClass *child) {
    s32 rootClass;

    GetBasicClassMethods()->addChild((BasicClass *)self, child);
    rootClass = child->methods->header & CLASS_ID_ROOT_MASK;
    if (rootClass == SCENENODE_CLASS_ID) {
        self->viewNode = (SceneNode *)child;
        self->refView.super = ((SceneNode *)child)->coord2;
    } else if (rootClass == DRAWSYSTEM_CLASS_ID) {
        self->drawSystem = (DrawSystem *)child;
    }
}

void Viewport__RemoveChild(Viewport *self, BasicClass *child) {
    s32 rootClass;

    rootClass = child->methods->header & CLASS_ID_ROOT_MASK;
    if (rootClass == SCENENODE_CLASS_ID) {
        self->refView.super = NULL;
        self->viewNode = NULL;
    } else if (rootClass == DRAWSYSTEM_CLASS_ID) {
        self->drawSystem = NULL;
    }
    GetBasicClassMethods()->removeChild((BasicClass *)self, child);
}

/* Viewport's removeAllChildren override (+0x018 of gViewportMethods and of
 * gNodeGuardedViewportMethods): clears the three child caches AddChild fills, then
 * the base. */
void Viewport__RemoveAllChildren(Viewport *self) {
    self->refView.super = NULL;
    self->viewNode = NULL;
    self->drawSystem = NULL;
    GetBasicClassMethods()->removeAllChildren((BasicClass *)self);
}

/*
 * Viewport's methods from onNotify (+0x038) to the end of gViewportMethods,
 * in table order (include/viewport.h documents the class); drawNode is in
 * viewport_draw.c. Then the table getter and GetRootNode (update's helper).
 */

/* Forwards to the base onNotify, then dispatches on the sender's class-id
 * nibble: a FrameClock to onFrameClockEvent, the DrawSystem to onDrawSystemEvent,
 * anything else nowhere. */
void Viewport__OnNotify(Viewport *self, BasicClass *sender, s32 event) {
    s32 tag;

    GetBasicClassMethods()->onNotify((BasicClass *)self, sender, event);

    tag = sender->methods->header & CLASS_ID_ROOT_MASK;
    if (tag == FRAMECLOCK_CLASS_ID) {
        self->methods->onFrameClockEvent(self, sender, event);
    } else if (tag == DRAWSYSTEM_CLASS_ID) {
        self->methods->onDrawSystemEvent(self, sender, event);
    }
}

/* InitDefaults' values. The OT has 1 << VIEWPORT_DEFAULT_OT_LENGTH (8192)
 * tags; with the near and far defaults Update's zDiv comes out 8. The packet
 * size is also TaskCore's reset value for what it passes to setPacketSize. */
#define VIEWPORT_DEFAULT_OT_LENGTH 13
#define VIEWPORT_DEFAULT_MAX_PACKETS 2000
#define VIEWPORT_DEFAULT_PACKET_SIZE 64
#define VIEWPORT_DEFAULT_PROJ_H 256 /* GsSetProjection's h */
#define VIEWPORT_DEFAULT_NEAR_Z 10
#define VIEWPORT_DEFAULT_FAR_Z 65536
#define VIEWPORT_DEFAULT_FOG_NEAR 20000

/* Every field's default; both colours start black (sDefaultViewportColor). */
void Viewport__InitDefaults(Viewport *self) {
    self->clockEventCount = 0;
    self->otReady = 0;
    /* MATCHING: an ordering barrier; without it the two size reads move above the zero stores. */
    __asm__("");
    self->screenSize.width = sDefaultViewportWidth;
    self->screenSize.height = sDefaultViewportHeight;
    /* MATCHING: the same; without it the size stores move below the constant stores. */
    __asm__("");
    self->otLength = VIEWPORT_DEFAULT_OT_LENGTH;
    self->maxPackets = VIEWPORT_DEFAULT_MAX_PACKETS;
    self->packetSize = VIEWPORT_DEFAULT_PACKET_SIZE;
    self->projH = VIEWPORT_DEFAULT_PROJ_H;
    self->nearZ = VIEWPORT_DEFAULT_NEAR_Z;
    self->farZ = VIEWPORT_DEFAULT_FAR_Z;
    self->lightMode = GsLMODE_NORMAL;
    self->fogNear = VIEWPORT_DEFAULT_FOG_NEAR;
    self->farColor = sDefaultViewportColor;
    self->clearColor = sDefaultViewportColorAlias;
    self->extraSwap = 0;
    self->drawEnabled = 1;
}

void Viewport__SetScreenSize(Viewport *self, ScreenDims *size) {
    self->screenSize = *size;
}

void Viewport__SetOtLength(Viewport *self, s32 length) {
    self->otLength = length;
}

/* Takes only before InitOt, which sizes the packet areas from it. */
void Viewport__SetMaxPackets(Viewport *self, s32 maxPackets) {
    if (self->otReady == 0) {
        self->maxPackets = maxPackets;
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

void Viewport__SetClearColor(Viewport *self, ColorRgb *color) {
    self->clearColor = *color;
}

void Viewport__SetFarColor(Viewport *self, ColorRgb *color) {
    self->farColor = *color;
}

void Viewport__SetFogNear(Viewport *self, s32 fogNear) {
    self->fogNear = fogNear;
}

/* One-time init, skipped once a view node is set: adds `node` as a child
 * (addChild caches it as viewNode), sets the viewpoint, reference point and
 * twist (sDefaultViewTwist when `twist` is NULL), then hands refView to
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
    methods->setTwist(self, twist != NULL ? twist : &sDefaultViewTwist);
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
 * is a GsOT header, its 1 << otLength tags, then packetSize * maxPackets
 * bytes of packet area. */
void Viewport__InitOt(Viewport *self) {
    s32 size;
    u8 *buf;
    /* MATCHING: written inline, the constant reassociates out of the sum. */
    s32 hdrSize = sizeof(GsOT);

    if (self->otReady != 0) {
        return;
    }

    size = (sizeof(GsOT_TAG) << self->otLength) + (self->packetSize * self->maxPackets + hdrSize);

    buf = BMemPMgrAlloc(size * 2);
    if (buf == NULL) {
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
 * FRAMECLOCK_EVENT_STOPPED). */
void Viewport__OnFrameClockEvent(Viewport *self, BasicClass *sender, s32 event) {
    self->clockEventCount = self->clockEventCount + 1;
    if (event == FRAMECLOCK_EVENT_RUNNING || event == FRAMECLOCK_EVENT_PAUSED) {
        self->methods->update(self);
    }
}

/* A DrawSystem event: its per-VSync event runs flip. */
void Viewport__OnDrawSystemEvent(Viewport *self, BasicClass *sender, s32 event) {
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
 * buffer 0 when extraSwap is set; then flips otIndex to the other
 * half. */
void Viewport__Flip(Viewport *self) {
    s32 idx;

    if (self->otReady == 0) {
        return;
    }

    self->otIndex = self->drawSystem->methods->getActiveBuffer(self->drawSystem);
    if (self->drawEnabled != 0) {
        ResetGraph(1);
        self->drawSystem->methods->swapBuffers(self->drawSystem);

        if (self->extraSwap != 0 && self->otIndex == 0) {
            self->drawSystem->methods->swapBuffers(self->drawSystem);
        }

        idx = self->otIndex;
        GsSortClear(self->clearColor.r, self->clearColor.g, self->clearColor.b, self->ot[idx]);

        idx = self->otIndex;
        GsDrawOt(self->ot[idx]);

        if (self->extraSwap != 0 && self->otIndex == 0) {
            self->drawSystem->methods->swapBuffers(self->drawSystem);
        }
    }

    self->otIndex = (self->otIndex == 0);
}

/* Only while no view node is set: releases the current fade box, installs
 * `fadeBox`, and attaches it under sceneRoot at sFadeBoxAttachPos
 * (-100, -100). A FadeBox's attachToParent (BoxFill__AttachToParent) takes a
 * screen position where SceneNode's slot types a LongVec3 offset, hence the
 * cast (include/viewport.h, "Not settled here"). */
void Viewport__SetFadeBox(Viewport *self, SceneNode *fadeBox) {
    if (self->viewNode != NULL) {
        return;
    }

    if (self->fadeBox != NULL) {
        self->fadeBox->methods->release(self->fadeBox);
    }

    self->fadeBox = fadeBox;
    if (fadeBox != NULL) {
        fadeBox->methods->attachToParent(fadeBox, self->sceneRoot, (LongVec3 *)&sFadeBoxAttachPos);
    }
}

SceneNode *Viewport__GetFadeBox(Viewport *self) {
    return self->fadeBox;
}

void Viewport__SetExtraSwap(Viewport *self, s32 on) {
    self->extraSwap = on;
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

/* Viewport (include/viewport.h): BasicClass's slots with the screen, OT,
 * projection, fog and view setters, the frame and draw hooks, and the fade
 * box. */
ViewportMethods gViewportMethods = {
    /* +0x000 header */ VIEWPORT_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ Viewport__Viewport,
    /* +0x00C finalize */ Viewport__Finalize,
    /* +0x010 addChild */ Viewport__AddChild,
    /* +0x014 removeChild */ Viewport__RemoveChild,
    /* +0x018 removeAllChildren */ Viewport__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)Viewport__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 initDefaults */ Viewport__InitDefaults,
    /* +0x044 setScreenSize */ Viewport__SetScreenSize,
    /* +0x048 setOtLength */ Viewport__SetOtLength,
    /* +0x04C setMaxPackets */ Viewport__SetMaxPackets,
    /* +0x050 setPacketSize */ Viewport__SetPacketSize,
    /* +0x054 setProjection */ Viewport__SetProjection,
    /* +0x058 slot58 */ Viewport__NoOpSlot58,
    /* +0x05C slot5C */ Viewport__NoOpSlot5C,
    /* +0x060 setLightMode */ Viewport__SetLightMode,
    /* +0x064 setClearColor */ Viewport__SetClearColor,
    /* +0x068 setFarColor */ Viewport__SetFarColor,
    /* +0x06C setFogNear */ Viewport__SetFogNear,
    /* +0x070 attachViewChild */ Viewport__AttachViewChild,
    /* +0x074 detachViewChild */ Viewport__DetachViewChild,
    /* +0x078 setViewPoint */ Viewport__SetViewPoint,
    /* +0x07C setViewRef */ Viewport__SetViewRef,
    /* +0x080 setTwist */ Viewport__SetTwist,
    /* +0x084 slot84 */ Viewport__NoOpSlot84,
    /* +0x088 slot88 */ Viewport__NoOpSlot88,
    /* +0x08C initOt */ Viewport__InitOt,
    /* +0x090 deinitOt */ Viewport__DeinitOt,
    /* +0x094 onFrameClockEvent */ Viewport__OnFrameClockEvent,
    /* +0x098 onDrawSystemEvent */ Viewport__OnDrawSystemEvent,
    /* +0x09C update */ Viewport__Update,
    /* +0x0A0 drawNode */ Viewport__DrawNode,
    /* +0x0A4 flip */ Viewport__Flip,
    /* +0x0A8 setFadeBox */ Viewport__SetFadeBox,
    /* +0x0AC getFadeBox */ Viewport__GetFadeBox,
    /* +0x0B0 setExtraSwap */ Viewport__SetExtraSwap,
    /* +0x0B4 setDrawEnabled */ Viewport__SetDrawEnabled,
};
