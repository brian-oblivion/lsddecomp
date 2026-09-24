#ifndef CODE_55DD4_H
#define CODE_55DD4_H

#include "common.h"

/*
 * Class65650: an articulated model that owns one BaseObjO "part" per TOD
 * object and plays TOD animations over them. Allocated by New_Class65650,
 * constructed by Class65650__Class65650; method table gClass65650Methods
 * (80 slots, header word 0x234). Entity (ENTITY_METHODS) derives from it.
 *
 * Inheritance, resolved with tools/classtable.py (never by counting):
 * BasicClass -> Class6B5CC -> BaseObjO (D_800878D4, 59 slots, header 0x34;
 * DreamSys's base too) -> Class65650. `classtable.py gClass65650Methods --vs
 * D_800878D4` says it overrides +0x008 ctor, +0x00C dtor, +0x038 OnNotify,
 * +0x040 InitDefaults, +0x04C AttachToParent, +0x050 DetachFromParent,
 * +0x060 SetDisplay, +0x070 SetLightMode and +0x098 (tag-5 notify), and adds
 * +0x0F0..+0x140. Instance offsets below +0x58 are the base classes'
 * (Class6B5CC's parent/coord2/tick, BaseObjO's companion2) and keep their
 * names.
 *
 * Only the slots this unit's functions call through are typed; the rest are
 * padding, so the struct keeps the right offsets without typing every
 * method up front.
 */

typedef struct Class65650 Class65650;

/* BaseObjO's method table (D_800878D4), this class's immediate base. Same
 * policy as Class6D3C8.h's MiddleClassMethods: only slots this unit
 * dispatches through are typed, named for their classtable occupant.
 * Resolved via DreamSys__GetBaseMethods, which returns &D_800878D4 (a plain
 * lui/addiu address-of, not a gp_rel load). */
typedef struct D800878D4Methods {
    s32 header;                                    /* +0x000 */
    void *unk04;                                    /* +0x004 BasicClass__Release, inherited */
    Class65650 *(*ctor)(Class65650 *self);            /* +0x008 BaseObjO__BaseObjO */
    void (*dtor)(Class65650 *self);                    /* +0x00C Class6B5CC__Finalize */
    void (*linkCompanion)(void *self, void *arg);             /* +0x010 BaseObjO__LinkCompanion */
    void *unk14;                                        /* +0x014 BaseObjO__UnlinkCompanion */
    void *unk18;                                         /* +0x018 BaseObjO__ClearCompanions */
    u8 pad1C[0x1C];                                       /* +0x01C .. +0x037, not yet needed */
    void (*onNotify)(Class65650 *self, void *arg1, s32 arg2); /* +0x038 Class6B5CC__OnNotify, chained by Class65650__OnNotify */
    u8 pad3C[0x10];                                         /* +0x03C .. +0x04B, not yet needed */
    void (*attachToParent)(Class65650 *self, void *arg1, void *arg2); /* +0x04C Class6B5CC__AttachToParent(self, parent, offset), chained by Class65650__AttachToParent */
    void (*detachFromParent)(Class65650 *self);                       /* +0x050 Class6B5CC__DetachFromParent, chained by Class65650__DetachFromParent */
    u8 pad54[0x0C];                                          /* +0x054 .. +0x05F, not yet needed */
    void (*setDisplay)(Class65650 *self, s32 arg);                /* +0x060 Class6B5CC__SetDisplay; Class65650__InitDefaults calls it with 0 */
    u8 pad64[0x0C];                                             /* +0x064 .. +0x06F, not yet needed */
    void (*setLightMode)(Class65650 *self, void *arg);              /* +0x070 Class6B5CC__SetLightMode, chained by Class65650__SetLightMode */
} D800878D4Methods;

extern D800878D4Methods *DreamSys__GetBaseMethods(void);

/* Class65650__OnNotify's sender: any object. Only its method table's header
 * word is read (the low 16 bits), compared with MODEL_DATA_CLASS_HEADER. */
typedef struct TaggedObj {
    u16 header;   /* +0x000, the method table's header word */
} TaggedObj;

typedef struct TagCheckArg {
    TaggedObj *methods;   /* +0x000 */
} TagCheckArg;

/* Header word of D_8006F384 (tools/classtable.py), the class func_8004468C
 * allocates and Class65650.modelData points at. */
#define MODEL_DATA_CLASS_HEADER 0x5F03

/* Whatever class self->arg2 (below) points at: unidentified, only its
 * vtable slot +0x080 is needed so far, by Class65650__func_800661D4. */
typedef struct UnkArg2Methods {
    u8 pad00[0x80];                                       /* +0x000 .. +0x07C, unknown */
    void (*slot80)(void *self, void *arg1, s32 a2, s32 a3); /* +0x080 */
} UnkArg2Methods;

typedef struct UnkArg2Obj {
    UnkArg2Methods *methods;
} UnkArg2Obj;

/* self->mainPart: one of self->parts (a BaseObjO from New_BaseObjO), picked
 * by the index modelData->getObjectIds writes to buf[0]. */
typedef struct Unk68Methods {
    u8 pad00[0x88];                                    /* +0x000 .. +0x084, unknown */
    void (*slot88)(void *self, s32 arg);                /* +0x088 BaseObjO__func_571f8; Class65650__TickCallbackA calls it with 6 */
} Unk68Methods;

typedef struct Unk68Obj {
    Unk68Methods *methods;   /* +0x00 */
    u8 pad04[0x1C];            /* +0x04 .. +0x1F, unknown */
    s32 unk20;                  /* +0x20 Class6B5CC's +0x20 (what Class6B5CC__LinkModel stores); Class65650__InitDefaults links self to the same value */
} Unk68Obj;

/* Each self->parts[i]: a BaseObjO allocated by New_BaseObjO, so these slots
 * are D_800878D4's (tools/classtable.py). */
typedef struct Unk70ElemMethods {
    u8 pad00[0x04];                          /* +0x000, unknown */
    void (*release)(void *self);                /* +0x004 BasicClass__Release; Class65650__DestroyParts */
    u8 pad08[0x44];                            /* +0x008 .. +0x04B, unknown */
    void (*attachToParent)(void *self, void *arg1, s32 arg2); /* +0x04C Class6B5CC__AttachToParent; Class65650__ApplyTodPacket (TOD parent packet) passes self or another part as the parent, hence void * */
    u8 pad50[0x10];                             /* +0x050 .. +0x05F, unknown */
    void (*setDisplay)(void *self, void *arg);    /* +0x060 Class6B5CC__SetDisplay; Class65650__SetDisplay */
    u8 pad64[0x0C];                            /* +0x064 .. +0x06C, unknown */
    void (*setLightMode)(void *self, void *arg);      /* +0x070 Class6B5CC__SetLightMode; Class65650__SetLightMode */
} Unk70ElemMethods;

/* A part's coord2 (Class6B5CC +0x14) is a GsCOORDINATE2 and its +0x44 a
 * GsCOORD2PARAM (identified in code_d294.h: Class6B5CCSub14/Class6B5CCSub44).
 * These are this unit's views of the members Class65650__ApplyTodPacket
 * writes from a TOD coordinate packet. */
typedef struct TimeTargetObj {
    s32 scale[3];    /* +0x000 .. +0x00B GsCOORD2PARAM.scale: TOD_COORD_SCALE */
    u8 pad0C[0x04];   /* +0x00C .. +0x00F */
    s16 rotate[3];      /* +0x010 .. +0x015 GsCOORD2PARAM.rotate: TOD_COORD_ROTATE */
    u8 pad16[0x02];     /* +0x016 .. +0x017 */
    s32 trans[3];         /* +0x018 .. +0x023 GsCOORD2PARAM.trans: TOD_COORD_TRANSLATE, then copied to coord.t */
} TimeTargetObj;

typedef struct Elem14Obj {
    s32 flg;                /* +0x000 GsCOORDINATE2.flg; zeroed (= recompute) on every packet */
    u8 pad04[0x14];            /* +0x004 .. +0x017, coord.m */
    s32 tx;                  /* +0x018 coord.t[0] */
    s32 ty;                   /* +0x01C coord.t[1] */
    s32 tz;                    /* +0x020 coord.t[2] */
    u8 pad24[0x20];                 /* +0x024 .. +0x043, workm */
    TimeTargetObj *param;             /* +0x044 GsCOORDINATE2.param */
} Elem14Obj;

typedef struct Unk70ElemObj {
    Unk70ElemMethods *methods;   /* +0x00 */
    u8 pad04[0x0C];                /* +0x04 .. +0x0F, unknown */
    s32 attribute;                       /* +0x10 Class6B5CC attribute word; a TOD attribute packet does `attribute = (attribute & mask) | value` */
    Elem14Obj *coord2;                  /* +0x14 */
    u8 pad18[0x08];                      /* +0x18 .. +0x1F, unknown */
    s32 unk20;                             /* +0x20 Class6B5CC's +0x20 (the linked model); a TOD model-id packet links only while it is 0 */
} Unk70ElemObj;

/* TOD packet types and coordinate-packet flag bits, as
 * Class65650__ApplyTodPacket decodes them (the decoded header is
 * {object id, type, flag, length in words}). */
#define TOD_PACKET_ATTRIBUTE   0
#define TOD_PACKET_COORDINATE  1
#define TOD_PACKET_MODEL_ID    2
#define TOD_PACKET_PARENT      3
#define TOD_COORD_DIFFERENTIAL 1
#define TOD_COORD_ROTATE       2
#define TOD_COORD_SCALE        4
#define TOD_COORD_TRANSLATE    8

/* self->modelData: an instance of D_8006F384 (header MODEL_DATA_CLASS_HEADER),
 * made by func_8004468C from a load request or borrowed from the ctor's
 * arg1. It holds a model source (tmd) and a TOD set (tods). */
typedef struct Unk5CObj Unk5CObj;
typedef struct Unk5CMethods {
    u8 pad00[0x04];                          /* +0x000, unknown */
    Unk5CObj *(*release)(Unk5CObj *self);       /* +0x004 DestroyChained; returns the value to store back */
    u8 pad08[0x78];                            /* +0x008 .. +0x07C, unknown */
    s32 (*getObjectIds)(Unk5CObj *self, void *arg1, s32 *outBuf); /* +0x080 func_8004497C; Class65650__CreateParts calls it with arg1 NULL for the count (low byte of the return), then with self->partIds to fill one id byte per part; outBuf[0] is the mainPart index */
    void *(*decodeTodPacket)(Unk5CObj *self, void *acc, u8 *out0, u8 *out1, u8 *out2, u8 *out3); /* +0x084 func_800449B8; writes the packet's object id, type, flag and length, returns its data pointer. out2/out3 go on the stack (o32 5th/6th arguments) */
} Unk5CMethods;

/* self->modelData->tmd: only its slot +0x080 is needed, by
 * Class65650__ApplyTodPacket's model-id packet. */
typedef struct Unk2CMethods {
    u8 pad00[0x80];                           /* +0x000 .. +0x07C, unknown */
    s32 (*getModel)(void *self, s32 arg);         /* +0x080 (self, modelId - 1) -> value passed to Class6B5CC__LinkModel */
} Unk2CMethods;

typedef struct Unk2CObj {
    Unk2CMethods *methods;
} Unk2CObj;

/* self->modelData->tods (Class65650__SetTod, Class65650__Tick): arr + 8 +
 * i * 4 holds a pointer to the i-th TOD's holder, whose +0x10 is the TOD
 * data itself: +0x4 the frame count, frames starting at +0x8. */
typedef struct EntryObj2 {
    u8 pad00[0x04];
    s32 frameCount;            /* +0x004 TOD header: number of frames */
} EntryObj2;

typedef struct GroupObj {
    u8 pad00[0x10];
    EntryObj2 *tod;    /* +0x010 */
} GroupObj;

typedef struct Unk30Obj {
    u8 pad00[0x10];
    u8 *arr;             /* +0x010 -- element i lives at *(GroupObj **)(arr + 8 + i * 4) */
} Unk30Obj;

struct Unk5CObj {
    Unk5CMethods *methods;   /* +0x00 */
    u8 pad04[0x28];            /* +0x04 .. +0x2B, not this unit's to name */
    Unk2CObj *tmd;             /* +0x2C from func_80043840; supplies models for TOD model-id packets */
    Unk30Obj *tods;            /* +0x30 from func_800451B8; the TOD set, see Unk30Obj */
};

/* The constructor's `arg1`, forwarded through setupModelData into
 * Class65650__AcquireModelData: if its +0x0C already holds a model-data
 * object it is borrowed, otherwise func_8004468C(arg1) makes one that this
 * instance owns. */
typedef struct UnkArg1Obj {
    u8 pad00[0x0C];
    Unk5CObj *modelData;
} UnkArg1Obj;

extern Unk5CObj *func_8004468C(UnkArg1Obj *arg);

typedef struct Class65650Methods {
    s32 header;                                                    /* +0x000 */
    void (*release)(Class65650 *self);                               /* +0x004 BasicClass__Release */
    Class65650 *(*ctor)(Class65650 *self, void *arg1, void *arg2);   /* +0x008 Class65650__Class65650 */
    void (*dtor)(Class65650 *self);                                   /* +0x00C Class65650__Destructor */
    void (*linkCompanion)(Class65650 *self, void *arg);                       /* +0x010 BaseObjO__LinkCompanion */
    void (*unlinkCompanion)(Class65650 *self, void *arg);                        /* +0x014 BaseObjO__UnlinkCompanion */
    u8 pad18[0x28];                                                      /* +0x018 .. +0x03F, not yet needed */
    void (*initDefaults)(Class65650 *self);                                    /* +0x040 Class65650__InitDefaults */
    u8 pad44[0x80];                                                       /* +0x044 .. +0x0C3, not yet needed */
    void (*slotC4)(Class65650 *self, s32 arg1, s32 arg2);                  /* +0x0C4 BaseObjO__func_5748c; Class65650__TickCallbackA calls it with (-0x1E, 0) */
    u8 padC8[0x1C];                                                        /* +0x0C8 .. +0x0E3, not yet needed */
    void (*setLastOffsetValue)(Class65650 *self, s32 arg);                              /* +0x0E4 DreamSys__SetLastOffsetValue */
    u8 padE8[0x08];                                                          /* +0x0E8 .. +0x0EF, not yet needed */
    void (*setUnk64)(Class65650 *self, s32 arg);                                /* +0x0F0 Class65650__SetUnk64 */
    s32 (*setupModelData)(Class65650 *self, void *arg1);                     /* +0x0F4 Class65650__SetupModelData */
    void (*teardownModelData)(Class65650 *self);                              /* +0x0F8 Class65650__TeardownModelData */
    u8 padFC[0x04];                                                          /* +0x0FC Class65650__FindPartIndex, called directly */
    s32 (*setupParts)(Class65650 *self);                                         /* +0x100 Class65650__SetupParts */
    void (*teardownParts)(Class65650 *self);                                /* +0x104 Class65650__TeardownParts */
    void (*tick)(Class65650 *self);                                        /* +0x108 Class65650__Tick */
    void (*selectTickCallback)(Class65650 *self, s32 arg);                                 /* +0x10C Class65650__SelectTickCallback */
    u8 pad110[0x04];                                                             /* +0x110 Class65650__EnableTickCallback, not dispatched here */
    void (*disableTickCallback)(Class65650 *self);                                           /* +0x114 Class65650__DisableTickCallback */
    void *tickCallbackA;                                                              /* +0x118 Class65650__TickCallbackA; only its VALUE is taken (Class65650__SelectTickCallback) */
    void *tickCallbackB;                                                               /* +0x11C Class65650__TickCallbackB (Entity overrides: Entity__TickSoundCue) */
    void *tickCallbackC;                                                               /* +0x120 Class65650__TickCallbackC */
    u8 pad124[0x04];                                                             /* +0x124 Class65650__func_800661D4, not dispatched here */
    void (*setTod)(Class65650 *self, s32 arg);                                    /* +0x128 Class65650__SetTod */
    u8 pad12C[0x04];                                                                /* +0x12C Class65650__PlayTod, not dispatched here */
    void (*stopTod)(Class65650 *self);                                               /* +0x130 Class65650__StopTod */
    u8 *(*applyTodFrame)(Class65650 *self, void *ptr, s32 flag);                        /* +0x134 Class65650__ApplyTodFrame; returns the next frame (Class65650__Tick stores it in todFramePtr) */
    void *(*applyTodPacket)(Class65650 *self, void *acc, void *extra);                    /* +0x138 Class65650__ApplyTodPacket; returns the next packet */
    void (*linkPeer)(Class65650 *self, Class65650 *other);                            /* +0x13C Class65650__LinkPeer */
    void (*unlinkPeer)(Class65650 *self);                                               /* +0x140 Class65650__UnlinkPeer */
} Class65650Methods;

/* Tick-callback selectors for Class65650__SelectTickCallback ('A'..'C'). */
#define TICK_CALLBACK_A 0x41
#define TICK_CALLBACK_B 0x42
#define TICK_CALLBACK_C 0x43

/* Object size is 0x98 (from the allocator call in New_Class65650). Field
 * offsets below are only the ones observed so far in this unit's functions. */
struct Class65650 {
    Class65650Methods *methods;   /* +0x00 */
    u8 pad04[0x08];                  /* +0x04 .. +0x0B, BasicClass instance fields */
    s32 parent;                       /* +0x0C Class6B5CC's parent pointer: AttachToParent runs only while it is 0, DetachFromParent only while it is set */
    u8 pad10[0x04];                   /* +0x10 .. +0x13, Class6B5CC attribute */
    s32 *coord2;                        /* +0x14 Class6B5CC's GsCOORDINATE2; Class65650__Tick zeroes its flg (first word) */
    u8 pad18[0x0C];                     /* +0x18 .. +0x23, not this unit's to name */
    s32 tick;                           /* +0x24 Class6B5CC's tick counter, incremented by Class65650__Tick */
    u8 pad28[0x28];                       /* +0x28 .. +0x4F, not this unit's to name */
    Class65650 *companion2;                 /* +0x50 BaseObjO companion2 (the tag-5 companion); set via linkCompanion in AttachToParent, unlinked in DetachFromParent */
    u8 pad54[0x04];                     /* +0x54 .. +0x57, not this unit's to name */

    UnkArg2Obj *arg2;               /* +0x58 the constructor's third parameter, stashed verbatim; Class65650__func_800661D4 calls arg2->methods->slot80(arg2, arg, 0x6E, 0x6E) when non-NULL */
    Unk5CObj *modelData;                /* +0x5C see Unk5CObj; set by Class65650__AcquireModelData, cleared by Class65650__ReleaseModelData */
    s32 ownsModelData;                     /* +0x60 1 if func_8004468C made modelData for this instance, 0 if borrowed from the ctor's arg1; only an owned one is released */
    s32 unk64;                     /* +0x64 set by Class65650__SetUnk64 (1 in InitDefaults); Class65650__TickCallbackA acts only while it is 1 */
    Unk68Obj *mainPart;                /* +0x68 parts[buf[0]] after Class65650__CreateParts; NULL after DestroyParts */
    s32 partCount;                     /* +0x6C number of entries in parts/partIds */
    Unk70ElemObj **parts;           /* +0x70 one BaseObjO per TOD object, made by Class65650__CreateParts */
    u8 *partIds;                     /* +0x74 TOD object id of each part; searched by Class65650__FindPartIndex */
    void *tickCallback;                    /* +0x78 called by Class65650__Tick while tickCallbackEnabled; one of tickCallbackA/B/C */

    s32 todIndex;                      /* +0x7C index of the current TOD in modelData->tods */
    s32 todFrameCount;                       /* +0x80 the current TOD's frame count */
    s32 todFrame;                        /* +0x84 current frame number, wraps at todFrameCount */
    u8 *todFramePtr;                         /* +0x88 the next frame to apply */

    s32 tickCallbackEnabled;                     /* +0x8C Class65650__EnableTickCallback / DisableTickCallback */
    s32 todPlaying;                     /* +0x90 Class65650__PlayTod / StopTod; gates frame advance in Class65650__Tick */
    Class65650 *peer;              /* +0x94 set by Class65650__LinkPeer (mutual linkCompanion), cleared by Class65650__UnlinkPeer */
};

extern Class65650Methods gClass65650Methods;
extern Class65650Methods *Get_vtable_Class65650(void);

extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);
extern void Class6B5CC__LinkModel(void *self, s32 arg); /* first param confirmed generic: Class65650__InitDefaults passes a Class65650 *, Class65650__ApplyTodPacket passes an Unk70ElemObj * */
extern void *New_BaseObjO(void);

/* Same-unit helpers called by name ahead of their definitions. AcquireModelData
 * / ReleaseModelData are the bodies behind setupModelData/teardownModelData;
 * CreateParts/DestroyParts the bodies behind setupParts/teardownParts. The
 * parts pair reads no $a1, so takes only `self`; AcquireModelData reads
 * a1->0xC, so keeps its second argument. */
extern s32 Class65650__AcquireModelData(Class65650 *self, UnkArg1Obj *other);
extern void Class65650__ReleaseModelData(Class65650 *self);
extern s32 Class65650__CreateParts(Class65650 *self);
extern void Class65650__DestroyParts(Class65650 *self);

#endif
