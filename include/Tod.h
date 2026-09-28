#ifndef TOD_H
#define TOD_H

#include "file_resource.h"

/*
 * Tod -- a FileResource data source (class id 0x4F03, method table gTodMethods)
 * over one TOD animation's packet stream. Methods in src/graphics/graphics_resources.c; one
 * subclass, TodSet (gTodSetMethods, 0x14F03), whose ctor calls this class's
 * first (TodSet__TodSet: GetTodMethods()->ctor(self, arg)) and whose
 * TodSet__BuildTods makes one Tod per sub-block of its buffer (New_Tod).
 *
 * What its own methods do: walk
 * the buffer's packet words (ScanTodPackets, from buffer +8 with the u16
 * packet count at +2 before it) decoding each into a low byte, the nibbles
 * at bits 16 and 20 and a top-byte length in words (DecodeTodPacketWord);
 * ModelData forwards its TOD packet scans to the TodSet it holds
 * (ModelData__ForwardScanPackets: todSet's +0x078), and TodActor plays
 * TODs through that ModelData.
 *
 * PARENT BY CTOR CHAIN, NOT BY ID. The id 0x4F03 puts it under TimBlockSrc
 * (0xF03), but Tod__Tod's first call is GetActiveDataSourceMethods()->ctor,
 * as TimBlockSrc__TimBlockSrc's is: it is TimBlockSrc's sibling under
 * FileResource and carries none of TimBlockSrc's layout (include/TimBlockSrc.h).
 *
 * NO OWN FIELDS. The object is 0x2C bytes (New_Tod), FileResource's own size;
 * TodSet's is 0x2C too (New_TodSet). Everything a Tod reads is in the
 * adopted or loaded `buffer`.
 *
 * +0x078 is FileResource's processBuffer (NULL there): this table's occupant is
 * Tod__ScanPackets(self, out, sel), u8, which runs +0x07C over the buffer
 * past its first two words (TodSet's occupant runs it past its counted
 * array); ModelData__ForwardScanPackets casts it (an inherited slot keeps
 * the parent's name).
 *
 * The ctor's descriptor is include/file_resource.h's ResourceSource ({buffer
 * to adopt, file name to request}). The allocators reach the ctor through graphics_resources.c's unprototyped
 * UnprototypedCtorTable view.
 */

struct ResourceSource;

/* A TOD packet's header word, as DecodeTodPacketWord splits it (Sony's TOD
 * format): object id in bits 0..15 (the low byte is kept), packet type in
 * 16..19, flag in 20..23, length in words in 24..31. */
#define TOD_PACKET_TYPE_SHIFT 16
#define TOD_PACKET_FLAG_SHIFT 20
#define TOD_PACKET_LEN_SHIFT 24
#define TOD_PACKET_NIBBLE 0xF /* the type and flag fields' mask */

/* The packet types the game reads: ScanTodPackets tests model-id and
 * object-control packets, TodActor__ApplyTodPacket applies the first four. */
#define TOD_PACKET_ATTRIBUTE 0
#define TOD_PACKET_COORDINATE 1
#define TOD_PACKET_MODEL_ID 2 /* data: the TMD id the object is drawn with */
#define TOD_PACKET_PARENT 3
#define TOD_PACKET_OBJECT_CONTROL 8 /* the flag says create or kill */
#define TOD_OBJECT_CREATE 0         /* an object-control packet's flag: create */

/* A coordinate packet's flag bits. */
#define TOD_COORD_DIFFERENTIAL 1
#define TOD_COORD_ROTATE 2
#define TOD_COORD_SCALE 4
#define TOD_COORD_TRANSLATE 8

/* A TOD file (Sony's TOD format): id, version and resolution, the frame
 * count, then the frames, each a word-aligned run of words. */
typedef struct TodFile {
    /* +0x00 */ u8 pad0[4]; /* id, version, resolution: not read */
    /* +0x04 */ s32 frameCount;
    /* +0x08 */ u32 frames[1]; /* the first frame (a TodFrame) */
} TodFile;

/* A TOD frame (Sony's TOD format): its size in words, its packet count and
 * its frame number, then the packets. */
typedef struct TodFrame {
    /* +0x00 */ u8 pad0[2]; /* size in words: not read */
    /* +0x02 */ u16 packetCount;
    /* +0x04 */ u8 pad4[4];     /* frame number: not read */
    /* +0x08 */ u32 packets[1]; /* the first packet (a TodPacket) */
} TodFrame;

/* A TOD packet's header word as DecodeTodPacketWord writes it out, one
 * byte per field (its four out-pointers). */
typedef struct TodPacketHeader {
    /* +0x00 */ u8 objectId; /* the low byte of the object id */
    /* +0x01 */ u8 type;     /* TOD_PACKET_* */
    /* +0x02 */ u8 flag; /* TOD_OBJECT_* for object control, TOD_COORD_* for a coordinate packet */
    /* +0x03 */ u8 length; /* the packet's length in words, header included */
} TodPacketHeader;

/* A TOD packet: the header word DecodeTodPacketWord splits, then the data;
 * a TOD_PACKET_MODEL_ID packet's data starts with the TMD id. */
typedef struct TodPacket {
    /* +0x00 */ u32 header;
    /* +0x04 */ u16 tmdId;
} TodPacket;

typedef struct Tod Tod;
typedef struct TodMethods TodMethods;

/* Both own slots are typed as their occupants; +0x07C returns u8. */
/* clang-format off */
#define TOD_SLOTS(Self, CtorParams)                                                                \
    FILERESOURCE_SLOTS(Self, CtorParams);                                                            \
    /* +0x07C */ u8 (*scanTodPackets)(Self *self, u8 *out, u32 *sel, u32 *data); /* ScanTodPackets, in both tables */ \
    /* +0x080 */ u32 *(*decodePacketWord)(Self *self, u32 *packet, u8 *objId, u8 *type, u8 *flag, u8 *len) /* DecodeTodPacketWord, in both tables */
/* clang-format on */

/* clang-format off */
#define TOD_FIELDS(Methods)                                                                        \
    FILERESOURCE_FIELDS(Methods) /* no own fields: the object is 0x2C bytes (New_Tod), and so is TodSet's (New_TodSet) */
/* clang-format on */

struct TodMethods {
    TOD_SLOTS(Tod, (Tod * self, struct ResourceSource *src));
};

struct Tod {
    TOD_FIELDS(TodMethods);
};

extern TodMethods gTodMethods;
extern TodMethods *GetTodMethods(void);

Tod *New_Tod(struct ResourceSource *src);
void Tod__Tod(Tod *self, struct ResourceSource *src);
void Tod__Finalize(Tod *self);
u8 Tod__ScanPackets(Tod *self, u8 *out, u32 *sel);
u8 ScanTodPackets(Tod *self, u8 *out, u32 *sel, u32 *data);
u32 *DecodeTodPacketWord(Tod *self, u32 *packet, u8 *objId, u8 *type, u8 *flag, u8 *len);

#endif
