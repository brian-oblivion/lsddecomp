#ifndef TODACTOR_H
#define TODACTOR_H

#include "Actor.h"

/*
 * TodActor -- an Actor animated by a TOD: it owns one Actor "part" per
 * object of a TOD animation and plays the TOD's frames over them. Class id
 * 0x234, method table gTodActorMethods, getter GetTodActorMethods; methods
 * in src/world/TodActor.c. Its ctor chains to Actor's (include/Actor.h). One
 * class derives from it, Entity (0x1F234, include/entity.h), and it is only
 * ever built as one: New_Entity (from dream_aux's SetDreamAuxWorld and
 * SpawnDreamAuxTriggerEntity) runs this ctor first; New_TodActor has no
 * caller.
 *
 * Lifecycle.
 *   ctor(desc, sound)  Actor's ctor, then setupModelData(desc): the ModelData
 *                      (include/ModelData.h: a TMD LinkResource and a TodSet
 *                      over one file) at desc +0x00C is borrowed, or, when
 *                      there is none, New_ModelData(desc) makes one this
 *                      object owns (ownsModelData). setupParts then asks the
 *                      ModelData for the TOD's object ids (scanPackets) and
 *                      makes one Actor (New_Actor) per object: `parts`, their
 *                      ids in `partIds`, `mainPart` the one the scan names
 *                      first. The ModelData becomes a child, and reset runs.
 *                      `sound` is kept in `sound`. Returns self, or NULL.
 *   reset              setDisplay(0), mainPartNotifies 1, lastOffsetValue
 *                      300, tick callback 'A' selected but disabled, TOD 0
 *                      set and stopped, and mainPart's model linked as this
 *                      object's own.
 *   update (+0x098)    event 2 (the FrameClock ticker) runs tick, event 4
 *                      releases the object.
 *   finalize           teardownModelData: the parts are released, and the
 *                      ModelData too when this object made it. onNotify
 *                      releases the object when a ModelData sends event 1
 *                      while its own ModelData is borrowed.
 *
 * Playback. setTod(index) selects a TOD of the TodSet and applies its first
 * frame; playTod/stopTod gate frame advance. tick runs the selected tick
 * callback (tickMoveZ, tickCallbackB or tickCallbackC, chosen by selectTickCallback, while
 * enableTickCallback holds) and, while playing, applies the next frame,
 * wrapping at the frame count. applyTodFrame walks a frame's packets through
 * applyTodPacket, which writes attribute, coordinate (the part's
 * GsCOORD2PARAM), model-id and parent packets into the part the packet's
 * object id names. tickMoveZ moves the object -30 along local Z and,
 * while mainPartNotifies is 1, has mainPart send event 6 (notifyWithHull);
 * B and C are empty here (Entity fills B).
 *
 * Peer and companion. attachToParent (+0x04C) takes two more leading
 * arguments than SceneNode's slot it overrides: (self, peer, companion,
 * parent, offset). It passes (parent, offset) to Actor's attachToParent,
 * adds `companion` as a child when no ticker is recorded yet (a FrameClock
 * one becomes Actor.ticker), and links `peer` both ways through linkPeer.
 * The slot keeps SceneNode's type, as an inherited slot does; a caller casts
 * it to TodActorAttachToParentFn (a function-pointer cast emits no code).
 * detachFromParent undoes all three.
 *
 * Sound. `sound`, the ctor's second argument, is a VabStreamObj (include/
 * VabStreamObj.h; dream_aux passes the same bank to every Entity). playTone
 * (+0x124) plays one of its tones at volume 0x6E; Entity drives its
 * SoundCueSet on it.
 *
 * The object is 0x98 bytes (New_TodActor); Entity's own fields start at
 * +0x098.
 */

typedef struct TodActor TodActor;
typedef struct TodActorMethods TodActorMethods;

/* Tags completed in the unit that reads them (src/world/TodActor.c), so
 * that any header may repeat these declarations. */
struct ModelData;    /* include/ModelData.h */
struct TodActorDesc; /* the ctor's descriptor: +0x00C a ModelData to borrow */
struct VabStreamObj; /* include/VabStreamObj.h: the sound bank the ctor's second argument names */

/* Occupants in gTodActorMethods named at each slot; `tools/classtable.py
 * gEntityMethods --vs gTodActorMethods` lists Entity's overrides. The
 * inherited slots keep Actor's names; this class overrides +0x008, +0x00C
 * (TodActor__Finalize), +0x038 (TodActor__OnNotify), +0x040
 * (TodActor__Reset), +0x04C/+0x050 (TodActor__AttachToParent/
 * DetachFromParent), +0x060 (TodActor__SetDisplay), +0x070
 * (TodActor__SetLightMode) and +0x098 (TodActor__Update). */
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

struct TodActorMethods {
    TODACTOR_SLOTS(TodActor, (TodActor * self, void *desc, void *sound));
};

struct TodActor {
    TODACTOR_FIELDS(TodActorMethods);
};

/* selectTickCallback's selectors ('A'..'C'). */
#define TICK_CALLBACK_A 0x41
#define TICK_CALLBACK_B 0x42
#define TICK_CALLBACK_C 0x43

extern TodActorMethods gTodActorMethods;
extern TodActorMethods *GetTodActorMethods(void); /* returns &gTodActorMethods */

/* +0x04C's occupant in this class's table, as a caller reaching it through
 * the inherited slot casts it (see the banner). */
typedef void (*TodActorAttachToParentFn)(TodActor *self, TodActor *peer, void *companion,
                                         void *parent, void *offset);

/* The class's own methods, in ROM order (TodActor). A subclass reaches
 * the base ones through GetTodActorMethods() and upcasts. */
void *New_TodActor(void *desc, void *sound);
TodActor *TodActor__TodActor(TodActor *self, void *desc, void *sound);
void TodActor__Finalize(TodActor *self);
void TodActor__OnNotify(TodActor *self, BasicClass *sender, s32 event);
void TodActor__Reset(TodActor *self);
void TodActor__AttachToParent(TodActor *self, TodActor *peer, void *companion, void *parent, void *offset);
void TodActor__DetachFromParent(TodActor *self);
void TodActor__SetDisplay(TodActor *self, void *on);
void TodActor__SetLightMode(TodActor *self, void *mode);
void TodActor__Update(TodActor *self, void *sender, s32 event);
void TodActor__SetMainPartNotifies(TodActor *self, s32 on);
s32 TodActor__SetupModelData(TodActor *self, void *desc);
void TodActor__TeardownModelData(TodActor *self);
s32 TodActor__AcquireModelData(TodActor *self, struct TodActorDesc *desc);
void TodActor__ReleaseModelData(TodActor *self);
s32 TodActor__FindPartIndex(TodActor *self, s32 id);
s32 TodActor__SetupParts(TodActor *self);
void TodActor__TeardownParts(TodActor *self);
s32 TodActor__CreateParts(TodActor *self);
void TodActor__DestroyParts(TodActor *self);
void TodActor__Tick(TodActor *self);
void TodActor__SelectTickCallback(TodActor *self, s32 which);
s32 TodActor__EnableTickCallback(TodActor *self);
void TodActor__DisableTickCallback(TodActor *self);
void TodActor__TickMoveZ(TodActor *self);
void TodActor__TickCallbackB(void);
void TodActor__TickCallbackC(void);
void TodActor__PlayTone(TodActor *self, s32 index);
void TodActor__SetTod(TodActor *self, s32 index);
s32 TodActor__PlayTod(TodActor *self);
void TodActor__StopTod(TodActor *self);
void *TodActor__ApplyTodFrame(TodActor *self, void *frame, void *extra);
void *TodActor__ApplyTodPacket(TodActor *self, void *packet, void *extra);
void TodActor__LinkPeer(TodActor *self, TodActor *other);
void TodActor__UnlinkPeer(TodActor *self);

#endif
