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

/* A TOD packet's header word, as decodePacketWord writes it out byte by
 * byte. */
typedef struct TodPacketHeader {
    u8 objectId; /* +0x000 the low byte of the object id */
    u8 type;     /* +0x001 TOD_PACKET_* */
    u8 flag;     /* +0x002 TOD_COORD_* for a coordinate packet */
    u8 length;   /* +0x003 the packet's length in words, header included */
} TodPacketHeader;

/* A TOD file's header (Sony's TOD format: id, version, resolution, then the
 * frame count), then its frames. */
typedef struct TodHeader {
    u8 pad00[0x04]; /* +0x000 id, version, resolution: not read */
    s32 frameCount; /* +0x004 */
    u8 frames[1];   /* +0x008 the first frame (a TodFrame) */
} TodHeader;

/* A TOD frame's header (Sony's TOD format: size in words, packet count,
 * frame number), then its packets. */
typedef struct TodFrame {
    u8 pad00[0x02];  /* +0x000 size in words: not read */
    u16 packetCount; /* +0x002 */
    u8 pad04[0x04];  /* +0x004 frame number: not read */
    u8 packets[1];   /* +0x008 the first packet */
} TodFrame;

/* A TodSet's buffer once TodSet__BuildTods has run: a word, a count, then
 * the table whose each entry (an offset into the buffer) it replaced with
 * the Tod it built over that sub-block. */
typedef struct TodSetBuffer {
    u8 pad00[0x08]; /* +0x000 a word and the entry count: not read here */
    Tod *tods[1];   /* +0x008 */
} TodSetBuffer;

/* The i-th Tod of a TodSet (TodActor__SetTod, TodActor__Tick). */
#define TODSET_TOD(set, i) (((TodSetBuffer *)(set)->buffer)->tods[i])

/* The ctor's descriptor, forwarded through setupModelData into
 * TodActor__AcquireModelData: the ModelData at +0x0C is borrowed; when there
 * is none, New_ModelData(&desc->src) makes one and the TodActor owns it. */
typedef struct TodActorDesc {
    /* +0x000 */ ResourceSource src;
    /* +0x008 */ u8 pad8[4];
    /* +0x00C */ ModelData *modelData;
} TodActorDesc;

extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

#endif
