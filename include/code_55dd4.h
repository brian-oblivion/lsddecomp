#ifndef CODE_55DD4_H
#define CODE_55DD4_H

#include "common.h"
#include "TodActor.h"
#include "ModelData.h"
#include "Tod.h"
#include "LinkResource.h"
#include "VabStreamObj.h"

/*
 * code_55dd4's own readings of what TodActor (include/TodActor.h) reaches
 * that is not TodActor: the ctor's descriptor, onNotify's sender, the TOD
 * format (file header, packet types) and a TodSet's table of Tods. The
 * class itself -- object, table, getter, method prototypes -- is
 * include/TodActor.h's; the sound bank is include/VabStreamObj.h's and a
 * part's coordinate parameters are Sony's GsCOORD2PARAM.
 */

/* TodActor__OnNotify's sender: any object. Only its method table's header
 * word is read (the low 16 bits), compared with MODEL_DATA_CLASS_HEADER. */
typedef struct TaggedObj {
    u16 header; /* +0x000, the method table's header word */
} TaggedObj;

typedef struct TagCheckArg {
    TaggedObj *methods; /* +0x000 */
} TagCheckArg;

/* Header word of gModelDataMethods (tools/classtable.py), the class New_ModelData
 * allocates and TodActor.modelData points at. */
#define MODEL_DATA_CLASS_HEADER 0x5F03

/* TOD packet types and coordinate-packet flag bits, as
 * TodActor__ApplyTodPacket decodes them (the decoded header is
 * {object id, type, flag, length in words}). */
#define TOD_PACKET_ATTRIBUTE 0
#define TOD_PACKET_COORDINATE 1
#define TOD_PACKET_MODEL_ID 2
#define TOD_PACKET_PARENT 3
#define TOD_COORD_DIFFERENTIAL 1
#define TOD_COORD_ROTATE 2
#define TOD_COORD_SCALE 4
#define TOD_COORD_TRANSLATE 8

/* A TOD file's header (Sony's TOD format: id, version, resolution, then the
 * frame count); its frames start at +0x8. */
typedef struct TodHeader {
    u8 pad00[0x04]; /* +0x000 id, version, resolution: not read */
    s32 frameCount; /* +0x004 */
} TodHeader;

/* The i-th Tod of a TodSet (TodActor__SetTod, TodActor__Tick):
 * TodSet__BuildTods replaces each word of its buffer's counted offset
 * table, from +0x8, with the Tod it built over that sub-block. */
#define TODSET_TOD(set, i) (*(Tod **)((u8 *)(set)->buffer + 8 + (i) * 4))

/* The ctor's descriptor, forwarded through setupModelData into
 * TodActor__AcquireModelData: the ModelData at +0x0C is borrowed; when there
 * is none, New_ModelData(desc) makes one from the descriptor's leading
 * {buffer, name} (code_33808.c's ResourceSource) and the TodActor owns it. */
typedef struct TodActorDesc {
    u8 pad00[0x0C];       /* +0x000 New_ModelData's source; not read here */
    ModelData *modelData; /* +0x00C */
} TodActorDesc;

extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

#endif
