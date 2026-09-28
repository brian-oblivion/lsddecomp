#ifndef TOD_H
#define TOD_H

#include "FileResource.h"

/*
 * Tod -- a FileResource data source (class id 0x4F03, method table gTodMethods)
 * over one TOD animation's packet stream. Methods in src/GraphicsResources.c; one
 * subclass, TodSet (gTodSetMethods, 0x14F03), whose ctor calls this class's
 * first (TodSet__TodSet: GetTodMethods()->ctor(self, arg)) and whose
 * TodSet__BuildTods makes one Tod per sub-block of its buffer (New_Tod).
 *
 * The name is round 83's, kept on this evidence: its own methods walk
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
 * +0x078 is FileResource's slot78 (NULL there): this table's occupant is
 * Tod__ScanPackets(self, out, sel), u8, which runs +0x07C over the buffer
 * past its first two words (TodSet's occupant runs it past its counted
 * array); ModelData__ForwardScanPackets casts it (an inherited slot keeps
 * the parent's name).
 *
 * The ctor's descriptor is include/FileResource.h's ResourceSource ({buffer
 * to adopt, file name to request}). The allocators reach the ctor through GraphicsResources.c's unprototyped
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

/* The packet types ScanTodPackets tests. TOD_PACKET_MODEL_ID is spelled
 * exactly as src/TodActor.c's, which defines the other types. */
#define TOD_PACKET_MODEL_ID 2       /* data: the TMD id the object is drawn with */
#define TOD_PACKET_OBJECT_CONTROL 8 /* the flag says create or kill */
#define TOD_OBJECT_CREATE 0         /* an object-control packet's flag: create */

typedef struct Tod Tod;
typedef struct TodMethods TodMethods;

/* Both own slots are typed as their occupants. +0x07C returns u8: cc1 still
 * emits Tod__ScanPackets' trailing `andi 0xFF` over a u8 slot, so the bytes
 * do not need the s32 the unit-local view had (round 86). */
/* clang-format off */
#define TOD_SLOTS(Self, CtorParams)                                                                \
    FILERESOURCE_SLOTS(Self, CtorParams);                                                            \
    /* +0x07C */ u8 (*scanTodPackets)(Self *self, u8 *out, u32 *sel, u32 *data); /* ScanTodPackets, in both tables */ \
    /* +0x080 */ u32 *(*decodePacketWord)(Self *self, u32 *acc, u8 *out0, u8 *out1, u8 *out2, u8 *out3) /* DecodeTodPacketWord, in both tables */
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
u32 *DecodeTodPacketWord(Tod *self, u32 *acc, u8 *out0, u8 *out1, u8 *out2, u8 *out3);

#endif
