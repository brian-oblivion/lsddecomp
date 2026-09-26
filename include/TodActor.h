#ifndef TODACTOR_H
#define TODACTOR_H

#include "Actor.h"

/*
 * TodActor -- an Actor that owns one Actor "part" per object of a TOD
 * animation and plays TODs over them (class id 0x234, method table
 * gTodActorMethods): an Actor subclass (include/Actor.h). Methods in
 * src/code_55dd4.c; one class derives from it, Entity (gEntityMethods,
 * 0x1F234, include/Entity.h), whose ctor calls this class's first
 * (Entity__Entity: GetTodActorMethods()->ctor).
 *
 * Its ctor calls Actor's first (TodActor__TodActor:
 * GetActorMethods()->ctor), so the id parent is the ctor-chain parent.
 *
 * Model data. The ctor's first argument is a descriptor whose +0x00C may
 * already hold a ModelData (include/ModelData.h: a TMD LinkResource and a
 * TOD TodSet over one file); setupModelData borrows it or makes one with
 * New_ModelData, and owns only the one it made (ownsModelData). setupParts
 * then asks the ModelData for its TOD object ids (+0x080, scanPackets) and
 * makes one Actor (New_Actor) per object in `parts`, with the ids in
 * `partIds`; `mainPart` is the part the id scan names first.
 *
 * TOD playback. setTod picks a TOD of the TodSet, stopTod/playTod gate
 * frame advance, and tick (reached through update, +0x098, on event 2 from
 * the class-5 ticker) runs the selected tick callback (+0x118..+0x120,
 * chosen by selectTickCallback 'A'..'C') and applies the next frame:
 * applyTodFrame walks its packets through applyTodPacket, which writes
 * attribute, coordinate, model-id and parent packets into the part the
 * packet's object id names.
 *
 * Peer and companion. attachToParent (+0x04C) takes TWO more leading
 * arguments than SceneNode's slot it overrides: (self, peer, companion,
 * parent, offset). It adds `companion` as a child (Actor__AddChild records
 * a class-5 one in Actor.ticker), links `peer` mutually through linkPeer
 * (+0x13C), and passes (parent, offset) to SceneNode's attachToParent. The
 * slot keeps SceneNode's type, as an inherited slot does; a caller that
 * reaches TodActor__AttachToParent through it casts the slot to
 * TodActorAttachToParentFn (a function-pointer cast emits no code).
 *
 * The object is 0x98 bytes (New_TodActor); Entity's own fields start at
 * +0x098. The ctor returns self or NULL, as Actor's does.
 */

typedef struct TodActor TodActor;
typedef struct TodActorMethods TodActorMethods;

/* Tags completed in the unit that reads them (include/code_55dd4.h), so
 * that any header may repeat these declarations. */
struct ModelData;   /* include/ModelData.h */
struct UnkArg1Obj;  /* the ctor's descriptor: +0x00C a ModelData to borrow */
struct UnkArg2Obj;  /* the ctor's second argument: unidentified, its +0x080 slot is called */
struct TagCheckArg; /* onNotify's sender, read only for its table's low id halfword */

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
    /* +0x0F0 */ void (*setUnk64)(Self *self, s32 value);       /* TodActor__SetUnk64 */         \
    /* +0x0F4 */ s32 (*setupModelData)(Self *self, void *desc); /* TodActor__SetupModelData: 0 on success */ \
    /* +0x0F8 */ void (*teardownModelData)(Self *self);         /* TodActor__TeardownModelData */ \
    /* +0x0FC */ s32 (*findPartIndex)(Self *self, s32 id);      /* TodActor__FindPartIndex: -1 when no part has the id; called directly */ \
    /* +0x100 */ s32 (*setupParts)(Self *self);                 /* TodActor__SetupParts: 0 on success */ \
    /* +0x104 */ void (*teardownParts)(Self *self);             /* TodActor__TeardownParts */    \
    /* +0x108 */ void (*tick)(Self *self);                      /* TodActor__Tick */             \
    /* +0x10C */ void (*selectTickCallback)(Self *self, s32 which); /* TodActor__SelectTickCallback: TICK_CALLBACK_A..C */ \
    /* +0x110 */ s32 (*enableTickCallback)(Self *self);         /* TodActor__EnableTickCallback */ \
    /* +0x114 */ void (*disableTickCallback)(Self *self);       /* TodActor__DisableTickCallback */ \
    /* +0x118 */ void *tickCallbackA;                           /* TodActor__TickCallbackA; only the VALUE is read (selectTickCallback) */ \
    /* +0x11C */ void *tickCallbackB;                           /* TodActor__TickCallbackB, empty (Entity: Entity__TickSoundCue) */ \
    /* +0x120 */ void *tickCallbackC;                           /* TodActor__TickCallbackC, empty */ \
    /* +0x124 */ void (*slot124)(Self *self, void *arg);        /* TodActor__PlayTone: arg2->+0x080(arg2, arg, 0x6E, 0x6E); not dispatched in C */ \
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
    /* +0x058 */ struct UnkArg2Obj *arg2;  /* the ctor's second argument, kept verbatim; slot124 calls its +0x080 */ \
    /* +0x05C */ struct ModelData *modelData; /* borrowed from the ctor's descriptor or made by New_ModelData; NULL when none */ \
    /* +0x060 */ s32 ownsModelData;        /* 1 when New_ModelData made modelData; only an owned one is released */ \
    /* +0x064 */ s32 unk64;                /* setUnk64 (1 in reset); TickCallbackA forwards the move to mainPart only while it is 1 */ \
    /* +0x068 */ Actor *mainPart;          /* parts[the index scanPackets writes first]; NULL after the parts are destroyed */ \
    /* +0x06C */ s32 partCount;            /* entries in parts/partIds */                          \
    /* +0x070 */ Actor **parts;            /* one New_Actor per TOD object */                      \
    /* +0x074 */ u8 *partIds;              /* each part's TOD object id; findPartIndex searches it */ \
    /* +0x078 */ void *tickCallback;       /* tickCallbackA/B/C as selected; tick calls it while tickCallbackEnabled */ \
    /* +0x07C */ s32 todIndex;             /* the current TOD's index in the TodSet */             \
    /* +0x080 */ s32 todFrameCount;        /* the current TOD's frame count (setTod) */            \
    /* +0x084 */ s32 todFrame;             /* the current frame number; wraps at todFrameCount */  \
    /* +0x088 */ u8 *todFramePtr;          /* the next frame to apply */                           \
    /* +0x08C */ s32 tickCallbackEnabled;  /* enableTickCallback / disableTickCallback */          \
    /* +0x090 */ s32 todPlaying;           /* playTod / stopTod; gates frame advance in tick */    \
    /* +0x094 */ TodActor *peer          /* linkPeer's other, NULL after unlinkPeer. The object is 0x98 bytes (New_TodActor) */
/* clang-format on */

struct TodActorMethods {
    TODACTOR_SLOTS(TodActor, (TodActor * self, void *desc, void *arg2));
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

/* The class's own methods, in ROM order (code_55dd4). A subclass reaches
 * the base ones through GetTodActorMethods() and upcasts. */
void *New_TodActor(void *desc, void *arg2);
TodActor *TodActor__TodActor(TodActor *self, void *desc, void *arg2);
void TodActor__Finalize(TodActor *self);
void TodActor__OnNotify(TodActor *self, struct TagCheckArg *sender, s32 event);
void TodActor__Reset(TodActor *self);
void TodActor__AttachToParent(TodActor *self, TodActor *peer, void *companion, void *parent,
                                void *offset);
void TodActor__DetachFromParent(TodActor *self);
void TodActor__SetDisplay(TodActor *self, void *arg);
void TodActor__SetLightMode(TodActor *self, void *arg);
void TodActor__Update(TodActor *self, void *sender, s32 event);
void TodActor__SetUnk64(TodActor *self, s32 value);
s32 TodActor__SetupModelData(TodActor *self, void *desc);
void TodActor__TeardownModelData(TodActor *self);
s32 TodActor__AcquireModelData(TodActor *self, struct UnkArg1Obj *desc);
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
void TodActor__TickCallbackA(TodActor *self);
void TodActor__TickCallbackB(void);
void TodActor__TickCallbackC(void);
void TodActor__PlayTone(TodActor *self, void *arg);
void TodActor__SetTod(TodActor *self, s32 index);
s32 TodActor__PlayTod(TodActor *self);
void TodActor__StopTod(TodActor *self);
void *TodActor__ApplyTodFrame(TodActor *self, void *frame, void *extra);
void *TodActor__ApplyTodPacket(TodActor *self, void *packet, void *extra);
void TodActor__LinkPeer(TodActor *self, TodActor *other);
void TodActor__UnlinkPeer(TodActor *self);

#endif
