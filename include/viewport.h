#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "basic_class.h"
#include "scene_node.h"
#include "draw_system.h"

/**
 * @file viewport.h
 * @brief Viewport, the object that renders a scene: the libgs reference view,
 * projection, lighting and fog, and a double-buffered ordering table.
 *
 * Declares the class (VIEWPORT_SLOTS, VIEWPORT_FIELDS for its subclass
 * NodeGuardedViewport), the ViewportRefView the view is kept in, and its
 * methods, defined in src/app/viewport.c except drawNode
 * (src/graphics/viewport_draw.c). The OT pair is Sony's (GsOT headers,
 * GsOT_TAG arrays, PACKET areas), so an includer takes Sony's headers first
 * (`common.h`, <libgte.h>, <libgpu.h>, <libgs.h>).
 */

typedef struct Viewport Viewport;
typedef struct ViewportMethods ViewportMethods;

/** Viewport's class id (gViewportMethods word +0x000). */
#define VIEWPORT_CLASS_ID 0x7

typedef struct ViewportRefView ViewportRefView;

/** libgs GsRVIEW2, 0x20 bytes, field for field: the argument GsSetRefView2
 * takes, hence the (GsRVIEW2 *) cast there. Kept local rather than Sony's
 * GsRVIEW2 because the game uses vp and vr as whole vectors:
 * setViewPoint/setViewRef copy each as one LongVec3, and DreamSys hands
 * &vp/&vr to InterpolateYAtZ as its two points. */
struct ViewportRefView {
    LongVec3 vp;          /**< +0x000, viewpoint: setViewPoint */
    LongVec3 vr;          /**< +0x00C, reference point: setViewRef */
    s32 rz;               /**< +0x018, twist, 20.12 (setTwist) */
    GsCOORDINATE2 *super; /**< +0x01C, the view node's GsCOORDINATE2 (AddChild) */
};

/** Viewport's slots: BasicClass's (ctor, finalize, addChild, removeChild,
 * removeAllChildren and onNotify overridden), then its own. */
/* clang-format off */
#define VIEWPORT_SLOTS(Self, CtorParams)                                                           \
    BASICCLASS_SLOTS(Self, CtorParams);                                                            \
    /* +0x040 */ void (*initDefaults)(Self *self);                  /**< @see Viewport__InitDefaults; NodeGuardedViewport: NodeGuardedViewport__InitDefaults, empty */ \
    /* +0x044 */ void (*setScreenSize)(Self *self, ScreenDims *size); /**< @see Viewport__SetScreenSize */ \
    /* +0x048 */ void (*setOtLength)(Self *self, s32 length);      /**< @see Viewport__SetOtLength */       \
    /* +0x04C */ void (*setMaxPackets)(Self *self, s32 n);         /**< @see Viewport__SetMaxPackets, before InitOt only */ \
    /* +0x050 */ void (*setPacketSize)(Self *self, s32 n);         /**< @see Viewport__SetPacketSize, before InitOt only */ \
    /* +0x054 */ void (*setProjection)(Self *self, s32 h);         /**< @see Viewport__SetProjection */     \
    /* +0x058 */ void (*slot58)(void);                             /**< @see Viewport__NoOpSlot58, empty */        \
    /* +0x05C */ void (*slot5C)(void);                             /**< @see Viewport__NoOpSlot5C, empty */        \
    /* +0x060 */ void (*setLightMode)(Self *self, s32 mode);       /**< @see Viewport__SetLightMode */      \
    /* +0x064 */ void (*setClearColor)(Self *self, ColorRgb *color); /**< @see Viewport__SetClearColor */ \
    /* +0x068 */ void (*setFarColor)(Self *self, ColorRgb *color);   /**< @see Viewport__SetFarColor */ \
    /* +0x06C */ void (*setFogNear)(Self *self, s32 fogNear);      /**< @see Viewport__SetFogNear */        \
    /* +0x070 */ void (*attachViewChild)(Self *self, BasicClass *node, LongVec3 *vp, LongVec3 *vr, Ratio16 *twist); /**< @see Viewport__AttachViewChild; NULL twist: sDefaultViewTwist */ \
    /* +0x074 */ void (*detachViewChild)(Self *self);              /**< @see Viewport__DetachViewChild */   \
    /* +0x078 */ void (*setViewPoint)(Self *self, LongVec3 *vp);  /**< @see Viewport__SetViewPoint */      \
    /* +0x07C */ void (*setViewRef)(Self *self, LongVec3 *vr);    /**< @see Viewport__SetViewRef */        \
    /* +0x080 */ void (*setTwist)(Self *self, Ratio16 *twist); /**< @see Viewport__SetTwist */       \
    /* +0x084 */ void (*slot84)(void);                             /**< @see Viewport__NoOpSlot84, empty */        \
    /* +0x088 */ void (*slot88)(void);                             /**< @see Viewport__NoOpSlot88, empty */        \
    /* +0x08C */ void (*initOt)(Self *self);                       /**< @see Viewport__InitOt */            \
    /* +0x090 */ void (*deinitOt)(Self *self);                     /**< @see Viewport__DeinitOt */          \
    /* +0x094 */ void (*onFrameClockEvent)(Self *self, BasicClass *sender, s32 event); /**< @see Viewport__OnFrameClockEvent: onNotify's class-5 (FrameClock) case */ \
    /* +0x098 */ void (*onDrawSystemEvent)(Self *self, BasicClass *sender, s32 event); /**< @see Viewport__OnDrawSystemEvent: onNotify's DrawSystem (1) case */ \
    /* +0x09C */ void (*update)(Self *self);                       /**< @see Viewport__Update; NodeGuardedViewport__Update */ \
    /* +0x0A0 */ void (*drawNode)(Self *self, SceneNode *node);   /**< @see Viewport__DrawNode (viewport_draw.c) */ \
    /* +0x0A4 */ void (*flip)(Self *self);                         /**< @see Viewport__Flip */              \
    /* +0x0A8 */ void (*setFadeBox)(Self *self, SceneNode *fadeBox); /**< @see Viewport__SetFadeBox */  \
    /* +0x0AC */ SceneNode *(*getFadeBox)(Self *self);          /**< @see Viewport__GetFadeBox */      \
    /* +0x0B0 */ void (*setExtraSwap)(Self *self, s32 on);          /**< @see Viewport__SetExtraSwap */          \
    /* +0x0B4 */ void (*setDrawEnabled)(Self *self, s32 on)        /**< @see Viewport__SetDrawEnabled */
/* clang-format on */

/** Viewport's fields, BasicClass's first. maxPackets and packetSize
 * multiply to each buffer's packet area (InitOt; defaults 2000 and 64); which
 * is the count and which the size comes from Sony's PACKETMAX * size idiom
 * and the values callers pass, not from the code. */
/* clang-format off */
#define VIEWPORT_FIELDS(Methods)                                                                   \
    BASICCLASS_FIELDS(Methods);                                                                    \
    /* +0x00C */ DrawSystem *drawSystem;  /**< the class-1 child (AddChild); Flip's getActiveBuffer/swapBuffers */ \
    /* +0x010 */ SceneNode *viewNode;    /**< the class-4 child (AddChild); refView.super is its coord2 */ \
    /* +0x014 */ ViewportRefView refView; /**< GsSetRefView2's argument */                          \
    /* +0x034 */ ScreenDims screenSize; /**< setScreenSize; InitDefaults' default size */          \
    /* +0x03C */ s32 otLength;            /**< GsOT length: 1 << otLength tags; drawNode's priority range */ \
    /* +0x040 */ s32 projH;               /**< GsSetProjection's h; drawNode's sprite projection */  \
    /* +0x044 */ s32 maxPackets;          /**< maxPackets * packetSize: each buffer's packet area */  \
    /* +0x048 */ s32 packetSize;          /**< bytes per packet (see above) */                  \
    /* +0x04C */ s32 nearZ;               /**< GsSetNearClip; drawNode's sprite near limit */        \
    /* +0x050 */ s32 farZ;                /**< zDiv = (farZ - nearZ) / (1 << otLength) + 1 */        \
    /* +0x054 */ s32 lightMode;           /**< GsSetLightMode; 1 or 3 also sets the fog */           \
    /* +0x058 */ ColorRgb clearColor;  /**< Flip's GsSortClear */                                 \
    /* +0x05B */ ColorRgb farColor;    /**< Update's SetFarColor */                               \
    /* +0x05E */ u8 pad05E[2];                                                                     \
    /* +0x060 */ s32 fogNear;             /**< Update's SetFogNear */                                \
    /* +0x064 */ u8 pad064[0x070 - 0x064];                                                         \
    /* +0x070 */ s32 otReady;             /**< InitOt sets, DeinitOt clears; Update/Flip need it */  \
    /* +0x074 */ s32 otIndex;             /**< the half being drawn; Flip takes the next one */      \
    /* +0x078 */ GsOT *ot[2];             /**< InitOt's two GsOT headers */                          \
    /* +0x080 */ GsOT_TAG *otTags[2];     /**< each header's org: its tag array */                   \
    /* +0x088 */ PACKET *workBase[2];     /**< each half's packet area: GsSetWorkBase */             \
    /* +0x090 */ s32 clockEventCount;     /**< counts FrameClock notifications (OnFrameClockEvent) */     \
    /* +0x094 */ u8 pad094[0x098 - 0x094];                                                         \
    /* +0x098 */ s32 zDiv;                /**< Update: the depth per OT tag; drawNode's sprite z */  \
    /* +0x09C */ u8 pad09C[0x0AC - 0x09C];                                                         \
    /* +0x0AC */ SceneNode *sceneRoot;   /**< the ctor's New_SceneNode; Update draws it; finalize releases it */ \
    /* +0x0B0 */ SceneNode *fadeBox;   /**< the ctor's New_FadeBox, attached under sceneRoot */ \
    /* +0x0B4 */ s32 extraSwap;           /**< Flip: nonzero swaps before and after drawing buffer 0 */ \
    /* +0x0B8 */ s32 drawEnabled          /**< Flip: 0 skips the clear and draw; default 1 */
/* clang-format on */

/** Viewport's method table: BasicClass's slots and VIEWPORT_SLOTS' own. */
struct ViewportMethods {
    VIEWPORT_SLOTS(Viewport, (Viewport * self));
};

/**
 * Viewport -- the object that renders a scene: class id 0x7, method table
 * gViewportMethods, a BasicClass subclass. Its methods are in
 * src/app/viewport.c, except drawNode, in src/graphics/viewport_draw.c. IntermediateBase and
 * TaskCore hold one as `viewport` (New_Viewport, or the caller's own). The
 * ctor chains to BasicClass's first, and NodeGuardedViewport's (0x17,
 * node_guarded_viewport.h) chains to this one, so the id tree
 * (0x0 -> 0x7 -> 0x17) is the ctor chain. NodeGuardedViewport expands these
 * macros. The object is 0xBC bytes (New_Viewport).
 *
 * What it does:
 *  - it holds a libgs GsRVIEW2 in `refView`: AttachViewChild and Update hand
 *    it to GsSetRefView2, setViewPoint/setViewRef/setTwist write its vp, vr
 *    and rz, and AddChild points its `super` at the view node's
 *    GsCOORDINATE2 (SceneNode::coord2), which Update marks for recompute
 *    (flg = 0) every frame;
 *  - Update, on each FrameClock tick, sets the projection distance, near
 *    clip, light mode and fog (GsSetProjection(projH), GsSetNearClip(nearZ),
 *    GsSetLightMode, SetFarColor, SetFogNear) and draws the scene tree
 *    through drawNode: the view node, `sceneRoot` (a New_SceneNode), and the
 *    root of the view node's parent chain (GetRootNode);
 *  - InitOt allocates a double-buffered pair of GsOT headers, each followed
 *    by its 1 << otLength tags and its packet area; drawNode sorts into
 *    ot[otIndex], and Flip, on each DrawSystem VSync, clears and draws that
 *    half (GsSortClear, GsDrawOt), then takes the next index from the
 *    DrawSystem child's getActiveBuffer and swapBuffers slots;
 *  - the setters before InitOt configure it (screen size, OT length, packet
 *    count and size, which only take before InitOt, projection, light mode,
 *    clear and far colours, fog near); four slots are empty.
 *
 * Children are cached by their class-id nibble (AddChild/RemoveChild): 1 is
 * the DrawSystem (draw_system.h), 4 a SceneNode (scene_node.h), the node the
 * view is attached to.
 *
 * Not settled: fadeBox holds a FadeBox (0x164, below BoxFill, 0x64), whose
 * +0x04C override (BoxFill__AttachToParent) takes a two-word screen position
 * where SceneNode's attachToParent slot takes a LongVec3 offset; the ctor and
 * Viewport__SetFadeBox pass sFadeBoxAttachPos (-100, -100) through the
 * inherited slot with a pointer cast. drawNode (Viewport__DrawNode) reads its
 * node through viewport_draw.c's own DrawNode view, so it is not prototyped
 * here.
 */
struct Viewport {
    VIEWPORT_FIELDS(ViewportMethods);
};

/** Viewport's own method table. */
extern ViewportMethods gViewportMethods;

/** @brief Viewport's method-table getter.
 * @return &gViewportMethods */
extern ViewportMethods *GetViewportMethods(void);

/** @brief Constructor: BasicClass's, this table, no children, a new
 * SceneNode as sceneRoot with a new FadeBox attached under it, then
 * initDefaults.
 * @param self the viewport */
void Viewport__Viewport(Viewport *self);

/** @brief Finalize: closes the OT, detaches the view, releases sceneRoot and
 * the fade box, then BasicClass's finalize.
 * @param self the viewport */
void Viewport__Finalize(Viewport *self);

/** @brief addChild override: BasicClass's, then caches a SceneNode child as
 * the view node (refView.super = its coord2) and a DrawSystem child as
 * drawSystem.
 * @param self the viewport
 * @param child the object to add */
void Viewport__AddChild(Viewport *self, BasicClass *child);

/** @brief removeChild override: clears the cache AddChild filled for this
 * child's class, then BasicClass's.
 * @param self the viewport
 * @param child the object to remove */
void Viewport__RemoveChild(Viewport *self, BasicClass *child);

/** @brief removeAllChildren override: clears the view node, refView.super and
 * drawSystem, then BasicClass's.
 * @param self the viewport */
void Viewport__RemoveAllChildren(Viewport *self);

/** @brief onNotify override: BasicClass's, then a FrameClock sender goes to
 * onFrameClockEvent and the DrawSystem to onDrawSystemEvent.
 * @param self the viewport
 * @param sender the notifying object
 * @param event its event code */
void Viewport__OnNotify(Viewport *self, BasicClass *sender, s32 event);

/** @brief Sets every field's default: no OT, the default screen size, an OT
 * of 1 << 13 tags, 2000 packets of 64 bytes, projection 256, near 10, far
 * 65536, normal lighting, fog near 20000, black clear and far colours,
 * drawing on.
 * @param self the viewport */
void Viewport__InitDefaults(Viewport *self);

/** @brief Sets the screen size.
 * @param self the viewport
 * @param size width and height */
void Viewport__SetScreenSize(Viewport *self, ScreenDims *size);

/** @brief Sets the OT length: 1 << length tags per buffer.
 * @param self the viewport
 * @param length the log2 of the tag count */
void Viewport__SetOtLength(Viewport *self, s32 length);

/** @brief Sets the packet count per buffer; ignored once the OT is
 * allocated.
 * @param self the viewport
 * @param maxPackets the count */
void Viewport__SetMaxPackets(Viewport *self, s32 maxPackets);

/** @brief Sets the bytes per packet; ignored once the OT is allocated.
 * @param self the viewport
 * @param size the size */
void Viewport__SetPacketSize(Viewport *self, s32 size);

/** @brief Sets the projection distance Update hands GsSetProjection.
 * @param self the viewport
 * @param h the distance */
void Viewport__SetProjection(Viewport *self, s32 h);

/** @brief Slot +0x058: empty. */
void Viewport__NoOpSlot58(void);

/** @brief Slot +0x05C: empty. */
void Viewport__NoOpSlot5C(void);

/** @brief Sets the libgs light mode; the two fog modes also set the fog
 * each frame.
 * @param self the viewport
 * @param mode a GsLMODE_* value */
void Viewport__SetLightMode(Viewport *self, s32 mode);

/** @brief Sets the colour Flip clears the screen to.
 * @param self the viewport
 * @param color the colour */
void Viewport__SetClearColor(Viewport *self, ColorRgb *color);

/** @brief Sets the fog's far colour.
 * @param self the viewport
 * @param color the colour */
void Viewport__SetFarColor(Viewport *self, ColorRgb *color);

/** @brief Sets the fog's near distance.
 * @param self the viewport
 * @param fogNear the distance */
void Viewport__SetFogNear(Viewport *self, s32 fogNear);

/** @brief Attaches the view to `node` once (nothing while a view node is
 * set): adds it as a child, sets the viewpoint, reference point and twist,
 * and hands the view to GsSetRefView2.
 * @param self the viewport
 * @param node the SceneNode the view hangs from
 * @param vp the viewpoint
 * @param vr the reference point
 * @param twist the twist, or NULL for sDefaultViewTwist */
void Viewport__AttachViewChild(Viewport *self, BasicClass *node, LongVec3 *vp, LongVec3 *vr,
                               Ratio16 *twist);

/** @brief Removes the view node as a child, if there is one.
 * @param self the viewport */
void Viewport__DetachViewChild(Viewport *self);

/** @brief Sets the viewpoint, only while a view node is set.
 * @param self the viewport
 * @param vp the viewpoint */
void Viewport__SetViewPoint(Viewport *self, LongVec3 *vp);

/** @brief Sets the reference point, only while a view node is set.
 * @param self the viewport
 * @param vr the reference point */
void Viewport__SetViewRef(Viewport *self, LongVec3 *vr);

/** @brief Sets the twist from a ratio, as 20.12 fixed point, only while a
 * view node is set.
 * @param self the viewport
 * @param twist num / den */
void Viewport__SetTwist(Viewport *self, Ratio16 *twist);

/** @brief Slot +0x084: empty. */
void Viewport__NoOpSlot84(void);

/** @brief Slot +0x088: empty. */
void Viewport__NoOpSlot88(void);

/** @brief Allocates and clears the two ordering tables once: one pool block
 * holding both halves, each a GsOT header, its tags and its packet area.
 * @param self the viewport */
void Viewport__InitOt(Viewport *self);

/** @brief Waits for drawing to finish and frees the ordering tables, if
 * allocated.
 * @param self the viewport */
void Viewport__DeinitOt(Viewport *self);

/** @brief Counts every FrameClock event and runs update on a running or
 * paused tick (not on FRAMECLOCK_EVENT_STOPPED).
 * @param self the viewport
 * @param sender the FrameClock
 * @param event its event code */
void Viewport__OnFrameClockEvent(Viewport *self, BasicClass *sender, s32 event);

/** @brief Runs flip on the DrawSystem's VSync event.
 * @param self the viewport
 * @param sender the DrawSystem
 * @param event its event code */
void Viewport__OnDrawSystemEvent(Viewport *self, BasicClass *sender, s32 event);

/** @brief Per-frame draw, once the OT exists: draws the view node (when it
 * has a parent), sets the projection, near clip, light mode and fog and the
 * reference view, sets up this half's packet area and cleared OT, then draws
 * sceneRoot and the view node's root into it.
 * @param self the viewport */
void Viewport__Update(Viewport *self);

/** @brief Draws a SceneNode and its SceneNode children into this half's OT,
 * each by its kind (BgLayer, GridCell, BoxFill, ScreenSprite, a world-space
 * Sprite, or a TMD object); defined in src/graphics/viewport_draw.c.
 * @param self the viewport
 * @param node the node to draw */
void Viewport__DrawNode(Viewport *self, SceneNode *node);

/** @brief Takes the buffer index from the DrawSystem; when drawing is
 * enabled, resets the GPU, swaps, sorts the clear into this half's OT and
 * draws it (with extra swaps on buffer 0 when extraSwap is set); then moves
 * otIndex to the other half.
 * @param self the viewport */
void Viewport__Flip(Viewport *self);

/** @brief Replaces the fade box, only while no view node is set: releases
 * the old one and attaches the new one under sceneRoot at (-100, -100).
 * @param self the viewport
 * @param fadeBox the new fade box, or NULL */
void Viewport__SetFadeBox(Viewport *self, SceneNode *fadeBox);

/** @brief The fade box.
 * @param self the viewport
 * @return fadeBox */
SceneNode *Viewport__GetFadeBox(Viewport *self);

/** @brief Sets whether Flip swaps once more before and after drawing
 * buffer 0.
 * @param self the viewport
 * @param on the flag */
void Viewport__SetExtraSwap(Viewport *self, s32 on);

/** @brief Sets whether Flip clears and draws.
 * @param self the viewport
 * @param on the flag */
void Viewport__SetDrawEnabled(Viewport *self, s32 on);

/** @brief Allocates a Viewport from the pool and runs its ctor.
 * @return the new viewport, or NULL when the pool allocation fails */
Viewport *New_Viewport(void);

/** @brief Follows `parent` from `node` to the top of its hierarchy.
 * @param node any SceneNode
 * @return the root */
SceneNode *GetRootNode(SceneNode *node);

#endif
