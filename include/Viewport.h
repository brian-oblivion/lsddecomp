#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "BasicClass.h"
#include "SceneNode.h"
#include "DrawSystem.h"

/*
 * Viewport -- the object that renders a scene (class id 0x7, method table
 * gViewportMethods): a BasicClass subclass. Its methods are in src/app/Task.c,
 * except drawNode, in src/graphics/ViewportDraw.c. IntermediateBase and
 * TaskCore hold one as `viewport` (New_Viewport, or the caller's own).
 *
 * The name is for what the class's own methods do:
 *  - it holds a libgs GsRVIEW2 at +0x014: AttachViewChild and Update hand
 *    &refView to GsSetRefView2, setViewPoint/setViewRef/setTwist write its
 *    vp, vr and rz, and AddChild points its `super` at the view node's
 *    GsCOORDINATE2 (SceneNode::coord2), which Update marks for recompute
 *    (flg = 0) every frame;
 *  - Update sets the projection distance, near clip, light mode and fog
 *    (GsSetProjection(projH), GsSetNearClip(nearZ), GsSetLightMode,
 *    SetFarColor, SetFogNear) and draws the scene tree through drawNode:
 *    the view node, `sceneRoot` (a New_SceneNode), and the root of the view
 *    node's parent chain (GetRootNode);
 *  - InitOt allocates a double-buffered pair of 0x14-byte GsOT headers, each
 *    followed by its 1 << otLength tags and its packet area; drawNode sorts
 *    into ot[otIndex], and Flip clears and draws that half (GsSortClear,
 *    GsDrawOt), then takes the next index from the DrawSystem child's
 *    getActiveBuffer and swapBuffers slots.
 *
 * Children are cached by their class-id nibble (AddChild/RemoveChild): 1 is
 * the DrawSystem (gDrawSystemMethods, id 0x1), 4 a SceneNode, the node the view is
 * attached to. Each is typed by its class (include/DrawSystem.h,
 * include/SceneNode.h).
 *
 * The ctor chains to BasicClass's first (GetBasicClassMethods()->ctor),
 * and gNodeGuardedViewportMethods's (0x17, include/NodeGuardedViewport.h) chains to this one,
 * so the id tree (0x0 -> 0x7 -> 0x17) is the ctor chain. NodeGuardedViewport expands
 * these macros.
 *
 * Not settled here: fadeBox holds a FadeBox (0x164, below gBoxFillMethods,
 * 0x64), whose +0x04C override (BoxFill__AttachToParent) takes a two-word screen
 * position where SceneNode's attachToParent slot takes a LongVec3 offset;
 * the ctor and Viewport__SetFadeBox pass gFadeBoxAttachPos (-100, -100) through the
 * inherited slot with a pointer cast. maxPackets and packetSize multiply
 * to each buffer's packet area (InitOt; defaults 2000 and 64); which is the
 * count and which the size comes from Sony's PACKETMAX * size idiom and the
 * values callers pass, not from the code. drawNode (Viewport__DrawNode) reads its
 * node through ViewportDraw.c's own DrawNode view, so it is not prototyped
 * here.
 *
 * The OT pair is Sony's (GsOT headers, GsOT_TAG arrays, PACKET areas), so an
 * includer takes Sony's headers first (`common.h`, <libgte.h>, <libgpu.h>,
 * <libgs.h>).
 *
 * The object is 0xBC bytes (New_Viewport).
 */

typedef struct Viewport Viewport;
typedef struct ViewportMethods ViewportMethods;
typedef struct ViewportSize ViewportSize;
typedef struct ViewportRgb ViewportRgb;
typedef struct ViewportRefView ViewportRefView;

/* The screen size: drawNode reads it as the width and height the box and
 * screen-space sprite paths take percentages of. Defaults 256 x 240
 * (gDefaultViewportWidth, gDefaultViewportHeight). setScreenSize copies it whole: retail loads both
 * words before storing either. */
struct ViewportSize {
    s32 width;
    s32 height;
};

/* An RGB triple, copied whole (retail loads all three bytes before storing
 * any). Written as signed bytes, read unsigned by GsSortClear/SetFarColor. */
struct ViewportRgb {
    s8 r;
    s8 g;
    s8 b;
};

/* libgs GsRVIEW2, 0x20 bytes, field for field: the argument GsSetRefView2
 * takes. Kept local rather than Sony's GsRVIEW2 because the game uses vp and
 * vr as whole vectors: setViewPoint/setViewRef copy each as one LongVec3
 * (three field stores do not match), and DreamSys hands &vp/&vr to
 * InterpolateKeyframeValue as its two points. Sony's flat vpx..vrz would put
 * a LongVec3 cast at each copy to save the two at GsSetRefView2. */
struct ViewportRefView {
    LongVec3 vp;          /* +0x000, viewpoint: setViewPoint */
    LongVec3 vr;          /* +0x00C, reference point: setViewRef */
    s32 rz;               /* +0x018, twist, 20.12 (setTwist) */
    GsCOORDINATE2 *super; /* +0x01C, the view node's GsCOORDINATE2 (AddChild) */
};

/* BasicClass's slots, then this class's own. `tools/classtable.py
 * gViewportMethods --vs gBasicClassMethods` lists the overrides of the inherited
 * ones (ctor, finalize, addChild, removeChild, removeAllChildren, onNotify). */
/* clang-format off */
#define VIEWPORT_SLOTS(Self, CtorParams)                                                           \
    BASICCLASS_SLOTS(Self, CtorParams);                                                            \
    /* +0x040 */ void (*initDefaults)(Self *self);                  /* Viewport__InitDefaults; NodeGuardedViewport: NodeGuardedViewport__InitDefaults, empty */ \
    /* +0x044 */ void (*setScreenSize)(Self *self, ViewportSize *size); /* Viewport__SetScreenSize */ \
    /* +0x048 */ void (*setOtLength)(Self *self, s32 length);      /* Viewport__SetOtLength */       \
    /* +0x04C */ void (*setMaxPackets)(Self *self, s32 n);         /* Viewport__SetMaxPackets, before InitOt only */ \
    /* +0x050 */ void (*setPacketSize)(Self *self, s32 n);         /* Viewport__SetPacketSize, before InitOt only */ \
    /* +0x054 */ void (*setProjection)(Self *self, s32 h);         /* Viewport__SetProjection */     \
    /* +0x058 */ void (*slot58)(void);                             /* Viewport__NoOpSlot58, empty */        \
    /* +0x05C */ void (*slot5C)(void);                             /* Viewport__NoOpSlot5C, empty */        \
    /* +0x060 */ void (*setLightMode)(Self *self, s32 mode);       /* Viewport__SetLightMode */      \
    /* +0x064 */ void (*setClearColor)(Self *self, ViewportRgb *color); /* Viewport__SetClearColor */ \
    /* +0x068 */ void (*setFarColor)(Self *self, ViewportRgb *color);   /* Viewport__SetFarColor */ \
    /* +0x06C */ void (*setFogNear)(Self *self, s32 fogNear);      /* Viewport__SetFogNear */        \
    /* +0x070 */ void (*attachViewChild)(Self *self, BasicClass *node, LongVec3 *vp, LongVec3 *vr, Ratio16 *twist); /* Viewport__AttachViewChild; NULL twist: gDefaultViewTwist */ \
    /* +0x074 */ void (*detachViewChild)(Self *self);              /* Viewport__DetachViewChild */   \
    /* +0x078 */ void (*setViewPoint)(Self *self, LongVec3 *vp);  /* Viewport__SetViewPoint */      \
    /* +0x07C */ void (*setViewRef)(Self *self, LongVec3 *vr);    /* Viewport__SetViewRef */        \
    /* +0x080 */ void (*setTwist)(Self *self, Ratio16 *twist); /* Viewport__SetTwist */       \
    /* +0x084 */ void (*slot84)(void);                             /* Viewport__NoOpSlot84, empty */        \
    /* +0x088 */ void (*slot88)(void);                             /* Viewport__NoOpSlot88, empty */        \
    /* +0x08C */ void (*initOt)(Self *self);                       /* Viewport__InitOt */            \
    /* +0x090 */ void (*deinitOt)(Self *self);                     /* Viewport__DeinitOt */          \
    /* +0x094 */ void (*onNotifyTag5)(Self *self, BasicClass *sender, s32 event); /* Viewport__OnFrameClockEvent: onNotify's class-5 (FrameClock) case */ \
    /* +0x098 */ void (*onNotifyTag1)(Self *self, BasicClass *sender, s32 event); /* Viewport__OnDrawSystemEvent: onNotify's DrawSystem (1) case */ \
    /* +0x09C */ void (*update)(Self *self);                       /* Viewport__Update; NodeGuardedViewport__Update */ \
    /* +0x0A0 */ void (*drawNode)(Self *self, SceneNode *node);   /* Viewport__DrawNode (ViewportDraw) */ \
    /* +0x0A4 */ void (*flip)(Self *self);                         /* Viewport__Flip */              \
    /* +0x0A8 */ void (*setFadeBox)(Self *self, SceneNode *handle); /* Viewport__SetFadeBox */  \
    /* +0x0AC */ SceneNode *(*getFadeBox)(Self *self);          /* Viewport__GetFadeBox */      \
    /* +0x0B0 */ void (*setExtraSwap)(Self *self, s32 on);          /* Viewport__SetExtraSwap */          \
    /* +0x0B4 */ void (*setDrawEnabled)(Self *self, s32 on)        /* Viewport__SetDrawEnabled */
/* clang-format on */

/* clang-format off */
#define VIEWPORT_FIELDS(Methods)                                                                   \
    BASICCLASS_FIELDS(Methods);                                                                    \
    /* +0x00C */ DrawSystem *drawSystem;  /* the class-1 child (AddChild); Flip's getActiveBuffer/swapBuffers */ \
    /* +0x010 */ SceneNode *viewNode;    /* the class-4 child (AddChild); refView.super is its coord2 */ \
    /* +0x014 */ ViewportRefView refView; /* GsSetRefView2's argument */                          \
    /* +0x034 */ ViewportSize screenSize;                                                          \
    /* +0x03C */ s32 otLength;            /* GsOT length: 1 << otLength tags; drawNode's priority range */ \
    /* +0x040 */ s32 projH;               /* GsSetProjection's h; drawNode's sprite projection */  \
    /* +0x044 */ s32 maxPackets;          /* maxPackets * packetSize: each buffer's packet area */  \
    /* +0x048 */ s32 packetSize;          /* bytes per packet (see the banner) */                  \
    /* +0x04C */ s32 nearZ;               /* GsSetNearClip; drawNode's sprite near limit */        \
    /* +0x050 */ s32 farZ;                /* zDiv = (farZ - nearZ) / (1 << otLength) + 1 */        \
    /* +0x054 */ s32 lightMode;           /* GsSetLightMode; 1 or 3 also sets the fog */           \
    /* +0x058 */ ViewportRgb clearColor;  /* Flip's GsSortClear */                                 \
    /* +0x05B */ ViewportRgb farColor;    /* Update's SetFarColor */                               \
    /* +0x05E */ u8 pad05E[2];                                                                     \
    /* +0x060 */ s32 fogNear;             /* Update's SetFogNear */                                \
    /* +0x064 */ u8 pad064[0x070 - 0x064];                                                         \
    /* +0x070 */ s32 otReady;             /* InitOt sets, DeinitOt clears; Update/Flip need it */  \
    /* +0x074 */ s32 otIndex;             /* the half being drawn; Flip takes the next one */      \
    /* +0x078 */ GsOT *ot[2];             /* InitOt's two GsOT headers */                          \
    /* +0x080 */ GsOT_TAG *otTags[2];     /* each header's org: its tag array */                   \
    /* +0x088 */ PACKET *workBase[2];     /* each half's packet area: GsSetWorkBase */             \
    /* +0x090 */ s32 clockEventCount;     /* counts FrameClock notifications (OnNotifyTag5) */     \
    /* +0x094 */ u8 pad094[0x098 - 0x094];                                                         \
    /* +0x098 */ s32 zDiv;                /* Update: the depth per OT tag; drawNode's sprite z */  \
    /* +0x09C */ u8 pad09C[0x0AC - 0x09C];                                                         \
    /* +0x0AC */ SceneNode *sceneRoot;   /* the ctor's New_SceneNode; Update draws it; finalize releases it */ \
    /* +0x0B0 */ SceneNode *fadeBox;   /* the ctor's New_FadeBox, attached under sceneRoot */ \
    /* +0x0B4 */ s32 extraSwap;           /* Flip: nonzero swaps before and after drawing buffer 0 */ \
    /* +0x0B8 */ s32 drawEnabled          /* Flip: 0 skips the clear and draw; default 1 */
/* clang-format on */

struct ViewportMethods {
    VIEWPORT_SLOTS(Viewport, (Viewport * self));
};

struct Viewport {
    VIEWPORT_FIELDS(ViewportMethods);
};

extern ViewportMethods gViewportMethods;
extern ViewportMethods *GetViewportMethods(void); /* returns &gViewportMethods */

/* The occupants of gViewportMethods, in slot order (drawNode excepted, see
 * the banner), then the class's allocator and one helper. */
void Viewport__Viewport(Viewport *self);
void Viewport__Finalize(Viewport *self);
void Viewport__AddChild(Viewport *self, BasicClass *child);
void Viewport__RemoveChild(Viewport *self, BasicClass *child);
void Viewport__RemoveAllChildren(Viewport *self);
void Viewport__OnNotify(Viewport *self, BasicClass *sender, s32 event);
void Viewport__InitDefaults(Viewport *self);
void Viewport__SetScreenSize(Viewport *self, ViewportSize *size);
void Viewport__SetOtLength(Viewport *self, s32 length);
void Viewport__SetMaxPackets(Viewport *self, s32 value);
void Viewport__SetPacketSize(Viewport *self, s32 value);
void Viewport__SetProjection(Viewport *self, s32 h);
void Viewport__NoOpSlot58(void);
void Viewport__NoOpSlot5C(void);
void Viewport__SetLightMode(Viewport *self, s32 mode);
void Viewport__SetClearColor(Viewport *self, ViewportRgb *color);
void Viewport__SetFarColor(Viewport *self, ViewportRgb *color);
void Viewport__SetFogNear(Viewport *self, s32 fogNear);
void Viewport__AttachViewChild(Viewport *self, BasicClass *node, LongVec3 *vp, LongVec3 *vr,
                               Ratio16 *twist);
void Viewport__DetachViewChild(Viewport *self);
void Viewport__SetViewPoint(Viewport *self, LongVec3 *vp);
void Viewport__SetViewRef(Viewport *self, LongVec3 *vr);
void Viewport__SetTwist(Viewport *self, Ratio16 *twist);
void Viewport__NoOpSlot84(void);
void Viewport__NoOpSlot88(void);
void Viewport__InitOt(Viewport *self);
void Viewport__DeinitOt(Viewport *self);
void Viewport__OnFrameClockEvent(Viewport *self, BasicClass *sender, s32 event);
void Viewport__OnDrawSystemEvent(Viewport *self, BasicClass *sender, s32 event);
void Viewport__Update(Viewport *self);
void Viewport__Flip(Viewport *self);
void Viewport__SetFadeBox(Viewport *self, SceneNode *handle);
SceneNode *Viewport__GetFadeBox(Viewport *self);
void Viewport__SetExtraSwap(Viewport *self, s32 value);
void Viewport__SetDrawEnabled(Viewport *self, s32 on);

Viewport *New_Viewport(void);            /* BMemPMgrAlloc(0xBC), then ctor */
SceneNode *GetRootNode(SceneNode *node); /* follow `parent` to the top */

#endif
