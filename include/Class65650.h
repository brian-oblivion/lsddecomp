#ifndef CLASS65650_H
#define CLASS65650_H

#include "Actor.h"

/*
 * Class65650 -- an Actor that owns one Actor "part" per object of a TOD
 * animation and plays TODs over them (class id 0x234, method table
 * gClass65650Methods): an Actor subclass (include/Actor.h). Methods in
 * src/code_55dd4.c; one class derives from it, Entity (ENTITY_METHODS,
 * 0x1F234, include/Entity.h), whose ctor calls this class's first
 * (Entity__Entity: Get_vtable_Class65650()->ctor).
 *
 * Its ctor calls Actor's first (Class65650__Class65650:
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
 * arguments than Class6B5CC's slot it overrides: (self, peer, companion,
 * parent, offset). It adds `companion` as a child (Actor__AddChild records
 * a class-5 one in Actor.ticker), links `peer` mutually through linkPeer
 * (+0x13C), and passes (parent, offset) to Class6B5CC's attachToParent. The
 * slot keeps Class6B5CC's type, as an inherited slot does; a caller that
 * reaches Class65650__AttachToParent through it casts the slot to
 * Class65650AttachToParentFn (a function-pointer cast emits no code).
 *
 * The object is 0x98 bytes (New_Class65650); Entity's own fields start at
 * +0x098. The ctor returns self or NULL, as Actor's does.
 */

typedef struct Class65650 Class65650;
typedef struct Class65650Methods Class65650Methods;

/* Tags completed in the unit that reads them (include/code_55dd4.h), so
 * that any header may repeat these declarations. */
struct ModelData;   /* include/ModelData.h */
struct UnkArg1Obj;  /* the ctor's descriptor: +0x00C a ModelData to borrow */
struct UnkArg2Obj;  /* the ctor's second argument: unidentified, its +0x080 slot is called */
struct TagCheckArg; /* onNotify's sender, read only for its table's low id halfword */

/* Occupants in gClass65650Methods named at each slot; `tools/classtable.py
 * ENTITY_METHODS --vs gClass65650Methods` lists Entity's overrides. The
 * inherited slots keep Actor's names; this class overrides +0x008, +0x00C
 * (Class65650__Finalize), +0x038 (Class65650__OnNotify), +0x040
 * (Class65650__Reset), +0x04C/+0x050 (Class65650__AttachToParent/
 * DetachFromParent), +0x060 (Class65650__SetDisplay), +0x070
 * (Class65650__SetLightMode) and +0x098 (Class65650__Update). */
/* clang-format off */
#define CLASS65650_SLOTS(Self, CtorParams)                                                         \
    ACTOR_SLOTS(Self, CtorParams);                                                                 \
    /* +0x0F0 */ void (*setUnk64)(Self *self, s32 value);       /* Class65650__SetUnk64 */         \
    /* +0x0F4 */ s32 (*setupModelData)(Self *self, void *desc); /* Class65650__SetupModelData: 0 on success */ \
    /* +0x0F8 */ void (*teardownModelData)(Self *self);         /* Class65650__TeardownModelData */ \
    /* +0x0FC */ s32 (*findPartIndex)(Self *self, s32 id);      /* Class65650__FindPartIndex: -1 when no part has the id; called directly */ \
    /* +0x100 */ s32 (*setupParts)(Self *self);                 /* Class65650__SetupParts: 0 on success */ \
    /* +0x104 */ void (*teardownParts)(Self *self);             /* Class65650__TeardownParts */    \
    /* +0x108 */ void (*tick)(Self *self);                      /* Class65650__Tick */             \
    /* +0x10C */ void (*selectTickCallback)(Self *self, s32 which); /* Class65650__SelectTickCallback: TICK_CALLBACK_A..C */ \
    /* +0x110 */ s32 (*enableTickCallback)(Self *self);         /* Class65650__EnableTickCallback */ \
    /* +0x114 */ void (*disableTickCallback)(Self *self);       /* Class65650__DisableTickCallback */ \
    /* +0x118 */ void *tickCallbackA;                           /* Class65650__TickCallbackA; only the VALUE is read (selectTickCallback) */ \
    /* +0x11C */ void *tickCallbackB;                           /* Class65650__TickCallbackB, empty (Entity: Entity__TickSoundCue) */ \
    /* +0x120 */ void *tickCallbackC;                           /* Class65650__TickCallbackC, empty */ \
    /* +0x124 */ void (*slot124)(Self *self, void *arg);        /* Class65650__func_800661D4: arg2->+0x080(arg2, arg, 0x6E, 0x6E); not dispatched in C */ \
    /* +0x128 */ void (*setTod)(Self *self, s32 index);         /* Class65650__SetTod */           \
    /* +0x12C */ s32 (*playTod)(Self *self);                    /* Class65650__PlayTod */          \
    /* +0x130 */ void (*stopTod)(Self *self);                   /* Class65650__StopTod */          \
    /* +0x134 */ u8 *(*applyTodFrame)(Self *self, void *frame, s32 flag); /* Class65650__ApplyTodFrame: returns the next frame */ \
    /* +0x138 */ void *(*applyTodPacket)(Self *self, void *packet, void *extra); /* Class65650__ApplyTodPacket: returns the next packet */ \
    /* +0x13C */ void (*linkPeer)(Self *self, Class65650 *other); /* Class65650__LinkPeer: addChild both ways, peer = other */ \
    /* +0x140 */ void (*unlinkPeer)(Self *self)                 /* Class65650__UnlinkPeer */
/* clang-format on */

/* clang-format off */
#define CLASS65650_FIELDS(Methods)                                                                 \
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
    /* +0x094 */ Class65650 *peer          /* linkPeer's other, NULL after unlinkPeer. The object is 0x98 bytes (New_Class65650) */
/* clang-format on */

struct Class65650Methods {
    CLASS65650_SLOTS(Class65650, (Class65650 * self, void *desc, void *arg2));
};

struct Class65650 {
    CLASS65650_FIELDS(Class65650Methods);
};

/* selectTickCallback's selectors ('A'..'C'). */
#define TICK_CALLBACK_A 0x41
#define TICK_CALLBACK_B 0x42
#define TICK_CALLBACK_C 0x43

extern Class65650Methods gClass65650Methods;
extern Class65650Methods *Get_vtable_Class65650(void); /* returns &gClass65650Methods */

/* +0x04C's occupant in this class's table, as a caller reaching it through
 * the inherited slot casts it (see the banner). */
typedef void (*Class65650AttachToParentFn)(Class65650 *self, Class65650 *peer, void *companion,
                                           void *parent, void *offset);

/* The class's own methods, in ROM order (code_55dd4). A subclass reaches
 * the base ones through Get_vtable_Class65650() and upcasts. */
void *New_Class65650(void *desc, void *arg2);
Class65650 *Class65650__Class65650(Class65650 *self, void *desc, void *arg2);
void Class65650__Finalize(Class65650 *self);
void Class65650__OnNotify(Class65650 *self, struct TagCheckArg *sender, s32 event);
void Class65650__Reset(Class65650 *self);
void Class65650__AttachToParent(Class65650 *self, Class65650 *peer, void *companion, void *parent,
                                void *offset);
void Class65650__DetachFromParent(Class65650 *self);
void Class65650__SetDisplay(Class65650 *self, void *arg);
void Class65650__SetLightMode(Class65650 *self, void *arg);
void Class65650__Update(Class65650 *self, void *sender, s32 event);
void Class65650__SetUnk64(Class65650 *self, s32 value);
s32 Class65650__SetupModelData(Class65650 *self, void *desc);
void Class65650__TeardownModelData(Class65650 *self);
s32 Class65650__AcquireModelData(Class65650 *self, struct UnkArg1Obj *desc);
void Class65650__ReleaseModelData(Class65650 *self);
s32 Class65650__FindPartIndex(Class65650 *self, s32 id);
s32 Class65650__SetupParts(Class65650 *self);
void Class65650__TeardownParts(Class65650 *self);
s32 Class65650__CreateParts(Class65650 *self);
void Class65650__DestroyParts(Class65650 *self);
void Class65650__Tick(Class65650 *self);
void Class65650__SelectTickCallback(Class65650 *self, s32 which);
s32 Class65650__EnableTickCallback(Class65650 *self);
void Class65650__DisableTickCallback(Class65650 *self);
void Class65650__TickCallbackA(Class65650 *self);
void Class65650__TickCallbackB(void);
void Class65650__TickCallbackC(void);
void Class65650__func_800661D4(Class65650 *self, void *arg);
void Class65650__SetTod(Class65650 *self, s32 index);
s32 Class65650__PlayTod(Class65650 *self);
void Class65650__StopTod(Class65650 *self);
void *Class65650__ApplyTodFrame(Class65650 *self, void *frame, void *extra);
void *Class65650__ApplyTodPacket(Class65650 *self, void *packet, void *extra);
void Class65650__LinkPeer(Class65650 *self, Class65650 *other);
void Class65650__UnlinkPeer(Class65650 *self);

#endif
