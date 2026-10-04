#ifndef TOD_ACTOR_H
#define TOD_ACTOR_H

#include "actor.h"
#include "file_resource.h"

/**
 * @file tod_actor.h
 * @brief TodActor, an Actor that plays a TOD animation over one Actor part
 *        per TOD object: its object, method table, getter and methods.
 */

typedef struct TodActor TodActor;
typedef struct TodActorMethods TodActorMethods;

/* Tags completed in the unit that reads them (src/world/tod_actor.c), so
 * that any header may repeat these declarations: the ModelData
 * (include/model_data.h), the ctor's descriptor (+0x00C a ModelData to
 * borrow) and the sound bank (include/vab_stream_obj.h). */
struct ModelData;
struct TodActorDesc;
struct VabStreamObj;

/**
 * Actor's slots, then TodActor's own, for TodActorMethods and Entity's table
 * to expand first. The inherited slots keep Actor's names; gTodActorMethods
 * overrides +0x008 (TodActor__TodActor), +0x00C (TodActor__Finalize), +0x038
 * (TodActor__OnNotify), +0x040 (TodActor__Reset), +0x04C/+0x050
 * (TodActor__AttachToParent/DetachFromParent), +0x060
 * (TodActor__SetDisplay), +0x070 (TodActor__SetLightMode) and +0x098
 * (TodActor__Update). Each own slot's comment names its occupant.
 */
/* clang-format off */
#define TODACTOR_SLOTS(Self, CtorParams)                                                         \
    ACTOR_SLOTS(Self, CtorParams);                                                                 \
    /* +0x0F0 */ void (*setMainPartNotifies)(Self *self, s32 on); /* TodActor__SetMainPartNotifies */ \
    /* +0x0F4 */ s32 (*setupModelData)(Self *self, void *desc); /* TodActor__SetupModelData: 0 on success */ \
    /* +0x0F8 */ void (*teardownModelData)(Self *self);         /* TodActor__TeardownModelData */ \
    /* +0x0FC */ s32 (*findPartIndex)(Self *self, s32 id);      /* TodActor__FindPartIndex: -1 when no part has the id; called directly */ \
    /* +0x100 */ s32 (*setupParts)(Self *self);                 /* TodActor__SetupParts: 0 on success */ \
    /* +0x104 */ void (*teardownParts)(Self *self);             /* TodActor__TeardownParts */    \
    /* +0x108 */ void (*tick)(Self *self);                      /* TodActor__Tick */             \
    /* +0x10C */ void (*selectTickCallback)(Self *self, s32 which); /* TodActor__SelectTickCallback: TICK_CALLBACK_A..C */ \
    /* +0x110 */ s32 (*enableTickCallback)(Self *self);         /* TodActor__EnableTickCallback */ \
    /* +0x114 */ void (*disableTickCallback)(Self *self);       /* TodActor__DisableTickCallback */ \
    /* +0x118 */ void *tickMoveZ;                             /* TodActor__TickMoveZ; only the VALUE is read (selectTickCallback) */ \
    /* +0x11C */ void *tickCallbackB;                           /* TodActor__TickCallbackB, empty (Entity: Entity__TickSoundCue) */ \
    /* +0x120 */ void *tickCallbackC;                           /* TodActor__TickCallbackC, empty */ \
    /* +0x124 */ void (*playTone)(Self *self, s32 index);       /* TodActor__PlayTone: sound's playTone(index, 0x6E, 0x6E); not dispatched in C */ \
    /* +0x128 */ void (*setTod)(Self *self, s32 index);         /* TodActor__SetTod */           \
    /* +0x12C */ s32 (*playTod)(Self *self);                    /* TodActor__PlayTod */          \
    /* +0x130 */ void (*stopTod)(Self *self);                   /* TodActor__StopTod */          \
    /* +0x134 */ u8 *(*applyTodFrame)(Self *self, void *frame, s32 flag); /* TodActor__ApplyTodFrame: returns the next frame */ \
    /* +0x138 */ void *(*applyTodPacket)(Self *self, void *packet, void *extra); /* TodActor__ApplyTodPacket: returns the next packet */ \
    /* +0x13C */ void (*linkPeer)(Self *self, TodActor *other); /* TodActor__LinkPeer: addChild both ways, peer = other */ \
    /* +0x140 */ void (*unlinkPeer)(Self *self)                 /* TodActor__UnlinkPeer */
/* clang-format on */

/**
 * Actor's fields, then TodActor's own, for TodActor and Entity's object to
 * expand first. The object is 0x98 bytes (New_TodActor); Entity's own fields
 * start at +0x098.
 */
/* clang-format off */
#define TODACTOR_FIELDS(Methods)                                                                 \
    ACTOR_FIELDS(Methods);                                                                         \
    /* +0x058 */ struct VabStreamObj *sound; /* the ctor's second argument, the sound bank playTone plays on; NULL when none */ \
    /* +0x05C */ struct ModelData *modelData; /* borrowed from the ctor's descriptor or made by New_ModelData; NULL when none */ \
    /* +0x060 */ s32 ownsModelData;        /* 1 when New_ModelData made modelData; only an owned one is released */ \
    /* +0x064 */ s32 mainPartNotifies;     /* setMainPartNotifies (1 in reset); while 1, TickMoveZ has mainPart send the move notification */ \
    /* +0x068 */ Actor *mainPart;          /* parts[the index scanPackets writes first]; NULL after the parts are destroyed */ \
    /* +0x06C */ s32 partCount;            /* entries in parts/partIds */                          \
    /* +0x070 */ Actor **parts;            /* one New_Actor per TOD object */                      \
    /* +0x074 */ u8 *partIds;              /* each part's TOD object id; findPartIndex searches it */ \
    /* +0x078 */ void *tickCallback;       /* tickMoveZ, tickCallbackB or tickCallbackC as selected; tick calls it while tickCallbackEnabled */ \
    /* +0x07C */ s32 todIndex;             /* the current TOD's index in the TodSet */             \
    /* +0x080 */ s32 todFrameCount;        /* the current TOD's frame count (setTod) */            \
    /* +0x084 */ s32 todFrame;             /* the current frame number; wraps at todFrameCount */  \
    /* +0x088 */ void *todFramePtr;        /* the next frame to apply */                           \
    /* +0x08C */ s32 tickCallbackEnabled;  /* enableTickCallback / disableTickCallback */          \
    /* +0x090 */ s32 todPlaying;           /* playTod / stopTod; gates frame advance in tick */    \
    /* +0x094 */ TodActor *peer           /* linkPeer's other, NULL after unlinkPeer. The object is 0x98 bytes (New_TodActor) */
/* clang-format on */

/** TodActor's method table: TODACTOR_SLOTS with TodActor's ctor parameters. */
struct TodActorMethods {
    TODACTOR_SLOTS(TodActor, (TodActor * self, void *desc, void *sound));
};

/**
 * TodActor: an Actor animated by a TOD. It owns one Actor "part" per object
 * of a TOD animation and plays the TOD's frames over them. Class id 0x234,
 * table gTodActorMethods, parent Actor (include/actor.h), whose ctor its own
 * chains to; methods in src/world/tod_actor.c. One class derives from it,
 * Entity (0x1F234, include/entity.h), and it is only ever built as one:
 * New_Entity (from dream_aux.c's SetDreamAuxWorld and
 * SpawnDreamAuxTriggerEntity) runs this ctor first; New_TodActor has no
 * caller.
 *
 * Lifecycle.
 * - ctor(desc, sound): Actor's ctor, then setupModelData(desc). The ModelData
 *   (include/model_data.h: a TMD LinkResource and a TodSet over one file) at
 *   desc +0x00C is borrowed, or, when there is none, New_ModelData(desc) makes
 *   one this object owns (ownsModelData). setupParts then asks the ModelData
 *   for the TOD's object ids (scanPackets) and makes one Actor (New_Actor) per
 *   object: `parts`, their ids in `partIds`, `mainPart` the one the scan
 *   names first. The ModelData becomes a child, and reset runs. `sound` is
 *   kept in `sound`. Returns self, or NULL.
 * - reset: setDisplay(0), mainPartNotifies 1, lastOffsetValue 300, tick
 *   callback 'A' selected but disabled, TOD 0 set and stopped, and mainPart's
 *   model linked as this object's own.
 * - update (+0x098): event 2 (the FrameClock ticker) runs tick, event 4
 *   releases the object.
 * - finalize: teardownModelData, which releases the parts, and the ModelData
 *   too when this object made it. onNotify releases the object when a
 *   ModelData sends event 1 while its own ModelData is borrowed.
 *
 * Playback. setTod(index) selects a TOD of the TodSet and applies its first
 * frame; playTod/stopTod gate frame advance. tick runs the selected tick
 * callback (tickMoveZ, tickCallbackB or tickCallbackC, chosen by
 * selectTickCallback, while enableTickCallback holds) and, while playing,
 * applies the next frame, wrapping at the frame count. applyTodFrame walks a
 * frame's packets through applyTodPacket, which writes attribute, coordinate
 * (the part's GsCOORD2PARAM), model-id and parent packets into the part the
 * packet's object id names. tickMoveZ moves the object -30 along local Z
 * and, while mainPartNotifies is 1, has mainPart send event 6
 * (notifyWithHull); B and C are empty here (Entity fills B).
 *
 * Peer and companion. attachToParent (+0x04C) takes two more leading
 * arguments than SceneNode's slot it overrides: (self, peer, companion,
 * parent, offset). It passes (parent, offset) to Actor's attachToParent, adds
 * `companion` as a child when no ticker is recorded yet (a FrameClock one
 * becomes Actor.ticker), and links `peer` both ways through linkPeer. The
 * slot keeps SceneNode's type, as an inherited slot does; a caller casts it
 * to TodActorAttachToParentFn. detachFromParent undoes all three.
 *
 * Sound. `sound`, the ctor's second argument, is a VabStreamObj
 * (include/vab_stream_obj.h; dream_aux.c passes the same bank to every
 * Entity). playTone (+0x124) plays one of its tones at volume 0x6E; Entity
 * drives its SoundCueSet on it.
 */
struct TodActor {
    TODACTOR_FIELDS(TodActorMethods);
};

/* selectTickCallback's selectors ('A'..'C'). */
#define TICK_CALLBACK_A 0x41 /**< tickMoveZ */
#define TICK_CALLBACK_B 0x42 /**< tickCallbackB */
#define TICK_CALLBACK_C 0x43 /**< tickCallbackC */

/** TodActor's method table (class id 0x234). */
extern TodActorMethods gTodActorMethods;

/**
 * @brief The TodActor method table.
 * @return &gTodActorMethods.
 */
extern TodActorMethods *GetTodActorMethods(void);

/** attachToParent (+0x04C)'s occupant in this class's table,
 * TodActor__AttachToParent, as a caller reaching it through the inherited
 * slot casts it (see TodActor). */
typedef void (*TodActorAttachToParentFn)(TodActor *self, TodActor *peer, void *companion,
                                         void *parent, void *offset);

/* The class's own methods, in ROM order. A subclass reaches the base ones
 * through GetTodActorMethods() and upcasts. */

/** @brief The ctor's descriptor, forwarded through setupModelData into
 * TodActor__AcquireModelData: the ModelData at +0x0C is borrowed; when there
 * is none, New_ModelData(&desc->src) makes one and the TodActor owns it. */
typedef struct TodActorDesc {
    /* +0x000 */ ResourceSource src; /**< the file to make a ModelData from when modelData is NULL */
    /* +0x008 */ u8 pad8[4];
    /* +0x00C */ struct ModelData *modelData; /**< a ModelData to borrow, or NULL */
} TodActorDesc;

/**
 * @brief Allocates a TodActor and runs its ctor through the table.
 * @param desc  The ctor's descriptor (a struct TodActorDesc).
 * @param sound The sound bank (a VabStreamObj), or NULL.
 * @return The new object, or NULL when the allocation or the ctor fails (the
 *         allocation is then freed).
 */
void *New_TodActor(void *desc, void *sound);

/**
 * @brief Constructor (slot +0x008): Actor's ctor, then the ModelData and the
 *        parts (setupModelData), the ModelData added as a child, and reset.
 * @param self  The object to construct.
 * @param desc  The descriptor: its ModelData is borrowed, or one is made from it.
 * @param sound The sound bank playTone plays on, or NULL.
 * @return self, or NULL when Actor's ctor or setupModelData fails (Actor's
 *         finalize has then run).
 */
TodActor *TodActor__TodActor(TodActor *self, void *desc, void *sound);

/**
 * @brief Slot +0x00C: releases the parts and an owned ModelData
 *        (teardownModelData), then Actor's finalize.
 * @param self The object.
 */
void TodActor__Finalize(TodActor *self);

/**
 * @brief Slot +0x038: Actor's onNotify, then releases this object when a
 *        ModelData it borrowed reports that it was finalized.
 * @param self   The object.
 * @param sender The notifying object.
 * @param event  The event; BASICCLASS_EVENT_FINALIZED from a ModelData
 *               releases self while ownsModelData is 0.
 */
void TodActor__OnNotify(TodActor *self, BasicClass *sender, s32 event);

/**
 * @brief Slot +0x040: hides the object, sets mainPartNotifies to 1 and
 *        lastOffsetValue to 300, selects tick callback 'A' disabled, sets TOD 0
 *        stopped, and links mainPart's model as this object's own.
 * @param self The object.
 */
void TodActor__Reset(TodActor *self);

/**
 * @brief Slot +0x04C: when not yet attached, Actor's attachToParent(parent,
 *        offset), then `companion` as a child while no ticker is recorded, then
 *        linkPeer(peer).
 * @param self      The object.
 * @param peer      The TodActor to link both ways, or NULL.
 * @param companion A child to add (a FrameClock becomes the ticker), or NULL.
 * @param parent    The node to attach to.
 * @param offset    Handed on to Actor's attachToParent.
 */
void TodActor__AttachToParent(TodActor *self, TodActor *peer, void *companion, void *parent, void *offset);

/**
 * @brief Slot +0x050: when attached, unlinks the peer, removes the ticker child
 *        and runs Actor's detachFromParent.
 * @param self The object.
 */
void TodActor__DetachFromParent(TodActor *self);

/**
 * @brief Slot +0x060: calls setDisplay(on) on every part (not on the object
 *        itself).
 * @param self The object.
 * @param on   Non-zero to display, 0 to hide; passed on as an s32.
 */
void TodActor__SetDisplay(TodActor *self, s32 on);

/**
 * @brief Slot +0x070: calls setLightMode(mode) on every part, then Actor's
 *        setLightMode on the object itself.
 * @param self The object.
 * @param mode The light mode, passed on as a u32.
 */
void TodActor__SetLightMode(TodActor *self, u32 mode);

/**
 * @brief Slot +0x098: the ticker's callback. FRAMECLOCK_EVENT_RUNNING runs
 *        tick; FRAMECLOCK_EVENT_STOPPED releases the object.
 * @param self   The object.
 * @param sender The ticker (unused).
 * @param event  The FrameClock event.
 */
void TodActor__Update(TodActor *self, void *sender, s32 event);

/**
 * @brief Slot +0x0F0: sets mainPartNotifies.
 * @param self The object.
 * @param on   1 to have tickMoveZ send mainPart's move notification.
 */
void TodActor__SetMainPartNotifies(TodActor *self, s32 on);

/**
 * @brief Slot +0x0F4: acquires the ModelData and the parts unless a ModelData
 *        is already held.
 * @param self The object.
 * @param desc The ctor's descriptor.
 * @return 0 on success (or when already set up), 1 on failure.
 */
s32 TodActor__SetupModelData(TodActor *self, void *desc);

/**
 * @brief Slot +0x0F8: releases the ModelData and the parts, if a ModelData is
 *        held.
 * @param self The object.
 */
void TodActor__TeardownModelData(TodActor *self);

/**
 * @brief Borrows desc's ModelData or makes one with New_ModelData, then sets up
 *        the parts.
 * @param self The object.
 * @param desc The descriptor: its +0x00C ModelData, or its source for New_ModelData.
 * @return setupParts' result (0 on success), or 1 when no ModelData could be
 *         had (the parts and ModelData are then released).
 */
s32 TodActor__AcquireModelData(TodActor *self, struct TodActorDesc *desc);

/**
 * @brief Tears down the parts, releases the ModelData when this object made
 *        it, and clears modelData.
 * @param self The object.
 */
void TodActor__ReleaseModelData(TodActor *self);

/**
 * @brief Slot +0x0FC: finds the part for a TOD object id.
 * @param self The object.
 * @param id   The object id; only its low byte is compared.
 * @return The index into parts/partIds of the first match, or -1 when none
 *         matches or there are no parts.
 */
s32 TodActor__FindPartIndex(TodActor *self, s32 id);

/**
 * @brief Slot +0x100: creates the parts unless they already exist.
 * @param self The object.
 * @return 0 on success (or when they exist), 1 on failure.
 */
s32 TodActor__SetupParts(TodActor *self);

/**
 * @brief Slot +0x104: destroys the parts, if there are any.
 * @param self The object.
 */
void TodActor__TeardownParts(TodActor *self);

/**
 * @brief Makes one Actor per object of the TOD's first frame, fills partIds
 *        from the ModelData's packet scan, and picks mainPart.
 * @param self The object.
 * @return 0 on success; 1 when an allocation fails, with the parts made so far
 *         released.
 */
s32 TodActor__CreateParts(TodActor *self);

/**
 * @brief Releases every part and frees parts and partIds (both NULL after).
 *        mainPart is cleared when both arrays existed.
 * @param self The object.
 */
void TodActor__DestroyParts(TodActor *self);

/**
 * @brief Slot +0x108: counts the tick, runs the tick callback while enabled
 *        and, while playing a TOD of two or more frames, applies the next frame,
 *        wrapping to the first; then marks the coordinate for recalculation.
 * @param self The object.
 */
void TodActor__Tick(TodActor *self);

/**
 * @brief Slot +0x10C: installs tickMoveZ, tickCallbackB or tickCallbackC as the
 *        tick callback.
 * @param self  The object.
 * @param which TICK_CALLBACK_A, _B or _C (low byte); any other value changes
 *              nothing.
 */
void TodActor__SelectTickCallback(TodActor *self, s32 which);

/**
 * @brief Slot +0x110: lets tick run the tick callback.
 * @param self The object.
 * @return 1.
 */
s32 TodActor__EnableTickCallback(TodActor *self);

/**
 * @brief Slot +0x114: stops tick running the tick callback.
 * @param self The object.
 */
void TodActor__DisableTickCallback(TodActor *self);

/**
 * @brief Slot +0x118's value, tick callback 'A': moves the object -30 along
 *        local Z and, while mainPartNotifies is 1, has mainPart send
 *        ACTOR_EVENT_MOVED_Z.
 * @param self The object.
 */
void TodActor__TickMoveZ(TodActor *self);

/**
 * @brief Slot +0x11C's value, tick callback 'B': does nothing (Entity
 *        overrides it).
 */
void TodActor__TickCallbackB(void);

/** @brief Slot +0x120's value, tick callback 'C': does nothing. */
void TodActor__TickCallbackC(void);

/**
 * @brief Slot +0x124: plays a tone of the sound bank at volume 110, if there is
 *        a bank.
 * @param self  The object.
 * @param index The tone, as the bank's playTone takes it.
 */
void TodActor__PlayTone(TodActor *self, s32 index);

/**
 * @brief Slot +0x128: selects a TOD of the ModelData's TodSet, rewinds to its
 *        first frame and applies it.
 * @param self  The object.
 * @param index The TOD's index in the TodSet.
 */
void TodActor__SetTod(TodActor *self, s32 index);

/**
 * @brief Slot +0x12C: lets tick advance the TOD's frames.
 * @param self The object.
 * @return 1.
 */
s32 TodActor__PlayTod(TodActor *self);

/**
 * @brief Slot +0x130: stops tick advancing the TOD's frames.
 * @param self The object.
 */
void TodActor__StopTod(TodActor *self);

/**
 * @brief Slot +0x134: applies each packet of one TOD frame through
 *        applyTodPacket.
 * @param self  The object.
 * @param frame The frame's header.
 * @param extra Handed to applyTodPacket.
 * @return The next frame (the word after the frame's last packet).
 */
void *TodActor__ApplyTodFrame(TodActor *self, void *frame, void *extra);

/**
 * @brief Slot +0x138: applies one TOD packet to the part its object id names:
 *        attribute, coordinate (rotation, scale and translation, absolute or
 *        relative), model id or parent. A packet naming no part is skipped.
 * @param self   The object.
 * @param packet The packet's header word.
 * @param extra  Unused.
 * @return The next packet.
 */
void *TodActor__ApplyTodPacket(TodActor *self, void *packet, void *extra);

/**
 * @brief Slot +0x13C: makes self and `other` children of each other and
 *        records `other` as the peer; NULL does nothing.
 * @param self  The object.
 * @param other The peer.
 */
void TodActor__LinkPeer(TodActor *self, TodActor *other);

/**
 * @brief Slot +0x140: undoes linkPeer, if there is a peer.
 * @param self The object.
 */
void TodActor__UnlinkPeer(TodActor *self);

#endif
