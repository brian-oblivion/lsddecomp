/**
 * @file model_data.h
 * @brief ModelData, the FileResource that splits one model file into its TMD
 *        models and its TOD animations, and its method table.
 */
#ifndef MODEL_DATA_H
#define MODEL_DATA_H

#include "file_resource.h"
#include "tod_set.h"

struct ResourceSource;

typedef struct ModelData ModelData;
typedef struct ModelDataMethods ModelDataMethods;

/**
 * @brief ModelData's slots, for its table and TriggerWorld's: FileResource's,
 *        then three of its own.
 *
 * - +0x07C releaseResources (ModelData__ReleaseResources);
 * - +0x080 scanPackets (ModelData__ForwardScanPackets: the TodSet's +0x078);
 * - +0x084 decodePacketWord (ModelData__ForwardDecodePacketWord: the TodSet's
 *   +0x080).
 *
 * The inherited +0x078 processBuffer keeps its name and void type; its
 * occupant is ModelData__BuildResources, which returns s32, and the callers
 * cast it.
 */
/* clang-format off */
#define MODELDATA_SLOTS(Self, CtorParams)                                                          \
    FILERESOURCE_SLOTS(Self, CtorParams);                                                            \
    /* +0x07C */ void (*releaseResources)(Self *self);          /* ModelData__ReleaseResources */  \
    /* +0x080 */ u8 (*scanPackets)(Self *self, u8 *out, u32 *tmdId); /* ModelData__ForwardScanPackets: todSet's +0x078 */ \
    /* +0x084 */ void *(*decodePacketWord)(Self *self, u32 *packet, u8 *objId, u8 *type, u8 *flag, u8 *len) /* ModelData__ForwardDecodePacketWord: todSet's +0x080 */
/* clang-format on */

/**
 * @brief ModelData's fields, for itself and TriggerWorld: FileResource's,
 *        then three of its own.
 *
 * - +0x02C linkResource: the TMD models (New_LinkResource), released by
 *   ReleaseResources;
 * - +0x030 todSet: the TOD animations (New_TodSet); scanPackets and
 *   decodePacketWord forward to it;
 * - +0x034 ownsResources: the ctor's `owns`, 1 from New_ModelData and 0 from
 *   TriggerWorld; BuildResources and ReleaseResources act only while it is
 *   set.
 *
 * A ModelData is 0x38 bytes (New_ModelData), so TriggerWorld's own fields
 * start at +0x038.
 */
/* clang-format off */
#define MODELDATA_FIELDS(Methods)                                                                  \
    FILERESOURCE_FIELDS(Methods);                                                                    \
    /* +0x02C */ struct LinkResource *linkResource; /* New_LinkResource (include/link_resource.h); released by ReleaseResources */ \
    /* +0x030 */ TodSet *todSet;       /* New_TodSet (gTodSetMethods); +0x080/+0x084 forward to it */ \
    /* +0x034 */ s32 ownsResources         /* the ctor's third argument: New_ModelData 1, TriggerWorld 0; BuildResources and ReleaseResources act only while it is set */
/* clang-format on */

/** @brief ModelData's method table, gModelDataMethods (see MODELDATA_SLOTS). */
struct ModelDataMethods {
    MODELDATA_SLOTS(ModelData, (ModelData * self, struct ResourceSource *src, s32 owns));
};

/**
 * @brief An actor's model and animation data (class id 0x5F03): one buffer
 *        split into a LinkResource over its TMD and a TodSet over its TODs.
 *
 * The buffer is a MOM file (ModelDataHeader, in graphics_resources.c): the
 * TMD at the offset in its third word, the TODs from +0x0C. TOD packet scans
 * are forwarded to the TodSet.
 *
 * Parent FileResource, through the active data-source driver: although the
 * id 0x5F03 sits under TimBlockSrc's 0xF03, the ctor chains to
 * GetActiveDataSourceMethods()->ctor, so it is TimBlockSrc's sibling and
 * carries none of its layout. One subclass, TriggerWorld, which builds its
 * ModelData part not owning its resources. Methods in
 * src/graphics/graphics_resources.c.
 *
 * Holders: TodActor (TodActor.modelData) and InitDreamAux's MOM files
 * (src/world/dream_aux.c).
 */
struct ModelData {
    MODELDATA_FIELDS(ModelDataMethods);
};

/** ModelData's method table. */
extern ModelDataMethods gModelDataMethods;

/**
 * @brief Returns ModelData's method table.
 * @return &gModelDataMethods.
 */
extern ModelDataMethods *GetModelDataMethods(void);

/**
 * @brief Allocates a ModelData that owns its resources, and constructs it.
 * @param src The descriptor: a MOM buffer to adopt, else a file name to
 *            request.
 * @return The new object, or NULL when the pool is exhausted or an adopted
 *         buffer fails to build (the object is then freed).
 */
ModelData *New_ModelData(struct ResourceSource *src);

/**
 * @brief Constructor (slot +0x008): the active driver's, then either adopts
 *        the descriptor's buffer and loads it at once, or requests the named
 *        file.
 * @param self The object to construct.
 * @param src  The descriptor.
 * @param owns Nonzero when the object builds and releases its own
 *             LinkResource and TodSet.
 * @return self, or NULL when loading an adopted buffer fails.
 */
void *ModelData__ModelData(ModelData *self, struct ResourceSource *src, s32 owns);

/**
 * @brief Finalizer (slot +0x00C): releaseResources, then the active driver's
 *        finalizer.
 * @param self The object being destroyed.
 */
void ModelData__Finalize(ModelData *self);

/**
 * @brief Slot +0x064 (onRequestDone): the driver's, then BuildResources.
 * @param self The object, its buffer loaded.
 */
void ModelData__Load(ModelData *self);

/**
 * @brief Slot +0x078: when it owns them, builds the LinkResource and the
 *        TodSet over the buffer.
 * @param self The object, its buffer holding a MOM file.
 * @return 0 when both were built or it owns none; 1 when either fails, both
 *         then released.
 */
s32 ModelData__BuildResources(ModelData *self);

/**
 * @brief Slot +0x07C: when it owns them, releases the TodSet and the
 *        LinkResource that exist.
 * @param self The object.
 */
void ModelData__ReleaseResources(ModelData *self);

/**
 * @brief Slot +0x080: the TodSet's packet scan (TodSet__ScanPackets) over its
 *        first TOD's first frame.
 * @param self  The object.
 * @param out   Receives the ids of the objects the frame creates, or NULL.
 * @param tmdId In: a TMD id to look up; out: see ScanTodPackets. May be NULL.
 * @return The number of object-create packets in the frame.
 */
u8 ModelData__ForwardScanPackets(ModelData *self, u8 *out, u32 *tmdId);

/**
 * @brief Slot +0x084: the TodSet's packet header decoder (DecodeTodPacketWord).
 * @param self   The object.
 * @param packet The packet's header word.
 * @param objId  Receives the low byte of the object id.
 * @param type   Receives the packet type (TOD_PACKET_*).
 * @param flag   Receives the packet's flag nibble.
 * @param len    Receives the packet's length in words.
 * @return The word after the header.
 */
void *ModelData__ForwardDecodePacketWord(ModelData *self, u32 *packet, u8 *objId, u8 *type,
                                         u8 *flag, u8 *len);

#endif
