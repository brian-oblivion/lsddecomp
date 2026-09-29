/**
 * @file tod_set.h
 * @brief TodSet, the Tod over a buffer of several TOD animations, and its
 *        method table.
 */
#ifndef TOD_SET_H
#define TOD_SET_H

#include "tod.h"

struct ResourceSource;

typedef struct TodSet TodSet;
typedef struct TodSetMethods TodSetMethods;

/** TodSet's class id (gTodSetMethods word +0x000). */
#define TODSET_CLASS_ID 0x14F03

/**
 * @brief TodSet's method table, gTodSetMethods: Tod's slots, with no new ones.
 *
 * It overrides +0x008 ctor (TodSet__TodSet), +0x00C finalize
 * (TodSet__Finalize), +0x064 onRequestDone (TodSet__BuildTods, which returns
 * s32; the ctor casts the call) and +0x078 processBuffer
 * (TodSet__ScanPackets).
 */
struct TodSetMethods {
    TOD_SLOTS(TodSet, (TodSet * self, struct ResourceSource *src));
};

/**
 * @brief A set of TOD animations (class id 0x14F03): one buffer holding a
 *        counted table of offsets, one Tod per entry, then the packet data.
 *
 * Building it replaces each table entry with the Tod made over it. Parent Tod
 * (its ctor chains to Tod's first); no subclasses, and no fields of its own:
 * the object is 0x2C bytes (New_TodSet). Methods in
 * src/graphics/graphics_resources.c. ModelData__BuildResources builds one over
 * a MOM file's TODs, and ModelData forwards its packet scans to it.
 */
struct TodSet {
    TOD_FIELDS(TodSetMethods);
};

/** TodSet's method table. */
extern TodSetMethods gTodSetMethods;

/**
 * @brief Returns TodSet's method table.
 * @return &gTodSetMethods.
 */
extern TodSetMethods *GetTodSetMethods(void);

/**
 * @brief Allocates a TodSet from the pool and constructs it.
 * @param src The descriptor: a buffer to adopt, else a file name to request.
 * @return The new object, or NULL when the pool is exhausted or an adopted
 *         buffer's Tods cannot be built (the object is then freed).
 */
TodSet *New_TodSet(struct ResourceSource *src);

/**
 * @brief Constructor (slot +0x008): Tod's, then, with an adopted buffer,
 *        builds its Tods at once.
 * @param self The object to construct.
 * @param src  The descriptor.
 * @return self, or NULL when building the Tods fails.
 */
void *TodSet__TodSet(TodSet *self, struct ResourceSource *src);

/**
 * @brief Finalizer (slot +0x00C): releases the Tods in the buffer's table,
 *        then Tod's finalizer.
 * @param self The object being destroyed.
 */
void TodSet__Finalize(TodSet *self);

/**
 * @brief Slot +0x064 (onRequestDone): builds a Tod over each of the buffer's
 *        sub-blocks, storing it in place of that entry's offset.
 * @param self The object, its buffer loaded.
 * @return 0 when every Tod was built; 1 when one fails, those built so far
 *         then released.
 */
s32 TodSet__BuildTods(TodSet *self);

/**
 * @brief Slot +0x078: scanTodPackets over the first frame of the TOD that
 *        follows the counted table.
 * @param self  The object.
 * @param out   As ScanTodPackets'.
 * @param tmdId As ScanTodPackets'.
 * @return The number of object-create packets in that frame.
 */
u8 TodSet__ScanPackets(TodSet *self, u8 *out, u32 *tmdId);

#endif
