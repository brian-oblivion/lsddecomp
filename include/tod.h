/**
 * @file tod.h
 * @brief Tod, the FileResource over one TOD animation, its method table, and
 *        the TOD file, frame and packet layouts it reads.
 */
#ifndef TOD_H
#define TOD_H

#include "file_resource.h"

struct ResourceSource;

/** @name TOD packet header word
 * How DecodeTodPacketWord splits a packet's header word (Sony's TOD format):
 * object id in bits 0..15 (the low byte is kept), packet type in 16..19,
 * flag in 20..23, length in words in 24..31.
 * @{ */
#define TOD_PACKET_TYPE_SHIFT 16 /**< the packet type's first bit */
#define TOD_PACKET_FLAG_SHIFT 20 /**< the flag's first bit */
#define TOD_PACKET_LEN_SHIFT 24  /**< the length's first bit */
#define TOD_PACKET_NIBBLE 0xF    /**< the type and flag fields' mask */
/** @} */

/** @name TOD packet types
 * The packet types the game reads: ScanTodPackets tests model-id and
 * object-control packets, TodActor__ApplyTodPacket applies the first four.
 * @{ */
#define TOD_PACKET_ATTRIBUTE 0 /**< data: a mask and bits, ANDed and ORed into the attribute */
/** data: rotation, scale and translation, as the flag's TOD_COORD_* bits say */
#define TOD_PACKET_COORDINATE 1
#define TOD_PACKET_MODEL_ID 2 /**< data: the TMD id the object is drawn with */
#define TOD_PACKET_PARENT 3 /**< data: the parent's object id; 0 or TOD_PARENT_ROOT for the actor */
#define TOD_PACKET_OBJECT_CONTROL 8 /**< the flag says create or kill */
#define TOD_OBJECT_CREATE 0         /**< an object-control packet's flag: create */
/** @} */

/** @name TOD coordinate packet flags
 * @{ */
/** add to (rotate, translate) or multiply (scale) the current values */
#define TOD_COORD_DIFFERENTIAL 1
#define TOD_COORD_ROTATE 2    /**< the packet carries a rotation */
#define TOD_COORD_SCALE 4     /**< the packet carries a scale */
#define TOD_COORD_TRANSLATE 8 /**< the packet carries a translation */

/** @} */

/** @brief A TOD file (Sony's TOD format): id, version and resolution, the
 *         frame count, then the frames, each a word-aligned run of words. */
typedef struct TodFile {
    /* +0x00 */ u8 pad0[4];     /* id, version, resolution: not read */
    /* +0x04 */ s32 frameCount; /**< the number of frames */
    /* +0x08 */ u32 frames[1];  /**< the first frame (a TodFrame) */
} TodFile;

/** @brief A TOD frame (Sony's TOD format): its size in words, its packet
 *         count and its frame number, then the packets. */
typedef struct TodFrame {
    /* +0x00 */ u8 pad0[2];      /* size in words: not read */
    /* +0x02 */ u16 packetCount; /**< the packets that follow */
    /* +0x04 */ u8 pad4[4];      /* frame number: not read */
    /* +0x08 */ u32 packets[1];  /**< the first packet (a TodPacket) */
} TodFrame;

/** @brief A TOD packet's header word as DecodeTodPacketWord writes it out,
 *         one byte per field. */
typedef struct TodPacketHeader {
    /* +0x00 */ u8 objectId; /**< the low byte of the object id */
    /* +0x01 */ u8 type;     /**< TOD_PACKET_* */
    /* +0x02 */ u8 flag; /**< TOD_OBJECT_* for object control, TOD_COORD_* for a coordinate packet */
    /* +0x03 */ u8 length; /**< the packet's length in words, header included */
} TodPacketHeader;

/** @brief A TOD packet: the header word, then the data; a
 *         TOD_PACKET_MODEL_ID packet's data starts with the TMD id. */
typedef struct TodPacket {
    /* +0x00 */ u32 header; /**< split by DecodeTodPacketWord */
    /* +0x04 */ u16 tmdId;  /**< a model-id packet's TMD id */
} TodPacket;

typedef struct Tod Tod;
typedef struct TodMethods TodMethods;

/**
 * @brief Tod's slots, for its table and TodSet's: FileResource's, then two of
 *        its own, both filled by the same functions in either table.
 *
 * - +0x07C scanTodPackets (ScanTodPackets);
 * - +0x080 decodePacketWord (DecodeTodPacketWord).
 *
 * The inherited +0x078 processBuffer keeps its name and void type; its
 * occupant is Tod__ScanPackets (TodSet__ScanPackets in TodSet's table), which
 * returns u8, and ModelData__ForwardScanPackets casts it.
 */
/* clang-format off */
#define TOD_SLOTS(Self, CtorParams)                                                                \
    FILERESOURCE_SLOTS(Self, CtorParams);                                                            \
    /* +0x07C */ u8 (*scanTodPackets)(Self *self, u8 *out, u32 *tmdId, u32 *data); /* ScanTodPackets, in both tables */ \
    /* +0x080 */ u32 *(*decodePacketWord)(Self *self, u32 *packet, u8 *objId, u8 *type, u8 *flag, u8 *len) /* DecodeTodPacketWord, in both tables */
/* clang-format on */

/** @brief Tod's fields: FileResource's and no more, for itself and TodSet
 *         (both 0x2C bytes). */
/* clang-format off */
#define TOD_FIELDS(Methods)                                                                        \
    FILERESOURCE_FIELDS(Methods) /* no own fields */
/* clang-format on */

/** @brief Tod's method table, gTodMethods (see TOD_SLOTS). */
struct TodMethods {
    TOD_SLOTS(Tod, (Tod * self, struct ResourceSource *src));
};

/**
 * @brief One TOD animation (class id 0x4F03): a FileResource whose buffer is
 *        a TOD file, with no fields of its own.
 *
 * Its methods walk a frame's packets, listing the objects it creates and
 * finding the object a TMD id belongs to. TodActor plays TODs through the
 * ModelData that holds them, which forwards its scans to its TodSet.
 *
 * Parent FileResource, through the active data-source driver: although the
 * id 0x4F03 sits under TimBlockSrc's 0xF03, the ctor chains to
 * GetActiveDataSourceMethods()->ctor, so it is TimBlockSrc's sibling and
 * carries none of its layout. One subclass, TodSet, which builds a Tod over
 * each TOD in its buffer. Methods in src/graphics/graphics_resources.c. The
 * object is 0x2C bytes (New_Tod).
 */
struct Tod {
    TOD_FIELDS(TodMethods);
};

/** Tod's method table. */
extern TodMethods gTodMethods;

/**
 * @brief Returns Tod's method table.
 * @return &gTodMethods.
 */
extern TodMethods *GetTodMethods(void);

/**
 * @brief Allocates a Tod from the pool and constructs it.
 * @param src The descriptor: a TOD buffer to adopt, else a file name to
 *            request.
 * @return The new object, or NULL when the pool is exhausted.
 */
Tod *New_Tod(struct ResourceSource *src);

/**
 * @brief Constructor (slot +0x008): the active driver's, then either adopts
 *        the descriptor's buffer (and runs onRequestDone) or requests the named
 *        file.
 * @param self The object to construct.
 * @param src  The descriptor; must not be NULL.
 */
void Tod__Tod(Tod *self, struct ResourceSource *src);

/**
 * @brief Finalizer (slot +0x00C): the active driver's, nothing of its own.
 * @param self The object being destroyed.
 */
void Tod__Finalize(Tod *self);

/**
 * @brief Slot +0x078: scanTodPackets over the TOD's first frame.
 * @param self  The object, its buffer holding a TOD file.
 * @param out   As ScanTodPackets'.
 * @param tmdId As ScanTodPackets'.
 * @return The number of object-create packets in the first frame.
 */
u8 Tod__ScanPackets(Tod *self, u8 *out, u32 *tmdId);

/**
 * @brief Slot +0x07C: walks one TOD frame's packets.
 *
 * With `out`, the ids of the objects the frame creates are written there, and
 * a model-id packet naming TMD `*tmdId` makes `*tmdId` the index in `out` of the
 * object it belongs to. Without `out`, `*tmdId` becomes the number of model-id
 * packets.
 * @param self The object (passed on to DecodeTodPacketWord).
 * @param out  Receives the created objects' ids, or NULL.
 * @param tmdId In: the TMD id to look up; out: the index or count above, 0
 *              when nothing matched. May be NULL.
 * @param data The frame (a TodFrame).
 * @return The number of object-create packets in the frame.
 */
u8 ScanTodPackets(Tod *self, u8 *out, u32 *tmdId, u32 *data);

/**
 * @brief Slot +0x080: splits a packet's header word into its fields.
 * @param self   The object (unused).
 * @param packet The packet's header word.
 * @param objId  Receives the low byte of the object id.
 * @param type   Receives the packet type (TOD_PACKET_*).
 * @param flag   Receives the flag nibble.
 * @param len    Receives the packet's length in words.
 * @return The word after the header.
 */
u32 *DecodeTodPacketWord(Tod *self, u32 *packet, u8 *objId, u8 *type, u8 *flag, u8 *len);

#endif
