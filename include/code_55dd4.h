#ifndef CODE_55DD4_H
#define CODE_55DD4_H

#include "common.h"
#include "Class65650.h"
#include "ModelData.h"
#include "LinkResource.h"

/*
 * code_55dd4's own readings of the objects Class65650 (include/Class65650.h)
 * reaches that are not Class65650: the ctor's two arguments, onNotify's
 * sender, the TOD coordinate parameters of a part, and the TodSet
 * internals behind ModelData's `todSet` (its `linkResource` is
 * include/LinkResource.h's, round 89). The
 * class itself -- object, table, getter, method prototypes -- is
 * include/Class65650.h's.
 */

/* Class65650__OnNotify's sender: any object. Only its method table's header
 * word is read (the low 16 bits), compared with MODEL_DATA_CLASS_HEADER. */
typedef struct TaggedObj {
    u16 header;   /* +0x000, the method table's header word */
} TaggedObj;

typedef struct TagCheckArg {
    TaggedObj *methods;   /* +0x000 */
} TagCheckArg;

/* Header word of D_8006F384 (tools/classtable.py), the class New_ModelData
 * allocates and Class65650.modelData points at. */
#define MODEL_DATA_CLASS_HEADER 0x5F03

/* Whatever class self->arg2 points at: unidentified, only its
 * vtable slot +0x080 is needed so far, by Class65650__func_800661D4. */
typedef struct UnkArg2Methods {
    u8 pad00[0x80];                                       /* +0x000 .. +0x07C, unknown */
    void (*slot80)(void *self, void *arg1, s32 a2, s32 a3); /* +0x080 */
} UnkArg2Methods;

typedef struct UnkArg2Obj {
    UnkArg2Methods *methods;
} UnkArg2Obj;

/* A part's coord2->param (Class6B5CCSub44, GsCOORD2PARAM) as
 * Class65650__ApplyTodPacket writes it from a TOD coordinate packet: the
 * scale, rotate and trans vectors as arrays. */
typedef struct TimeTargetObj {
    s32 scale[3];    /* +0x000 .. +0x00B GsCOORD2PARAM.scale: TOD_COORD_SCALE */
    u8 pad0C[0x04];   /* +0x00C .. +0x00F */
    s16 rotate[3];      /* +0x010 .. +0x015 GsCOORD2PARAM.rotate: TOD_COORD_ROTATE */
    u8 pad16[0x02];     /* +0x016 .. +0x017 */
    s32 trans[3];         /* +0x018 .. +0x023 GsCOORD2PARAM.trans: TOD_COORD_TRANSLATE, then copied to coord.t */
} TimeTargetObj;

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

/* self->modelData->todSet (Class65650__SetTod, Class65650__Tick): its
 * buffer (Class6D430 +0x010) + 8 + i * 4 holds a pointer to the i-th TOD's
 * holder, whose +0x10 is the TOD data itself: +0x4 the frame count, frames
 * starting at +0x8. */
typedef struct EntryObj2 {
    u8 pad00[0x04];
    s32 frameCount;            /* +0x004 TOD header: number of frames */
} EntryObj2;

typedef struct GroupObj {
    u8 pad00[0x10];
    EntryObj2 *tod;    /* +0x010 */
} GroupObj;

/* The constructor's `arg1`, forwarded through setupModelData into
 * Class65650__AcquireModelData: if its +0x0C already holds a model-data
 * object it is borrowed, otherwise New_ModelData(arg1) makes one that this
 * instance owns. */
typedef struct UnkArg1Obj {
    u8 pad00[0x0C];
    ModelData *modelData;
} UnkArg1Obj;

extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

#endif
